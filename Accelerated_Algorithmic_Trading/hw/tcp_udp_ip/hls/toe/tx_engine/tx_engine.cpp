/*
 * Copyright (c) 2016, 2019-2020, Xilinx, Inc.
 * Copyright (c) 2019, Systems Group, ETH Zurich
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
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
#include "tx_engine.hpp"
#include "../../ipv4/ipv4.hpp"
#include "../two_complement_subchecksums.hpp"
#include "../../axi_utils.hpp"

/** @ingroup tx_engine
 * Converts from mmCmd structure to ap_axi<128,0,0,0>
 */
ap_axiu<128,0,0,0> mmCmd2axiu2(mmCmd inword) {
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
    tmp_axiu.last = 1;
    return tmp_axiu;
}

/** @ingroup tx_engine
 *  @name metaLoader
 *  The metaLoader reads the Events from the EventEngine then it loads all the necessary MetaData from the data
 *  structures (RX & TX Sar Table). Depending on the Event type it generates the necessary MetaData for the
 *  ipHeaderConstruction and the pseudoHeaderConstruction.
 *  Additionally it requests the IP Tuples from the Session. In some special cases the IP Tuple is delivered directly
 *  from @ref rx_engine and does not have to be loaded from the Session Table. The isLookUpFifo indicates this special
 * cases.
 *  Lookup Table for the current session.
 *  Depending on the Event Type the retransmit or/and probe Timer is set.
 *  @param[in]		eventEng2txEng_event
 *  @param[in]		rxSar2txEng_upd_rsp
 *  @param[in]		txSar2txEng_upd_rsp
 *  @param[out]		txEng2rxSar_upd_req
 *  @param[out]		txEng2txSar_upd_req
 *  @param[out]		txEng2timer_setRetransmitTimer
 *  @param[out]		txEng2timer_setProbeTimer
 *  @param[out]		txEng_ipMetaFifoOut
 *  @param[out]		txEng_tcpMetaFifoOut
 *  @param[out]		txBufferReadCmd
 *  @param[out]		txEng2sLookup_rev_req
 *  @param[out]		txEng_isLookUpFifoOut
 *  @param[out]		txEng_tupleShortCutFifoOut
 *  @param[out]		readCountFifo
 */
