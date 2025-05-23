/************************************************
Copyright (c) 2016,2019-2020 Xilinx, Inc.
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

#include "arp_server_subnet.hpp"

/**
 * @ingroup arp_server
 *
 * Detect a multicast IP address.
 *
 * @param[in] addr IP address
 * @return         true if addr is a multicast IP address
 */
bool isMulticastAddr(ap_uint<32> addr) {
    return (addr(7, 4) == 0xE);
}

/** @ingroup arp_server
 *
 */
void arp_pkg_receiver(hls::stream<ap_axiu<64,0,0,0> >& arpDataIn,
                      hls::stream<arpReplyMeta>& arpReplyMetaFifo,
                      hls::stream<ap_uint<32> >& arpReplyRecvFifo,
                      hls::stream<arpTableEntry>& arpTableInsertFifo,
                      ap_uint<32> myIpAddress,
                      hls::stream<ap_uint<1> >& reqRecvCnt,
                      hls::stream<ap_uint<1> >& replyRecvCnt) {
    #pragma HLS PIPELINE II = 1
    static ap_uint<4> wordCount = 0;
    static ap_uint<16> opCode;
    static ap_uint<32> protoAddrDst;
    static ap_uint<32> inputIP;
    static arpReplyMeta meta;

    ap_axiu<64,0,0,0> currWord;

    currWord.last = 0; // probably not necessary
    if (!arpDataIn.empty()) {
        arpDataIn.read(currWord);

        switch (wordCount) {
            // TODO DO MAC ADDRESS Filtering somewhere, should be done in/after mac
            case 0:
                // MAC_DST = currWord.data(47, 0);
                meta.srcMac(15, 0) = currWord.data(63, 48);
                wordCount++;
                break;
            case 1:
                meta.srcMac(47, 16) = currWord.data(31, 0);
                meta.ethType = currWord.data(47, 32);
                meta.hwType = currWord.data(63, 48);
                wordCount++;
                break;
            case 2:
                meta.protoType = currWord.data(15, 0);
                meta.hwLen = currWord.data(23, 16);
                meta.protoLen = currWord.data(31, 24);
                opCode = currWord.data(47, 32);
                meta.hwAddrSrc(15, 0) = currWord.data(63, 48);
                wordCount++;
                break;
            case 3:
                meta.hwAddrSrc(47, 16) = currWord.data(31, 0);
                meta.protoAddrSrc = currWord.data(63, 32);
                wordCount++;
                break;
            case 4:
                // hwAddrDst = currWord.data(47, 0);
                protoAddrDst(15, 0) = currWord.data(63, 48);
                wordCount++;
                break;
            case 5:
                protoAddrDst(31, 16) = currWord.data(15, 0);
                wordCount++;
                break;
            default:
                break;
        } // switch
        if (currWord.last == 1) {
            if (protoAddrDst == myIpAddress || meta.protoAddrSrc == protoAddrDst) {
                if (meta.protoAddrSrc != 0x00000000) {
                    // Dont add ARP probes to the ARP table.
                    arpTableInsertFifo.write(arpTableEntry(meta.protoAddrSrc, meta.hwAddrSrc, true));
                    arpReplyRecvFifo.write(meta.protoAddrSrc);
                }
                if (opCode == REPLY) {
                    replyRecvCnt.write(1);
                }
            }
            if (opCode == REQUEST && protoAddrDst == myIpAddress) {
                // Trigger ARP reply
                reqRecvCnt.write(1);
                arpReplyMetaFifo.write(meta);
            }            
            wordCount = 0;
        }
    }
}

/** @ingroup arp_server
 *
 */
