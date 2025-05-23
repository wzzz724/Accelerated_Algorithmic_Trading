/************************************************
Copyright (c) 2016, 2019 Xilinx, Inc.
All rights reserved.

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

#include "mac_ip_encode.hpp"
#include "../ethernet/ethernet.hpp"
#include "../ipv4/ipv4.hpp"

/**
 * @ingroup mac_ip_encode
 *
 * Detect a multicast IP address. (Network Order)
 *
 * @param[in] addr IP address
 * @return         true if addr is a multicast IP address
 */
bool isMulticastAddr(ap_uint<32> addr) {
    return (addr(7, 4) == 0xE);
}

/** @ingroup mac_ip_encode
 * Extract the IP address fields and send to ARP Table. (ARP Table request is in Network Order (BE)) 
 */
void extract_ip_address(hls::stream<ap_axiu<64,0,0,0> >& dataIn,
                        hls::stream<axiWord>& dataOut,
                        hls::stream<ap_uint<32> >& arpTableOut,
                        ap_uint<32> regSubNetMask,
                        ap_uint<32> regDefaultGateway) {
#pragma HLS PIPELINE II = 1

    static ipv4Header<AXI_WIDTH> header;
    static bool metaWritten = false;
    axiWord currWord;

    if (!dataIn.empty()) {
        ap_axiu<64,0,0,0> inWord = dataIn.read();
        currWord.data = inWord.data;
        currWord.keep = inWord.keep;
        currWord.last = inWord.last;
        header.parseWord(currWord.data);
        dataOut.write(currWord);

        if (header.isReady() && !metaWritten) {
            // Covert to BE as that is the format of Subnet and Gateway
            ap_uint<32> dstIpAddress = reverse(header.getDstAddr());
            if (isMulticastAddr(dstIpAddress) ||
                ((dstIpAddress & regSubNetMask) == (regDefaultGateway & regSubNetMask))) {
                arpTableOut.write(dstIpAddress);
            } else {
                arpTableOut.write(regDefaultGateway);
            }
            metaWritten = true;
        }

        if (currWord.last) {
            metaWritten = false;
            header.clear();
        }
    }
}

/** @ingroup mac_ip_encode
 *
 */
template <int WIDTH>
void insert_ip_checksum(hls::stream<axiWord>& dataIn,
                        hls::stream<ap_uint<16> >& checksumFifoIn,
                        hls::stream<axiWord>& dataOut) {
#pragma HLS PIPELINE II = 1

    static ap_uint<4> wordCount = 0;
    static ap_uint<16> checksum;

    switch (wordCount) {
        case 0:
            if (!dataIn.empty() && !checksumFifoIn.empty()) {
                axiWord currWord = dataIn.read();
                checksumFifoIn.read(checksum);
                if (WIDTH > 64) {
                    currWord.data(95, 80) = reverse(checksum);
                }
                dataOut.write(currWord);
                wordCount++;
                if (currWord.last) {
                    wordCount = 0;
                }
            }
            break;
        case 1:
            if (!dataIn.empty()) {
                axiWord currWord = dataIn.read();
                if (WIDTH == 64) {
                    currWord.data(31, 16) = reverse(checksum);
                }
                dataOut.write(currWord);
                wordCount++;
                if (currWord.last) {
                    wordCount = 0;
                }
            }
            break;
        default:
            if (!dataIn.empty()) {
                axiWord currWord = dataIn.read();
                dataOut.write(currWord);
                if (currWord.last) {
                    wordCount = 0;
                }
            }
            break;
    }
}

template <int WIDTH>
void handle_arp_reply(hls::stream<arpTableReply>& arpTableIn,
                      hls::stream<axiWord>& dataIn,
                      hls::stream<ethHeader<WIDTH> >& headerOut,
                      hls::stream<axiWord>& dataOut,
                      ap_uint<48> myMacAddress,
                      uint32_t& droppedPkts)

