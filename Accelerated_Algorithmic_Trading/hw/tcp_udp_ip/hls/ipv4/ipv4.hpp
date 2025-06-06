/*
 * Copyright (c) 2018, Systems Group, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef IP4_HPP
#define IP4_HPP

#include "../axi_utils.hpp"
#include "../packet.hpp"

const uint32_t IPV4_HEADER_SIZE = 160;

struct __attribute__((packed)) ipv4Meta {
    ap_uint<32> their_address;
    ap_uint<16> length;
    ipv4Meta() {}
    ipv4Meta(ap_uint<32> addr, ap_uint<16> len) : their_address(addr), length(len) {}
    ipv4Meta(ap_uint<128> addr, ap_uint<16> len) : their_address(addr(31, 0)), length(len) {}
};

template <int N>
struct subSums {
    ap_uint<17> sum[N];
};

template <int WIDTH, class T>
void drop_optional_ip_header(hls::stream<ap_uint<4> >& process2dropLengthFifo,
                             hls::stream<net_axis<WIDTH> >& process2dropFifo,
                             hls::stream<T> & dataOut) {
#pragma HLS INLINE off
#pragma HLS pipeline II = 1

    enum fsmStateType { META, DROP, BODY, SHIFT, LAST, SHIFT_FIVE, LAST_FIVE };
    static fsmStateType doh_state = META;
    static ap_uint<4> length;

    static net_axis<WIDTH> prevWord;
    net_axis<WIDTH> currWord;
    T sendWord;

    switch (doh_state) {
        case META:
            if (!process2dropLengthFifo.empty() && !process2dropFifo.empty()) {
                process2dropLengthFifo.read(length);
                std::cout << "(Optional) Header length: " << length << std::endl;

                // TODO for WIDTH == 128 this only works if no IP options
                if (WIDTH == 64 || WIDTH == 128) {
                    if (length > 1) {
                        doh_state = DROP;
                    } else {
                        process2dropFifo.read(prevWord);
                        doh_state = SHIFT;
                    }
                }
                if (WIDTH == 256 || WIDTH == 512) {
                    // TODO this is a hack and only works if there are no IP options
                    if (length == 5) {
                        process2dropFifo.read(prevWord);
                        doh_state = SHIFT_FIVE;
                        if (prevWord.last) {
                            doh_state = LAST_FIVE;
                        }
                    }
                }
            }
            break;
        case DROP:
            if (!process2dropFifo.empty()) {
                process2dropFifo.read(prevWord);
                length -= 2;
                if (length == 1) {
                    doh_state = SHIFT;
                } else if (length == 0) {
                    doh_state = BODY;
                }
            }
            break;
        case BODY:
            if (!process2dropFifo.empty()) {
                process2dropFifo.read(currWord);
                sendWord.data = currWord.data;
                sendWord.keep = currWord.keep;
                sendWord.last = currWord.last;
                dataOut.write(sendWord);
                if (currWord.last) {
                    doh_state = META;
                }
            }
            break;
        case SHIFT:
            if (!process2dropFifo.empty()) {
                process2dropFifo.read(currWord);
                sendWord.data(WIDTH - 32 - 1, 0) = prevWord.data(WIDTH - 1, 32);
                sendWord.keep((WIDTH / 8) - 4 - 1, 0) = prevWord.keep((WIDTH / 8) - 1, 4);
                sendWord.data(WIDTH - 1, WIDTH - 32) = currWord.data(31, 0);
                sendWord.keep((WIDTH / 8) - 1, (WIDTH / 8) - 4) = currWord.keep(3, 0);
                sendWord.last = (currWord.keep[4] == 0);
                dataOut.write(sendWord);
                prevWord = currWord;
                if (sendWord.last) {
                    doh_state = META;
                } else if (currWord.last) {
                    doh_state = LAST;
                }
            }
            break;
        case SHIFT_FIVE:
            if (!process2dropFifo.empty()) {
                process2dropFifo.read(currWord);
                sendWord.data(WIDTH - 160 - 1, 0) = prevWord.data(WIDTH - 1, 160);
                sendWord.keep((WIDTH / 8) - 20 - 1, 0) = prevWord.keep((WIDTH / 8) - 1, 20);
                sendWord.data(WIDTH - 1, WIDTH - 160) = currWord.data(160 - 1, 0);
                sendWord.keep((WIDTH / 8) - 1, (WIDTH / 8) - 20) = currWord.keep(20 - 1, 0);
                sendWord.last = (currWord.keep[20] == 0);
                dataOut.write(sendWord);
                prevWord = currWord;
                if (sendWord.last) {
                    doh_state = META;
                } else if (currWord.last) {
                    doh_state = LAST_FIVE;
                }
            }
            break;
        case LAST:
            sendWord.data(WIDTH - 32 - 1, 0) = prevWord.data(WIDTH - 1, 32);
            sendWord.keep((WIDTH / 8) - 4 - 1, 0) = prevWord.keep((WIDTH / 8) - 1, 4);
            sendWord.data(WIDTH - 1, WIDTH - 32) = 0;
            sendWord.keep((WIDTH / 8) - 1, (WIDTH / 8) - 4) = 0x0;
            sendWord.last = 0x1;
            dataOut.write(sendWord);
            doh_state = META;
            break;
        case LAST_FIVE:
            sendWord.data(WIDTH - 160 - 1, 0) = prevWord.data(WIDTH - 1, 160);
            sendWord.keep((WIDTH / 8) - 20 - 1, 0) = prevWord.keep((WIDTH / 8) - 1, 20);
            sendWord.data(WIDTH - 1, WIDTH - 160) = 0;
            sendWord.keep((WIDTH / 8) - 1, (WIDTH / 8) - 20) = 0;
            sendWord.last = 0x1;
            dataOut.write(sendWord);
            doh_state = META;
            break;
    } // switch
}

void compute_ipv4_checksum(hls::stream<net_axis<64> >& dataIn,
                           hls::stream<net_axis<64> >& dataOut,
                           hls::stream<subSums<4> >& subSumFiFoOut,
                           const bool skipChecksum);

template <int N>
void check_ipv4_checksum(hls::stream<subSums<N> >& subSumFiFoIn, hls::stream<bool>& checkSumFifoOut) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    if (!subSumFiFoIn.empty()) {
        subSums<N> icic_ip_sums = subSumFiFoIn.read();

        if (N >= 32) {
            for (int i = 0; i < 16; i++) {
#pragma HLS unroll
                icic_ip_sums.sum[i] += icic_ip_sums.sum[i + 16];
                icic_ip_sums.sum[i] = (icic_ip_sums.sum[i] + (icic_ip_sums.sum[i] >> 16)) & 0xFFFF;
            }
        }

        if (N >= 16) {
            for (int i = 0; i < 8; i++) {
#pragma HLS unroll
                icic_ip_sums.sum[i] += icic_ip_sums.sum[i + 8];
                icic_ip_sums.sum[i] = (icic_ip_sums.sum[i] + (icic_ip_sums.sum[i] >> 16)) & 0xFFFF;
            }
        }

        if (N >= 8) {
            for (int i = 0; i < 4; i++) {
#pragma HLS unroll
                icic_ip_sums.sum[i] += icic_ip_sums.sum[i + 4];
                icic_ip_sums.sum[i] = (icic_ip_sums.sum[i] + (icic_ip_sums.sum[i] >> 16)) & 0xFFFF;
            }
        }

        // N >= 4 -> 64bit data
        icic_ip_sums.sum[0] += icic_ip_sums.sum[2];
        icic_ip_sums.sum[1] += icic_ip_sums.sum[3];
        icic_ip_sums.sum[0] = (icic_ip_sums.sum[0] + (icic_ip_sums.sum[0] >> 16)) & 0xFFFF;
        icic_ip_sums.sum[1] = (icic_ip_sums.sum[1] + (icic_ip_sums.sum[1] >> 16)) & 0xFFFF;
        icic_ip_sums.sum[0] += icic_ip_sums.sum[1];
        icic_ip_sums.sum[0] = (icic_ip_sums.sum[0] + (icic_ip_sums.sum[0] >> 16)) & 0xFFFF;
        icic_ip_sums.sum[0] = ~icic_ip_sums.sum[0];
        checkSumFifoOut.write(icic_ip_sums.sum[0](15, 0) == 0);
    }
}

template <int N>
void finalize_ipv4_checksum(hls::stream<subSums<N> >& subSumFiFoIn, hls::stream<ap_uint<16> >& checkSumFifoOut) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    if (!subSumFiFoIn.empty()) {
        subSums<N> icic_ip_sums = subSumFiFoIn.read();

        if (N >= 32) {
            for (int i = 0; i < 16; i++) {
#pragma HLS unroll
                icic_ip_sums.sum[i] += icic_ip_sums.sum[i + 16];
                icic_ip_sums.sum[i] = (icic_ip_sums.sum[i] + (icic_ip_sums.sum[i] >> 16)) & 0xFFFF;
            }
        }

        if (N >= 16) {
            for (int i = 0; i < 8; i++) {
#pragma HLS unroll
                icic_ip_sums.sum[i] += icic_ip_sums.sum[i + 8];
                icic_ip_sums.sum[i] = (icic_ip_sums.sum[i] + (icic_ip_sums.sum[i] >> 16)) & 0xFFFF;
            }
        }

        if (N >= 8) {
            for (int i = 0; i < 4; i++) {
#pragma HLS unroll
                icic_ip_sums.sum[i] += icic_ip_sums.sum[i + 4];
                icic_ip_sums.sum[i] = (icic_ip_sums.sum[i] + (icic_ip_sums.sum[i] >> 16)) & 0xFFFF;
            }
        }

        // N >= 4 -> 64bit data
        icic_ip_sums.sum[0] += icic_ip_sums.sum[2];
        icic_ip_sums.sum[1] += icic_ip_sums.sum[3];
        icic_ip_sums.sum[0] = (icic_ip_sums.sum[0] + (icic_ip_sums.sum[0] >> 16)) & 0xFFFF;
        icic_ip_sums.sum[1] = (icic_ip_sums.sum[1] + (icic_ip_sums.sum[1] >> 16)) & 0xFFFF;
        icic_ip_sums.sum[0] += icic_ip_sums.sum[1];
        icic_ip_sums.sum[0] = (icic_ip_sums.sum[0] + (icic_ip_sums.sum[0] >> 16)) & 0xFFFF;
        icic_ip_sums.sum[0] = ~icic_ip_sums.sum[0];
        checkSumFifoOut.write(icic_ip_sums.sum[0](15, 0));
    }
}

/**
 * ipv4Header. All fields are stored internally in their network order format
 * i.e. IP Address 10.0.0.1 is stored as [31:0] 0x0100000A
 * [7:4] = version;
 * [3:0] = IHL;
 * [13:8] = DSCP;
 * [15:14] = ECN;
 * [31:16] = length;
 * [47:32] = Idendification;
 * [50:48] = Flags;
 * [63:51] = fragment offset;
 * [71:64] = TTL;
 * [79:72] = Protocol;
 * [95:80] = HeaderChecksum;
 * [127:96] = SrcAddr;
 * [159:128] = DstAddr;
 * [...] = IHL;
 */
