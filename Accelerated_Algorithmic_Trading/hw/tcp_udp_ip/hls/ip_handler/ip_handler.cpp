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

#include "ip_handler.hpp"

struct packet_valid {
    bool ipMatch;
    bool hdrValid;
};

/**
 * @ingroup ip_handler
 *
 * Detect a broadcast IP address.
 *
 * @param[in] addr IP address
 * @return         true if addr is a broadcast IP address
 */
bool isBroadcastAddr(ap_uint<32> addr) {
#pragma HLS INLINE
    return (addr == 0xFFFFFFFF);
}

/**
 * @ingroup ip_handler
 *
 * Detect a multicast IP address.
 *
 * @param[in] addr IP address
 * @return         true if addr is a multicast (Class D) IP address
 */
bool isMulticastAddr(ap_uint<32> addr) {
#pragma HLS INLINE
    return (addr(7, 4) == 0xE);
}

/**
 * @ingroup ip_handler
 *
 * Detect a class E IP address.
 *
 * @param[in] addr IP address
 * @return         true if addr is a Class E IP address
 */
bool isClassEAddr(ap_uint<32> addr) {
#pragma HLS INLINE
    return (addr(7, 4) == 0xF);
}

/**
 * @ingroup hdr_valid
 *
 * Detect an error in the IP header
 *
 * @param[in]      IP version field
 * @return         true if version field is equal to 4
 */
bool hdr_valid(ap_uint<4> version) {
#pragma HLS INLINE
    return (version == 4);
}

/**
 * @ingroup ip_handler
 *
 * Determine if we're matching or dropping the frame based on IPv4 Fields.
 *
 * @param[in] myIP IP address
 * @param[in] dstIP IP address
 * @param[in] version IPv4 version field
 * @return true if it's a match to forward the packet
 */
bool isMatch(ap_uint<32> myIP, ap_uint<32> dstIP) {
#pragma HLS INLINE
    return ((dstIP == myIP) || isMulticastAddr(dstIP) || isBroadcastAddr(dstIP));
}

/**
 * @ingroup ip_handler
 *
 * Determine if we're matching or dropping the frame based on IPv4 Fields.
 *
 * @param[in] protocol IPv4 Protocol field
 * @return true if handled by this configuration
 */
template <ip_handler_cfg cfg>
bool isHandledProtocol(uint8_t protocol) {
#pragma HLS INLINE

    if ((protocol == IGMP) || (protocol == ICMP)) {
        return true;
    } else {
        if (cfg == CONFIG_UDP) {
            if (protocol == UDP) {
                return true;
            }
        } else if (cfg == CONFIG_TCP) {
            if (protocol == TCP) {
                return true;
            }
        } else if (cfg == CONFIG_TCP_UDP) {
            if ((protocol == TCP) || (protocol == UDP)) {
                return true;
            }
        }
    }
    return false;
}

/** @ingroup ip_handler
 *
 *  Detects the MAC protocol in the header of the packet, ARP and IP packets
 *  are forwarded accordingly, packets of other protocols are discarded.
 *
 *  @param[in]		dataIn
 *  @param[out]		ARPdataOut
 *  @param[out]		IPdataOut
 */
void detect_eth_protocol(hls::stream<ap_axiu<64,0,0,0> >& dataIn,
                         hls::stream<ap_uint<16> >& etherTypeFifo,
                         hls::stream<axiWord>& dataOut) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off
    enum stateType { FIRST, MIDDLE, LAST };
    static stateType state = FIRST;
    static ethHeader<AXI_WIDTH> header;
    static axiWord prevWord;
    static bool metaWritten = false;

    switch (state) {
        case FIRST:
            if (!dataIn.empty()) {
                ap_axiu<64,0,0,0> word = dataIn.read();
                header.parseWord(word.data);
                prevWord.data = word.data;
                prevWord.keep = word.keep;
                prevWord.last = word.last;

                state = MIDDLE;
                if (word.last) {
                    state = LAST;
                }
            }
            break;
        case MIDDLE:
            if (!dataIn.empty()) {
                ap_axiu<64,0,0,0> word = dataIn.read();
                header.parseWord(word.data);

                if (!metaWritten) {
                    etherTypeFifo.write(header.getEthertype());
                    metaWritten = true;
#if AXI_WIDTH < 128
                    if (header.getEthertype() == ARP)
#endif
                    {
                        dataOut.write(prevWord);
                    }
                } else {
                    dataOut.write(prevWord);
                }
                prevWord.data = word.data;
                prevWord.keep = word.keep;
                prevWord.last = word.last;
                if (word.last) {
                    state = LAST;
                }
            }
            break;
        case LAST:
            if (!metaWritten) {
                etherTypeFifo.write(header.getEthertype());
            }
            dataOut.write(prevWord);
            header.clear();
            metaWritten = false;
            state = FIRST;
            break;
    } // switch
}