{
#pragma HLS PIPELINE II = 1

    enum fsmStateType { ARP, FWD, DROP };
    static fsmStateType har_state = ARP;
    static uint32_t dropPktCnt = 0;

    switch (har_state) {
        case ARP:
            if (!arpTableIn.empty()) {
                arpTableReply reply = arpTableIn.read();
                if (reply.hit) {
                    // Construct Header
                    ethHeader<WIDTH> header;
                    header.clear();
                    header.setDstAddress(reply.macAddress);
                    header.setSrcAddress(myMacAddress);
                    header.setEthertype(0x0800);
                    headerOut.write(header);
                    har_state = FWD;
                } else {
                    dropPktCnt++;
                    har_state = DROP;
                }
            }
            droppedPkts = dropPktCnt;            
            break;
        case FWD:
            if (!dataIn.empty()) {
                axiWord word = dataIn.read();
                dataOut.write(word);
                if (word.last) {
                    har_state = ARP;
                }
            }
            break;
        case DROP:
            if (!dataIn.empty()) {
                axiWord word = dataIn.read();
                if (word.last) {
                    har_state = ARP;
                }
            }
            break;
    }
}

/** @ingroup mac_ip_encode
 *
 */
template <int WIDTH>
void insert_ethernet_header(hls::stream<ethHeader<WIDTH> >& headerIn,
                            hls::stream<axiWord>& dataIn,
                            hls::stream<ap_axiu<64,0,0,0> >& dataOut,
                            uint32_t& stats_ipv4_packets_sent) {
#pragma HLS PIPELINE II = 1

    enum fsmStateType { HEADER, PARTIAL_HEADER, BODY };
    static fsmStateType ge_state = (ETH_HEADER_SIZE >= WIDTH) ? HEADER : PARTIAL_HEADER;
    static ethHeader<WIDTH> header;
    static uint32_t packets_sent = 0;
    ap_axiu<WIDTH,0,0,0> outWord;

    switch (ge_state) {
        case HEADER: {
            if (!headerIn.empty()) // This works because for 64bit there is only one full header word
            {
                headerIn.read(header);
                axiWord currWord;
                // Always holds, no check required
                header.consumeWord(currWord.data);
                ge_state = PARTIAL_HEADER;
                currWord.keep = ~0;
                currWord.last = 0;
                outWord.data = currWord.data;
                outWord.keep = currWord.keep;
                outWord.last = currWord.last;
                dataOut.write(outWord);
            }
            break;
        }
        case PARTIAL_HEADER:
            if ((!headerIn.empty() || (ETH_HEADER_SIZE >= WIDTH)) && !dataIn.empty()) {
                if (ETH_HEADER_SIZE < WIDTH) {
                    headerIn.read(header);
                }
                axiWord currWord = dataIn.read();
                header.consumeWord(currWord.data);
                outWord.data = currWord.data;
                outWord.keep = currWord.keep;
                outWord.last = currWord.last;                
                dataOut.write(outWord);

                if (!currWord.last) {
                    ge_state = BODY;
                } else {
                    ge_state = (ETH_HEADER_SIZE >= WIDTH) ? HEADER : PARTIAL_HEADER;
                }
            }
            break;
        case BODY:
            if (!dataIn.empty()) {
                axiWord currWord = dataIn.read();
                outWord.data = currWord.data;
                outWord.keep = currWord.keep;
                outWord.last = currWord.last;                
                dataOut.write(outWord);
                if (currWord.last) {
                    ge_state = (ETH_HEADER_SIZE >= WIDTH) ? HEADER : PARTIAL_HEADER;
                    packets_sent++;
                }
            }
            break;
    } // switch
    stats_ipv4_packets_sent = packets_sent;
}

void send_arp(hls::stream<ap_uint<32> >& dataIn,
              hls::stream<ap_uint<32> >& dataOut) {
#pragma HLS PIPELINE II = 1    
    dataOut.write(dataIn.read());
}

/** @ingroup mac_ip_encode
 *
 */
