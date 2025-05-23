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
#include "tx_app_stream_if.hpp"

/** @ingroup tx_app_stream_if
 * Converts from mmCmd structure to ap_axi<128,0,0,0>
 */
ap_axiu<128,0,0,0> mmCmd2axiu(mmCmd inword) {
#pragma HLS inline
    ap_axiu<128,0,0,0> tmp_axiu;
    tmp_axiu.data(22,0) = inword.bbt;
    tmp_axiu.data(23,23) = inword.type;
    tmp_axiu.data(29,24) = inword.dsa;
    tmp_axiu.data(30,30) = inword.eof;
    tmp_axiu.data(31,31) = inword.drr;
    tmp_axiu.data(63,32) = inword.saddr;
    tmp_axiu.data(67,64) = inword.tag;
    tmp_axiu.data(71,68) = inword.rsvd;
    tmp_axiu.data(127,72) = 0;
    tmp_axiu.last = 1;
    return tmp_axiu;
}

/** @ingroup tx_app_stream_if
 * Converts from native appTxRsp structure to ap_axi<64,0,0,0>
 */
ap_axiu<64, 0, 0, 0> appTxRsp2axiu(appTxRsp inword) {
#pragma HLS inline
    ap_axiu<64, 0, 0, 0> tmp_axiu;
    tmp_axiu.data(15, 0) = inword.sessionID;
    tmp_axiu.data(31, 16) = inword.length;
    tmp_axiu.data(61, 32) = inword.remaining_space;
    tmp_axiu.data(63, 62) = inword.error;
    tmp_axiu.last = 1;
    return tmp_axiu;
}

/** @ingroup tx_app_stream_if
 * Converts from ap_axi<32,0,0,0> to appTxMeta structure
 */
appTxMeta axiu2appTxMeta(ap_axiu<64,0,0,0> inword) {
#pragma HLS inline
    appTxMeta tmp_meta;
    tmp_meta.sessionID = inword.data(15, 0);
    tmp_meta.length = inword.data(31, 16);
    tmp_meta.subSum = inword.data(47, 32);
    tmp_meta.validSum = inword.data(48,48);
    return tmp_meta;
}

/** @ingroup tx_app_stream_if
 *  Reads the request from the application and loads the necessary metadata,
 *  the FSM decides if the packet is written to the TX buffer or discarded.
 */
