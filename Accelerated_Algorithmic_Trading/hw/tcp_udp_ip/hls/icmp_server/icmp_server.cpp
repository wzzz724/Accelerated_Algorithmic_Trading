/************************************************
 * Copyright (c) 2016, 2020 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ************************************************/
#include "icmp_server.hpp"

/**
 * @ingroup icmp_server
 *
 * Detect a multicast IP address.
 *
 * @param[in] addr IP address
 * @return         true if addr is a multicast IP address
 */
bool isMulticastAddr(ap_uint<32> addr) {
    return (addr(7, 4) == 0xE);
}

/** @ingroup icmp_server
 *  No MAC Header, already shaved off
 *  Assumption no options in IP header
 *  @param[in]		dataIn
 *  @param[out]		dataOut
 *  @param[out]		icmpValidFifoOut
 *  @param[out]		checksumFifoOut
 *  @param[out]         icmpInEchos
 */
void check_icmp_checksum(hls::stream<ap_axiu<64,0,0,0> >& dataIn,
                         hls::stream<axiWord>& dataOut,
                         hls::stream<bool>& ValidFifoOut,
                         hls::stream<ap_uint<16> >& checksumFifoOut,
                         uint32_t& icmpInEchos,
                         bool enable) {
#pragma HLS INLINE off
#pragma HLS pipeline II = 1

    static ap_uint<17> cics_sums[4] = {0, 0, 0, 0};
    static ap_uint<16> cics_wordCount = 0;
    static axiWord cics_prevWord;
    static bool cics_writeLastOne = false;
    static bool cics_computeCs = false;
    static ap_uint<2> cics_state = 0;

    static ap_uint<8> newTTL = 0x40;
    static ap_uint<17> icmpChecksum = 0;
    static ap_uint<8> icmpType;
    static ap_uint<8> icmpCode;
    static bool is_multicast = false;
    static uint32_t cnt_icmpInEchos = 0;

    ap_axiu<64,0,0,0> currWord;
    axiWord sendWord;
    uint16_t iph_cs;

    currWord.last = 0;
    if (cics_writeLastOne) {
        dataOut.write(cics_prevWord);
        cics_writeLastOne = false;
    } else if (cics_computeCs) {
        switch (cics_state) {
            case 0:
                cics_sums[0] += cics_sums[2];
                cics_sums[0] = (cics_sums[0] + (cics_sums[0] >> 16)) & 0xFFFF;
                cics_sums[1] += cics_sums[3];
                cics_sums[1] = (cics_sums[1] + (cics_sums[1] >> 16)) & 0xFFFF;
                icmpChecksum = ~icmpChecksum;
                break;
            case 1:
                cics_sums[0] += cics_sums[1];
                cics_sums[0] = (cics_sums[0] + (cics_sums[0] >> 16)) & 0xFFFF;
                icmpChecksum -= ECHO_REQUEST;
                icmpChecksum = (icmpChecksum - (icmpChecksum >> 16)) & 0xFFFF;
                break;
            case 2:
                cics_sums[0] = ~cics_sums[0];
                icmpChecksum = ~icmpChecksum;
                break;
            case 3:
                // Check for 0 in checksum
                // Silent drop of multicast frames. RFC-1122 3.2.2.6
                if ((cics_sums[0](15, 0) == 0) && (icmpType == ECHO_REQUEST) && (icmpCode == 0) && !is_multicast) {
                    cnt_icmpInEchos++;
                    if (enable) {
                        ValidFifoOut.write(true);
                        checksumFifoOut.write(icmpChecksum);
                    } else {
                        ValidFifoOut.write(false);
                    }
                } else
                    ValidFifoOut.write(false);
                cics_computeCs = false;
                break;
        }
        cics_state++;
    } else if (!dataIn.empty()) {
        dataIn.read(currWord);
        switch (cics_wordCount) {
            case WORD_0:
                cics_sums[0] = 0;
                cics_sums[1] = 0;
                cics_sums[2] = 0;
                cics_sums[3] = 0;
                break;
            case WORD_1:
                sendWord = cics_prevWord;
                dataOut.write(sendWord); // 1
                // Contains Src IP address
                // Modify the TTL to be our default rather than received value
                iph_cs = ~currWord.data(31, 16);
                iph_cs -= currWord.data(7, 0);
                iph_cs += newTTL;
                iph_cs = ~iph_cs;
                currWord.data(31, 16) = iph_cs;
                currWord.data(7, 0) = newTTL;
                break;
            case WORD_2:
                // Read Word contains Dest IP address which becomes our source IP.
                sendWord.data(31, 0) = cics_prevWord.data(31, 0);
                // If we've received a multicast echo request. note this.
                is_multicast = isMulticastAddr(currWord.data(31, 0));
                // This field is the received source IP address which becomes our dest IP.
                sendWord.data(63, 32) = currWord.data(31, 0);
                icmpType = currWord.data(39, 32);
                icmpCode = currWord.data(47, 40);
                icmpChecksum(15, 0) = currWord.data(63, 48);
                icmpChecksum[16] = 1;
                sendWord.keep = 0xFF;
                sendWord.last = 0;
                dataOut.write(sendWord);
                for (int i = 2; i < 4; i++) {
#pragma HLS UNROLL
                    ap_uint<16> temp;
                    temp(7, 0) = currWord.data.range(i * 16 + 15, i * 16 + 8);
                    temp(15, 8) = currWord.data.range(i * 16 + 7, i * 16);
                    cics_sums[i] += temp;
                    cics_sums[i] = (cics_sums[i] + (cics_sums[i] >> 16)) & 0xFFFF;
                }
                currWord.data(31, 0) = cics_prevWord.data(63, 32);
                currWord.data.range(39, 32) = ECHO_REPLY;
                break;
            default:
                for (int i = 0; i < 4; i++) {
#pragma HLS UNROLL
                    ap_uint<16> temp;
                    if (currWord.keep.range(i * 2 + 1, i * 2) == 0x3) {
                        temp(7, 0) = currWord.data.range(i * 16 + 15, i * 16 + 8);
                        temp(15, 8) = currWord.data.range(i * 16 + 7, i * 16);
                        cics_sums[i] += temp;
                        cics_sums[i] = (cics_sums[i] + (cics_sums[i] >> 16)) & 0xFFFF;
                    } else if (currWord.keep[i * 2] == 0x1) {
                        temp(7, 0) = 0;
                        temp(15, 8) = currWord.data.range(i * 16 + 7, i * 16);
                        cics_sums[i] += temp;
                        cics_sums[i] = (cics_sums[i] + (cics_sums[i] >> 16)) & 0xFFFF;
                    }
                }
                sendWord = cics_prevWord;
                dataOut.write(sendWord);
                break;
        } // switch
        cics_prevWord.data = currWord.data;
        cics_prevWord.keep = currWord.keep;
        cics_prevWord.last = currWord.last;
        cics_wordCount++;
        if (currWord.last == 1) {
            cics_wordCount = 0;
            cics_writeLastOne = true;
            cics_computeCs = true;
        }
    }
    icmpInEchos = cnt_icmpInEchos;
}