void arp_pkg_sender(hls::stream<arpReplyMeta>& arpReplyMetaFifo,
                    hls::stream<ap_uint<32> >& arpRequestMetaFifo,
                    hls::stream<ap_axiu<64,0,0,0> >& arpDataOut,
                    ap_uint<48> myMacAddress,
                    ap_uint<32> myIpAddress,
                    hls::stream<ap_uint<1> >& req_sent,
                    hls::stream<ap_uint<1> >& reply_sent) {
    #pragma HLS PIPELINE II = 1
    enum arpSendStateType { ARP_IDLE, ARP_REPLY, ARP_SENTRQ };
    static arpSendStateType aps_fsmState = ARP_IDLE;

    static uint16_t sendCount = 0;
    static arpReplyMeta replyMeta;
    static ap_uint<32> inputIP;

    ap_axiu<64,0,0,0> sendWord;

    switch (aps_fsmState) {
        case ARP_IDLE:
            sendCount = 0;
            if (!arpReplyMetaFifo.empty()) {
                arpReplyMetaFifo.read(replyMeta);
                reply_sent.write(1);
                aps_fsmState = ARP_REPLY;
            } else if (!arpRequestMetaFifo.empty()) {
                arpRequestMetaFifo.read(inputIP);
                req_sent.write(1);
                aps_fsmState = ARP_SENTRQ;
            }
            break;
        case ARP_SENTRQ:
            switch (sendCount) {
                case 0:
                    sendWord.data(47, 0) = BROADCAST_MAC;
                    sendWord.data(63, 48) = myMacAddress(15, 0);
                    sendWord.keep = 0xff;
                    sendWord.last = 0;
                    break;
                case 1:
                    sendWord.data(31, 0) = myMacAddress(47, 16);
                    sendWord.data(47, 32) = 0x0608;
                    sendWord.data(63, 48) = 0x0100;
                    sendWord.keep = 0xff;
                    sendWord.last = 0;
                    break;
                case 2:
                    sendWord.data(15, 0) = 0x0008; // IP Address
                    sendWord.data(23, 16) = 6;     // HW Address Length
                    sendWord.data(31, 24) = 4;     // Protocol Address Length
                    sendWord.data(47, 32) = REQUEST;
                    sendWord.data(63, 48) = myMacAddress(15, 0);
                    sendWord.keep = 0xff;
                    sendWord.last = 0;
                    break;
                case 3:
                    sendWord.data(31, 0) = myMacAddress(47, 16);
                    sendWord.data(63, 32) = myIpAddress; // MY_IP_ADDR;
                    sendWord.keep = 0xff;
                    sendWord.last = 0;
                    break;
                case 4:
                    sendWord.data(47, 0) = 0; // Sought-after MAC pt.1
                    sendWord.data(63, 48) = inputIP(15, 0);
                    sendWord.keep = 0xff;
                    sendWord.last = 0;
                    break;
                case 5:
                    sendWord.data(63, 16) = 0; // Sought-after MAC pt.1
                    sendWord.data(15, 0) = inputIP(31, 16);
                    sendWord.keep = 0x03;
                    sendWord.last = 1;
                    aps_fsmState = ARP_IDLE;
                    break;
            } // switch sendcount
            arpDataOut.write(sendWord);
            sendCount++;
            break;
        case ARP_REPLY:
            switch (sendCount) {
                case 0:
                    sendWord.data(47, 0) = replyMeta.srcMac;
                    sendWord.data(63, 48) = myMacAddress(15, 0);
                    sendWord.keep = 0xff;
                    sendWord.last = 0;
                    break;
                case 1:
                    sendWord.data(31, 0) = myMacAddress(47, 16);
                    sendWord.data(47, 32) = replyMeta.ethType;
                    sendWord.data(63, 48) = replyMeta.hwType;
                    sendWord.keep = 0xff;
                    sendWord.last = 0;
                    break;
                case 2:
                    sendWord.data(15, 0) = replyMeta.protoType;
                    sendWord.data(23, 16) = replyMeta.hwLen;
                    sendWord.data(31, 24) = replyMeta.protoLen;
                    sendWord.data(47, 32) = REPLY;
                    sendWord.data(63, 48) = myMacAddress(15, 0);
                    sendWord.keep = 0xff;
                    sendWord.last = 0;
                    break;
                case 3:
                    sendWord.data(31, 0) = myMacAddress(47, 16);
                    sendWord.data(63, 32) = myIpAddress; // MY_IP_ADDR, maybe use proto instead
                    sendWord.keep = 0xff;
                    sendWord.last = 0;
                    break;
                case 4:
                    sendWord.data(47, 0) = replyMeta.hwAddrSrc;
                    sendWord.data(63, 48) = replyMeta.protoAddrSrc(15, 0);
                    sendWord.keep = 0xff;
                    sendWord.last = 0;
                    break;
                case 5:
                    sendWord.data(63, 16) = 0;
                    sendWord.data(15, 0) = replyMeta.protoAddrSrc(31, 16);
                    sendWord.keep = 0x03;
                    sendWord.last = 1;
                    aps_fsmState = ARP_IDLE;
                    break;
            } // switch
            arpDataOut.write(sendWord);
            sendCount++;
            break;
    } // switch
}