void metaLoader(hls::stream<extendedEvent>& eventEng2txEng_event,
                hls::stream<rxSarReply>& rxSar2txEng_rsp,
                hls::stream<txTxSarReply>& txSar2txEng_upd_rsp,
                hls::stream<ap_uint<16> >& txEng2rxSar_req,
                hls::stream<txTxSarQuery>& txEng2txSar_upd_req,
                hls::stream<txRetransmitTimerSet>& txEng2timer_setRetransmitTimer,
                hls::stream<ap_uint<16> >& txEng2timer_setProbeTimer,
                hls::stream<ap_uint<16> >& txEng_ipMetaFifoOut,
                hls::stream<tx_engine_meta>& txEng_tcpMetaFifoOut,
                hls::stream<mmCmd>& txBufferReadCmd,
                hls::stream<ap_uint<16> >& txEng2sLookup_rev_req,
                hls::stream<bool>& txEng_isLookUpFifoOut,
#if (TCP_NODELAY)
                hls::stream<bool>& txEng_isDDRbypass,
#endif
                hls::stream<fourTuple>& txEng_tupleShortCutFifoOut,
                hls::stream<ap_uint<1> >& readCountFifo,
				hls::stream<ap_uint<16> >& partialSubSum,
                hls::stream<ap_uint<1> >& validSubSum,
                hls::stream<ap_uint<1> >& stats_tcpOutSegs,
                hls::stream<ap_uint<1> >& stats_tcpRetransSegs) {
#pragma HLS INLINE off
#pragma HLS pipeline II = 1

    static ap_uint<1> ml_FsmState = 0;
    static bool ml_sarLoaded = false;
    static extendedEvent ml_curEvent;
    static ap_uint<32> ml_randomValue = 0x562301af; // Random seed initialization

    static ap_uint<2> ml_segmentCount = 0;
    static rxSarReply rxSar;
    static txTxSarReply txSarReg;
    ap_uint<WINDOW_BITS> slowstart_threshold;
    static tx_engine_meta meta;
    rstAckEvent resetAckEvent;
    rstEvent resetEvent;
    static txTxSarReply txSar;
    static ap_uint<WINDOW_BITS> currLength;

    switch (ml_FsmState) {
        case 0:
            if (!eventEng2txEng_event.empty()) {
                eventEng2txEng_event.read(ml_curEvent);
                readCountFifo.write(1);
                ml_sarLoaded = false;
                //Not sure if I should put here or in TX state...
                partialSubSum.write(ml_curEvent.subSum);
                validSubSum.write(ml_curEvent.validSum);

                switch (ml_curEvent.type) {
                    case RT:
                    case TX:
                    case SYN_ACK:
                    case FIN:
                    case ACK_NODELAY:
                    case ACK:
                        txEng2rxSar_req.write(ml_curEvent.sessionID);
                        txEng2txSar_upd_req.write(txTxSarQuery(ml_curEvent.sessionID));
                        break;
                    case RST_ACK:
                        // Get txSar for SEQ numb
                        resetAckEvent = ml_curEvent;
                        if (resetAckEvent.hasSessionID()) {
                            txEng2txSar_upd_req.write(txTxSarQuery(ml_curEvent.sessionID));
                        }
                        break;
                    case RST:
                        resetEvent = ml_curEvent;
                        if (resetEvent.hasSessionID()) {
                            txEng2txSar_upd_req.write(txTxSarQuery(ml_curEvent.sessionID));
                        }
                        break;                        
                    case SYN:
                        if (ml_curEvent.rt_count != 0) {
                            txEng2txSar_upd_req.write(txTxSarQuery(ml_curEvent.sessionID));
                        }
                        break;
                    default:
                        break;
                }
                ml_FsmState = 1;
                ml_randomValue++; // make sure it doesn't become zero TODO move this out of if, but breaks my testsuite
            }                     // if not empty
            ml_segmentCount = 0;
            break;
        case 1:
            switch (ml_curEvent.type) {
// When Nagle's algorithm disabled
// Can bypass DDR
#if (TCP_NODELAY)
                case TX:
                    if ((!rxSar2txEng_rsp.empty() && !txSar2txEng_upd_rsp.empty())) {
                        rxSar2txEng_rsp.read(rxSar);
                        txSar2txEng_upd_rsp.read(txSar);

                        meta.ackNumb = rxSar.recvd;
                        meta.seqNumb = txSar.not_ackd;
                        meta.window_size =
                            rxSar.windowSize; // Precalcualted in rx_sar_table ((rxSar.appd - rxSar.recvd) - 1)
                        meta.ack = 1;         // ACK is always set when established
                        meta.rst = 0;
                        meta.syn = 0;
                        meta.fin = 0;
                        meta.length = ml_curEvent.length;

                        // this is hack, makes sure that probeTimer is never set.
                        // ProbeTimer is not used, since application checks space before transmitting
                        if (0x7FFF < ml_curEvent.length) // Ox7FFF = 32767
                        {
                            txEng2timer_setProbeTimer.write(ml_curEvent.sessionID);
                        }

                        // TODO some checking
                        txSar.not_ackd += ml_curEvent.length;

                        txEng2txSar_upd_req.write(txTxSarQuery(ml_curEvent.sessionID, txSar.not_ackd, 1));
                        // This state is always left
                        ml_FsmState = 0;

                        // Send a packet only if there is data or we want to send an empty probing message
                        if (meta.length != 0) // || ml_curEvent.retransmit) //TODO retransmit boolean currently not set,
                                              // should be removed
                        {
                            txEng_ipMetaFifoOut.write(meta.length);
                            txEng_tcpMetaFifoOut.write(meta);
                            txEng_isLookUpFifoOut.write(true);
                            txEng_isDDRbypass.write(true);
                            txEng2sLookup_rev_req.write(ml_curEvent.sessionID);

                            // Only set RT timer if we actually send sth, TODO only set if we change state and sent sth
                            txEng2timer_setRetransmitTimer.write(txRetransmitTimerSet(ml_curEvent.sessionID));
                            stats_tcpOutSegs.write(1);
                        }
                    }

                    break;
#else
                case TX:
                    // Sends everything between txSar.not_ackd and txSar.app
                    if ((!rxSar2txEng_rsp.empty() && !txSar2txEng_upd_rsp.empty()) || ml_sarLoaded) {
                        if (!ml_sarLoaded) {
                            rxSar2txEng_rsp.read(rxSar);
                            txSar2txEng_upd_rsp.read(txSar);
                        }

                        // Compute our space, Advertise at least a quarter/half, otherwise 0
                        meta.ackNumb = rxSar.recvd;
                        meta.seqNumb = txSar.not_ackd;
                        meta.window_size =
                            rxSar.windowSize; // Precalcualted in rx_sar_table ((rxSar.appd - rxSar.recvd) - 1)
                        meta.ack = 1;         // ACK is always set when established
                        meta.rst = 0;
                        meta.syn = 0;
                        meta.fin = 0;
                        meta.length = 0;

                        currLength = (txSar.app - ((ap_uint<WINDOW_BITS>)txSar.not_ackd));
                        // Construct address before modifying txSar.not_ackd
                        ap_uint<32> pkgAddr;
                        pkgAddr(31, 30) = 0x01;
                        pkgAddr(29, WINDOW_BITS) = ml_curEvent.sessionID(13, 0);
                        pkgAddr(WINDOW_BITS - 1, 0) = txSar.not_ackd(WINDOW_BITS - 1, 0); // ml_curEvent.address;

                        // Check length, if bigger than Usable Window or MMS
                        // precomputed in txSar Table: usableWindow = (min(recv_window, cong_windwo) < usedLength ?
                        // (min(recv_window, cong_window) - usedLength) : 0)
                        if (currLength <= txSar.usableWindow) {
                            if (currLength >= MSS) // TODO change to >= MSS, use maxSegmentCount
                            {
                                // We stay in this state and sent immediately another packet
                                txSar.not_ackd += MSS;
                                meta.length = MSS;
                            } else {
                                // If we sent all data, there might be a fin we have to sent too
                                if (txSar.finReady && (txSar.ackd == txSar.not_ackd || currLength == 0)) {
                                    ml_curEvent.type = FIN;
                                } else {
                                    ml_FsmState = 0;
                                }
                                // Check if small segment and if unacknowledged data in pipe (Nagle)
                                if (txSar.ackd == txSar.not_ackd)
                                {
                                    txSar.not_ackd += currLength;
                                    meta.length = currLength;
                                }
                                else {
                                    txEng2timer_setProbeTimer.write(ml_curEvent.sessionID);
                                }
                                // Write back txSar not_ackd pointer
                                txEng2txSar_upd_req.write(txTxSarQuery(ml_curEvent.sessionID, txSar.not_ackd, 1));
                            }
                        } else {
                            // code duplication, but better timing..
                            if (txSar.usableWindow >= MSS) {
                                // We stay in this state and sent immediately another packet
                                txSar.not_ackd += MSS;
                                meta.length = MSS;
                            } else {
                                // Check if we sent >= MSS data
                                if (txSar.ackd == txSar.not_ackd)
                                {
                                    txSar.not_ackd += txSar.usableWindow;
                                    meta.length = txSar.usableWindow;
                                }
                                // Set probe Timer to try again later
                                txEng2timer_setProbeTimer.write(ml_curEvent.sessionID);
                                txEng2txSar_upd_req.write(txTxSarQuery(ml_curEvent.sessionID, txSar.not_ackd, 1));
                                ml_FsmState = 0;
                            }
                        }

                        if (meta.length != 0) {
                            txBufferReadCmd.write(mmCmd(pkgAddr, meta.length));
                        }
                        // Send a packet only if there is data or we want to send an empty probing message
                        if (meta.length != 0) // || ml_curEvent.retransmit) //TODO retransmit boolean currently not set,
                                              // should be removed
                        {
                            txEng_ipMetaFifoOut.write(meta.length);
                            txEng_tcpMetaFifoOut.write(meta);
                            txEng_isLookUpFifoOut.write(true);
                            txEng2sLookup_rev_req.write(ml_curEvent.sessionID);

                            // Only set RT timer if we actually send sth, TODO only set if we change state and sent sth
                            txEng2timer_setRetransmitTimer.write(txRetransmitTimerSet(ml_curEvent.sessionID));
                            stats_tcpOutSegs.write(1);
                        } // TODO if probe send msg length 1
                        ml_sarLoaded = true;
                    }
                    break;
#endif
                case RT:
                    if ((!rxSar2txEng_rsp.empty() && !txSar2txEng_upd_rsp.empty()) || ml_sarLoaded) {
                        txTxSarReply txSar;
                        if (!ml_sarLoaded) {
                            rxSar2txEng_rsp.read(rxSar);
                            txSar = txSar2txEng_upd_rsp.read();
                        } else {
                            txSar = txSarReg;
                        }

                        // TODO use  usedLengthWithFIN
                        currLength = txSar.usedLength; //((ap_uint<WINDOW_BITS>) txSar.not_ackd - txSar.ackd);

                        meta.ackNumb = rxSar.recvd;
                        meta.seqNumb = txSar.ackd;
                        meta.window_size =
                            rxSar.windowSize; // Precalcualted in rx_sar_table ((rxSar.appd - rxSar.recvd) - 1)
                        meta.ack = 1;         // ACK is always set when session is established
                        meta.rst = 0;
                        meta.syn = 0;
                        meta.fin = 0;

                        // Construct address before modifying txSar.ackd
                        ap_uint<32> pkgAddr;
                        pkgAddr(31, 30) = 0x01;
                        pkgAddr(29, WINDOW_BITS) = ml_curEvent.sessionID(13, 0);
                        pkgAddr(WINDOW_BITS - 1, 0) = txSar.ackd(WINDOW_BITS - 1, 0); // ml_curEvent.address;

                        // Decrease Slow Start Threshold, only on first RT from retransmitTimer
                        if (!ml_sarLoaded && (ml_curEvent.rt_count == 1)) {
                            if (currLength > (4 * MSS)) // max( FlightSize/2, 2*MSS) RFC:5681
                            {
                                slowstart_threshold = currLength / 2;
                            } else {
                                slowstart_threshold = (2 * MSS);
                            }
                            txEng2txSar_upd_req.write(txTxSarRtQuery(ml_curEvent.sessionID, slowstart_threshold));
                        }

                        // Since we are retransmitting from txSar.ackd to txSar.not_ackd, this data is already inside
                        // the usableWindow
                        // => no check is required
                        // Only check if length is bigger than MMS
                        if (currLength > MSS) {
                            // We stay in this state and sent immediately another packet
                            meta.length = MSS;
                            txSar.ackd += MSS;
                            txSar.usedLength -= MSS;
                            // TODO replace with dynamic count, remove this
                            if (ml_segmentCount == 3) {
                                // Should set a probe or sth??
                                // txEng2txSar_upd_req.write(txTxSarQuery(ml_curEvent.sessionID, txSar.not_ackd, 1));
                                ml_FsmState = 0;
                            }
                            ml_segmentCount++;
                        } else {
                            meta.length = currLength;
                            if (txSar.finSent) {
                                ml_curEvent.type = FIN;
                            } else {
                                // set RT here???
                                ml_FsmState = 0;
                            }
                        }

                        // Only send a packet if there is data
                        if (meta.length != 0) {
                            txBufferReadCmd.write(mmCmd(pkgAddr, meta.length));
                            txEng_ipMetaFifoOut.write(meta.length);
                            txEng_tcpMetaFifoOut.write(meta);
                            txEng_isLookUpFifoOut.write(true);
#if (TCP_NODELAY)
                            txEng_isDDRbypass.write(false);
#endif
                            txEng2sLookup_rev_req.write(ml_curEvent.sessionID);

                            // Only set RT timer if we actually send sth
                            txEng2timer_setRetransmitTimer.write(txRetransmitTimerSet(ml_curEvent.sessionID));
                            stats_tcpRetransSegs.write(1);
                        }
                        ml_sarLoaded = true;
                        txSarReg = txSar;
                    }
                    break;
                case ACK:
                case ACK_NODELAY:
                    if (!rxSar2txEng_rsp.empty() && !txSar2txEng_upd_rsp.empty()) {
                        rxSar2txEng_rsp.read(rxSar);
                        txTxSarReply txSar = txSar2txEng_upd_rsp.read();

                        meta.ackNumb = rxSar.recvd;
                        meta.seqNumb = txSar.not_ackd; // Always send SEQ
                        meta.window_size =
                            rxSar.windowSize; // Precalcualted in rx_sar_table ((rxSar.appd - rxSar.recvd) - 1)
                        meta.length = 0;
                        meta.ack = 1;
                        meta.rst = 0;
                        meta.syn = 0;
                        meta.fin = 0;
                        txEng_ipMetaFifoOut.write(meta.length);
                        txEng_tcpMetaFifoOut.write(meta);
                        txEng_isLookUpFifoOut.write(true);
                        txEng2sLookup_rev_req.write(ml_curEvent.sessionID);
                        ml_FsmState = 0;
                    }
                    break;
                case SYN:
                    if (((ml_curEvent.rt_count != 0) && !txSar2txEng_upd_rsp.empty()) || (ml_curEvent.rt_count == 0)) {
                        txTxSarReply txSar;
                        if (ml_curEvent.rt_count != 0) {
                            txSar = txSar2txEng_upd_rsp.read();
                            meta.seqNumb = txSar.ackd;
                        } else {
                            txSar.not_ackd = ml_randomValue; // FIXME better rand()
                            ml_randomValue = (ml_randomValue * 8) xor ml_randomValue;
                            meta.seqNumb = txSar.not_ackd;
                            txEng2txSar_upd_req.write(
                                txTxSarQuery(ml_curEvent.sessionID, txSar.not_ackd + 1, 1, 1, true));
                        }
                        meta.ackNumb = 0;
                        meta.window_size = 0xFFFF;
                        meta.length = 4; // For MSS Option, 4 bytes
                        meta.ack = 0;
                        meta.rst = 0;
                        meta.syn = 1;
                        meta.fin = 0;
#if (WINDOW_SCALE)
                        meta.length = 8;
                        meta.win_shift = WINDOW_SCALE_BITS;
#endif

                        txEng_ipMetaFifoOut.write(meta.length);
                        txEng_tcpMetaFifoOut.write(meta);
                        txEng_isLookUpFifoOut.write(true);
                        txEng2sLookup_rev_req.write(ml_curEvent.sessionID);
                        // set retransmit timer
                        txEng2timer_setRetransmitTimer.write(txRetransmitTimerSet(ml_curEvent.sessionID, SYN));
                        ml_FsmState = 0;
                    }
                    break;
                case SYN_ACK:
                    if (!rxSar2txEng_rsp.empty() && !txSar2txEng_upd_rsp.empty()) {
                        rxSar2txEng_rsp.read(rxSar);
                        txTxSarReply txSar = txSar2txEng_upd_rsp.read();

                        // construct SYN_ACK message
                        meta.ackNumb = rxSar.recvd;
                        meta.window_size = 0xFFFF;
                        meta.length = 4; // For MSS Option, 4 bytes
                        meta.ack = 1;
                        meta.rst = 0;
                        meta.syn = 1;
                        meta.fin = 0;
                        if (ml_curEvent.rt_count != 0) {
                            meta.seqNumb = txSar.ackd;
                        } else {
                            // Catch a simulaneous open. In this case we need to reuse the sequence number.
                            if (!txSar.synSent) {
                                txSar.not_ackd = ml_randomValue; // FIXME better rand();
                                ml_randomValue = (ml_randomValue * 8) xor ml_randomValue;
                            }
                            meta.seqNumb = txSar.not_ackd;
                            txEng2txSar_upd_req.write(
                                txTxSarQuery(ml_curEvent.sessionID, txSar.not_ackd + 1, 1, 1, false));
                        }

#if (WINDOW_SCALE)
                        // An additional 4 bytes for WScale option -> total: 8 bytes
                        if (rxSar.win_shift != 0) {
                            meta.length = 8;
                        }
                        meta.win_shift = rxSar.win_shift;
#endif

                        txEng_ipMetaFifoOut.write(meta.length);
                        txEng_tcpMetaFifoOut.write(meta);
                        txEng_isLookUpFifoOut.write(true);
                        txEng2sLookup_rev_req.write(ml_curEvent.sessionID);

                        // set retransmit timer
                        txEng2timer_setRetransmitTimer.write(txRetransmitTimerSet(ml_curEvent.sessionID, SYN_ACK));
                        ml_FsmState = 0;
                    }
                    break;
                case FIN:
                    if ((!rxSar2txEng_rsp.empty() && !txSar2txEng_upd_rsp.empty()) || ml_sarLoaded) {
                        txTxSarReply txSar;
                        if (!ml_sarLoaded) {
                            rxSar2txEng_rsp.read(rxSar);
                            txSar = txSar2txEng_upd_rsp.read();
                        } else {
                            txSar = txSarReg;
                        }

                        // construct FIN message
                        meta.ackNumb = rxSar.recvd;
                        // meta.seqNumb = txSar.not_ackd;
                        meta.window_size =
                            rxSar.windowSize; // Precalcualted in rx_sar_table ((rxSar.appd - rxSar.recvd) - 1)
                        meta.length = 0;
                        meta.ack = 1; // has to be set for FIN message as well
                        meta.rst = 0;
                        meta.syn = 0;
                        meta.fin = 1;

                        // Check if retransmission, in case of RT, we have to reuse not_ackd number
                        if (ml_curEvent.rt_count != 0) {
                            meta.seqNumb = txSar.not_ackd - 1; // Special case, or use ackd?
                        } else {
                            meta.seqNumb = txSar.not_ackd;
// Check if all data is sent, otherwise we have to delay FIN message
// Set fin flag, such that probeTimer is informed
#if !(TCP_NODELAY)
                            if (txSar.app == txSar.not_ackd(WINDOW_BITS - 1, 0))
#endif
                            {
                                txEng2txSar_upd_req.write(
                                    txTxSarQuery(ml_curEvent.sessionID, txSar.not_ackd + 1, 1, 0, false, true, true));
                            }
#if !(TCP_NODELAY)
                            else {
                                txEng2txSar_upd_req.write(
                                    txTxSarQuery(ml_curEvent.sessionID, txSar.not_ackd, 1, 0, false, true, false));
                            }
#endif
                        }

// Check if there is a FIN to be sent //TODO maybe restruce this
#if !(TCP_NODELAY)
                        if (meta.seqNumb(WINDOW_BITS - 1, 0) == txSar.app)
#endif
                        {
                            txEng_ipMetaFifoOut.write(meta.length);
                            txEng_tcpMetaFifoOut.write(meta);
                            txEng_isLookUpFifoOut.write(true);
                            txEng2sLookup_rev_req.write(ml_curEvent.sessionID);
                            // set retransmit timer
                            // txEng2timer_setRetransmitTimer.write(txRetransmitTimerSet(ml_curEvent.sessionID, FIN));
                            txEng2timer_setRetransmitTimer.write(txRetransmitTimerSet(ml_curEvent.sessionID));
                        }

                        ml_FsmState = 0;
                    }
                    break;
                case RST_ACK:
                    // Assumption RST length == 0
                    resetAckEvent = ml_curEvent;
                    if (!resetAckEvent.hasSessionID()) {
                        txEng_ipMetaFifoOut.write(0);
                        txEng_tcpMetaFifoOut.write(tx_engine_meta(0, resetAckEvent.getSeqNumb(), 1, 1, 0, 0));
                        txEng_isLookUpFifoOut.write(false);
                        txEng_tupleShortCutFifoOut.write(ml_curEvent.tuple);
                        ml_FsmState = 0;
                    } else if (!txSar2txEng_upd_rsp.empty()) {
                        txTxSarReply txSar = txSar2txEng_upd_rsp.read();
                        txEng_ipMetaFifoOut.write(0);
                        txEng_isLookUpFifoOut.write(true);
                        txEng2sLookup_rev_req.write(resetAckEvent.sessionID); // there is no sessionID??
                        txEng_tcpMetaFifoOut.write(tx_engine_meta(txSar.not_ackd, resetAckEvent.getSeqNumb(), 1, 1, 0, 0));
                        ml_FsmState = 0;
                    }
                    break;
                case RST:
                    resetEvent = ml_curEvent;
                    if (!resetEvent.hasSessionID()) {
                        txEng_ipMetaFifoOut.write(0);
                        // Sent RST only. This is sent in reponse to a segment with no sessionID. SEQ = SEG.ACK
                        txEng_tcpMetaFifoOut.write(tx_engine_meta(resetEvent.getAckNumb(), 0, 0, 1, 0, 0));
                        txEng_isLookUpFifoOut.write(false);
                        txEng_tupleShortCutFifoOut.write(ml_curEvent.tuple);
                        ml_FsmState = 0;                    
                    } else if (!txSar2txEng_upd_rsp.empty()) {
                        txTxSarReply txSar = txSar2txEng_upd_rsp.read();
                        txEng_ipMetaFifoOut.write(0);
                        txEng_isLookUpFifoOut.write(true);
                        txEng2sLookup_rev_req.write(resetEvent.sessionID);
                        txEng_tcpMetaFifoOut.write(tx_engine_meta(resetEvent.getAckNumb(), 0, 0, 1, 0, 0));
                        ml_FsmState = 0;                        
                    }
            } // switch
            break;
    } // switch
}