void tasi_metaLoader(hls::stream<ap_axiu<64,0,0,0> >& appTxDataReqMetaData,
                     hls::stream<sessionState>& stateTable2txApp_rsp,
                     hls::stream<txAppTxSarReply>& txSar2txApp_upd_rsp,
                     hls::stream<ap_axiu<64, 0, 0, 0> >& appTxDataRsp,
                     hls::stream<ap_uint<16> >& txApp2stateTable_req,
                     hls::stream<txAppTxSarQuery>& txApp2txSar_upd_req,
                     hls::stream<mmCmd>& tasi_meta2pkgPushCmd,
                     hls::stream<event>& txAppStream2eventEng_setEvent) {
#pragma HLS pipeline II = 1

    enum tai_states { READ_REQUEST, READ_META };
    static tai_states tai_state = READ_REQUEST;
    static appTxMeta tasi_writeMeta;

    txAppTxSarReply writeSar;
    sessionState state;
    appTxRsp appTxRsp_i;
    ap_axiu<64, 0, 0, 0> rsp;

    // FSM requests metadata, decides if packet goes to buffer or not
    switch (tai_state) {
        case READ_REQUEST:
            if (!appTxDataReqMetaData.empty()) {
                // Read sessionID
                tasi_writeMeta = axiu2appTxMeta(appTxDataReqMetaData.read());
                // Get session state
                txApp2stateTable_req.write(tasi_writeMeta.sessionID);
                // Get Ack pointer
                txApp2txSar_upd_req.write(txAppTxSarQuery(tasi_writeMeta.sessionID));
                tai_state = READ_META;
            }
            break;
        case READ_META:
            if (!txSar2txApp_upd_rsp.empty() && !stateTable2txApp_rsp.empty()) {
                stateTable2txApp_rsp.read(state);
                txSar2txApp_upd_rsp.read(writeSar);
                ap_uint<WINDOW_BITS> maxWriteLength = (writeSar.ackd - writeSar.mempt) - 1;
                ap_uint<WINDOW_BITS> available_space = maxWriteLength;
#if (TCP_NODELAY)
                // Reduce the maximum write length to our MSS if the space is greatercd .
                if (maxWriteLength > MSS) {
                    maxWriteLength = MSS;
                }
                // tasi_writeSar.mempt and txSar.not_ackd are supposed to be equal (with a few cycles delay)
                ap_uint<WINDOW_BITS> usedLength = ((ap_uint<WINDOW_BITS>)writeSar.mempt - writeSar.ackd);
                ap_uint<WINDOW_BITS> usableWindow = writeSar.min_window;
                // Remove the unacknowledged data from the window to give the amount we can send.
                if (usableWindow > usedLength) {
                    usableWindow -= usedLength;
                }
                available_space = usableWindow;
#endif
                if (state != ESTABLISHED) {
                    // Notify app about fail
                    appTxRsp_i =
                        appTxRsp(tasi_writeMeta.sessionID, tasi_writeMeta.length, maxWriteLength, ERROR_NOCONNECTION);
                    appTxDataRsp.write(appTxRsp2axiu(appTxRsp_i));
                } else if (tasi_writeMeta.length == 0) {
                    appTxRsp_i =
                        appTxRsp(tasi_writeMeta.sessionID, tasi_writeMeta.length, maxWriteLength, ERROR_NOSPACE);
                    appTxDataRsp.write(appTxRsp2axiu(appTxRsp_i));
                }
#if !(TCP_NODELAY)
                else if (tasi_writeMeta.length > maxWriteLength) {
                    appTxRsp_i =
                        appTxRsp(tasi_writeMeta.sessionID, tasi_writeMeta.length, maxWriteLength, ERROR_NOSPACE);
                    appTxDataRsp.write(appTxRsp2axiu(appTxRsp_i));
                }
#else
                else if (tasi_writeMeta.length > maxWriteLength || usableWindow < tasi_writeMeta.length) {
                    // Notify app about fail
                    appTxRsp_i =
                        appTxRsp(tasi_writeMeta.sessionID, tasi_writeMeta.length, available_space, ERROR_NOSPACE);
                    appTxDataRsp.write(appTxRsp2axiu(appTxRsp_i));
                }
#endif
                else // if (state == ESTABLISHED && pkgLen <= tasi_maxWriteLength)
                {
                    // TODO there seems some redundancy
                    ap_uint<32> pkgAddr;
                    pkgAddr(31, 30) = 0x01;
                    pkgAddr(29, WINDOW_BITS) = tasi_writeMeta.sessionID(13, 0);
                    pkgAddr(WINDOW_BITS - 1, 0) = writeSar.mempt;

                    tasi_meta2pkgPushCmd.write(mmCmd(pkgAddr, tasi_writeMeta.length));
                    appTxRsp_i = appTxRsp(tasi_writeMeta.sessionID, tasi_writeMeta.length, available_space, NO_ERROR);
                    appTxDataRsp.write(appTxRsp2axiu(appTxRsp_i));
                    txAppStream2eventEng_setEvent.write(
                        event(TX, tasi_writeMeta.sessionID, writeSar.mempt, tasi_writeMeta.length, tasi_writeMeta.subSum, tasi_writeMeta.validSum));
                    txApp2txSar_upd_req.write(
                        txAppTxSarQuery(tasi_writeMeta.sessionID, writeSar.mempt + tasi_writeMeta.length));
                }
                tai_state = READ_REQUEST;
            }
            break;
    } // switch
}

/** @ingroup tx_app_stream_if
 *  In case the @tasi_metaLoader decides to write the packet to the memory,
 *  it writes the memory command and pushes the data to the DataMover,
 *  otherwise the packet is dropped.
 */
template <int WIDTH, typename Tin>
void tasi_pkg_pusher(hls::stream<mmCmd>& tasi_meta2pkgPushCmd,
                     hls::stream<Tin>& appTxDataIn,
                     hls::stream<ap_axiu<128,0,0,0> >& txBufferWriteCmd,
                     hls::stream<ap_axiu<WIDTH,0,0,0> >& txBufferWriteData)

