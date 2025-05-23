/*
 * Copyright (c) 2019-2020, Xilinx, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE#
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "../toe/toe.hpp"
#include "../toe/toe_config.hpp"
#include "uram_datamover.hpp"
#include "uram_array.hpp"

mmCmd axiu2mmCmd(ap_axiu<128,0,0,0> inword) {
#pragma HLS inline
    mmCmd tmp_cmd;
    tmp_cmd.bbt =inword.data(22,0) ;
    tmp_cmd.type = inword.data(23,23);
    tmp_cmd.dsa = inword.data(29,24);
    tmp_cmd.eof = inword.data(30,30);
    tmp_cmd.drr = inword.data(31,31);
    tmp_cmd.saddr = inword.data(63,32);
    tmp_cmd.tag = inword.data(67,64);
    tmp_cmd.rsvd = inword.data(71,68);
    return tmp_cmd;
}

ap_axiu<8,0,0,0> mmStatus2axiu(mmStatus inword) {
#pragma HLS inline
    ap_axiu<8,0,0,0> tmp_axiu;
    tmp_axiu.data(3,0) = inword.tag;
    tmp_axiu.data(4,4) = inword.interr;
    tmp_axiu.data(5,5) = inword.decerr;
    tmp_axiu.data(6,6) = inword.slverr;
    tmp_axiu.data(7,7) = inword.okay;
    tmp_axiu.last = 1;
    return tmp_axiu;
}

uint64_t rightShiftOctets(uint64_t word, uint8_t nOctets) {
#pragma HLS inline
    uint8_t nshift = nOctets;
    uint64_t outWord = word;
    if (nshift >= 4) {
        outWord = outWord >> 32;
        nshift -= 4;
    }

    if (nshift >= 2) {
        outWord = outWord >> 16;
        nshift -= 2;
    }

    if (nshift >= 1) {
        outWord = outWord >> 8;
    }
    return outWord;
}

uint64_t leftShiftOctets(uint64_t word, uint8_t nOctets) {
#pragma HLS inline
    uint8_t nshift = nOctets;
    uint64_t outWord = word;
    if (nshift >= 4) {
        outWord = outWord << 32;
        nshift -= 4;
    }

    if (nshift >= 2) {
        outWord = outWord << 16;
        nshift -= 2;
    }

    if (nshift >= 1) {
        outWord = outWord << 8;
    }
    return outWord;
}

/** @ingroup uram_datamover
 *   calculateTrailBytes
 *   @param[in] btt bytes to transfer. This must have been adjusted to start at a 64-bit word-aligned offset.
 *   returns the number of trailing bytes that are unused at the end of the transfer
 */
uint8_t calculateTrailBytes(uint32_t btt) {
#pragma HLS inline
    uint8_t trailBytes = btt % 8;
    if (trailBytes > 0) {
        trailBytes = 8 - trailBytes;
    }
    return trailBytes;
}

template <class T>
T reg(T x) {
#pragma HLS pipeline
#pragma HLS inline self off
#pragma HLS interface ap_ctrl_none register port = return
    return x;
}

struct commonVars {
    uint64_t rshiftWord;
    uint8_t nBuffer;
    uint32_t nBufferAddr;
    uint8_t nStartWordByte;
    uint32_t nCurrWordAddr;
    ap_uint<23> btt;
    uint16_t nTransferWords;
    ap_uint<WINDOW_BITS> taddr;
    ap_uint<3> nShift;
};

/** @ingroup uram_datamover
 *   set_common
 *   Sets a number of variables common to both read and write procedures
 *   @param[inout] cvars - Struct containing the common variables to be written
 *   @param[in] saddr - The starting byte address 
 *   @param[in] btt - Number of bytes to transfer
 */
void set_common(commonVars& cvars, ap_uint<32> saddr, ap_uint<23> btt) {
#pragma HLS inline
    cvars.nBuffer = saddr(WINDOW_BITS + 8, WINDOW_BITS);
    cvars.nStartWordByte = saddr(2, 0);
    cvars.rshiftWord = 0;
    cvars.nCurrWordAddr = saddr(WINDOW_BITS - 1, 3) << 3;
    cvars.nBufferAddr = cvars.nBuffer << WINDOW_BITS;
    // Total bytes to transfer is the number of bytes transferred in the operation + the byte offset of starting word
    cvars.btt = btt + saddr(2, 0);
    cvars.nTransferWords = cvars.btt / 8;
    if (cvars.btt % 8) {
        cvars.nTransferWords++;
    }
    // Calculate the terminate address - the last byte written to by this transfer
    cvars.taddr = saddr(WINDOW_BITS-1,0) + btt - 1;

    // Number of bytes to shift. 
    cvars.nShift = (8 - cvars.nStartWordByte) & 0x7;
}