/** @ingroup tx_engine
 *  Forwards the incoming tuple from the SmartCam or RX Engine to the 2 header construction modules
 *  @param[in]	sLookup2txEng_rev_rsp
 *  @param[in]	txEng_tupleShortCutFifoIn
 *  @param[in]	txEng_isLookUpFifoIn
 *  @param[out]	txEng_ipTupleFifoOut
 *  @param[out]	txEng_tcpTupleFifoOut
 */
void tupleSplitter(hls::stream<fourTuple>& sLookup2txEng_rev_rsp,
                   hls::stream<fourTuple>& txEng_tupleShortCutFifoIn,
                   hls::stream<bool>& txEng_isLookUpFifoIn,
                   hls::stream<twoTuple>& txEng_ipTupleFifoOut,
                   hls::stream<fourTuple>& txEng_tcpTupleFifoOut) {
//#pragma HLS INLINE off
#pragma HLS pipeline II = 1
    static bool ts_getMeta = true;
    static bool ts_isLookUp;

    fourTuple tuple;

    if (ts_getMeta) {
        if (!txEng_isLookUpFifoIn.empty()) {
            txEng_isLookUpFifoIn.read(ts_isLookUp);
            ts_getMeta = false;
        }
    } else {
        if (!sLookup2txEng_rev_rsp.empty() && ts_isLookUp) {
            sLookup2txEng_rev_rsp.read(tuple);
            txEng_ipTupleFifoOut.write(twoTuple(tuple.srcIp, tuple.dstIp));
            txEng_tcpTupleFifoOut.write(tuple);
            ts_getMeta = true;
        } else if (!txEng_tupleShortCutFifoIn.empty() && !ts_isLookUp) {
            txEng_tupleShortCutFifoIn.read(tuple);
            txEng_ipTupleFifoOut.write(twoTuple(tuple.srcIp, tuple.dstIp));
            txEng_tcpTupleFifoOut.write(tuple);
            ts_getMeta = true;
        }
    }
}