void route_by_eth_protocol(hls::stream<ap_uint<16> >& etherTypeFifoIn,
                           hls::stream<axiWord>& dataIn,
                           hls::stream<ap_axiu<64,0,0,0> >& ARPdataOut,
                           hls::stream<axiWord>& IPdataOut) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static ap_uint<1> rep_fsmState = 0;
    static ap_uint<16> rep_etherType;
    ap_axiu<64,0,0,0> outArp;

    switch (rep_fsmState) {
        case 0:
            if (!etherTypeFifoIn.empty() && !dataIn.empty()) {
                rep_etherType = etherTypeFifoIn.read();
                axiWord word = dataIn.read();
                if (rep_etherType == ARP) {
                    outArp.data = word.data;
                    outArp.keep = word.keep;
                    outArp.last = word.last;
                    ARPdataOut.write(outArp);
                } else if (rep_etherType == IPv4) {
                    IPdataOut.write(word);
                }
                if (!word.last) {
                    rep_fsmState = 1;
                }
            }
            break;
        case 1:
            if (!dataIn.empty()) {
                axiWord word = dataIn.read();
                if (rep_etherType == ARP) {
                    outArp.data = word.data;
                    outArp.keep = word.keep;
                    outArp.last = word.last;                    
                    ARPdataOut.write(outArp);
                } else if (rep_etherType == IPv4) {
                    IPdataOut.write(word);
                }

                if (word.last) {
                    rep_fsmState = 0;
                }
            }
            break;
    } // switch
}

/** @ingroup ip_handler
 *
 *  Checks IP checksum and removes Ethernet layer 2 frame encapsulation.
 *
 *  @param[in]      dataIn              incoming data stream
 *  @param[in]      myIpAddress         our IP address which is set externally
 *  @param[out]     dataOut             outgoing data stream
 *  @param[out]     iph_subSumsFifoIn
 */