/** @ingroup icmp_server
 *  Reads valid bit from validBufffer, if package is invalid it is dropped otherwise it is forwarded
 *  Stats generator. However as we are swapping everything the icmp checksum
 *  it makes slightly more sense to do the reply count here as only valid replies will get this far
 *  @param[in]		dataIn
 *  @param[in]		validFifoIn
 *  @param[out]		dataOut
 *  @param[out]		icmpOutEchoReps
 */
void dropper(hls::stream<axiWord>& dataIn,
             hls::stream<bool>& validFifoIn,
             hls::stream<axiWord>& dataOut,
             uint32_t& icmpOutEchoReps) {
    enum drop_statetype { GET_VALID, FWD, DROP };
    static drop_statetype state = GET_VALID;
    static uint32_t cnt_icmpOutEchoReps = 0;

    switch (state) {
        case GET_VALID:
            if (!validFifoIn.empty()) {
                bool valid = validFifoIn.read();
                if (valid) {
                    state = FWD;
                } else {
                    state = DROP;
                }
            }
            break;
        case FWD:
            if (!dataIn.empty()) {
                axiWord currWord = dataIn.read();
                dataOut.write(currWord);
                if (currWord.last) {
                    state = GET_VALID;
                    cnt_icmpOutEchoReps++;
                }
            }
            break;
        case DROP:
            if (!dataIn.empty()) {
                axiWord currWord = dataIn.read();
                if (currWord.last) {
                    state = GET_VALID;
                }
            }
            break;
    }
    icmpOutEchoReps = cnt_icmpOutEchoReps;
}