/** @ingroup tx_engine
 * 	generate_ipv4
 *  TODO - import and use general Ipv4 command
 *  @param[in]		txEng_ipMetaDataFifoIn
 *  @param[in]      txEng_ipTupleFifoIn
 *  @param[in]      tx_shift2ipv4Fifo
 *  @param[in]      tx_checksumFifo
 *  @param[out]     m_axis_tx_data
 */
template <int WIDTH>
void generate_ipv4(
    hls::stream<ap_uint<16> >& txEng_ipMetaDataFifoIn,
    hls::stream<twoTuple>& txEng_ipTupleFifoIn,
    hls::stream<net_axis<WIDTH> >& tx_shift2ipv4Fifo,
    hls::stream<bool>& tx_checksumFifo,
    hls::stream<ap_axiu<WIDTH,0,0,0> >& m_axis_tx_data)
{
#pragma HLS INLINE off
#pragma HLS pipeline II = 1

    enum fsmStateType { META, HEADER, PARTIAL_HEADER, BODY };
    static fsmStateType gi_state = META;
    static ipv4Header<WIDTH> header;
    ap_axiu<WIDTH,0,0,0> outWord;

    switch (gi_state) {
        case META:
            // Wait for all the FIFOs to have data. This stops the header being sent and then stalling the pipeline
            // while the checksum is calculated
            if (!txEng_ipMetaDataFifoIn.empty() && !txEng_ipTupleFifoIn.empty() && !tx_checksumFifo.empty()) {
                // This is only used to delay until the checksum was calculated so the header isn't sent, then a big
                // wait.
                tx_checksumFifo.read();
                ap_uint<16> metaLength = txEng_ipMetaDataFifoIn.read();
                twoTuple tuples = txEng_ipTupleFifoIn.read();
                header.clear();

                // length = meta.length + 20; //was adding +40
                ap_uint<16> length = metaLength + 40;
                header.setLength(length);
                // Tuple fields are stored in Network Order - Big Endian. Convert to host order 
                // before using the accessor functions in the IPv4 Header
                header.setDstAddr(reverse(tuples.dstIp));
                header.setSrcAddr(reverse(tuples.srcIp));
                header.setProtocol(TCP_PROTOCOL);
                if (IPV4_HEADER_SIZE >= WIDTH) {
                    gi_state = HEADER;
                } else {
                    gi_state = PARTIAL_HEADER;
                }
            }
            break;
        case HEADER: {
            net_axis<WIDTH> sendWord;
            if (header.consumeWord(sendWord.data) < (WIDTH / 8)) {
                /*currWord.keep = 0xFFFFFFFF; //TODO, set as much as required
                currWord.last = 0;
                m_axis_tx_data.write(currWord);*/
                gi_state = PARTIAL_HEADER;
            }
            // else
            {
                sendWord.keep = 0xFFFFFFFF; // TODO, set as much as required
                sendWord.last = 0;
                outWord.data = sendWord.data;
                outWord.keep = sendWord.keep;
                outWord.last = sendWord.last;
                m_axis_tx_data.write(outWord);
                // gi_state = PARTIAL_HEADER;
            }
            break;
        }
        case PARTIAL_HEADER:
            if (!tx_shift2ipv4Fifo.empty()) {
                net_axis<WIDTH> currWord = tx_shift2ipv4Fifo.read();
                header.consumeWord(currWord.data);
                outWord.data = currWord.data;
                outWord.keep = currWord.keep;
                outWord.last = currWord.last;
                m_axis_tx_data.write(outWord);
                gi_state = BODY;

                if (currWord.last) {
                    gi_state = META;
                }
            }
            break;
        case BODY:
            if (!tx_shift2ipv4Fifo.empty()) {
                net_axis<WIDTH> currWord = tx_shift2ipv4Fifo.read();
                outWord.data = currWord.data;
                outWord.keep = currWord.keep;
                outWord.last = currWord.last;
                m_axis_tx_data.write(outWord);
                if (currWord.last) {
                    gi_state = META;
                }
            }
            break;
    }
}

/** @ingroup tx_engine
 * 	Reads the TCP header metadata and the IP tuples. From this data it generates the TCP pseudo header and streams
 * it out.
 *  @param[in]		tcpMetaDataFifoIn
 *  @param[in]      tcpTupleFifoIn
 *  @param[in]      dataIn
 *  @param[out]     dataOut
 */
template <int WIDTH>
void pseudoHeaderConstructionNew(hls::stream<tx_engine_meta>& tcpMetaDataFifoIn,
                                 hls::stream<fourTuple>& tcpTupleFifoIn,
                                 hls::stream<net_axis<WIDTH> >& dataIn,
                                //  hls::stream<net_axis<WIDTH> >& dataOut,
								 hls::stream<net_axis<WIDTH> >& headerOut,
                                 hls::stream<net_axis<WIDTH> >& payloadOut,
                                 hls::stream<bool >& hasPayload) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    enum fsmState { META, HEADER, PARTIAL_HEADER, BODY, HEADER_MSS_OPTION };
    static fsmState state = META;
    static tcpFullPseudoHeader<WIDTH> header; // size 256Bit
    static bool hasBody = false;
    static bool isSYN = false;
    static ap_uint<4> win_shift = 0;

    switch (state) {
        case META:
            if (!tcpMetaDataFifoIn.empty() && !tcpTupleFifoIn.empty()) {
                tx_engine_meta meta = tcpMetaDataFifoIn.read();
                fourTuple tuple = tcpTupleFifoIn.read();

                header.clear();
                // Tuple fields in NO, header accessor functions are little-endian
                // Convert before using
                header.setSrcAddr(reverse(tuple.srcIp));
                header.setDstAddr(reverse(tuple.dstIp));
                header.setLength(meta.length + 0x14); // +20 bytes for the header without options
                header.setSrcPort(reverse(tuple.srcPort));
                header.setDstPort(reverse(tuple.dstPort));
                header.setSeqNumb(meta.seqNumb);
                header.setAckNumb(meta.ackNumb);
#if !(WINDOW_SCALE)
                header.setDataOffset(0x5 + meta.syn);
#else
                win_shift = meta.win_shift;
                if (meta.syn) {
                    if (meta.win_shift != 0) {
                        header.setDataOffset(0x7);
                    } else {
                        header.setDataOffset(0x6);
                    }
                } else {
                    header.setDataOffset(0x5);
                }
#endif
                // flags
                header.setFinFlag(meta.fin);
                header.setSynFlag(meta.syn);
                header.setRstFlag(meta.rst);
                header.setAckFlag(meta.ack);

                hasBody = meta.length != 0 && !meta.syn;
                hasPayload.write(hasBody);
                isSYN = meta.syn;

                header.setWindowSize(meta.window_size);
                header.setChecksum(0);

                if (WIDTH > 256 && hasBody) {
                    state = PARTIAL_HEADER;
                } else {
                    // TODO handle 512
                    state = HEADER;
                }
            }
            break;
        case HEADER: {
            net_axis<WIDTH> sendWord;
            net_axis<WIDTH> sendHeader;
            sendWord.last = 0;
            sendHeader.last = 0;
            ap_uint<8> remainingLength = header.consumeWord(sendWord.data);
            if (remainingLength < (WIDTH / 8))
            // if (header.consumeWord(sendWord.data) < WIDTH)
            {
                if (hasBody) {
                    sendHeader.last = 1;
                    state = BODY; // PARTIAL_HEADER;
                } else {
                    if (isSYN && WIDTH <= 256) {
                        state = HEADER_MSS_OPTION;
                    } else {
                        sendWord.last = 1;
                        sendHeader.last = 1;
                        state = META;
                    }
                }
            }
            sendWord.keep = 0xffffffff; // Keep for 256bit (size of the header)

            // In case of WIDTH == 512, we can add the MSS option into the first word
            if (isSYN && WIDTH == 512) {
                // MSS negotiation is only used in SYN packets
                sendWord.data(263, 256) = 0x02;                      // Option Kind
                sendWord.data(271, 264) = 0x04;                      // Option length
                sendWord.data(287, 272) = reverse((ap_uint<16>)MSS); // 0xB405; // 0x05B4 = 1460
#ifndef __SYNTHESIS__
                sendWord.data(511, 288) = 0;
#endif
                sendWord.keep(35, 32) = 0xF;
#if (WINDOW_SCALE)
                // WSopt negotiation, only send in SYN-ACK if received with SYN as in RFC 7323 1.3
                if (win_shift != 0) {
                    sendWord.data(295, 288) = 0x03; // Option Kind
                    sendWord.data(303, 296) = 0x03; // Option length
                    sendWord.data(311, 304) = win_shift;
                    sendWord.data(319, 312) = 0x0; // End of option list
                    sendWord.keep(39, 36) = 0xF;
                }
#endif
            }

            sendHeader.data = sendWord.data;
            sendHeader.keep = sendWord.keep;
            headerOut.write(sendHeader);
            break;
        }
        // TODO PARTIAL HEADER only required for 512
        case PARTIAL_HEADER:
            if (!dataIn.empty()) {
                net_axis<WIDTH> currWord = dataIn.read();
                header.consumeWord(currWord.data);
                // dataOut.write(currWord);
                state = BODY;
                if (currWord.last) {
                    state = META;
                }
            }
            break;
        case BODY:
            if (!dataIn.empty()) {
                net_axis<WIDTH> currWord = dataIn.read();
                // dataOut.write(currWord);
                payloadOut.write(currWord);
                if (currWord.last) {
                    state = META;
                }
            }
            break;
        case HEADER_MSS_OPTION: // TODO Rename
        {
            net_axis<WIDTH> sendWord;
            // MSS negotiation is only used in SYN packets
            sendWord.data(7, 0) = 0x02;                        // Option Kind
            sendWord.data(15, 8) = 0x04;                       // Option length
            sendWord.data(31, 16) = reverse((ap_uint<16>)MSS); // 0xB405; // 0x05B4 = 1460
            sendWord.keep = 0x0F;
#ifndef __SYNTHESIS__
            sendWord.data(63, 32) = 0;
#endif
#if (WINDOW_SCALE)
            // WSopt negotiation, only send in SYN-ACK if received with SYN as in RFC 7323 1.3
            if (win_shift != 0) {
                sendWord.data(39, 32) = 0x03; // Option Kind
                sendWord.data(47, 40) = 0x03; // Option length
                sendWord.data(55, 48) = win_shift;
                sendWord.data(63, 56) = 0x0; // End of option list
                sendWord.keep = 0xFF;
            }
#endif
            sendWord.last = 1;
            headerOut.write(sendWord);
            // dataOut.write(sendWord);
            state = META;
            break;
        } // HEADER_LAST
    }     // switch
}