void check_ip_checksum(hls::stream<axiWord>& dataIn,
                       ap_uint<32> myIpAddress,
                       hls::stream<axiWord>& dataOut,
                       hls::stream<subSums>& iph_subSumsFifoIn) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static ap_uint<17> cics_ip_sums[4] = {0, 0, 0, 0};
    static ap_uint<8> cics_ipHeaderLen = 0;
    static ap_uint<3> cics_wordCount = 0;
    static ap_uint<32> cics_dstIpAddress = 0;
    static ap_uint<4> cics_ipVersion = 0;
    static bool cics_csumSent = false;
    static bool cics_csumSend = false;
    static subSums subSums_i;

    ap_uint<16> temp;

    if (!dataIn.empty()) {
        axiWord currWord = dataIn.read();
        dataOut.write(currWord);
        switch (cics_wordCount) {
            case 0:
                cics_ipHeaderLen = currWord.data(3, 0);
                cics_ipVersion = currWord.data(7, 4);
                // Fall through
            case 1:
                for (int i = 0; i < 4; i++) {
#pragma HLS unroll
                    temp(7, 0) = currWord.data.range(i * 16 + 15, i * 16 + 8);
                    temp(15, 8) = currWord.data.range(i * 16 + 7, i * 16);
                    cics_ip_sums[i] += temp;
                    cics_ip_sums[i] = (cics_ip_sums[i] + (cics_ip_sums[i] >> 16)) & 0xFFFF;
                }
                cics_ipHeaderLen -= 2;
                cics_wordCount++;
                break;
            default:
                if (cics_wordCount == 2) {
                    cics_wordCount++;
                    cics_dstIpAddress = currWord.data(31, 0);
                }
                switch (cics_ipHeaderLen) {
                    case 0:
                        // length 0 means we are just handling payload
                        break;
                    case 1:
                        // Sum up part 0-1
                        for (int i = 0; i < 2; i++) {
#pragma HLS unroll
                            temp(7, 0) = currWord.data.range(i * 16 + 15, i * 16 + 8);
                            temp(15, 8) = currWord.data.range(i * 16 + 7, i * 16);
                            cics_ip_sums[i] += temp;
                            cics_ip_sums[i] = (cics_ip_sums[i] + (cics_ip_sums[i] >> 16)) & 0xFFFF;
                        }
                        cics_ipHeaderLen = 0;
                        subSums_i =
                            subSums(cics_ip_sums, isMatch(myIpAddress, cics_dstIpAddress), hdr_valid(cics_ipVersion));
                        cics_csumSend = true;
                        break;
                    default:
                        // Sum up everything
                        for (int i = 0; i < 4; i++) {
#pragma HLS unroll
                            if (currWord.keep(i * 2 + 1, i * 2) == 3) {
                                temp(7, 0) = currWord.data.range(i * 16 + 15, i * 16 + 8);
                                temp(15, 8) = currWord.data.range(i * 16 + 7, i * 16);
                                cics_ip_sums[i] += temp;
                                cics_ip_sums[i] = (cics_ip_sums[i] + (cics_ip_sums[i] >> 16)) & 0xFFFF;
                            }
                        }
                        if (cics_ipHeaderLen == 2) {
                            subSums_i = subSums(cics_ip_sums, isMatch(myIpAddress, cics_dstIpAddress),
                                                hdr_valid(cics_ipVersion));
                            cics_csumSend = true;
                        }
                        cics_ipHeaderLen -= 2;
                        break;
                }
                break;
        } // switch WORD_N

        if (!cics_csumSent) {
            if (cics_csumSend) {
                iph_subSumsFifoIn.write(subSums_i);
                cics_csumSent = true;
            } else if (currWord.last) {
                // If this condition is true, then we've had a malformed packet due to incorrect IHL.
                // Sent the csums anyway and mark isMatch as false, hdrError true to ensure the packet has a 'checksum'
                // to be processed.
                iph_subSumsFifoIn.write(subSums(cics_ip_sums, false, true));
            }
        }

        if (currWord.last) {
            cics_csumSend = false;
            cics_csumSent = false;
            cics_wordCount = 0;
            for (int i = 0; i < 4; i++) {
#pragma HLS unroll
                cics_ip_sums[i] = 0;
            }
        }
    }
}

/**
 * @ingroup ip_handler
 *
 * Validate IP checksum from partial checksums and IP version field
 *
 * @param[in] iph_subSumsFifoOut  Partial checksums in
 * @param[out] iph_validFifoOut   Signal valid checksums & metadata match out
 *
 */
void iph_check_ip_checksum(hls::stream<subSums>& iph_subSumsFifoOut, hls::stream<packet_valid>& iph_validFifoOut) {
#pragma HLS PIPELINE II = 1
#pragma HlS INLINE off

    if (!iph_subSumsFifoOut.empty()) {
        subSums icic_ip_sums = iph_subSumsFifoOut.read();
        icic_ip_sums.sum0 += icic_ip_sums.sum2;
        icic_ip_sums.sum1 += icic_ip_sums.sum3;
        icic_ip_sums.sum0 = (icic_ip_sums.sum0 + (icic_ip_sums.sum0 >> 16)) & 0xFFFF;
        icic_ip_sums.sum1 = (icic_ip_sums.sum1 + (icic_ip_sums.sum1 >> 16)) & 0xFFFF;
        icic_ip_sums.sum0 += icic_ip_sums.sum1;
        icic_ip_sums.sum0 = (icic_ip_sums.sum0 + (icic_ip_sums.sum0 >> 16)) & 0xFFFF;
        icic_ip_sums.sum0 = ~icic_ip_sums.sum0;
        packet_valid tmp_valid;
        tmp_valid.ipMatch = icic_ip_sums.ipMatch;
        tmp_valid.hdrValid = icic_ip_sums.hdrValid && (icic_ip_sums.sum0(15, 0) == 0x0000);
        iph_validFifoOut.write(tmp_valid);
    }
}