template <int WIDTH>
void mac_ip_encode_body(hls::stream<ap_axiu<WIDTH,0,0,0> >& dataIn,
                        hls::stream<arpTableReply>& arpTableIn,
                        hls::stream<ap_axiu<WIDTH,0,0,0> >& dataOut,
                        hls::stream<ap_uint<32> >& arpTableOut,
                        ap_uint<48> myMacAddress,
                        ap_uint<32> regSubNetMask,
                        ap_uint<32> regDefaultGateway,
                        uint32_t &ipv4_packets_sent,
                        uint32_t &packets_dropped) {
#pragma HLS INLINE
    // FIFOs
    static hls::stream<axiWord> dataStreamBuffer0("dataStreamBuffer0");
    static hls::stream<axiWord> dataStreamBuffer1("dataStreamBuffer1");
    static hls::stream<axiWord> dataStreamBuffer2("dataStreamBuffer2");
    static hls::stream<axiWord> dataStreamBuffer3("dataStreamBuffer3");
    static hls::stream<axiWord> dataStreamBuffer4("dataStreamBuffer4");
    static hls::stream<ap_uint<32> > ipaddr("ipaddr");

#pragma HLS stream variable = dataStreamBuffer0 depth = 4
#pragma HLS stream variable = dataStreamBuffer1 depth = 32
#pragma HLS stream variable = dataStreamBuffer2 depth = 4
#pragma HLS stream variable = dataStreamBuffer3 depth = 4
#pragma HLS stream variable = dataStreamBuffer4 depth = 4
#pragma HLS stream variable = ipaddr depth = 2

    static hls::stream<subSums<WIDTH / 16> > subSumFifo("subSumFifo");
    static hls::stream<ap_uint<16> > checksumFifo("checksumFifo");
    static hls::stream<ethHeader<WIDTH> > headerFifo("headerFifo");
#pragma HLS stream variable = subSumFifo depth = 2
#pragma HLS stream variable = checksumFifo depth = 16
#pragma HLS stream variable = headerFifo depth = 3

    extract_ip_address(dataIn, dataStreamBuffer0, ipaddr, regSubNetMask, regDefaultGateway);
    // Copy the IP address to intermediata FIFO to avoid the Vitis synchronization warning of non-PIPO output
    // because extract_ip_address writes to both an internal fifo and an external port.
    send_arp(ipaddr,arpTableOut);
    compute_ipv4_checksum(dataStreamBuffer0, dataStreamBuffer1, subSumFifo, true);
    finalize_ipv4_checksum<WIDTH / 16>(subSumFifo, checksumFifo);
    insert_ip_checksum<WIDTH>(dataStreamBuffer1, checksumFifo, dataStreamBuffer2);
    handle_arp_reply<WIDTH>(arpTableIn, dataStreamBuffer2, headerFifo, dataStreamBuffer3, myMacAddress,
    		                packets_dropped);
    lshiftWordByOctet<WIDTH,axiWord, axiWord, 1>(((ETH_HEADER_SIZE % WIDTH) / 8), dataStreamBuffer3, dataStreamBuffer4);
    insert_ethernet_header<WIDTH>(headerFifo, dataStreamBuffer4, dataOut, ipv4_packets_sent);
}

/** @ingroup mac_ip_encode
 *
 */
void mac_ip_encode(hls::stream<ap_axiu<64,0,0,0> >& dataIn,
                   hls::stream<arpTableReply>& arpTableIn,
                   hls::stream<ap_axiu<64,0,0,0> >& dataOut,
                   hls::stream<ap_uint<32> >& arpTableOut,
                   ap_uint<48> myMacAddress,
                   ap_uint<32> regSubNetMask,
                   ap_uint<32> regDefaultGateway,
                   uint32_t &ipv4_packets_sent,
                   uint32_t &packets_dropped) {
#pragma HLS DATAFLOW disable_start_propagation
#pragma HLS INTERFACE ap_ctrl_none port = return

#pragma HLS INTERFACE axis register port = dataIn name = s_axis_ip
#pragma HLS INTERFACE axis register port = dataOut name = m_axis_ip

#pragma HLS INTERFACE axis register port = arpTableIn name = s_axis_arp_lookup_reply
#pragma HLS INTERFACE axis register port = arpTableOut name = m_axis_arp_lookup_request

#pragma HLS INTERFACE ap_stable register port = myMacAddress
#pragma HLS INTERFACE ap_stable register port = regSubNetMask
#pragma HLS INTERFACE ap_stable register port = regDefaultGateway

#pragma HLS INTERFACE s_axilite port = ipv4_packets_sent bundle = control
#pragma HLS INTERFACE s_axilite port = packets_dropped bundle = control

    mac_ip_encode_body<AXI_WIDTH>(dataIn, arpTableIn, dataOut, arpTableOut, myMacAddress, regSubNetMask,
                                  regDefaultGateway, ipv4_packets_sent, packets_dropped);
}