/** @ingroup tx_engine
 * If the payload had to break into two DDR commands, it it concatenated by this module
 *  @param[in]		memAccessBreakdown2readPkgStitcher
 *  @param[in]      readDataIn
 *  @param[out]     readDataOut
 */
template <int WIDTH>
void read_data_stitching(hls::stream<bool>& memAccessBreakdown2readPkgStitcher,
                         hls::stream<ap_axiu<WIDTH,0,0,0> >& readDataIn,
                         hls::stream<net_axis<WIDTH> >& readDataOut) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static net_axis<WIDTH> prevWord;
    static bool pkgNeedsMerge = false;
    enum fsmStateType { IDLE, FIRST_PART, STITCH, ATTACH_ALIGNED, RESIDUE };
    static fsmStateType state = IDLE;
    static ap_uint<8> offset = 0;

    switch (state) {
        case IDLE:
            if (!memAccessBreakdown2readPkgStitcher.empty() && !readDataIn.empty()) {
                memAccessBreakdown2readPkgStitcher.read(pkgNeedsMerge);
                ap_axiu<WIDTH,0,0,0> inWord = readDataIn.read();
                net_axis<WIDTH> currWord;
                currWord.data = inWord.data;
                currWord.keep = inWord.keep;
                currWord.last = inWord.last;                

                offset = keepToLen(currWord.keep);

                // Check if packet has only 1 word
                if (currWord.last) {
                    if (pkgNeedsMerge) {
                        currWord.last = 0;
                        // Check if next packet has to be aligned or not
                        if (currWord.keep[WIDTH / 8 - 1] == 0) {
                            // currWord is stored in prevWord
                            state = STITCH;
                        } else {
                            readDataOut.write(currWord);
                            state = ATTACH_ALIGNED;
                        }

                    } else // packet does not have to be merged
                    {
                        readDataOut.write(currWord);
                        // We remain in this state
                    }

                } else // packet contains more than 1 word
                {
                    readDataOut.write(currWord);
                    state = FIRST_PART;
                }
                prevWord = currWord;
            }
            break;
        case FIRST_PART: // This state outputs the all the data words in the 1st memory access of a segment but the 1st
                         // one.
            if (!readDataIn.empty()) {
            	ap_axiu<WIDTH,0,0,0> inWord = readDataIn.read();
                net_axis<WIDTH> currWord;
                currWord.data = inWord.data;
                currWord.keep = inWord.keep;
                currWord.last = inWord.last;                
                offset = keepToLen(currWord.keep);

                // Check if end of packet
                if (currWord.last) {
                    if (pkgNeedsMerge) {
                        currWord.last = 0;
                        // Check if next packet has to be aligned or not
                        if (currWord.keep[WIDTH / 8 - 1] == 0) {
                            // currWord is stored in prevWord
                            state = STITCH;
                        } else {
                            readDataOut.write(currWord);
                            state = ATTACH_ALIGNED;
                        }

                    } else // packet does not have to be merged
                    {
                        readDataOut.write(currWord);
                        state = IDLE;
                    }

                } else {
                    readDataOut.write(currWord);
                    // Remain in this state until last
                }
                prevWord = currWord;
            }
            break;
        case ATTACH_ALIGNED: // This state handles 2nd mem.access data when no realignment is required
            if (!readDataIn.empty()) {
            	ap_axiu<WIDTH,0,0,0> inWord = readDataIn.read();
                net_axis<WIDTH> currWord;
                currWord.data = inWord.data;
                currWord.keep = inWord.keep;
                currWord.last = inWord.last;    
                readDataOut.write(currWord);
                if (currWord.last) {
                    state = IDLE;
                }
            }
            break;
        case STITCH:
            if (!readDataIn.empty()) {
                net_axis<WIDTH> sendWord;
                sendWord.last = 0;
                ap_axiu<WIDTH,0,0,0> inWord = readDataIn.read();
                net_axis<WIDTH> currWord;
                currWord.data = inWord.data;
                currWord.keep = inWord.keep;
                currWord.last = inWord.last;    

                // Create new word consisting of current and previous word
                // offset specifies how many bytes of prevWord are valid
                sendWord.data((offset * 8) - 1, 0) = prevWord.data((offset * 8) - 1, 0);
                sendWord.keep(offset - 1, 0) = prevWord.keep(offset - 1, 0);

                sendWord.data(WIDTH - 1, (offset * 8)) = currWord.data(WIDTH - (offset * 8) - 1, 0);
                sendWord.keep(WIDTH / 8 - 1, offset) = currWord.keep(WIDTH / 8 - offset - 1, 0);
                sendWord.last = (currWord.keep[WIDTH / 8 - offset] == 0);

                readDataOut.write(sendWord);
                prevWord.data((offset * 8) - 1, 0) = currWord.data(WIDTH - 1, WIDTH - (offset * 8));
                prevWord.keep(offset - 1, 0) = currWord.keep(WIDTH / 8 - 1, WIDTH / 8 - offset);
                if (currWord.last) {
                    state = IDLE;
                    if (!sendWord.last) {
                        state = RESIDUE;
                    }
                }
            }
            break;
        case RESIDUE: {
            net_axis<WIDTH> sendWord;
            sendWord.data((offset * 8) - 1, 0) = prevWord.data((offset * 8) - 1, 0);
            sendWord.keep(offset - 1, 0) = prevWord.keep(offset - 1, 0);

#ifndef __SYNTHESIS__
            sendWord.data(WIDTH - 1, (offset * 8)) = 0;
#endif
            sendWord.keep(WIDTH / 8 - 1, offset) = 0;
            sendWord.last = 1;

            readDataOut.write(sendWord);
            state = IDLE;
        } break;
    }
}

// TODO call function only when NODELAY
/** @ingroup read_data_arbiter
 *
 * in TCP_NODELAY forwards on packets depending on txEng_isDDRbypass
 *
 *  @param[in]		txBufferReadData
 *  @param[in]      txEng_isDDRbypass
 *  @param[out]     txApp2txEng_data_stream
 *  @param[out]     txEng_tcpSegOut
 */
template <int WIDTH>
void read_data_arbiter(hls::stream<net_axis<WIDTH> >& txBufferReadData,
#if (TCP_NODELAY)
                       hls::stream<bool>& txEng_isDDRbypass,
                       hls::stream<net_axis<WIDTH> >& txApp2txEng_data_stream,
#endif
                       hls::stream<net_axis<WIDTH> >& txEng_tcpSegOut) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static ap_uint<2> tps_state = 0; // TODO rename

    switch (tps_state) {
        case 0:
#if (TCP_NODELAY)
            if (!txEng_isDDRbypass.empty()) {
                bool isBypass = txEng_isDDRbypass.read();
                if (isBypass) {
                    tps_state = 2;
                } else {
                    tps_state = 1;
                }
            }
#else
            tps_state = 1;
#endif
            break;
        case 1:
            if (!txBufferReadData.empty()) {                
                net_axis<WIDTH> currWord = txBufferReadData.read();
                txEng_tcpSegOut.write(currWord);
                if (currWord.last) {
                    tps_state = 0;
                }
            }
            break;
#if (TCP_NODELAY)
        case 2:
            if (!txApp2txEng_data_stream.empty()) {
                net_axis<WIDTH> currWord = txApp2txEng_data_stream.read();
                txEng_tcpSegOut.write(currWord);
                if (currWord.last) {
                    tps_state = 0;
                }
            }
            break;
#endif
    } // switch
}

