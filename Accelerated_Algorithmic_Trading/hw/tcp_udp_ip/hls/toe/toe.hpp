/*
 * Copyright (c) 2016, 2019-2020, Xilinx, Inc.
 * Copyright (c) 2019, Systems Group, ETH Zurich
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
#ifndef TOE_HPP_INCLUDED
#define TOE_HPP_INCLUDED

#include "../axi_utils.hpp"

const uint16_t TCP_PROTOCOL = 0x06;

// Forward declarations.
struct rtlSessionUpdateRequest;
struct rtlSessionUpdateReply;
struct rtlSessionLookupReply;
struct rtlSessionLookupRequest;

struct ipTuple {
    ap_uint<32> ip_address;
    ap_uint<16> ip_port;
};

struct  mmCmd {
    ap_uint<23> bbt;
    ap_uint<1> type;
    ap_uint<6> dsa;
    ap_uint<1> eof;
    ap_uint<1> drr;
    ap_uint<32> saddr;
    ap_uint<4> tag;
    ap_uint<4> rsvd;
    mmCmd() {}
    mmCmd(ap_uint<32> addr, ap_uint<16> len)
        : bbt(len), type(1), dsa(0), eof(1), drr(1), saddr(addr), tag(0), rsvd(0) {}
    /*mm_cmd(ap_uint<32> addr, ap_uint<16> len, ap_uint<1> last)
        :bbt(len), type(1), dsa(0), eof(last), drr(1), saddr(addr), tag(0), rsvd(0) {}*/
    /*mm_cmd(ap_uint<32> addr, ap_uint<16> len, ap_uint<4> dsa)
            :bbt(len), type(1), dsa(dsa), eof(1), drr(1), saddr(addr), tag(0), rsvd(0) {}*/
};

struct mmStatus {
    ap_uint<4> tag;
    ap_uint<1> interr;
    ap_uint<1> decerr;
    ap_uint<1> slverr;
    ap_uint<1> okay;
};

// TODO is this required??
struct mm_ibtt_status {
    ap_uint<4> tag;
    ap_uint<1> interr;
    ap_uint<1> decerr;
    ap_uint<1> slverr;
    ap_uint<1> okay;
    ap_uint<22> brc_vd;
    ap_uint<1> eop;
};

struct __attribute__((packed)) openStatus {
    ap_uint<16> sessionID;
    bool success;
    openStatus() {}
    openStatus(ap_uint<16> id, bool success) : sessionID(id), success(success) {}
};

struct appNotification {
    ap_uint<16> sessionID;
    ap_uint<16> length;
    ap_uint<32> ipAddress;
    ap_uint<16> dstPort;
    bool closed;
    bool opened;
    ap_uint<6> rsvd;
    appNotification() {}
    appNotification(ap_uint<16> id, ap_uint<16> len, ap_uint<32> addr, ap_uint<16> port)
        : sessionID(id), length(len), ipAddress(addr), dstPort(port), closed(false), opened(false), rsvd(0) {}
    appNotification(ap_uint<16> id, ap_uint<32> addr, ap_uint<16> port)
        : sessionID(id), length(0), ipAddress(addr), dstPort(port), closed(false), opened(false), rsvd(0) {}
    appNotification(ap_uint<16> id)
        : sessionID(id), length(0), ipAddress(0), dstPort(0), closed(false), opened(false), rsvd(0) {}
};

struct appReadRequest {
    ap_uint<16> sessionID;
    // ap_uint<16> address;
    ap_uint<16> length;
    appReadRequest() {}
    appReadRequest(ap_uint<16> id, ap_uint<16> len) : sessionID(id), length(len) {}
};

struct appTxMeta {
    ap_uint<16> sessionID;
    ap_uint<16> length;
    ap_uint<16> subSum;
    ap_uint<1>  validSum;
    appTxMeta() {}
    appTxMeta(ap_uint<16> id, ap_uint<16> len) : sessionID(id), length(len), subSum(0), validSum(0) {}
    appTxMeta(ap_uint<16> id, ap_uint<16> len, ap_uint<68> subSum, ap_uint<1> validSum) : sessionID(id), length(len), subSum(subSum), validSum(validSum) {}
};