/**
 *  @ingroup ip_handler
 *
 *  Reads a packet and its valid flag in, if the packet is valid it is forwarded,
 *  otherwise it is dropped.
 *
 *  @param[in]		inData          incoming data stream
 *  @param[in]		ipValidFifoIn   FIFO containing valid flag to indicate if
 *                                  packet is valid
 *  @param[out]		outData         outgoing data stream
 *  @param[out]		stats_ipInHdrErrors errored packet count
 *  @param[out]		stats_ipInAddrErrors number of packets discarded cause of IP address not valid
 *  @param[out]		stats_ipInReceives total number of input datagrams including those received in error.
 */
void ip_invalid_dropper(hls::stream<axiWord>& dataIn,
                        hls::stream<packet_valid>& ipValidFifoIn,
                        hls::stream<axiWord>& dataOut,
                        uint32_t& stats_ipInHdrErrors,
                        uint32_t& stats_ipInAddrErrors,
                        uint32_t& stats_ipInReceives) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    enum iid_StateType { GET_VALID, FWD, DROP };
    static iid_StateType iid_state = GET_VALID;
    static uint32_t cnt_ipInHdrErrors = 0;
    static uint32_t cnt_ipInAddrErrors = 0;
    static uint32_t cnt_ipInReceives = 0;

    axiWord currWord;
    packet_valid valid;

    switch (iid_state) {
        case GET_VALID: // Drop1
            if (!ipValidFifoIn.empty()) {
                ipValidFifoIn.read(valid);
                cnt_ipInReceives++;
                if (!valid.hdrValid) {
                    cnt_ipInHdrErrors++;
                } else {
                    if (!valid.ipMatch) {
                        cnt_ipInAddrErrors++;
                    }
                }
                if (valid.hdrValid && valid.ipMatch) {
                    iid_state = FWD;
                } else {
                    iid_state = DROP;
                }
            }
            break;
        case FWD:
            if (!dataIn.empty()) {
                dataIn.read(currWord);
                dataOut.write(currWord);
                if (currWord.last) {
                    iid_state = GET_VALID;
                }
            }
            break;
        case DROP:
            if (!dataIn.empty()) {
                dataIn.read(currWord);
                if (currWord.last) {
                    iid_state = GET_VALID;
                }
            }
            break;
    } // switch
    stats_ipInHdrErrors = cnt_ipInHdrErrors;
    stats_ipInAddrErrors = cnt_ipInAddrErrors;
    stats_ipInReceives = cnt_ipInReceives;
}

/**
 * @ingroup ip_handler
 *
 * Trim an IP packet to its declared length
 *
 * @param [in]  dataIn   input packet stream
 * @param [out] dataOut  output packet stream
 */
void cut_length(hls::stream<axiWord>& dataIn, hls::stream<axiWord>& dataOut) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    enum cl_stateType { PKG, DROP };
    static cl_stateType cl_state = PKG;
    static ap_uint<16> cl_wordCount = 0;
    static ap_uint<16> cl_totalLength = 0;
    // static bool cl_drop = false;

    axiWord currWord;
    ap_uint<4> leftLength = 0;

    switch (cl_state) {
        case PKG:
            if (!dataIn.empty()) {
                dataIn.read(currWord);
                switch (cl_wordCount) {
                    case 0:
                        cl_totalLength(7, 0) = currWord.data(31, 24);
                        cl_totalLength(15, 8) = currWord.data(23, 16);
                        break;
                    default:
                        if (((cl_wordCount + 1) * 8) >= cl_totalLength) // last real world
                        {
                            if (currWord.last == 0) {
                                cl_state = DROP;
                            }
                            currWord.last = 1;
                            leftLength = cl_totalLength - (cl_wordCount * 8);
                            currWord.keep = lenToKeep(leftLength);
                        }
                        break;
                }
                dataOut.write(currWord);
                cl_wordCount++;
                if (currWord.last) {
                    cl_wordCount = 0;
                }
            } // empty
            break;
        case DROP:
            if (!dataIn.empty()) {
                dataIn.read(currWord);
                if (currWord.last) {
                    cl_state = PKG;
                }
            }
            break;
    } // switch
}

