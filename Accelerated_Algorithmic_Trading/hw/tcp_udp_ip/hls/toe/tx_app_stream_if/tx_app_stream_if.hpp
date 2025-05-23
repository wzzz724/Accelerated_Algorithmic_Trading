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

#ifndef TX_APP_STREAM_IF_H
#define TX_APP_STREAM_IF_H

static const ap_uint<2> NO_ERROR = 0;
static const ap_uint<2> ERROR_NOCONNECTION = 1;
static const ap_uint<2> ERROR_NOSPACE = 2;

/** @ingroup tx_app_stream_if
 *
 */
struct eventMeta {
    ap_uint<16> sessionID;
    ap_uint<16> address;
    ap_uint<16> length;
    eventMeta() {}
    eventMeta(ap_uint<16> id, ap_uint<16> addr, ap_uint<16> len) : sessionID(id), address(addr), length(len) {}
};

/** @defgroup tx_app_stream_if TX Application Stream Interface
 *  @ingroup app_if
 */
template <int WIDTH>
void tx_app_stream_if(hls::stream<ap_axiu<64,0,0,0> >& appTxDataReqMetaData,
                      hls::stream<ap_axiu<WIDTH,0,0,0> >& appTxDataReq,
                      hls::stream<sessionState>& stateTable2txApp_rsp,
                      hls::stream<txAppTxSarReply>& txSar2txApp_upd_rsp, // TODO rename
                      hls::stream<ap_axiu<64, 0, 0, 0> >& appTxDataRsp,
                      hls::stream<ap_uint<16> >& txApp2stateTable_req,
                      hls::stream<txAppTxSarQuery>& txApp2txSar_upd_req, // TODO rename
                      hls::stream<ap_axiu<128,0,0,0> >& txBufferWriteCmd,
                      hls::stream<ap_axiu<WIDTH,0,0,0> >& txBufferWriteData,
#if (TCP_NODELAY)
                      hls::stream<net_axis<WIDTH> >& txApp2txEng_data_stream,
#endif
                      hls::stream<event>& txAppStream2eventEng_setEvent);
#endif /* !TX_APP_STREAM_IF_H */