struct appTxRsp {
    ap_uint<16> sessionID;
    ap_uint<16> length;
    ap_uint<30> remaining_space;
    ap_uint<2> error;
    appTxRsp() {}
    appTxRsp(ap_uint<16> id, ap_uint<16> len, ap_uint<30> rem_space, ap_uint<2> err)
        : sessionID(id), length(len), remaining_space(rem_space), error(err) {}
};

/** @ingroup
 *  This struct defines the stats of the @ref toe. RFC 1213
 *
 */
struct toe_stats {
    uint32_t tcpInSegs;
    uint32_t tcpInErrs;
    uint32_t tcpOutSegs;
    uint32_t tcpRetransSegs;
    uint32_t tcpActiveOpens;
    uint32_t tcpPassiveOpens;
    uint32_t tcpAttemptFails;
    uint32_t tcpEstabResets;
    uint32_t tcpCurrEstab;
};

void toe( // Data & Memory Interface
    hls::stream<ap_axiu<64,0,0,0> >& ipRxData,
#if !(RX_DDR_BYPASS)
    hls::stream<ap_axiu<8,0,0,0> >& rxBufferWriteStatus,
#endif
    hls::stream<ap_axiu<8,0,0,0> >& txBufferWriteStatus,
    hls::stream<ap_axiu<64,0,0,0> >& rxBufferReadData,
    hls::stream<ap_axiu<64,0,0,0> >& txBufferReadData,
    hls::stream<ap_axiu<64,0,0,0> >& ipTxData,
#if !(RX_DDR_BYPASS)
    hls::stream<ap_axiu<128,0,0,0> >& rxBufferWriteCmd,
    hls::stream<ap_axiu<128,0,0,0> >& rxBufferReadCmd,
#endif
    hls::stream<ap_axiu<128,0,0,0> >& txBufferWriteCmd,
    hls::stream<ap_axiu<128,0,0,0>>& txBufferReadCmd,
    hls::stream<ap_axiu<64,0,0,0> >& rxBufferWriteData,
    hls::stream<ap_axiu<64,0,0,0> >& txBufferWriteData,
    // SmartCam Interface
    hls::stream<ap_axiu<128,0,0,0> >& sessionLookup_rsp,
    hls::stream<ap_axiu<128,0,0,0> >& sessionUpdate_rsp,
    hls::stream<ap_axiu<128,0,0,0> >& sessionLookup_req,
    hls::stream<ap_axiu<128,0,0,0> >& sessionUpdate_req,

    // Application Interface
    hls::stream<ap_axiu<16,0,0,0> >& listenPortReq,
    // This is disabled for the time being, due to complexity concerns
    // hls::stream<ap_uint<16> >&                appClosePortIn,
    hls::stream<ap_axiu<32,0,0,0> >& rxDataReq,
    hls::stream<ap_axiu<64,0,0,0> >& openConnReq,
    hls::stream<ap_axiu<16,0,0,0> >& closeConnReq,
    hls::stream<ap_axiu<64,0,0,0> >& txDataReqMeta,
    hls::stream<ap_axiu<64,0,0,0> >& txDataReq,

    hls::stream<ap_axiu<8,0,0,0> >& listenPortRsp,
    hls::stream<ap_axiu<128,0,0,0> >& notification,
    hls::stream<ap_axiu<16,0,0,0> >& rxDataRspMeta,
    hls::stream<ap_axiu<64,0,0,0> >& rxDataRsp,
    hls::stream<ap_axiu<32,0,0,0> >& openConnRsp,
    hls::stream<ap_axiu<64,0,0,0> >& txDataRsp,
#if RX_DDR_BYPASS
    // Data counts for external FIFO
    ap_uint<16> axis_data_count,
    ap_uint<16> axis_max_data_count,
#endif
    // IP Address Input
    ap_uint<32> myIpAddress,
    uint32_t &tcpInSegs,
    uint32_t &tcpInErrs,
    uint32_t &tcpOutSegs,
    uint32_t &tcpRetransSegs,
    uint32_t &tcpActiveOpens,
    uint32_t &tcpPassiveOpens,
    uint32_t &tcpAttemptFails,
    uint32_t &tcpEstabResets,
    uint32_t &tcpCurrEstab);

#endif
