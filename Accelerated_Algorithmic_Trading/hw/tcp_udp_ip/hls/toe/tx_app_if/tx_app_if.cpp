/*
 * Copyright (c) 2016, 2020, Xilinx, Inc.
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

#include "tx_app_if.hpp"

// Multicast and broadcast addresses are not valid targets
bool isValidDstIp(ap_uint<32> addr) {
#pragma HLS inline
    // Check for multicast
    if (addr(7, 4) == 0xE) {
        return false;
    }
    // Check for broadcast
    if (addr == BROADCAST_ADDR) {
        return false;
    }
    return true;
}

/** @ingroup tx_engine
 * Converts from mmCmd structure to ap_axi<128,0,0,0>
 */
ap_axiu<32,0,0,0> openStatus2axiu(openStatus inword) {
#pragma HLS inline
    ap_axiu<32,0,0,0> tmp_axiu;
    tmp_axiu.data(15,0) = inword.sessionID;
    tmp_axiu.data(23,16) = inword.success;
    tmp_axiu.data(31,24) = 0;
    tmp_axiu.last = 1;
    tmp_axiu.keep = 0xF;
    return tmp_axiu;
}

/** @ingroup tx_app_if
 * Converts from ap_axi<64,0,0,0> to native ipTuple structure
 */
ipTuple axiu2ipTuple(ap_axiu<64, 0, 0, 0> inword) {
#pragma HLS inline
    ipTuple tmp_ipTuple;
    tmp_ipTuple.ip_address = inword.data(31, 0);
    tmp_ipTuple.ip_port = inword.data(47, 32);
    return tmp_ipTuple;
}

/** @ingroup tx_app_if
 *  This interface exposes the creation and tear down of connetions to the application.
 *  The IP tuple for a new connection is read from @param appOpenConIn, the interface then
 *  requests a free port number from the @ref port_table and fires a SYN event. Once the connetion
 *  is established it notifies the application through @p appOpenConOut and delivers the Session-ID
 *  belonging to the new connection.
 *  If opening of the connection is not successful this is also indicated through the @p
 *  appOpenConOut.
 *  By sending the Session-ID through @p closeConIn the application can initiate the teardown of
 *  the connection.
 *  @param[in]		appOpenConIn
 *  @param[in]		closeConIn
 *  @param[in]		txAppSessionLookupIn
 *  @param[in]		portTableIn
 *  @param[in]		stateTableIn
 *  @param[in]		conEstablishedIn
 *  @param[out]		appOpenConOut
 *  @param[out]		txAppSessionLookupOut
 *  @param[out]		portTableOut
 *  @param[out]		stateTableOut
 *  @param[out]		eventFifoOut
 *  @TODO reorganize code
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
               ap_uint<32> myIpAddress) {
#pragma HLS INLINE off
#pragma HLS pipeline II = 1

    enum taiFsmStateType { IDLE, CLOSE_CONN };
    static taiFsmStateType tai_fsmState = IDLE;
    static ap_uint<16> tai_closeSessionID;
    static ap_axiu<16,0,0,0> closeConn;
    static bool fInvalidOpen = false;
    
    if (!appOpenConnReq.empty() && !portTable2txApp_port_rsp.empty()) {
        ipTuple server_addr = axiu2ipTuple(appOpenConnReq.read());
        ap_uint<16> freePort = portTable2txApp_port_rsp.read();
        // Implicit creationAllowed <= true
        // Confirm IP address is valid first.
        if (isValidDstIp(reverse(server_addr.ip_address))) {
            txApp2sLookup_req.write(fourTuple(myIpAddress, reverse(server_addr.ip_address), reverse(freePort),
                                              reverse(server_addr.ip_port)));
        } else {
            fInvalidOpen = true;
        }
    }

    switch (tai_fsmState) {
        case IDLE:
            if (!sLookup2txApp_rsp.empty()) {
                // Read session
                sessionLookupReply session = sLookup2txApp_rsp.read();
                // Get session state
                if (session.hit) {
                    txApp2eventEng_setEvent.write(event(SYN, session.sessionID));
                    txApp2stateTable_upd_req.write(stateQuery(session.sessionID, SYN_SENT, 1));
                } else {
                    // Tell application that openConnection failed
                    appOpenConnRsp.write(openStatus2axiu(openStatus(0, false)));
                }
            } else if (!conEstablishedIn.empty()) {
                // Maybe check if we are actually waiting for this one
                appOpenConnRsp.write(openStatus2axiu(conEstablishedIn.read()));
            } else if (!rtTimer2txApp_notification.empty()) {
                appOpenConnRsp.write(openStatus2axiu(rtTimer2txApp_notification.read()));
            } else if (!closeConnReq.empty()) // Close Request
            {
                closeConnReq.read(closeConn);
                tai_closeSessionID = closeConn.data;
                // Check if this is a valid sessionID to avoid aliasing into the state table
                if (tai_closeSessionID < MAX_SESSIONS) {
                    txApp2stateTable_upd_req.write(stateQuery(tai_closeSessionID));
                    tai_fsmState = CLOSE_CONN;
                }
            } else if (fInvalidOpen) {
                appOpenConnRsp.write(openStatus2axiu(openStatus(0, false)));
                fInvalidOpen = false;
            }
            break;
        case CLOSE_CONN:
            if (!stateTable2txApp_upd_rsp.empty()) {
                sessionState state = stateTable2txApp_upd_rsp.read();
                // TODO might add CLOSE_WAIT here???
                if ((state == ESTABLISHED) || (state == FIN_WAIT_2) ||
                    (state == FIN_WAIT_1)) // TODO Why if FIN already SENT
                {
                    txApp2stateTable_upd_req.write(stateQuery(tai_closeSessionID, FIN_WAIT_1, 1));
                    txApp2eventEng_setEvent.write(event(FIN, tai_closeSessionID));
                } else {
                    // Have to release lock
                    txApp2stateTable_upd_req.write(stateQuery(tai_closeSessionID, state, 1));
                }
                tai_fsmState = IDLE;
            }
            break;
    } // switch
}