/** @ingroup ip_handler
 *
 *  Detects the IP protocol in the packet. ICMP, IGMP, UDP and TCP packets are forwarded,
 *  packets of other IP protocols are discarded. ip_handler_cfg determines if UDP / TCP
 *  packets are handled and forwarded.
 *
 *  @param[in]		dataIn       incoming data stream
 *  @param[out]		ICMPdataOut  outgoing ICMP (Ping) data stream
 *  @param[out]		UDPdataOut   outgoing UDP data stream
 *  @param[out]		TCPdataOut   outgoing TCP data stream
 *  @param[out]     stats_ipInDelivers stats - ipInDelivers
 *  @param[out]     stats_ipInUnknownProtos stats - Unknown IP Protocol
 */
template <ip_handler_cfg cfg>
void detect_ipv4_protocol(hls::stream<axiWord>& dataIn,
                          hls::stream<ap_axiu<64,0,0,0> >& ICMPdataOut,
                          hls::stream<ap_axiu<64,0,0,0> >& IGMPdataOut,
#ifndef UDP_ONLY                          
                          hls::stream<ap_axiu<64,0,0,0> >& TCPdataOut,
#endif                          
                          uint32_t& stats_ipInDelivers,
                          uint32_t& stats_ipInUnknownProtos)
{
#pragma HLS inline off
#pragma HLS pipeline II = 1

    enum dip_stateType { PKG, LEFTOVER };
    static dip_stateType dip_state = PKG;
    static ap_uint<8> dip_ipProtocol;
    static ap_uint<2> dip_wordCount = 0;
    static ap_axiu<64,0,0,0> dip_prevWord;
    static uint32_t cnt_ipInDelivers = 0;
    static uint32_t cnt_ipInUnknownProtos = 0;

    axiWord currWord;

    switch (dip_state) {
        case PKG:
            if (!dataIn.empty()) {
                dataIn.read(currWord);
                switch (dip_wordCount) {
                    case 0:
                        dip_wordCount++;
                        break;
                    default:
                        if (dip_wordCount == 1) {
                            dip_ipProtocol = currWord.data(15, 8);
                            if (isHandledProtocol<cfg>(dip_ipProtocol)) {
                                cnt_ipInDelivers++;
                            } else {
                                cnt_ipInUnknownProtos++;
                            }
                            dip_wordCount++;
                        }
                        // There is no default, if package does not match any case it is automatically dropped
                        switch (dip_ipProtocol) {
                            case ICMP:
                                ICMPdataOut.write(dip_prevWord);
                                break;
                            case IGMP:
                                IGMPdataOut.write(dip_prevWord);
                                break;
                            case TCP:                            
                                if (isHandledProtocol<cfg>(dip_ipProtocol)) {
#ifndef UDP_ONLY                                    
                                    TCPdataOut.write(dip_prevWord);
#endif                                    
                                }
                                break;
                        }
                        break;
                }
                dip_prevWord.data = currWord.data;
                dip_prevWord.keep = currWord.keep;
                dip_prevWord.last = currWord.last;
                if (currWord.last) {
                    dip_wordCount = 0;
                    dip_state = LEFTOVER;
                }
            }
            break;
        case LEFTOVER:
            switch (dip_ipProtocol) {
                case ICMP:
                    ICMPdataOut.write(dip_prevWord);
                    break;
                case IGMP:
                    IGMPdataOut.write(dip_prevWord);
                    break;
                case TCP:
                    if (isHandledProtocol<cfg>(dip_ipProtocol)) {
#ifndef UDP_ONLY                                                        
                        TCPdataOut.write(dip_prevWord);
#endif                        
                    }
                    break;
            }
            dip_state = PKG;
            break;
    } // switch
    stats_ipInDelivers = cnt_ipInDelivers;
    stats_ipInUnknownProtos = cnt_ipInUnknownProtos;
}

