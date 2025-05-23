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
#include "../toe_config.hpp"
#include "tx_app_interface.hpp"

/** @ingroup tx_app_interface
 * Converts from native ap_axi<8,0,0,0> structure to mmStatus
 */
mmStatus axiu2mmStatus(ap_axiu<8,0,0,0> inword) {
#pragma HLS inline
    mmStatus tmp_mmStatus;
    tmp_mmStatus.tag = inword.data(3,0);
    tmp_mmStatus.interr = inword.data(4,4);
    tmp_mmStatus.decerr = inword.data(5,5);
    tmp_mmStatus.slverr = inword.data(6,6);
    tmp_mmStatus.okay = inword.data(7,7);
    return tmp_mmStatus;
}

/** @ingroup tx_app_interface
 *  @name txEventMerger
 *
 *  Merges events from the txApp and txAppStream modules to the single output fifo
 *  if TCP_NODELAY is enabled then it also duplicates Tx events to the eventCacheFifo
 *
 * @param[in] txApp2eventEng_mergeEvent     event stream #1
 * @param[in] txAppStream2event_mergeEvent  event stream #2
 * @param[out] tasi_txEventCacheFifo        Direct Tx Event Stream #TCP_NODELAY
 * @param[out] out                          Merged event stream
 *
 */
void txEventMerger(hls::stream<event>& txApp2eventEng_mergeEvent,
                   hls::stream<event>& txAppStream2event_mergeEvent,
#if (TCP_NODELAY)
                   hls::stream<event>& tasi_txEventCacheFifo,
#endif
                   hls::stream<event>& out) {
#pragma HLS PIPELINE II = 1

    event ev;
    // Merge Events
    if (!txApp2eventEng_mergeEvent.empty()) {
        out.write(txApp2eventEng_mergeEvent.read());
    } else if (!txAppStream2event_mergeEvent.empty()) {
        txAppStream2event_mergeEvent.read(ev);
        out.write(ev);
#if (TCP_NODELAY)
        if (ev.type == TX) {
            tasi_txEventCacheFifo.write(ev);
        }
#endif
    }
}

/** @ingroup tx_app_interface
 *  @name txAppStatusHandler
 *
 *  Reads events and processes the expected number of mmStatus responses. Once this is complete
 *  notify the txSar with the new pointer and associated sessionId. There can be
 *  two responses per write as this is split if the pointer wraps.
 *
 * @param[in] txBufferWriteStatus  Memory mover responses
 * @param[in] tasi_eventCacheFifo  Input event stream. If TCP_NODELAY this comes direct from @txEventMerger
 * @param[out] txAppStream2eventEng_setEvent notify the event engine of writes. (Only when we're bypassing memory)
 * @param[out[ txApp2txSar_app_push notify the TxSar when a write is complete
 *
 */
void txAppStatusHandler(hls::stream<ap_axiu<8,0,0,0> >& txBufferWriteStatus,
                        hls::stream<event>& tasi_eventCacheFifo,
#if !(TCP_NODELAY)
                        hls::stream<event>& txAppStream2eventEng_setEvent,
#endif
                        hls::stream<txAppTxSarPush>& txApp2txSar_app_push) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    enum fsmStatus { READ_EV, READ_STATUS_1, READ_STATUS_2 };
    static fsmStatus tash_state = READ_EV;
    static event ev;

    switch (tash_state) {
        case READ_EV:
            if (!tasi_eventCacheFifo.empty()) {
                tasi_eventCacheFifo.read(ev);
                if (ev.type == TX) {
                    tash_state = READ_STATUS_1;
                }
#if !(TCP_NODELAY)
                else {
                    txAppStream2eventEng_setEvent.write(ev);
                }
#endif
            }
            break;
        case READ_STATUS_1:
            if (!txBufferWriteStatus.empty()) {
                mmStatus status = axiu2mmStatus(txBufferWriteStatus.read());
                if (status.okay) {
                    ap_uint<WINDOW_BITS + 1> tempLength = ev.address + ev.length;
                    if (tempLength[WINDOW_BITS] == 1) // tempLength > BUFFER_SIZE
                    {
                        tash_state = READ_STATUS_2;
                    } else {
                        txApp2txSar_app_push.write(txAppTxSarPush(
                            ev.sessionID, ev.address + ev.length)); // App pointer update, pointer is released
#if !(TCP_NODELAY)
                        txAppStream2eventEng_setEvent.write(ev);
#endif
                        tash_state = READ_EV;
                    }
                }
            }
            break;
        case READ_STATUS_2:
            if (!txBufferWriteStatus.empty()) {
                mmStatus status = axiu2mmStatus(txBufferWriteStatus.read());
                if (status.okay) {
                    txApp2txSar_app_push.write(txAppTxSarPush(
                        ev.sessionID, ev.address + ev.length)); // App pointer update, pointer is released
#if !(TCP_NODELAY)
                    txAppStream2eventEng_setEvent.write(ev);
#endif
                    tash_state = READ_EV;
                }
            }
            break;
    } // switch
}