/** @ingroup arp_server
 *
 */
void arp_table(hls::stream<arpTableEntry>& arpTableInsertFifo,
               hls::stream<ap_uint<32> >& macIpEncode_req,
               hls::stream<arpTableReply>& macIpEncode_rsp,
               hls::stream<ap_uint<32> >& arpRequestMetaFifo,
               arpTableEntry& arpEntry,
               bool host_op_toggle,
               ap_uint<2> host_opcode,
               ap_uint<8> entryBin) {
    #pragma HLS PIPELINE II = 1
    #define IP_ADD 1
    #define IP_DELETE 2
    #define IP_FIND 3

    static arpTableEntry arpTable[256];
    #pragma HLS RESOURCE variable = arpTable core = RAM_T2P_BRAM
    #pragma HLS DEPENDENCE variable = arpTable inter false

    static bool toggle = false;
    static ap_uint<32> ipAddr_network;
    static ap_uint<48> macAddr_network;
    ap_uint<32> query_ip;
    arpTableEntry currEntry;

    if (host_op_toggle != toggle) {
        toggle = host_op_toggle;
        switch (host_opcode) {
            case IP_ADD:
                // Converting addresses to network order
                ipAddr_network = reverse(arpEntry.ipAddress);
                macAddr_network = reverse(arpEntry.macAddress);
                arpTable[ipAddr_network(31, 24)] = arpTableEntry(ipAddr_network, macAddr_network, arpEntry.valid);
                break;

            case IP_DELETE:
                arpTable[entryBin] = arpTableEntry(0x0, 0x0, 0x0);
                break;

            case IP_FIND:
                // Converting addresses to match what host inputs
                arpEntry.ipAddress = reverse(arpTable[entryBin].ipAddress);
                arpEntry.macAddress = reverse(arpTable[entryBin].macAddress);
                arpEntry.valid = arpTable[entryBin].valid;
                break;
        }
    } else {
        if (!arpTableInsertFifo.empty()) {
            arpTableInsertFifo.read(currEntry);
            arpTable[currEntry.ipAddress(31, 24)] = currEntry;
        } else if (!macIpEncode_req.empty()) {
            macIpEncode_req.read(query_ip);
            if (isMulticastAddr(query_ip)) {
                currEntry.valid = 1;
                currEntry.macAddress(7, 0) = 0x01;
                currEntry.macAddress(15, 8) = 0x00;
                currEntry.macAddress(23, 16) = 0x5E;
                currEntry.macAddress(31, 24) = query_ip(15, 8) & 0x7F;
                currEntry.macAddress(39, 32) = query_ip(23, 16);
                currEntry.macAddress(47, 40) = query_ip(31, 24);
            } else {
                currEntry = arpTable[query_ip(31, 24)];
                if (!currEntry.valid) {
                    // send ARP request
                    arpRequestMetaFifo.write(query_ip);
                }
            }
            macIpEncode_rsp.write(arpTableReply(currEntry.macAddress, currEntry.valid));
        }
    }
}

