/************************************************
Copyright (c) 2016, 2019-2020, Xilinx, Inc.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.// Copyright (c) 2015 Xilinx, Inc.
************************************************/

#include "../axi_utils.hpp"
#include "../packet.hpp"

const int ETH_HEADER_SIZE = 112;

const uint16_t ARP = 0x0806;
const uint16_t IPv4 = 0x0800;

const uint8_t ICMP = 0x01;
const uint8_t IGMP = 0x02;
const uint8_t UDP = 0x11;
const uint8_t TCP = 0x06;


enum ip_handler_cfg { CONFIG_UDP = 1, CONFIG_TCP = 2, CONFIG_TCP_UDP = 3};

struct subSums {
    ap_uint<17> sum0;
    ap_uint<17> sum1;
    ap_uint<17> sum2;
    ap_uint<17> sum3;
    bool ipMatch;
    bool hdrValid;
    subSums() {}
    subSums(ap_uint<17> sums[4], bool match, bool valid)
        : sum0(sums[0]), sum1(sums[1]), sum2(sums[2]), sum3(sums[3]), ipMatch(match), hdrValid(valid) {}
    subSums(ap_uint<17> s0, ap_uint<17> s1, ap_uint<17> s2, ap_uint<17> s3, bool match, bool valid)
        : sum0(s0), sum1(s1), sum2(s2), sum3(s3), ipMatch(match), hdrValid(valid) {}
};

/**
 * [47:0] MAC destination
 * [95:48] MAC source
 * [111:96] Ethertype
 */
template <int W>
class ethHeader : public packetHeader<W, ETH_HEADER_SIZE> {
    using packetHeader<W, ETH_HEADER_SIZE>::header;

   public:
    ap_uint<16> getEthertype() { return reverse((ap_uint<16>)header(111, 96)); }
};

/** @defgroup ip_handler TCP UDP IP handler
 *
 * The IP handler decodes Layer 2 Ethernet frames from the network interface
 * and forwards them on to the appropriate streaming interface
 * (ARP, ICMP, UDP or TCP). It also performs an IP address match and
 * verifies the IP header checksum.
 */
void ip_handler_tcp_udp(hls::stream<ap_axiu<64,0,0,0> >& s_axis_raw,
                        hls::stream<ap_axiu<64,0,0,0> >& m_axis_ARP,
                        hls::stream<ap_axiu<64,0,0,0> >& m_axis_ICMP,
                        hls::stream<ap_axiu<64,0,0,0> >& m_axis_IGMP,
                        // hls::stream<ap_axiu<64,0,0,0> >& m_axis_UDP,
                        hls::stream<ap_axiu<64,0,0,0> >& m_axis_TCP,
                        ap_uint<32> myIpAddress,
                        uint32_t& ipInHdrErrors,
                        uint32_t& ipInDelivers,
                        uint32_t& ipInUnknownProtos,
                        uint32_t& ipInAddrErrors,
                        uint32_t& ipInReceives);

/** @defgroup ip_handler UDP IP handler
 *
 * The IP handler decodes Layer 2 Ethernet frames from the network interface
 * and forwards them on to the appropriate streaming interface
 * (ARP, ICMP, UDP ). It also performs an IP address match and
 * verifies the IP header checksum.
 */
void ip_handler_udp(hls::stream<ap_axiu<64,0,0,0> >& s_axis_raw,
                    hls::stream<ap_axiu<64,0,0,0> >& m_axis_ARP,
                    hls::stream<ap_axiu<64,0,0,0> >& m_axis_ICMP,
                    hls::stream<ap_axiu<64,0,0,0> >& m_axis_IGMP,
                    // hls::stream<ap_axiu<64,0,0,0> >& m_axis_UDP,
                    ap_uint<32> myIpAddress,
                        uint32_t& ipInHdrErrors,
                        uint32_t& ipInDelivers,
                        uint32_t& ipInUnknownProtos,
                        uint32_t& ipInAddrErrors,
                        uint32_t& ipInReceives);

/** @defgroup ip_handler TCP IP handler
 *
 * The IP handler decodes Layer 2 Ethernet frames from the network interface
 * and forwards them on to the appropriate streaming interface
 * (ARP, ICMP, TCP ). It also performs an IP address match and
 * verifies the IP header checksum.
 */
void ip_handler_tcp(hls::stream<ap_axiu<64,0,0,0> >& s_axis_raw,
                    hls::stream<ap_axiu<64,0,0,0> >& m_axis_ARP,
                    hls::stream<ap_axiu<64,0,0,0> >& m_axis_ICMP,
                    hls::stream<ap_axiu<64,0,0,0> >& m_axis_IGMP,
                    hls::stream<ap_axiu<64,0,0,0> >& m_axis_TCP,
                    ap_uint<32> myIpAddress,
                    uint32_t& ipInHdrErrors,
                    uint32_t& ipInDelivers,
                    uint32_t& ipInUnknownProtos,
                    uint32_t& ipInAddrErrors,
                    uint32_t& ipInReceives);