/** @ingroup tx_app_interface
 *  @name tx_app_table
 *
 *  Tracks the pointers, ackd and min_window between the Tx Application Interface (One-way writes) and the TxSar.
 *
 * @param[in] txSar2txApp_ack_push
 * @param[in] txApp_upd_req
 * @param[out] txApp_upd_rsp
 *
 */
void tx_app_table(hls::stream<txSarAckPush>& txSar2txApp_ack_push,
                  hls::stream<txAppTxSarQuery>& txApp_upd_req,
                  hls::stream<txAppTxSarReply>& txApp_upd_rsp) {
#pragma HLS PIPELINE II = 1

    static txAppTableEntry app_table[MAX_SESSIONS];

    txSarAckPush ackPush;
    txAppTxSarQuery txAppUpdate;

    if (!txSar2txApp_ack_push.empty()) {
        txSar2txApp_ack_push.read(ackPush);
        if (ackPush.init) {
            // At init this is actually not_ackd
            app_table[ackPush.sessionID].ackd = ackPush.ackd - 1;
            app_table[ackPush.sessionID].mempt = ackPush.ackd;
#if (TCP_NODELAY)
            app_table[ackPush.sessionID].min_window = ackPush.min_window;
#endif
        } else {
            app_table[ackPush.sessionID].ackd = ackPush.ackd;
#if (TCP_NODELAY)
            app_table[ackPush.sessionID].min_window = ackPush.min_window;
#endif
        }
    } else if (!txApp_upd_req.empty()) {
        txApp_upd_req.read(txAppUpdate);
        // Write
        if (txAppUpdate.write) {
            app_table[txAppUpdate.sessionID].mempt = txAppUpdate.mempt;
        } else // Read
        {
#if !(TCP_NODELAY)
            txApp_upd_rsp.write(txAppTxSarReply(txAppUpdate.sessionID, app_table[txAppUpdate.sessionID].ackd,
                                                app_table[txAppUpdate.sessionID].mempt));
#else
            txApp_upd_rsp.write(txAppTxSarReply(txAppUpdate.sessionID, app_table[txAppUpdate.sessionID].ackd,
                                                app_table[txAppUpdate.sessionID].mempt,
                                                app_table[txAppUpdate.sessionID].min_window));
#endif
        }
    }
}

/** @ingroup tx_app_interface
 *  @name metaLoader
 *  Mainly a wrapper around the tx_app_stream_if (data) and tx_app_if.
 *  Merges events and tracks pointers based on sessionID. Handles mmstatus
 *  responses
 *
 */
template <int WIDTH>
void tx_app_interface(hls::stream<ap_axiu<64,0,0,0> >& appTxDataReqMetadata,
                      hls::stream<ap_axiu<WIDTH,0,0,0> >& appTxDataReq,
                      hls::stream<sessionState>& stateTable2txApp_rsp,
                      hls::stream<txSarAckPush>& txSar2txApp_ack_push,
                      hls::stream<ap_axiu<8,0,0,0> >& txBufferWriteStatus,

                      hls::stream<ap_axiu<64,0,0,0> >& appOpenConnReq,
                      hls::stream<ap_axiu<16,0,0,0> >& appCloseConnReq,
                      hls::stream<sessionLookupReply>& sLookup2txApp_rsp,
                      hls::stream<ap_uint<16> >& portTable2txApp_port_rsp,
                      hls::stream<sessionState>& stateTable2txApp_upd_rsp,
                      hls::stream<openStatus>& conEstablishedFifo,

                      hls::stream<ap_axiu<64, 0, 0, 0> >& appTxDataRsp,
                      hls::stream<ap_uint<16> >& txApp2stateTable_req,
                      hls::stream<ap_axiu<128,0,0,0> >& txBufferWriteCmd,
                      hls::stream<ap_axiu<WIDTH,0,0,0> >& txBufferWriteData,
#if (TCP_NODELAY)
                      hls::stream<net_axis<WIDTH> >& txApp2txEng_data_stream,
#endif
                      hls::stream<txAppTxSarPush>& txApp2txSar_push,

                      hls::stream<ap_axiu<32,0,0,0> >& appOpenConnRsp,
                      hls::stream<fourTuple>& txApp2sLookup_req,
                      hls::stream<stateQuery>& txApp2stateTable_upd_req,
                      hls::stream<event>& txApp2eventEng_setEvent,
                      hls::stream<openStatus>& rtTimer2txApp_notification,
                      ap_uint<32> myIpAddress) {
#pragma HLS INLINE

    // Fifos
    static hls::stream<event> txApp2eventEng_mergeEvent("txApp2eventEng_mergeEvent");
    static hls::stream<event> txAppStream2event_mergeEvent("txAppStream2event_mergeEvent");
    #pragma HLS stream variable = txApp2eventEng_mergeEvent depth = 2
    #pragma HLS stream variable = txAppStream2event_mergeEvent depth = 2

    static hls::stream<event> txApp_eventCacheFifo("txApp_eventCacheFifo");
    static hls::stream<event> txApp_txEventCache("txApp_txEventCache");
    #pragma HLS stream variable = txApp_eventCacheFifo depth = 2
    #pragma HLS stream variable = txApp_txEventCache depth = 64

    static hls::stream<txAppTxSarQuery> txApp2txSar_upd_req("txApp2txSar_upd_req");
    static hls::stream<txAppTxSarReply> txSar2txApp_upd_rsp("txSar2txApp_upd_rsp");
    #pragma HLS stream variable = txApp2txSar_upd_req depth = 2
    #pragma HLS stream variable = txSar2txApp_upd_rsp depth = 2

    // Before merging, check status for TX
    // Merge Events
    txEventMerger(txApp2eventEng_mergeEvent, txAppStream2event_mergeEvent,
#if (TCP_NODELAY)
                  txApp_txEventCache, txApp2eventEng_setEvent);
#else
                  txApp_eventCacheFifo);
