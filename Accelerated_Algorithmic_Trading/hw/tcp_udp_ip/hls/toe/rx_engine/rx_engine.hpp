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
#include "../toe.hpp"
#include "../toe_internals.hpp"
#include "../toe_config.hpp"
#include "../../ipv4/ipv4.hpp"
#include "../two_complement_subchecksums.hpp"
#include "../../axi_utils.hpp"

struct optionalFieldsMeta {
    ap_uint<4> dataOffset;
    ap_uint<1> syn;
    optionalFieldsMeta() {}
    optionalFieldsMeta(ap_uint<4> offset, ap_uint<1> syn) : dataOffset(offset), syn(syn) {}
};

struct pseudoMeta {
    ap_uint<32> their_address;
    ap_uint<32> our_address;
    ap_uint<16> length;
    pseudoMeta() {}
    pseudoMeta(ap_uint<32> src_addr, ap_uint<32> dst_addr, ap_uint<16> len)
        : their_address(src_addr), our_address(dst_addr), length(len) {}
};

/** @ingroup rx_engine
 *  @TODO check if same as in Tx engine
 */
struct rxEngineMetaData {
    ap_uint<32> seqNumb;
    ap_uint<32> ackNumb;
    ap_uint<16> winSize;
    ap_uint<4> winScale;
    ap_uint<16> length;
    ap_uint<1> ack;
    ap_uint<1> rst;
    ap_uint<1> syn;
    ap_uint<1> fin;
    ap_uint<4> dataOffset;
};

/** @ingroup rx_engine
 *
 */
struct rxFsmMetaData {
    ap_uint<16> sessionID;
    ap_uint<32> srcIpAddress;
    ap_uint<16> dstIpPort;
    rxEngineMetaData meta; // check if all needed
    rxFsmMetaData() {}
    rxFsmMetaData(ap_uint<16> id, ap_uint<32> ipAddr, ap_uint<16> ipPort, rxEngineMetaData meta)
        : sessionID(id), srcIpAddress(ipAddr), dstIpPort(ipPort), meta(meta) {}
};

/** @defgroup rx_engine RX Engine
 *  @ingroup tcp_module
 *  RX Engine
 */
template <int WIDTH>
void rx_engine(hls::stream<ap_axiu<WIDTH,0,0,0> >& ipRxData,
               hls::stream<sessionLookupReply>& sLookup2rxEng_rsp,
               hls::stream<sessionState>& stateTable2rxEng_upd_rsp,
               hls::stream<bool>& portTable2rxEng_rsp,
               hls::stream<rxSarEntry>& rxSar2rxEng_upd_rsp,
               hls::stream<rxTxSarReply>& txSar2rxEng_upd_rsp,
#if !(RX_DDR_BYPASS)
               hls::stream<ap_axiu<8,0,0,0> >& rxBufferWriteStatus,
#endif
               hls::stream<ap_axiu<WIDTH,0,0,0> >& rxBufferWriteData,
               hls::stream<sessionLookupQuery>& rxEng2sLookup_req,
               hls::stream<stateQuery>& rxEng2stateTable_upd_req,
               hls::stream<ap_uint<16> >& rxEng2portTable_req,
               hls::stream<rxSarRecvd>& rxEng2rxSar_upd_req,
               hls::stream<rxTxSarQuery>& rxEng2txSar_upd_req,
               hls::stream<rxRetransmitTimerUpdate>& rxEng2timer_clearRetransmitTimer,
               hls::stream<ap_uint<16> >& rxEng2timer_clearProbeTimer,
               hls::stream<ap_uint<16> >& rxEng2timer_setCloseTimer,
               hls::stream<openStatus>& openConStatusOut, // TODO remove
               hls::stream<extendedEvent>& rxEng2eventEng_setEvent,
               uint32_t& stats_tcpInSegs,
               uint32_t& stats_tcpInErrs,
               uint32_t& stats_tcpAttemptFails,
               uint32_t& stats_tcpEstabResets,
#if !(RX_DDR_BYPASS)
               hls::stream<ap_axiu<128,0,0,0> >& rxBufferWriteCmd,
               hls::stream<appNotification>& rxEng2rxApp_notification);
#else
               hls::stream<appNotification>& rxEng2rxApp_notification,
               ap_uint<16> rxbuffer_data_count,
               ap_uint<16> rxbuffer_max_data_count);
#endif