/** @ingroup txEngMemAccessBreakdown
 *
 * Breaks a memory mover command into two if the address will wrap.
 *
 *  @param[in]		inputMemAccess
 *  @param[out]     outputMemAccess
 *  @param[out]     memAccessBreakdown2txPkgStitcher
 */
void txEngMemAccessBreakdown(hls::stream<mmCmd>& inputMemAccess,
                             hls::stream<ap_axiu<128,0,0,0> >& outputMemAccess,
                             hls::stream<bool>& memAccessBreakdown2txPkgStitcher) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static ap_uint<1> txEngBreakdownState = 0;
    static mmCmd cmd;
    static ap_uint<WINDOW_BITS> lengthFirstAccess;

    switch (txEngBreakdownState) {
        case 0:
            if (!inputMemAccess.empty()) {
                inputMemAccess.read(cmd);
                std::cout << "TX read cmd: " << cmd.saddr << ", " << cmd.bbt << std::endl;
                if ((cmd.saddr(WINDOW_BITS - 1, 0) + cmd.bbt) > BUFFER_SIZE) {
                    lengthFirstAccess = BUFFER_SIZE - cmd.saddr;

                    memAccessBreakdown2txPkgStitcher.write(true);
                    outputMemAccess.write(mmCmd2axiu2(mmCmd(cmd.saddr, lengthFirstAccess)));
                    txEngBreakdownState = 1;
                } else {
                    memAccessBreakdown2txPkgStitcher.write(false);
                    outputMemAccess.write(mmCmd2axiu2(cmd));
                }
            }
            break;
        case 1:
            outputMemAccess.write(mmCmd2axiu2(mmCmd(0, cmd.bbt - lengthFirstAccess)));
            txEngBreakdownState = 0;
            break;
    }
}

/** @ingroup tx_engine
 *
 * Removes the first word in case the WIDTH == 64
 *  @param[in]		dataIn
 *  @param[out]     dataOut
 */
template <int WIDTH>
void remove_pseudo_header(hls::stream<net_axis<WIDTH> >& headerDataIn, 
                             hls::stream<net_axis<WIDTH> >& dataIn, 
                             hls::stream<bool >& hasPayload,
                             hls::stream<net_axis<WIDTH> >& dataOut
                             ) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off
    enum fsmStateType { PSEUDO, PSEUDO_1, IP_HEADER, BODY, REMAINDER };
    static fsmStateType state = PSEUDO;
    static net_axis<WIDTH> prevWord;
    static bool hasBody = 0;
    net_axis<WIDTH> curWord;
    net_axis<WIDTH> word;

    switch (state) {
        case PSEUDO:
            if (!headerDataIn.empty() && !hasPayload.empty()) {
                hasPayload.read(hasBody);
                //Drop first 64 bits of the Pseudo header
                headerDataIn.read();
                state = PSEUDO_1;             
            }
            break;

        case PSEUDO_1:
            if (!headerDataIn.empty()) {
                headerDataIn.read(prevWord);
                state = IP_HEADER;          
            }
            break;

        case IP_HEADER:
            if (!headerDataIn.empty()) {
                headerDataIn.read(curWord);
                word.data(31, 0) = prevWord.data(63,32);
                word.data(63,32) = curWord.data(31,0);
                word.keep(3,0) = prevWord.keep(7,4);
                word.keep(7,4) = curWord.keep(3,0);
                word.last = 0;
                prevWord = curWord;                
                if (curWord.last == 1) {
                    if (hasBody) {
                        state = BODY;
                    } else {
                        if (curWord.keep > 15) {
                            state = REMAINDER;
                        } else {
                            word.last = 1;
                            state = PSEUDO;
                        }
                    }                    
                }
                dataOut.write(word);
            }
            break;

        case BODY:
            if (!dataIn.empty()) {
                dataIn.read(curWord);
                word.data(31, 0) = prevWord.data(63,32);
                word.keep(3,0) = prevWord.keep(7,4);
                word.data(63,32) = curWord.data(31,0);
                word.keep(7,4) = curWord.keep(3,0);
                if (curWord.last == 1) {
                    if (curWord.keep > 15) {
                        word.last = 0;                        
                        state = REMAINDER;
                    } else {
                        word.last = 1;
                        state = PSEUDO;
                    }                    
                } else {
                    word.last = 0;
                }
                dataOut.write(word);
                prevWord = curWord;                           
            }
            break;

        case REMAINDER:
            word.data(31, 0) = prevWord.data(63,32);
            word.keep(3,0) = prevWord.keep(7,4);
            word.data(63,32) = 0x0000;            
            word.keep(7,4) = 0x0;
            word.last = 1;
            dataOut.write(word);
            state = PSEUDO;
            break;
    }      
}

/** @ingroup tx_engine
 *
 * Writes IP Checksum into appropriate section of data stream
 *  @param[in]		checksumIn
 *  @param[in]		dataIn
 *  @param[out]     dataOut
 */
template <int WIDTH>
void insert_checksum(hls::stream<ap_uint<16> >& checksumIn,
                     hls::stream<net_axis<WIDTH> >& dataIn,
                     hls::stream<net_axis<WIDTH> >& dataOut) {
    static ap_uint<2> state = (WIDTH > 128 ? 1 : 0);
    static ap_uint<3> wordCount = 0;

    switch (state) {
        case 0:
            if (!dataIn.empty()) {
                net_axis<WIDTH> word = dataIn.read();
                dataOut.write(word);
                wordCount++;
                if (wordCount == 128 / WIDTH) {
                    wordCount = 0;
                    state = 1;
                }
            }
            break;
        case 1:
            // Insert checksum
            if (!checksumIn.empty() && !dataIn.empty()) {
                ap_uint<16> checksum = checksumIn.read();
                net_axis<WIDTH> word = dataIn.read();
                if (WIDTH > 128) {
                    word.data(143, 128) = reverse(checksum);
                } else {
                    word.data(15, 0) = reverse(checksum);
                }
                dataOut.write(word);

                state = 2;
                if (word.last) {
                    state = (WIDTH > 128 ? 1 : 0);
                }
            }
            break;
        case 2:
            if (!dataIn.empty()) {
                net_axis<WIDTH> word = dataIn.read();
                dataOut.write(word);
                if (word.last) {
                    state = (WIDTH > 128 ? 1 : 0);
                }
            }
            break;
    }
}