template <int W>
class ipv4Header : public packetHeader<W, IPV4_HEADER_SIZE> {
    using packetHeader<W, IPV4_HEADER_SIZE>::header;

   public:
    ipv4Header() {
        header(7, 0) = 0x45;   // version & IHL
        header(71, 64) = 0x40; // TTL
    }
    // Set the fields (NO / Big Endian) from the input (Little-endian)    
    void setSrcAddr(const ap_uint<32>& addr) { header(127, 96) = reverse(addr); }
    ap_uint<32> getSrcAddr() { return reverse<32>(header(127, 96)); }
    void setDstAddr(const ap_uint<32>& addr) { header(159, 128) = reverse(addr); }
    // Call in map_ip_encode cannot deduce the correct template - override.
    ap_uint<32> getDstAddr() { return reverse<32>(header(159, 128)); }
    void setLength(const ap_uint<16> len) { header(31, 16) = reverse(len); }
    ap_uint<16> getLength() { return reverse((ap_uint<16>)header(31, 16)); }
    void setProtocol(const ap_uint<8>& protocol) { header(79, 72) = protocol; }
    ap_uint<8> getProtocol() { return header(79, 72); }
    ap_uint<4> getHeaderLength() { return header(3, 0); }
};

/**
 * @defgroup ipv4 IPv4 Module
 */

void ipv4(hls::stream<ap_axiu<64,0,0,0> >& s_axis_rx_data,
          hls::stream<ipv4Meta>& m_axis_rx_meta,
          hls::stream<ap_axiu<64,0,0,0> >& m_axis_rx_data,
          hls::stream<ipv4Meta>& s_axis_tx_meta,
          hls::stream<ap_axiu<64,0,0,0> >& s_axis_tx_data,
          hls::stream<ap_axiu<64,0,0,0> >& m_axis_tx_data,
          ap_uint<32> local_ipv4_address,
          ap_uint<8> protocol);

#endif