/** @ingroup icmp_server
 *  Inserts IP & ICMP checksum at the correct position
 *  @param[in]		dataIn
 *  @param[in]		checksumFifoIn
 *  @param[out]		dataOut
 */
template <int dummy>
void insertChecksum(hls::stream<axiWord>&inputStream,
                    hls::stream<ap_uint<16> >& checksumStream,
                    hls::stream<ap_axiu<64,0,0,0> >& outputStream) {
#pragma HLS INLINE off
#pragma HLS pipeline II = 1

    axiWord inputWord = {};
    ap_axiu<64,0,0,0> outWord;
    ap_uint<16> icmpChecksum = 0;
    static ap_uint<2> ic_wordCount = 0;

    switch (ic_wordCount) {
        case 0:
            if (!inputStream.empty()) {
                inputWord = inputStream.read();
                outWord.data = inputWord.data;
                outWord.keep = inputWord.keep;
                outWord.last = inputWord.last;
                outputStream.write(outWord);
                ic_wordCount++;
            }
            break;
        case 2:
            if (!inputStream.empty() && !checksumStream.empty()) {
                inputStream.read(inputWord);
                icmpChecksum = checksumStream.read();
                inputWord.data(63, 48) = icmpChecksum;
                outWord.data = inputWord.data;
                outWord.keep = inputWord.keep;
                outWord.last = inputWord.last;
                outputStream.write(outWord);
                ic_wordCount++;
            }
            break;
        default:
            if (!inputStream.empty()) {
                inputStream.read(inputWord);
                outWord.data = inputWord.data;
                outWord.keep = inputWord.keep;
                outWord.last = inputWord.last;
                outputStream.write(outWord);
                if (inputWord.last == 1) {
                    ic_wordCount = 0;
                } else if (ic_wordCount < 3) {
                    ic_wordCount++;
                }
            }
            break;
    }
}

/** @ingroup icmp_server
 *  Main function
 *  @param[in]		dataIn
 *  @param[out]		dataOut
 *
 * @todo Write descriptions for missing function parameters
 */
void icmp_server(hls::stream<ap_axiu<64,0,0,0> >& dataIn,
                 hls::stream<ap_axiu<64,0,0,0> >& dataOut,
                 bool enable,
                 uint32_t &icmpInEchos,
                 uint32_t &icmpOutEchoReps) {
#pragma HLS INTERFACE ap_ctrl_none port = return

#pragma HLS INTERFACE axis register port = dataIn
#pragma HLS INTERFACE axis register port = dataOut
#pragma HLS INTERFACE s_axilite port = enable bundle = control
#pragma HLS INTERFACE s_axilite port = icmpInEchos bundle = control
#pragma HLS INTERFACE s_axilite port = icmpOutEchoReps bundle = control
#pragma HLS DATAFLOW disable_start_propagation

    static hls::stream<axiWord> packageBuffer1("packageBuffer1");
    static hls::stream<ap_uint<16> > checksumStream("icmp_checksum");
    static hls::stream<bool> validFifo("validFifo");
    static hls::stream<axiWord> dropper2insert("dropper2insert");
#pragma HLS stream variable = packageBuffer1 depth = 192
#pragma HLS stream variable = validFifo depth = 8
#pragma HLS stream variable = dropper2insert depth = 16
#pragma HLS stream variable = checksumStream depth = 16

    check_icmp_checksum(dataIn, packageBuffer1, validFifo, checksumStream, icmpInEchos, enable);
    dropper(packageBuffer1, validFifo, dropper2insert, icmpOutEchoReps);
    insertChecksum<0>(dropper2insert, checksumStream, dataOut);
}