/** @ingroup ip_handler
 *
 *  Main function for the IP Handler Module. Accepts raw stream from
 *  the network interface, and dispatches packets to the respective protocol
 *  blocks. Configuration supplied via template parameter. CONFIG_UDP / CONFIG_TCP
 *  CONFIG_TCP_UDP
 *
 *  @param[in]		s_axis_raw     incoming data stream
 *  @param[in]		myIpAddress    our IP address
 *  @param[out]		m_axis_ARP     outgoing ARP data stream
 *  @param[out]		m_axis_ICMP    outgoing ICMP (Ping) data stream
 *  @param[out]		m_axis_IGMP    outgoing IGMP data stream
 *  @param[out]		m_axis_UDP     outgoing UDP data stream
 *  @param[out]		m_axis_TCP     outgoing TCP data stream
 *  @param[out]		stats          Statistics
 */
template <ip_handler_cfg cfg>
void ip_handler_body(hls::stream<ap_axiu<64,0,0,0> >& s_axis_raw,
                     hls::stream<ap_axiu<64,0,0,0> >& m_axis_ARP,
                     hls::stream<ap_axiu<64,0,0,0> >& m_axis_ICMP,
                     hls::stream<ap_axiu<64,0,0,0> >& m_axis_IGMP,                
#ifndef UDP_ONLY 
                     hls::stream<ap_axiu<64,0,0,0> >& m_axis_TCP,
#endif                     
                     ap_uint<32> myIpAddress,
                     uint32_t& ipInHdrErrors,
                     uint32_t& ipInDelivers,
                     uint32_t& ipInUnknownProtos,
                     uint32_t& ipInAddrErrors,
                     uint32_t& ipInReceives) {
    static hls::stream<ap_uint<16> > etherTypeFifo("etherTypeFifo");
    static hls::stream<axiWord> ethDataFifo("ethDataFifo");
    static hls::stream<axiWord> ipv4ShiftFifo("ipv4ShiftFifo");

    static hls::stream<axiWord> ipDataFifo("ipDataFifo");
    static hls::stream<axiWord> ipDataCheckFifo("ipDataCheckFifo");
    static hls::stream<axiWord> ipDataDropFifo("ipDataDropFifo");
    static hls::stream<axiWord> ipDataCutFifo("ipDataCutFifo");
    static hls::stream<subSums> iph_subSumsFifoOut("iph_subSumsFifoOut");
    static hls::stream<packet_valid> ipValidFifo("ipValidFifo");
    #pragma HLS INLINE


    #pragma HLS STREAM variable = etherTypeFifo depth = 2
    #pragma HLS STREAM variable = ethDataFifo depth = 4
    #pragma HLS STREAM variable = ipv4ShiftFifo depth = 4
    #pragma HLS STREAM variable = ipDataFifo depth = 4
    #pragma HLS STREAM variable = ipDataCheckFifo depth = 64 // 8, must hold IP header for checksum checking, max. 15 x
                                                         // 32bit
    #pragma HLS STREAM variable = ipDataDropFifo depth = 2
    #pragma HLS STREAM variable = ipDataCutFifo depth = 2
    #pragma HLS STREAM variable = iph_subSumsFifoOut depth = 2
    #pragma HLS STREAM variable = ipValidFifo depth = 2

    detect_eth_protocol(s_axis_raw, etherTypeFifo, ethDataFifo);

    route_by_eth_protocol(etherTypeFifo, ethDataFifo, m_axis_ARP, ipv4ShiftFifo);

    rshiftWordByOctet<AXI_WIDTH,axiWord, axiWord, 1>(((ETH_HEADER_SIZE % AXI_WIDTH) / 8), ipv4ShiftFifo, ipDataFifo);

    check_ip_checksum(ipDataFifo, myIpAddress, ipDataCheckFifo, iph_subSumsFifoOut);

    iph_check_ip_checksum(iph_subSumsFifoOut, ipValidFifo);

    ip_invalid_dropper(ipDataCheckFifo, ipValidFifo, ipDataDropFifo, ipInHdrErrors, ipInAddrErrors,
    		ipInReceives);

    cut_length(ipDataDropFifo, ipDataCutFifo);

    detect_ipv4_protocol<cfg>(ipDataCutFifo, m_axis_ICMP, m_axis_IGMP, 
    #ifndef UDP_ONLY 
    m_axis_TCP, 
    #endif
	ipInDelivers,
	ipInUnknownProtos);
};