template <int WIDTH, int N>
void tcp_ipv4_checksum(hls::stream<net_axis<WIDTH> >&   headerDataIn,
                       hls::stream<net_axis<WIDTH> >&   headerDataOut,
                       hls::stream<net_axis<WIDTH> >&   dataIn,
                       hls::stream<net_axis<WIDTH> >&   dataOut,
				       hls::stream<ap_uint<16> >&       partialSubSum,
                       hls::stream<ap_uint<1> >&        validSubSum,
                       hls::stream<bool >&              hasPayload,
                       hls::stream<bool >&              hasPayload_dup,
                       hls::stream<ap_uint<16> >&       checkSumFifoOut,
                       hls::stream<bool >&              tcpChecksumEventFifo)
{
   #pragma HLS PIPELINE II=1
   #pragma HLS INLINE off

	static subSums<WIDTH/16> tcts_tcp_sums;
    static ap_uint<3> state = 0;
    static bool hasBody = 0;
    ap_uint<16> preCalcSum;
    static ap_uint<1> validSum=0;
    static subSums<4> payloadSubSum;
    static ap_uint<1> firstWord=0;

    switch (state) {
        case 0:
            if (!hasPayload.empty() && !validSubSum.empty() && !partialSubSum.empty()) {
                hasPayload.read(hasBody);
                hasPayload_dup.write(hasBody);
              	validSubSum.read(validSum);
                partialSubSum.read(preCalcSum);
                if (validSum == 1) {
                    // Initialize Checksum calculation with partial payload sum
                    	tcts_tcp_sums.sum[0] = preCalcSum;
                }
                firstWord = 1;
                state = 1;
            }
            break;

        case 1:
            if (!headerDataIn.empty()) {
	        	net_axis<WIDTH> currWord = headerDataIn.read();
	        	headerDataOut.write(currWord);

	        	for (int i = 0; i < WIDTH/16; i++)	{
                    #pragma HLS UNROLL

	        		ap_uint<16> temp;
	        		if (currWord.keep(i*2+1, i*2) == 0x3) {
	        			temp(7, 0) = currWord.data(i*16+15, i*16+8);
	        			temp(15, 8) = currWord.data(i*16+7, i*16);
	        			tcts_tcp_sums.sum[i] += temp;
	        			tcts_tcp_sums.sum[i] = (tcts_tcp_sums.sum[i] + (tcts_tcp_sums.sum[i] >> 16)) & 0xFFFF;
	        		}
	        		else if (currWord.keep[i*2] == 0x1)	{
	        			temp(7, 0) = 0;
	        			temp(15, 8) = currWord.data(i*16+7, i*16);
	        			tcts_tcp_sums.sum[i] += temp;
	        			tcts_tcp_sums.sum[i] = (tcts_tcp_sums.sum[i] + (tcts_tcp_sums.sum[i] >> 16)) & 0xFFFF;
	        		}
	        	}
	        	if(currWord.last == 1) 	{
                    if (hasBody & !validSum) {
                        state = 2;
                    } else if (hasBody & validSum) {
                    	state = 3;
                    } else {
                        state = 4;
                    }
	        	}       
            }
            break;            
            
        case 2:
	        if (!dataIn.empty()) {
	        	net_axis<WIDTH> currWord = dataIn.read();
	        	dataOut.write(currWord);

	        	for (int i = 0; i < WIDTH/16; i++)	{
                    #pragma HLS UNROLL

	        		ap_uint<16> temp;
	        		if (currWord.keep(i*2+1, i*2) == 0x3) {
	        			temp(7, 0) = currWord.data(i*16+15, i*16+8);
	        			temp(15, 8) = currWord.data(i*16+7, i*16);
	        			tcts_tcp_sums.sum[i] += temp;
	        			tcts_tcp_sums.sum[i] = (tcts_tcp_sums.sum[i] + (tcts_tcp_sums.sum[i] >> 16)) & 0xFFFF;
	        		}
	        		else if (currWord.keep[i*2] == 0x1)	{
	        			temp(7, 0) = 0;
	        			temp(15, 8) = currWord.data(i*16+7, i*16);
	        			tcts_tcp_sums.sum[i] += temp;
	        			tcts_tcp_sums.sum[i] = (tcts_tcp_sums.sum[i] + (tcts_tcp_sums.sum[i] >> 16)) & 0xFFFF;
	        		}
	        	}
	        	if(currWord.last == 1) 	{
                    state = 4;
	        	}
	        }
            break;

        case 3:
	        if (!dataIn.empty()) {
	        	net_axis<WIDTH> currWord = dataIn.read();
	        	dataOut.write(currWord);
	        	if(currWord.last == 1) 	{
                    state = 0;
	        	}
	        }
            if (firstWord) {

                // ----------------------------------------------------------------------------------------------------
                // N >= 4 -> 64bit data
                tcts_tcp_sums.sum[0] += tcts_tcp_sums.sum[2];
                tcts_tcp_sums.sum[1] += tcts_tcp_sums.sum[3];
                tcts_tcp_sums.sum[0] = (tcts_tcp_sums.sum[0] + (tcts_tcp_sums.sum[0] >> 16)) & 0xFFFF;
                tcts_tcp_sums.sum[1] = (tcts_tcp_sums.sum[1] + (tcts_tcp_sums.sum[1] >> 16)) & 0xFFFF;
                tcts_tcp_sums.sum[0] += tcts_tcp_sums.sum[1];
                tcts_tcp_sums.sum[0] = (tcts_tcp_sums.sum[0] + (tcts_tcp_sums.sum[0] >> 16)) & 0xFFFF;
                tcts_tcp_sums.sum[0] = ~tcts_tcp_sums.sum[0];

            	checkSumFifoOut.write(tcts_tcp_sums.sum[0](15, 0));
                // ----------------------------------------------------------------------------------------------------
                tcpChecksumEventFifo.write(true);
	        	for (int i = 0; i < WIDTH/16; i++){
	        		#pragma HLS UNROLL
	        		tcts_tcp_sums.sum[i] = 0;
	        	}
                firstWord = 0;
            }
            break;

            case 4:            
                // ----------------------------------------------------------------------------------------------------
                // N >= 4 -> 64bit data
                tcts_tcp_sums.sum[0] += tcts_tcp_sums.sum[2];
                tcts_tcp_sums.sum[1] += tcts_tcp_sums.sum[3];
                tcts_tcp_sums.sum[0] = (tcts_tcp_sums.sum[0] + (tcts_tcp_sums.sum[0] >> 16)) & 0xFFFF;
                tcts_tcp_sums.sum[1] = (tcts_tcp_sums.sum[1] + (tcts_tcp_sums.sum[1] >> 16)) & 0xFFFF;
                tcts_tcp_sums.sum[0] += tcts_tcp_sums.sum[1];
                tcts_tcp_sums.sum[0] = (tcts_tcp_sums.sum[0] + (tcts_tcp_sums.sum[0] >> 16)) & 0xFFFF;
                tcts_tcp_sums.sum[0] = ~tcts_tcp_sums.sum[0];

            	checkSumFifoOut.write(tcts_tcp_sums.sum[0](15, 0));
                // ----------------------------------------------------------------------------------------------------

                tcpChecksumEventFifo.write(true);
	        	for (int i = 0; i < WIDTH/16; i++){
	        		#pragma HLS UNROLL
	        		tcts_tcp_sums.sum[i] = 0;
	        	}
                state = 0;
                break;         
    }
}

/** @ingroup tx_engine
 * Increments stat counters based on flag FIFOs
 *  @param[in] TCP packets sent FIFO
 *  @param[in] TCP retransmit packets FIFO 
 *  @param[out] stat counter for sent packets
 *  @param[out] stat counter for retransmited packets
 */
void statCounters(hls::stream<ap_uint<1> >& stats_tcpOutSegs_in,
                  hls::stream<ap_uint<1> >& stats_tcpRetransSegs_in,
                  uint32_t& stats_tcpOutSegs,
                  uint32_t& stats_tcpRetransSegs) {
#pragma HLS pipeline II = 1
#pragma HLS INLINE off

    static uint32_t cnt_tcpOutSegs = 0;
    static uint32_t cnt_tcpRetransSegs = 0;

    if (!stats_tcpOutSegs_in.empty()) {
        stats_tcpOutSegs_in.read();
        cnt_tcpOutSegs++;
    }
    if (!stats_tcpRetransSegs_in.empty()) {
        stats_tcpRetransSegs_in.read();
        cnt_tcpRetransSegs++;
    }  
    stats_tcpOutSegs = cnt_tcpOutSegs;
    stats_tcpRetransSegs  = cnt_tcpRetransSegs;
}

/** @ingroup tx_engine
 *  @param[in]		eventEng2txEng_event
 *  @param[in]		rxSar2txEng_upd_rsp
 *  @param[in]		txSar2txEng_upd_rsp
 *  @param[in]		txBufferReadData
 *  @param[in]		sLookup2txEng_rev_rsp
 *  @param[out]		txEng2rxSar_upd_req
 *  @param[out]		txEng2txSar_upd_req
 *  @param[out]		txEng2timer_setRetransmitTimer
 *  @param[out]		txEng2timer_setProbeTimer
 *  @param[out]		txBufferReadCmd
 *  @param[out]		txEng2sLookup_rev_req
 *  @param[out]		ipTxData
 *  @param[out]     readCountFifo
 */