{
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    enum fsmStateType { IDLE, CUT_FIRST, ALIGN_SECOND, FWD_ALIGNED, RESIDUE };
    static fsmStateType tasiPkgPushState = IDLE;
    static mmCmd cmd;
    static ap_uint<WINDOW_BITS> remainingLength = 0;
    static ap_uint<WINDOW_BITS> lengthFirstPkg;
    static ap_uint<8> offset = 0;
    static net_axis<WIDTH> prevWord;
    ap_axiu<WIDTH,0,0,0> outWord;

    switch (tasiPkgPushState) {
        case IDLE:
            if (!tasi_meta2pkgPushCmd.empty()) {
                tasi_meta2pkgPushCmd.read(cmd);

                if ((cmd.saddr(WINDOW_BITS - 1, 0) + cmd.bbt) > BUFFER_SIZE) {
                    lengthFirstPkg = BUFFER_SIZE - cmd.saddr;
                    remainingLength = lengthFirstPkg;
                    offset = lengthFirstPkg(DATA_KEEP_BITS - 1, 0);

                    txBufferWriteCmd.write(mmCmd2axiu(mmCmd(cmd.saddr, lengthFirstPkg)));
                    tasiPkgPushState = CUT_FIRST;

                } else {
                    txBufferWriteCmd.write(mmCmd2axiu(cmd));
                    tasiPkgPushState = FWD_ALIGNED;
                }
            }
            break;
        case CUT_FIRST:
            if (!appTxDataIn.empty()) {
#if (TCP_NODELAY)
                appTxDataIn.read(prevWord);
#else
                prevWord = axiu2netAxis(appTxDataIn.read());
#endif
                Tin sendWord;
                sendWord.data = prevWord.data;
                sendWord.keep = prevWord.keep;
                sendWord.last = prevWord.last;

                if (remainingLength > (WIDTH / 8)) {
                    remainingLength -= (WIDTH / 8);
                }
                // This means that the second packet is aligned
                else if (remainingLength == (WIDTH / 8)) {
                    sendWord.last = 1;

                    cmd.saddr(WINDOW_BITS - 1, 0) = 0;
                    cmd.bbt -= lengthFirstPkg;
                    txBufferWriteCmd.write(mmCmd2axiu(cmd));
                    tasiPkgPushState = FWD_ALIGNED;
                } else {
                    sendWord.keep = lenToKeep(remainingLength);
                    sendWord.last = 1;

                    cmd.saddr(WINDOW_BITS - 1, 0) = 0;
                    cmd.bbt -= lengthFirstPkg;
                    txBufferWriteCmd.write(mmCmd2axiu(cmd));
                    tasiPkgPushState = ALIGN_SECOND;
                    // If only part of a word is left
                    if (prevWord.last) {
                        tasiPkgPushState = RESIDUE;
                    }
                }
                outWord.data = sendWord.data;
                outWord.keep = sendWord.keep;
                outWord.last = sendWord.last;                
                txBufferWriteData.write(outWord);
            }
            break;
        case FWD_ALIGNED: // This is the non-realignment state
            if (!appTxDataIn.empty()) {
                Tin currWord = appTxDataIn.read();
                outWord.data = currWord.data;
                outWord.keep = currWord.keep;
                outWord.last = currWord.last;                
                txBufferWriteData.write(outWord);                
                if (currWord.last) {
                    tasiPkgPushState = IDLE;
                }
            }
            break;
        case ALIGN_SECOND: // We go into this state when we need to realign things
            if (!appTxDataIn.empty()) {
                Tin tmpWord = appTxDataIn.read();
                net_axis<WIDTH> currWord;
                currWord.data = tmpWord.data;
                currWord.keep = tmpWord.keep;
                currWord.last = tmpWord.last;
                net_axis<WIDTH> sendWord;
                sendWord = alignWords<WIDTH>(offset, prevWord, currWord);
                sendWord.last = (currWord.keep[offset] == 0);
                outWord.data = sendWord.data;
                outWord.keep = sendWord.keep;
                outWord.last = sendWord.last;                
                txBufferWriteData.write(outWord);
                prevWord = currWord;
                if (currWord.last) {
                    tasiPkgPushState = IDLE;
                    if (!sendWord.last) {
                        tasiPkgPushState = RESIDUE;
                    }
                }
            }
            break;
        case RESIDUE: // last word
            net_axis<WIDTH> sendWord;
#ifndef __SYNTHESIS__
            sendWord.data(WIDTH - 1, WIDTH - (offset * 8)) = 0;
#endif
            net_axis<WIDTH> emptyWord;
            sendWord = alignWords<WIDTH>(offset, prevWord, emptyWord);
            sendWord.last = 1;
            outWord.data = sendWord.data;
            outWord.keep = sendWord.keep;
            outWord.last = sendWord.last;                
            txBufferWriteData.write(outWord);                        
            tasiPkgPushState = IDLE;
            break;
    } // switch
}

