/*
 * Copyright (c) 2016, Xilinx, Inc.
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

/** @defgroup tx_app_if TX Application Interface
 *  @ingroup app_if
 */
void tx_app_if(hls::stream<ap_axiu<64,0,0,0> >& appOpenConnReq,
               hls::stream<ap_axiu<16,0,0,0> >& closeConnReq,
               hls::stream<sessionLookupReply>& sLookup2txApp_rsp,
               hls::stream<ap_uint<16> >& portTable2txApp_port_rsp,
               hls::stream<sessionState>& stateTable2txApp_upd_rsp,
               hls::stream<openStatus>& conEstablishedIn, // alter
               hls::stream<ap_axiu<32,0,0,0> >& appOpenConnRsp,
               hls::stream<fourTuple>& txApp2sLookup_req,
               // stream<ap_uint<1> >&			txApp2portTable_port_req,
               hls::stream<stateQuery>& txApp2stateTable_upd_req,
               hls::stream<event>& txApp2eventEng_setEvent,
               hls::stream<openStatus>& rtTimer2txApp_notification,
               ap_uint<32> myIpAddress);