/** @ingroup ip_handler
 *
 *  Top level function for the IP Handler Module. Accepts raw stream from
 *  the network interface, and dispatches packets to the respective protocol
 *  blocks.
 *
 *  @param[in]		s_axis_raw     incoming data stream
 *  @param[in]		myIpAddress    our IP address
 *  @param[out]		m_axis_ARP     outgoing ARP data stream
 *  @param[out]		m_axis_ICMP    outgoing ICMP (Ping) data stream
 *  @param[out]		m_axis_IGMP    outgoing IGMP data stream
 *  @param[out]		m_axis_UDP     outgoing UDP data stream
 *  @param[out]		m_axis_TCP     outgoing TCP data stream
 *  @param[out]		stats          Statistics
 */
#ifdef TCP_UDP
void ip_handler_tcp_udp(hls::stream<ap_axiu<64,0,0,0> >& s_axis_raw,
                        hls::stream<ap_axiu<64,0,0,0> >& m_axis_ARP,
                        hls::stream<ap_axiu<64,0,0,0> >& m_axis_ICMP,
                        hls::stream<ap_axiu<64,0,0,0> >& m_axis_IGMP,
                        hls::stream<ap_axiu<64,0,0,0> >& m_axis_TCP,
                        ap_uint<32> myIpAddress,
                        uint32_t& ipInHdrErrors,
                        uint32_t& ipInDelivers,
                        uint32_t& ipInUnknownProtos,
                        uint32_t& ipInAddrErrors,
                        uint32_t& ipInReceives) {
    #pragma HLS DATAFLOW disable_start_propagation
    #pragma HLS INTERFACE ap_ctrl_none port = return
    #pragma HLS INTERFACE axis register port = s_axis_raw
    #pragma HLS INTERFACE axis register port = m_axis_ARP
    #pragma HLS INTERFACE axis register port = m_axis_ICMP
    #pragma HLS INTERFACE axis register port = m_axis_IGMP
    #pragma HLS INTERFACE axis register port = m_axis_TCP
    #pragma HLS INTERFACE ap_stable port = myIpAddress
    #pragma HLS INTERFACE s_axilite port = ipInHdrErrors bundle = control
    #pragma HLS INTERFACE s_axilite port = ipInDelivers bundle = control
    #pragma HLS INTERFACE s_axilite port = ipInUnknownProtos bundle = control
    #pragma HLS INTERFACE s_axilite port = ipInAddrErrors bundle = control
    #pragma HLS INTERFACE s_axilite port = ipInReceives bundle = control

    ip_handler_body<CONFIG_TCP_UDP>(s_axis_raw, m_axis_ARP, m_axis_ICMP, m_axis_IGMP, m_axis_TCP,
                                    myIpAddress, ipInHdrErrors, ipInDelivers, ipInUnknownProtos, ipInAddrErrors, ipInReceives);
}
#endif

/** @ingroup ip_handler_udp
 *
 *  Top level function for the UDP IP Handler Module. Accepts raw stream from
 *  the network interface, and dispatches packets to the respective protocol
 *  blocks.
 *
 *  @param[in]		s_axis_raw     incoming data stream
 *  @param[in]		myIpAddress    our IP address
 *  @param[out]		m_axis_ARP     outgoing ARP data stream
 *  @param[out]		m_axis_ICMP    outgoing ICMP (Ping) data stream
 *  @param[out]		m_axis_IGMP    outgoing IGMP data stream
 *  @param[out]		m_axis_UDP     outgoing UDP data stream
 *  @param[out]		stats          Statistics
 */
