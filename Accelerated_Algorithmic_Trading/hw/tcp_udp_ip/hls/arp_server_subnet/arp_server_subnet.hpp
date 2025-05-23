/************************************************
Copyright (c) 2016, 2019-2020 Xilinx, Inc.
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

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <hls_stream.h>
#include "ap_int.h"
#include <stdint.h>
#include "ap_axi_sdata.h"

const uint32_t MAX_WAIT = 5; // 0.5s based on TIMER_PERIOD_NS
#ifndef __SYNTHESIS__
const uint32_t TIMER_NUM_CLOCKS = 20;
#else
const uint32_t PERIOD_NS = 4;
const uint32_t TIMER_PERIOD_NS = 1e8; // 0.1s in ns
const uint32_t TIMER_NUM_CLOCKS = TIMER_PERIOD_NS / PERIOD_NS;
#endif

const uint16_t REQUEST = 0x0100;
const uint16_t REPLY = 0x0200;
const ap_uint<32> replyTimeOut = 65536;

const ap_uint<48> BROADCAST_MAC = 0xFFFFFFFFFFFF; // Broadcast MAC Address
typedef ap_axiu<64, 0, 0, 0> axiWord;

template <int D>
ap_uint<D> reverse(const ap_uint<D>& w) {
    ap_uint<D> temp;
    for (int i = 0; i < D / 8; i++) {
#pragma HLS UNROLL
        temp(i * 8 + 7, i * 8) = w(D - (i * 8) - 1, D - (i * 8) - 8);
    }
    return temp;
}

struct arpTimerEntry {
    uint32_t time;
    bool fActive;
};

void arp_server_100ms_count(hls::stream<bool>& tick100ms);

void arp_server_request_filter(hls::stream<bool>& tick100ms,
                               hls::stream<ap_uint<32> >& arpRequestMetaFifo,
                               hls::stream<ap_uint<32> >& arpReplyFifo,
                               hls::stream<ap_uint<32> >& arpRequestMetaFilteredFifo,
                               hls::stream<ap_uint<1> >& reqLost);

/** @ingroup arp_server
 *
 */
struct __attribute__((packed)) arpTableEntry {
    ap_uint<32> ipAddress;
    ap_uint<48> macAddress;
    bool valid;
    arpTableEntry() {}
    arpTableEntry(ap_uint<32> protoAdd, ap_uint<48> hwAdd, bool valid)
        : ipAddress(protoAdd), macAddress(hwAdd), valid(valid) {}
};

struct __attribute__((packed)) arpTableReply {
    ap_uint<48> macAddress;
    bool hit;
    arpTableReply() {}
    arpTableReply(ap_uint<48> macAdd, bool hit) : macAddress(macAdd), hit(hit) {}
};

struct arpReplyMeta {
    ap_uint<48> srcMac; // rename
    ap_uint<16> ethType;
    ap_uint<16> hwType;
    ap_uint<16> protoType;
    ap_uint<8> hwLen;
    ap_uint<8> protoLen;
    ap_uint<48> hwAddrSrc;
    ap_uint<32> protoAddrSrc;
    arpReplyMeta() {}
};

/** @defgroup arp_server ARP Server
 *
 */
void arp_server_subnet(hls::stream<ap_axiu<64,0,0,0> >& inData,
                       hls::stream<ap_uint<32> >& queryIP,
                       hls::stream<ap_axiu<64,0,0,0> >& outData,
                       hls::stream<arpTableReply>& returnMAC,
                       ap_uint<48> myMacAddress,
                       ap_uint<32> myIpAddress,
                       arpTableEntry& arpTable,
                       bool host_op_toggle,
                       ap_uint<2> host_opcode,
                       ap_uint<8> entryBin,
                       uint32_t &req_sent,
                       uint32_t &replies_sent,
                       uint32_t &req_recv,
                       uint32_t &replies_recv,
                       uint32_t &req_lost);
