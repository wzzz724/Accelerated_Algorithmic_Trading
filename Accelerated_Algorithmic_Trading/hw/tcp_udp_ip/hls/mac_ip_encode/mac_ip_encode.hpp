/************************************************
Copyright (c) 2016, 2019, Xilinx, Inc.
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

#include "../axi_utils.hpp"

struct __attribute__((packed)) arpTableReply {
    ap_uint<48> macAddress;
    bool hit;
    arpTableReply() {}
    arpTableReply(ap_uint<48> macAdd, bool hit) : macAddress(macAdd), hit(hit) {}
};

/** @defgroup mac_ip_encode MAC-IP encode
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
                   uint32_t &packets_dropped);