#ifdef UDP_ONLY
void ip_handler_udp(hls::stream<ap_axiu<64,0,0,0> >& s_axis_raw,
                    hls::stream<ap_axiu<64,0,0,0> >& m_axis_ARP,
                    hls::stream<ap_axiu<64,0,0,0> >& m_axis_ICMP,
                    hls::stream<ap_axiu<64,0,0,0> >& m_axis_IGMP,
                    ap_uint<32> myIpAddress,
                    uint32_t& ipInHdrErrors,
                    uint32_t& ipInDelivers,
                    uint32_t& ipInUnknownProtos,
                    uint32_t& ipInAddrErrors,
                    uint32_t& ipInReceives) {
    #pragma HLS DATAFLOW disable_start_propagation
    #pragma HLS INTERFACE ap_ctrl_none port = return
    #pragma HLS INTERFACE axis register port = s_axis_raw
    #pragma HLS INTERFACE axis register port = m_axis_ARP
    #pragma HLS INTERFACE axis register port = m_axis_ICMP
    #pragma HLS INTERFACE axis register port = m_axis_IGMP
    #pragma HLS INTERFACE ap_stable port = myIpAddress
    #pragma HLS INTERFACE s_axilite port = ipInHdrErrors bundle = control
    #pragma HLS INTERFACE s_axilite port = ipInDelivers bundle = control
    #pragma HLS INTERFACE s_axilite port = ipInUnknownProtos bundle = control
    #pragma HLS INTERFACE s_axilite port = ipInAddrErrors bundle = control
    #pragma HLS INTERFACE s_axilite port = ipInReceives bundle = control

    ip_handler_body<CONFIG_UDP>(s_axis_raw, m_axis_ARP, m_axis_ICMP, m_axis_IGMP, myIpAddress,
                                 ipInHdrErrors, ipInDelivers, ipInUnknownProtos, ipInAddrErrors, ipInReceives);
}
#endif

/** @ingroup ip_handler_tcp
 *
 *  Top level function for the TCP IP Handler Module. Accepts raw stream from
 *  the network interface, and dispatches packets to the respective protocol
 *  blocks.
 *
 *  @param[in]		s_axis_raw     incoming data stream
 *  @param[in]		myIpAddress    our IP address
 *  @param[out]		m_axis_ARP     outgoing ARP data stream
 *  @param[out]		m_axis_ICMP    outgoing ICMP (Ping) data stream
 *  @param[out]		m_axis_IGMP    outgoing IGMP data stream
 *  @param[out]		m_axis_TCP     outgoing TCP data stream
 *  @param[out]		stats          Statistics
 */
#ifdef TCP_ONLY
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
                    uint32_t& ipInReceives) {
    #pragma HLS DATAFLOW disable_start_propagation
    #pragma HLS INTERFACE ap_ctrl_none port = return
    #pragma HLS INTERFACE axis register port = s_axis_raw
    #pragma HLS INTERFACE axis register port = m_axis_ARP
    #pragma HLS INTERFACE axis register port = m_axis_ICMP
    #pragma HLS INTERFACE axis register port = m_axis_IGMP
    #pragma HLS INTERFACE axis register port = m_axis_TCP
    #pragma HLS INTERFACE ap_stable port = myIpAddress
    #pragma HLS INTERFACE s_axilite port = ipInHdrErrors bundle = control
    #pragma HLS INTERFACE s_axilite port = ipInDelivers bundle = control
    #pragma HLS INTERFACE s_axilite port = ipInUnknownProtos bundle = control
    #pragma HLS INTERFACE s_axilite port = ipInAddrErrors bundle = control
    #pragma HLS INTERFACE s_axilite port = ipInReceives bundle = control

    ip_handler_body<CONFIG_TCP>(s_axis_raw, m_axis_ARP, m_axis_ICMP, m_axis_IGMP, m_axis_TCP, myIpAddress,
                                 ipInHdrErrors, ipInDelivers, ipInUnknownProtos, ipInAddrErrors, ipInReceives);
}
#endif