void send_rsp(hls::stream<arpTableReply>& dataIn,
              hls::stream<arpTableReply>& dataOut) {
    if (!dataIn.empty()) {
        dataOut.write(dataIn.read());
    }
}

/** @ingroup arp_server
 * Increments stat counters based on flag FIFOs
 */
void arp_statCounters(hls::stream<ap_uint<1> >& stats_req_sent_in,
                      hls::stream<ap_uint<1> >& stats_replies_sent_in,
                      hls::stream<ap_uint<1> >& stats_req_recv_in,
                      hls::stream<ap_uint<1> >& stats_replies_recv_in,
                      hls::stream<ap_uint<1> >& stats_req_lost_in,
                      uint32_t &req_sent,
                      uint32_t &replies_sent,
                      uint32_t &req_recv,
                      uint32_t &replies_recv,
                      uint32_t &req_lost) {
#pragma HLS pipeline II = 1
#pragma HLS INLINE off

    static uint32_t cnt_reqSent = 0;
    static uint32_t cnt_repliesSent = 0;
    static uint32_t cnt_reqRecv = 0;
    static uint32_t cnt_repliesRecv = 0;
    static uint32_t cnt_reqLost = 0;

    if (!stats_req_sent_in.empty()) {
        stats_req_sent_in.read();
        cnt_reqSent++;
    }
    if (!stats_replies_sent_in.empty()) {
        stats_replies_sent_in.read();
        cnt_repliesSent++;
    }  
    if (!stats_req_recv_in.empty()) {
        stats_req_recv_in.read();
        cnt_reqRecv++;
    }
    if (!stats_replies_recv_in.empty()) {
        stats_replies_recv_in.read();
        cnt_repliesRecv++;
    }
    if (!stats_req_lost_in.empty()) {
        stats_req_lost_in.read();
        cnt_reqLost++;
    }  
    req_sent     = cnt_reqSent;
    replies_sent = cnt_repliesSent;
    req_recv     = cnt_reqRecv;
    replies_recv = cnt_repliesRecv;
    req_lost     = cnt_reqLost;

}

/** @ingroup arp_server
 *
 */