template <int WIDTH>
void tx_engine(hls::stream<extendedEvent>& eventEng2txEng_event,
               hls::stream<rxSarReply>& rxSar2txEng_rsp,
               hls::stream<txTxSarReply>& txSar2txEng_upd_rsp,
               hls::stream<ap_axiu<WIDTH,0,0,0> >& txBufferReadData,
#if (TCP_NODELAY)
               hls::stream<net_axis<WIDTH> >& txApp2txEng_data_stream,
#endif
               hls::stream<fourTuple>& sLookup2txEng_rev_rsp,
               hls::stream<ap_uint<16> >& txEng2rxSar_req,
               hls::stream<txTxSarQuery>& txEng2txSar_upd_req,
               hls::stream<txRetransmitTimerSet>& txEng2timer_setRetransmitTimer,
               hls::stream<ap_uint<16> >& txEng2timer_setProbeTimer,
               hls::stream<ap_axiu<128,0,0,0> >& txBufferReadCmd,
               hls::stream<ap_uint<16> >& txEng2sLookup_rev_req,
               hls::stream<ap_axiu<WIDTH,0,0,0> >& ipTxData,
               hls::stream<ap_uint<1> >& readCountFifo,
               uint32_t& stats_tcpOutSegs,
               uint32_t& stats_tcpRetransSegs) {
#pragma HLS DATAFLOW disable_start_propagation
#pragma HLS INLINE off

    // Memory Read delay around 76 cycles, 10 cycles/packet, so keep meta of at least 8 packets
    static hls::stream<ap_uint<16> > txEng_ipMetaFifo("txEng_ipMetaFifo");
    static hls::stream<tx_engine_meta> txEng_tcpMetaFifo("txEng_tcpMetaFifo");
#pragma HLS stream variable = txEng_ipMetaFifo depth = 32
#pragma HLS stream variable = txEng_tcpMetaFifo depth = 32

    static hls::stream<net_axis<WIDTH> > txBufferReadDataStitched("txBufferReadDAtaStitched");
    static hls::stream<net_axis<WIDTH> > txEng_shift2pseudoFifo("txEng_shift2pseudoFifo");
    static hls::stream<net_axis<WIDTH> > txEng_tcpPkgBuffer0("txEng_tcpPkgBuffer0");
    static hls::stream<net_axis<WIDTH> > txEng_tcpPkgBuffer1("txEng_tcpPkgBuffer1");
    static hls::stream<net_axis<WIDTH> > txEng_tcpPkgBuffer2("txEng_tcpPkgBuffer2");
    static hls::stream<net_axis<WIDTH> > txEng_tcpPkgBuffer3("txEng_tcpPkgBuffer3");

#pragma HLS stream variable = txBufferReadDataStitched depth = 2
#pragma HLS stream variable = txEng_shift2pseudoFifo depth = 2
#pragma HLS stream variable = txEng_tcpPkgBuffer0 depth = 2
#pragma HLS stream variable = txEng_tcpPkgBuffer1 depth = 256 // critical, has to keep complete packet for checksum computation
#pragma HLS stream variable = txEng_tcpPkgBuffer2 depth = 2
#pragma HLS stream variable = txEng_tcpPkgBuffer3 depth = 2


    static hls::stream<ap_uint<16> > txEng_tcpChecksumFifo("txEng_tcpChecksumFifo");
    static hls::stream<bool> txEng_tcpChecksumEventFifo("txEng_tcpChecksumEventFifo");
#pragma HLS stream variable = txEng_tcpChecksumFifo depth = 4
#pragma HLS stream variable = txEng_tcpChecksumEventFifo depth = 5

    static hls::stream<fourTuple> txEng_tupleShortCutFifo("txEng_tupleShortCutFifo");
    static hls::stream<bool> txEng_isLookUpFifo("txEng_isLookUpFifo");
    static hls::stream<twoTuple> txEng_ipTupleFifo("txEng_ipTupleFifo");
    static hls::stream<fourTuple> txEng_tcpTupleFifo("txEng_tcpTupleFifo");
#pragma HLS stream variable = txEng_tupleShortCutFifo depth = 2
#pragma HLS stream variable = txEng_isLookUpFifo depth = 4
#pragma HLS stream variable = txEng_ipTupleFifo depth = 32
#pragma HLS stream variable = txEng_tcpTupleFifo depth = 32

    static hls::stream<mmCmd> txMetaloader2memAccessBreakdown("txMetaloader2memAccessBreakdown");
#pragma HLS stream variable = txMetaloader2memAccessBreakdown depth = 32
    static hls::stream<bool> memAccessBreakdown2txPkgStitcher("memAccessBreakdown2txPkgStitcher");
#pragma HLS stream variable = memAccessBreakdown2txPkgStitcher depth = 32

    static hls::stream<bool> txEng_isDDRbypass("txEng_isDDRbypass");
#pragma HLS stream variable = txEng_isDDRbypass depth = 32

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static hls::stream<net_axis<WIDTH> > txEng_headerOut("txEng_headerOut");
#pragma HLS stream variable = txEng_headerOut depth = 8

    static hls::stream<net_axis<WIDTH> > txEng_headerOut2("txEng_headerOut2");
#pragma HLS stream variable = txEng_headerOut2 depth = 8

    static hls::stream<net_axis<WIDTH> > txEng_payloadBuffer("txEng_payloadBuffer");
#pragma HLS stream variable = txEng_payloadBuffer depth = 16 

    static hls::stream<net_axis<WIDTH> > txEng_payloadBuffer2("txEng_payloadBuffer2");
#pragma HLS stream variable = txEng_payloadBuffer2 depth = 16 

    static hls::stream<bool > hasPayload("hasPayload");
#pragma HLS stream variable = hasPayload depth = 3

    static hls::stream<bool > hasPayload_dup("hasPayload_dup");
#pragma HLS stream variable = hasPayload_dup depth = 2

    static hls::stream<ap_uint<16> > partialSubSum("partialSubSum");
#pragma HLS stream variable = partialSubSum depth = 16

    static hls::stream<ap_uint<1> > validSubSum("validSubSum");
#pragma HLS stream variable = validSubSum depth = 16

    static hls::stream<ap_uint<1> > stats_tcpOutSegs_fifo("stats_tcpOutSegs_fifo");
    static hls::stream<ap_uint<1> > stats_tcpRetransSegs_fifo("stats_tcpRetransSegs_fifo");
#pragma HLS STREAM depth = 8 variable = stats_tcpOutSegs_fifo
#pragma HLS STREAM depth = 8 variable = stats_tcpRetransSegs_fifo
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    metaLoader(eventEng2txEng_event, rxSar2txEng_rsp, txSar2txEng_upd_rsp, txEng2rxSar_req, txEng2txSar_upd_req,
               txEng2timer_setRetransmitTimer, txEng2timer_setProbeTimer, txEng_ipMetaFifo, txEng_tcpMetaFifo,
               txMetaloader2memAccessBreakdown, txEng2sLookup_rev_req, txEng_isLookUpFifo,
#if (TCP_NODELAY)
               txEng_isDDRbypass,
#endif
               txEng_tupleShortCutFifo, readCountFifo, partialSubSum, validSubSum, stats_tcpOutSegs_fifo, stats_tcpRetransSegs_fifo);

    txEngMemAccessBreakdown(txMetaloader2memAccessBreakdown, txBufferReadCmd, memAccessBreakdown2txPkgStitcher);

    tupleSplitter(sLookup2txEng_rev_rsp, txEng_tupleShortCutFifo, txEng_isLookUpFifo, txEng_ipTupleFifo,
                  txEng_tcpTupleFifo);

    // Stitches splitted reads back toghether
    read_data_stitching<WIDTH>(memAccessBreakdown2txPkgStitcher, txBufferReadData, txBufferReadDataStitched);
    // Arbitrates between DRAM and bypass data, concatenas DRAM data if necessary
    read_data_arbiter<WIDTH>(txBufferReadDataStitched,
#if (TCP_NODELAY)
                             txEng_isDDRbypass, txApp2txEng_data_stream,
#endif
                             txEng_tcpPkgBuffer0);

    lshiftWordByOctet<WIDTH, net_axis<WIDTH>, net_axis<WIDTH>, 51>(((TCP_FULL_PSEUDO_HEADER_SIZE % WIDTH) / 8), txEng_tcpPkgBuffer0,
                                 txEng_shift2pseudoFifo);

    pseudoHeaderConstructionNew<WIDTH>(txEng_tcpMetaFifo, txEng_tcpTupleFifo, txEng_shift2pseudoFifo,
                                       txEng_headerOut, txEng_payloadBuffer, hasPayload);    
//---------------------------------------------------------------------------------------------------------------------------
    tcp_ipv4_checksum<WIDTH, WIDTH/16>(txEng_headerOut, txEng_headerOut2, txEng_payloadBuffer, txEng_payloadBuffer2, partialSubSum,
                                       validSubSum,hasPayload, hasPayload_dup, txEng_tcpChecksumFifo, txEng_tcpChecksumEventFifo);

    // Remove pseudoHeader and Combine Header with payload.
    remove_pseudo_header(txEng_headerOut2, txEng_payloadBuffer2, hasPayload_dup, txEng_tcpPkgBuffer1);
//---------------------------------------------------------------------------------------------------------------------------
    insert_checksum<WIDTH>(txEng_tcpChecksumFifo, txEng_tcpPkgBuffer1, txEng_tcpPkgBuffer2);

    lshiftWordByOctet<WIDTH, net_axis<WIDTH>, net_axis<WIDTH>, 52>(((IPV4_HEADER_SIZE % WIDTH) / 8), txEng_tcpPkgBuffer2, txEng_tcpPkgBuffer3);

    generate_ipv4<WIDTH>(txEng_ipMetaFifo, txEng_ipTupleFifo, txEng_tcpPkgBuffer3, txEng_tcpChecksumEventFifo,
                         ipTxData);

    statCounters(stats_tcpOutSegs_fifo, stats_tcpRetransSegs_fifo, stats_tcpOutSegs, stats_tcpRetransSegs);
}

template void tx_engine<DATA_WIDTH>(hls::stream<extendedEvent>& eventEng2txEng_event,
                                    hls::stream<rxSarReply>& rxSar2txEng_rsp,
                                    hls::stream<txTxSarReply>& txSar2txEng_upd_rsp,
                                    hls::stream<ap_axiu<DATA_WIDTH,0,0,0> >& txBufferReadData,
#if (TCP_NODELAY)
                                    hls::stream<net_axis<DATA_WIDTH> >& txApp2txEng_data_stream,
#endif
                                    hls::stream<fourTuple>& sLookup2txEng_rev_rsp,
                                    hls::stream<ap_uint<16> >& txEng2rxSar_req,
                                    hls::stream<txTxSarQuery>& txEng2txSar_upd_req,
                                    hls::stream<txRetransmitTimerSet>& txEng2timer_setRetransmitTimer,
                                    hls::stream<ap_uint<16> >& txEng2timer_setProbeTimer,
                                    hls::stream<ap_axiu<128,0,0,0> >& txBufferReadCmd,
                                    hls::stream<ap_uint<16> >& txEng2sLookup_rev_req,
                                    hls::stream<ap_axiu<DATA_WIDTH,0,0,0> >& ipTxData,
                                    hls::stream<ap_uint<1> >& readCountFifo,
                                    uint32_t& stats_tcpOutSegs,
                                    uint32_t& stats_tcpRetransSegs);