/** @ingroup uram_datamover_body
 *
 *  @param[in]		cmdRead
 *  @param[in]		cmdWrite
 *  @param[in]		dataIn
 *  @param[out]		status
 *  @param[out]		dataOut
 */
template <int WIDTH>
void uram_datamover_body(hls::stream<ap_axiu<128,0,0,0> >& cmdRead,
                         hls::stream<ap_axiu<128,0,0,0> >& cmdWrite,
                         hls::stream<ap_axiu<WIDTH,0,0,0> >& dataIn,
                         hls::stream<ap_axiu<8,0,0,0> >& writeStatus,
                         hls::stream<ap_axiu<WIDTH,0,0,0> >& dataOut) {
    static commonVars cVars;
    static mmCmd currCmd;
    static xf::common::utils_hw::UramArray<64, (BUFFER_SIZE / 8) * MAX_SESSIONS, 1> mem_buf;
    enum fsmStateType { IDLE, WRITE, RD_ERROR, WR_ERROR };
    static fsmStateType state = IDLE;
    uint64_t lshiftWord;

    ap_axiu<WIDTH,0,0,0> tmpWord;
    ap_uint<WIDTH> tmpWrWord;
    mmStatus tmpStatus;
    uint8_t nTrail = 0;
    uint32_t nInputWords = 0;

    uint64_t firstWord = 0;
    uint64_t lastWord = 0;

    switch (state) {
        case IDLE:
            if (!cmdRead.empty()) {
                currCmd = axiu2mmCmd(cmdRead.read());
                set_common(cVars, currCmd.saddr, currCmd.bbt);
            
                nTrail = calculateTrailBytes(cVars.btt);

                if (cVars.taddr(WINDOW_BITS - 1, 3) != (currCmd.saddr(WINDOW_BITS - 1, 3))) {
                    // If all the required bytes from the last word can be shifted into the previous word
                    if (8 - nTrail <= cVars.nStartWordByte) {
                        cVars.nTransferWords--;
                    }
                }
                // Add another cycle for latency (nothing output in the first cycle)
                cVars.nTransferWords++;

                if (cVars.nBuffer >= MAX_SESSIONS) {
                    state = RD_ERROR;
                    break;
                }

            read_loop:
                for (int jj = 0; jj < cVars.nTransferWords; jj++) {
#pragma HLS pipeline II = 1
                    tmpWord.data = mem_buf.read(cVars.nBufferAddr + (cVars.nCurrWordAddr >> 3));
                    if (cVars.nStartWordByte > 0) {
                        lshiftWord = leftShiftOctets(tmpWord.data, cVars.nShift);
                    } else {
                        lshiftWord = 0;
                    }
                    // take the rshift (from previous word) and OR with the lshift
                    lshiftWord |= cVars.rshiftWord;
                    // Calculate the new rshift.
                    cVars.rshiftWord = rightShiftOctets(tmpWord.data, cVars.nStartWordByte);
                    cVars.nCurrWordAddr += 8;

                    tmpWord.last = (jj == cVars.nTransferWords - 1);

                    // Drop the first word
                    if (jj > 0) {
                        tmpWord.data = lshiftWord;
                        tmpWord.keep = 0xFF;
                        if (tmpWord.last) {
                            if ((currCmd.bbt % 8) > 0) {
                                tmpWord.keep = 0xFF >> (8 - (currCmd.bbt % 8));
                            }
                        }
                        dataOut.write(tmpWord);
                    }
                }
            } else if (!cmdWrite.empty()) {
                currCmd = axiu2mmCmd(cmdWrite.read());
                set_common(cVars, currCmd.saddr, currCmd.bbt);
                state = WRITE;

                if (cVars.nBuffer >= MAX_SESSIONS) {
                    state = WR_ERROR;
                } else {
                    state = WRITE;
                }
            }
            break;
        case WRITE:
            // Do we need to read-modify-write the word from memory?
            // Check if:
            // 1. Start at a non-word aligned offset.
            // 2. We have a partial word at the end
            if (currCmd.saddr(WINDOW_BITS - 1, 0) > cVars.nCurrWordAddr) {
                firstWord = mem_buf.read(cVars.nCurrWordAddr + cVars.nBufferAddr);
            }
            if (cVars.taddr(2, 0) != 7) {
                lastWord = mem_buf.read(cVars.nCurrWordAddr + cVars.nTransferWords + cVars.nBufferAddr);
            }

            // Work out the number of transfers on the slave interface.
            nInputWords = currCmd.bbt / 8;
            if (currCmd.bbt % 8) {
                nInputWords++;
            }

        write_loop:
            for (int jj = 0; jj < nInputWords; jj++) {
#pragma HLS pipeline II = 1
#pragma HLS dependence variable = mem_buf.blocks inter false
                dataIn.read(tmpWord);

                // Left shift to align the data with the word being read from memory
                lshiftWord = leftShiftOctets(tmpWord.data, cVars.nStartWordByte);

                // Take the rshift (from previous word) and OR with the lshift
                lshiftWord |= cVars.rshiftWord;
                if (cVars.nStartWordByte == 0) {
                    cVars.rshiftWord = 0; // No need to shift, just zero all the data.
                } else {
                    cVars.rshiftWord = rightShiftOctets(tmpWord.data, cVars.nShift);
                }

                // Check if: Start at a non-word aligned offset.
                if (cVars.nCurrWordAddr < currCmd.saddr(WINDOW_BITS - 1, 0)) {
                    tmpWrWord = firstWord;
                    // Overwrite the relevant octets
                    for (int ii = 0; ii < 8; ii++) {
#pragma HLS unroll
                        if ((cVars.nCurrWordAddr + ii) >= currCmd.saddr(WINDOW_BITS - 1, 0)) {
                            tmpWrWord(ii * 8 + 7, ii * 8) = (lshiftWord >> (ii * 8)) & 0xFF;
                        }
                    }
                } else {
                    tmpWrWord = lshiftWord;
                }
                // Write word to memory
                mem_buf.write(((cVars.nCurrWordAddr >> 3) + cVars.nBufferAddr), tmpWrWord);
                cVars.nCurrWordAddr += 8;
            }

            // Do we have remaining shifted data still to write?
            if (cVars.nTransferWords > nInputWords) {
                // Load in the last word from memory
                tmpWrWord = lastWord;
                for (int ii = 0; ii < 8; ii++) {
#pragma HLS unroll
                    if (cVars.nCurrWordAddr + ii <= cVars.taddr) {
                        tmpWrWord(ii * 8 + 7, ii * 8) = (lshiftWord >> (ii * 8)) & 0xFF;
                    }
                }
                // Write word to memory
                mem_buf.write(( (cVars.nCurrWordAddr >> 3) + cVars.nBufferAddr), tmpWrWord);
            }

            // Finished transferring words
            tmpStatus.okay = 1;
            writeStatus.write(mmStatus2axiu(tmpStatus));
            state = IDLE;
            break;
        case WR_ERROR:
            tmpStatus.okay = 0;
            writeStatus.write(mmStatus2axiu(tmpStatus));
            state = IDLE;
            break;
        case RD_ERROR:
            state = IDLE;
            break;
    }
}

/** @ingroup uram_datamover
 *  Main function.
 *  @param[in]		cmdRead
 *  @param[in]		cmdWrite
 *  @param[in]		dataIn
 *  @param[out]		writeStatus
 *  @param[out]		dataOut
 */
void uram_datamover(hls::stream<ap_axiu<128,0,0,0> >& cmdRead,
                    hls::stream<ap_axiu<128,0,0,0> >& cmdWrite,
                    hls::stream<ap_axiu<64,0,0,0> >& dataIn,
                    hls::stream<ap_axiu<8,0,0,0> >& writeStatus,
                    hls::stream<ap_axiu<64,0,0,0> >& dataOut) {
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INTERFACE axis port = cmdRead name = s_axis_cmdRead
#pragma HLS INTERFACE axis port = cmdWrite name = s_axis_cmdWrite
#pragma HLS INTERFACE axis port = dataIn name = s_axis_data
#pragma HLS INTERFACE axis port = dataOut name = m_axis_data
#pragma HLS INTERFACE axis port = writeStatus name = m_axis_writeStatus

    uram_datamover_body<64>(cmdRead, cmdWrite, dataIn, writeStatus, dataOut);
}