#endif
    txAppStatusHandler(txBufferWriteStatus,
#if (TCP_NODELAY)
                       txApp_txEventCache,
#else
                       txApp_eventCacheFifo, txApp2eventEng_setEvent,
#endif
                       txApp2txSar_push);

    // TX application Stream Interface
    tx_app_stream_if<WIDTH>(appTxDataReqMetadata, appTxDataReq, stateTable2txApp_rsp, txSar2txApp_upd_rsp, appTxDataRsp,
                            txApp2stateTable_req, txApp2txSar_upd_req, txBufferWriteCmd, txBufferWriteData,
#if (TCP_NODELAY)
                            txApp2txEng_data_stream,
#endif
                            txAppStream2event_mergeEvent);

    // TX Application Interface
    tx_app_if(appOpenConnReq, appCloseConnReq, sLookup2txApp_rsp, portTable2txApp_port_rsp, stateTable2txApp_upd_rsp,
              conEstablishedFifo, appOpenConnRsp, txApp2sLookup_req, txApp2stateTable_upd_req,
              txApp2eventEng_mergeEvent, rtTimer2txApp_notification, myIpAddress);

    // TX App Meta Table
    tx_app_table(txSar2txApp_ack_push, txApp2txSar_upd_req, txSar2txApp_upd_rsp);
}

template void tx_app_interface<DATA_WIDTH>(hls::stream<ap_axiu<64,0,0,0> >& appTxDataReqMetadata,
                                           hls::stream<ap_axiu<DATA_WIDTH,0,0,0> >& appTxDataReq,
                                           hls::stream<sessionState>& stateTable2txApp_rsp,
                                           hls::stream<txSarAckPush>& txSar2txApp_ack_push,
                                           hls::stream<ap_axiu<8,0,0,0> >& txBufferWriteStatus,

                                           hls::stream<ap_axiu<64,0,0,0> >& appOpenConnReq,
                                           hls::stream<ap_axiu<16,0,0,0> >& appCloseConnReq,
                                           hls::stream<sessionLookupReply>& sLookup2txApp_rsp,
                                           hls::stream<ap_uint<16> >& portTable2txApp_port_rsp,
                                           hls::stream<sessionState>& stateTable2txApp_upd_rsp,
                                           hls::stream<openStatus>& conEstablishedFifo,

                                           hls::stream<ap_axiu<64, 0, 0, 0> >& appTxDataRsp,
                                           hls::stream<ap_uint<16> >& txApp2stateTable_req,
                                           hls::stream<ap_axiu<128,0,0,0> >& txBufferWriteCmd,
                                           hls::stream<ap_axiu<DATA_WIDTH,0,0,0> >& txBufferWriteData,
#if (TCP_NODELAY)
                                           hls::stream<net_axis<DATA_WIDTH> >& txApp2txEng_data_stream,
#endif
                                           hls::stream<txAppTxSarPush>& txApp2txSar_push,

                                           hls::stream<ap_axiu<32,0,0,0> >& appOpenConnRsp,
                                           hls::stream<fourTuple>& txApp2sLookup_req,
                                           hls::stream<stateQuery>& txApp2stateTable_upd_req,
                                           hls::stream<event>& txApp2eventEng_setEvent,
                                           hls::stream<openStatus>& rtTimer2txApp_notification,
                                           ap_uint<32> myIpAddress);