void arp_server_subnet(hls::stream<ap_axiu<64,0,0,0> >& arpDataIn,
                       hls::stream<ap_uint<32> >& macIpEncode_req,
                       hls::stream<ap_axiu<64,0,0,0> >& arpDataOut,
                       hls::stream<arpTableReply>& macIpEncode_rsp,
                       ap_uint<48> myMacAddress,
                       ap_uint<32> myIpAddress,
                       arpTableEntry& arpEntry,
                       bool host_op_toggle,
                       ap_uint<2> host_opcode,
                       ap_uint<8> entryBin,
                       uint32_t &req_sent,
                       uint32_t &replies_sent,
                       uint32_t &req_recv,
                       uint32_t &replies_recv,
                       uint32_t &req_lost) {
    #pragma HLS INTERFACE ap_ctrl_none port = return
    #pragma HLS DATAFLOW disable_start_propagation

    #pragma HLS INTERFACE axis register port = arpDataIn name = s_axis_arpDataIn
    #pragma HLS INTERFACE axis register port = arpDataOut name = m_axis_arpDataOut
    #pragma HLS INTERFACE axis register port = macIpEncode_req name = s_axis_arp_lookup_request
    #pragma HLS INTERFACE axis register port = macIpEncode_rsp name = m_axis_arp_lookup_reply

    #pragma HLS INTERFACE ap_stable register port = myMacAddress
    #pragma HLS INTERFACE ap_stable register port = myIpAddress

    #pragma HLS INTERFACE s_axilite bundle = control port = host_op_toggle offset=0x48
    #pragma HLS INTERFACE s_axilite bundle = control port = arpEntry offset=0x10
    #pragma HLS INTERFACE s_axilite bundle = control port = host_opcode offset=0x50
    #pragma HLS INTERFACE s_axilite bundle = control port = entryBin offset=0x58
    #pragma HLS INTERFACE s_axilite bundle = control port = req_sent offset = 0x60
    #pragma HLS INTERFACE s_axilite bundle = control port = replies_sent offset = 0x68
    #pragma HLS INTERFACE s_axilite bundle = control port = req_recv offset = 0x70
    #pragma HLS INTERFACE s_axilite bundle = control port = replies_recv offset = 0x78
    #pragma HLS INTERFACE s_axilite bundle = control port = req_lost offset = 0x80

    static hls::stream<arpReplyMeta> arpReplyMetaFifo("arpReplyMetaFifo");
    #pragma HLS STREAM variable = arpReplyMetaFifo depth = 4

    static hls::stream<ap_uint<32> > arpRequestMetaFifo("arpRequestMetaFifo");
    #pragma HLS STREAM variable = arpRequestMetaFifo depth = 4

    static hls::stream<ap_uint<32> > arpRequestMetaFilteredFifo("arpRequestMetaFilteredFifo");
    #pragma HLS STREAM variable = arpRequestMetaFilteredFifo depth = 4

    static hls::stream<arpTableEntry> arpTableInsertFifo("arpTableInsertFifo");
    #pragma HLS STREAM variable = arpTableInsertFifo depth = 4

    static hls::stream<ap_uint<32> > arpReplyRecvFifo("arpReplyRecvFifo");
    #pragma HLS STREAM variable = arpReplyRecvFifo depth = 4

    static hls::stream<bool> counter2timer("counter2timer");
    #pragma HLS STREAM depth = 3 variable = counter2timer

    static hls::stream<arpTableReply> macIpEncode_rsp_copy("macIpEncode_rsp_copy");
    #pragma HLS STREAM variable = macIpEncode_rsp_copy depth = 4

    static hls::stream<ap_uint<1> > req_sent_fifo("req_sent_fifo");
    static hls::stream<ap_uint<1> > replies_sent_fifo("replies_sent_fifo");
    static hls::stream<ap_uint<1> > req_recv_fifo("req_recv_fifo");
    static hls::stream<ap_uint<1> > replies_recv_fifo("replies_recv_fifo");
    static hls::stream<ap_uint<1> > req_lost_fifo("req_lost_fifo");
#pragma HLS STREAM depth = 4 variable = req_sent_fifo
#pragma HLS STREAM depth = 4 variable = replies_sent_fifo
#pragma HLS STREAM depth = 4 variable = req_recv_fifo
#pragma HLS STREAM depth = 4 variable = replies_recv_fifo
#pragma HLS STREAM depth = 4 variable = req_lost_fifo

    arp_server_100ms_count(counter2timer);

    arp_pkg_receiver(arpDataIn, arpReplyMetaFifo, arpReplyRecvFifo, arpTableInsertFifo, myIpAddress, req_recv_fifo,
                     replies_recv_fifo);

    arp_pkg_sender(arpReplyMetaFifo, arpRequestMetaFilteredFifo, arpDataOut, myMacAddress, myIpAddress, req_sent_fifo,
                   replies_sent_fifo);

    arp_table(arpTableInsertFifo, macIpEncode_req, macIpEncode_rsp_copy, arpRequestMetaFifo, arpEntry, host_op_toggle,
              host_opcode, entryBin);

    // Copy the reply to intermediate FIFO to avoid the Vitis synchronization warning of non-PIPO output
    // because arp_table writes to both an internal fifo and an external port.
    send_rsp(macIpEncode_rsp_copy, macIpEncode_rsp);

    arp_server_request_filter(counter2timer, arpRequestMetaFifo, arpReplyRecvFifo, arpRequestMetaFilteredFifo,
                              req_lost_fifo);

    arp_statCounters(req_sent_fifo, replies_sent_fifo, req_recv_fifo, replies_recv_fifo, req_lost_fifo, 
                     req_sent, replies_sent, req_recv, replies_recv, req_lost);
}