template <int WIDTH>
void duplicate_ap_axiu_stream(hls::stream<ap_axiu<WIDTH,0,0,0> >& in, hls::stream<net_axis<WIDTH> >& out0, hls::stream<net_axis<WIDTH> >& out1) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    if (!in.empty()) {
        ap_axiu<WIDTH,0,0,0> inWord = in.read();
        net_axis<WIDTH> tmp;
        tmp.data = inWord.data;
        tmp.keep = inWord.keep;
        tmp.last = inWord.last;
        out0.write(tmp);
        out1.write(tmp);
    }
}


/** @ingroup tx_app_stream_if
 *  This application interface is used to transmit data streams of established connections.
 *  The application sends the Session-ID on through @p writeMetaDataIn and the data stream
 *  on @p writeDataIn. The interface checks then the state of the connection and loads the
 *  application pointer into the memory. It then writes the data into the memory. The application
 *  is notified through @p writeReturnStatusOut if the write to the buffer succeeded. In case
 *  of success the length of the write is returned, otherwise -1;
 *  @param[in]		appTxDataReqMetaData
 *  @param[in]		appTxDataReq
 *  @param[in]		stateTable2txApp_rsp
 *  @param[in]		txSar2txApp_upd_rsp
 *  @param[out]		appTxDataRsp
 *  @param[out]		txApp2stateTable_req
 *  @param[out]		txApp2txSar_upd_req
 *  @param[out]		txBufferWriteCmd
 *  @param[out]		txBufferWriteData
 *  @param[out]		txAppStream2eventEng_setEvent
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
                      hls::stream<event>& txAppStream2eventEng_setEvent) {
#pragma HLS INLINE

    static hls::stream<mmCmd> tasi_meta2pkgPushCmd("tasi_meta2pkgPushCmd");
#pragma HLS stream variable = tasi_meta2pkgPushCmd depth = 32

    tasi_metaLoader(appTxDataReqMetaData, stateTable2txApp_rsp, txSar2txApp_upd_rsp, appTxDataRsp, txApp2stateTable_req,
                    txApp2txSar_upd_req, tasi_meta2pkgPushCmd, txAppStream2eventEng_setEvent);

#if (TCP_NODELAY)
    static hls::stream<net_axis<WIDTH> > tasi_dataFifo("tasi_dataFifo");
#pragma HLS stream variable = tasi_dataFifo depth = 3

    duplicate_ap_axiu_stream(appTxDataReq, tasi_dataFifo, txApp2txEng_data_stream);
#endif


#if (TCP_NODELAY)
    tasi_pkg_pusher<WIDTH, net_axis<WIDTH>>(tasi_meta2pkgPushCmd,tasi_dataFifo,txBufferWriteCmd, txBufferWriteData);
#else
    tasi_pkg_pusher<WIDTH, ap_axiu<WIDTH,0,0,0>>(tasi_meta2pkgPushCmd,appTxDataReq,txBufferWriteCmd, txBufferWriteData);
#endif
                           
}

template void tx_app_stream_if<DATA_WIDTH>(hls::stream<ap_axiu<64,0,0,0> >& appTxDataReqMetaData,
                                           hls::stream<ap_axiu<DATA_WIDTH,0,0,0> >& appTxDataReq,
                                           hls::stream<sessionState>& stateTable2txApp_rsp,
                                           hls::stream<txAppTxSarReply>& txSar2txApp_upd_rsp, // TODO rename
                                           hls::stream<ap_axiu<64, 0, 0, 0> >& appTxDataRsp,
                                           hls::stream<ap_uint<16> >& txApp2stateTable_req,
                                           hls::stream<txAppTxSarQuery>& txApp2txSar_upd_req, // TODO rename
                                           hls::stream<ap_axiu<128,0,0,0> >& txBufferWriteCmd,
                                           hls::stream<ap_axiu<DATA_WIDTH,0,0,0> >& txBufferWriteData,
#if (TCP_NODELAY)
                                           hls::stream<net_axis<DATA_WIDTH> >& txApp2txEng_data_stream,
#endif
                                           hls::stream<event>& txAppStream2eventEng_setEvent);
