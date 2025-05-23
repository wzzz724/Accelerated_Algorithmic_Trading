/************************************************
 * Copyright (c) 2019, Systems Groun, ETH Zurich
 * Copyright (c) 2016, 2020 Xilinx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 ************************************************/

#include "session_lookup_controller.hpp"

/** @ingroup session_lookup_controller
 *
 *
 *  @param[out]		new_id get a new SessionID from the SessionID free list.
 *  @param[in]      fin_id IDs that are released and appended to the SessionID free list.
 */
ap_axiu<128, 0, 0, 0> rtlSessionUpdateRequest2axiu(rtlSessionUpdateRequest inword) {
#pragma HLS inline
	ap_axiu<128, 0, 0, 0> tmp_axiu;
    if (inword.op == INSERT) {
    	tmp_axiu.data(0,0) = 0;
    } else if (inword.op == DELETE) {
    	tmp_axiu.data(0,0) = 1;
    }
    tmp_axiu.data(32,1) = inword.key.theirIp;
    tmp_axiu.data(48,33) = inword.key.myPort;
    tmp_axiu.data(64,49) = inword.key.theirPort;
    tmp_axiu.data(80,65) = inword.value;
    if (inword.source == RX) {
    	tmp_axiu.data(88, 81) = 0;
    } else if (inword.source == TX_APP) {
    	tmp_axiu.data(88, 81) = 1;
    }
    tmp_axiu.data(127, 89) = 0;
    tmp_axiu.last = 1;
    return tmp_axiu;
}

rtlSessionUpdateReply axiu2rtlSessionUpdateReply (ap_axiu<128,0,0,0> inword) {
#pragma HLS inline
	rtlSessionUpdateReply tmp_reply;
	if (inword.data(0,0) == 0) {
		tmp_reply.op = INSERT;
	} else if (inword.data(0,0) == 1) {
		tmp_reply.op = DELETE;
	}
	tmp_reply.key.theirIp = inword.data(32,1);
	tmp_reply.key.myPort = inword.data(48,33);
	tmp_reply.key.theirPort = inword.data(64,49);
	tmp_reply.sessionID = inword.data(80,65);
	tmp_reply.success = inword.data(88,81);
	if (inword.data(96,89) == 0) {
		tmp_reply.source = RX;
	} else if (inword.data(96,89) == 1) {
		tmp_reply.source = TX_APP;
	}
    return tmp_reply;
}

ap_axiu<128, 0, 0, 0> rtlSessionLookupRequest2axiu (rtlSessionLookupRequest inword) {
#pragma HLS inline
	ap_axiu<128, 0, 0, 0> tmp_axiu;
    tmp_axiu.data(31,0) = inword.key.theirIp;
    tmp_axiu.data(47,32) = inword.key.myPort;
    tmp_axiu.data(63,48) = inword.key.theirPort;
    if (inword.source == RX) {
    	tmp_axiu.data(71, 64) = 0;
    } else if (inword.source == TX_APP) {
    	tmp_axiu.data(71, 64) = 1;
    }
    tmp_axiu.data(127, 72) = 0;
	tmp_axiu.last = 1;
	return tmp_axiu;
}

rtlSessionLookupReply axiu2rtlSessionLookupReply (ap_axiu<128, 0, 0, 0> inword) {
#pragma HLS inline
	rtlSessionLookupReply tmp_lookupResp;
	tmp_lookupResp.key.theirIp= inword.data(31,0);
	tmp_lookupResp.key.myPort = inword.data(47,32);
	tmp_lookupResp.key.theirPort = inword.data(63,48);
	tmp_lookupResp.sessionID = inword.data(79,64);
	tmp_lookupResp.hit = inword.data(87,80);
	if (inword.data(95,88) == 0) {
		tmp_lookupResp.source = RX;
	} else if (inword.data(95,88) == 1) {
		tmp_lookupResp.source = TX_APP;
	}
	return tmp_lookupResp;
}

/** @ingroup session_lookup_controller
 *
 *  SessionID manager function. Either writes a returned session ID to the output
 *  stream @ref new_id or allocates a new one up to the MAX_SESSIONS limit.
 *
 *  @param[out]		new_id get a new SessionID from the SessionID free list.
 *  @param[in]      fin_id IDs that are released and appended to the SessionID free list.
 */
void sessionIdManager(hls::stream<ap_uint<14> >& new_id, hls::stream<ap_uint<14> >& fin_id) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static ap_uint<14> counter = 0;
    // clang-format off
    #pragma HLS reset variable = counter
    // clang-format on

    ap_uint<14> sessionID;

    if (!fin_id.empty()) {
        fin_id.read(sessionID);
        new_id.write(sessionID);
    } else if (counter < MAX_SESSIONS) {
        new_id.write(counter);
        counter++;
    }
}

/** @ingroup session_lookup_controller
 *
 *  Handles the Lookup relies from the RTL Lookup Table, if there was no hit,
 *  it checks if the request is allowed to create a new sessionID and does so.
 *  If it is a hit, the reply is forwarded to the corresponding source.
 *  It also handles the replies of the Session Updates [Inserts/Deletes], in case
 *  of insert the response with the new sessionID is replied to the request source.
 *
 *  @param[in]		sessionLookup_rsp
 *  @param[in]      sessionInsert_rsp
 *  @param[in]		rxEng2sLookup_req
 *  @param[in]		txApp2sLookup_req
 *  @param[in]		sessionIdFreeList
 *  @param[out]		sessionLookup_req
 *  @param[out]		sLookup2rxEng_rsp
 *  @param[out]		sLookup2txApp_rsp
 *  @param[out]		sessionInsert_req
 *  @param[out]		reverseTableInsertFifo
 */
void lookupReplyHandler(hls::stream<ap_axiu<128,0,0,0> >&     sessionLookup_rsp,
                        hls::stream<rtlSessionUpdateReply>&   sessionInsert_rsp,
                        hls::stream<sessionLookupQuery>&      rxEng2sLookup_req,
                        hls::stream<fourTuple>&               txApp2sLookup_req,
                        hls::stream<ap_uint<14> >&            sessionIdFreeList,
						hls::stream<ap_axiu<128,0,0,0> >&     sessionLookup_req,
                        hls::stream<sessionLookupReply>&      sLookup2rxEng_rsp,
                        hls::stream<sessionLookupReply>&      sLookup2txApp_rsp,
                        hls::stream<rtlSessionUpdateRequest>& sessionInsert_req,
                        hls::stream<revLupInsert>&            reverseTableInsertFifo) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static hls::stream<threeTupleInternal> slc_insertTuples("slc_insertTuples2");
    // clang-format off
    #pragma HLS STREAM variable = slc_insertTuples depth = 4
    // clang-format on

    static hls::stream<sessionLookupQueryInternal> slc_queryCache("slc_queryCache");
    // clang-format off
    #pragma HLS STREAM variable = slc_queryCache depth = 8
    // clang-format on

    fourTuple toeTuple;
    threeTupleInternal tuple;
    sessionLookupQuery query;
    sessionLookupQueryInternal intQuery;
    rtlSessionLookupReply lupReply;
    rtlSessionUpdateReply insertReply;
    ap_uint<16> sessionID;
    ap_uint<14> freeID = 0;


    enum slcFsmStateType { LUP_REQ, LUP_RSP, UPD_RSP };
    static slcFsmStateType slc_fsmState = LUP_REQ;

    switch (slc_fsmState) {
        case LUP_REQ:
            // Lookup request state
            if (!txApp2sLookup_req.empty()) {
                txApp2sLookup_req.read(toeTuple);
                intQuery.tuple.theirIp = toeTuple.dstIp;
                intQuery.tuple.theirPort = toeTuple.dstPort;
                // intQuery.tuple.myIp = toeTuple.srcIp;
                intQuery.tuple.myPort = toeTuple.srcPort;
                intQuery.allowCreation = true;
                intQuery.source = TX_APP;
                sessionLookup_req.write(rtlSessionLookupRequest2axiu(rtlSessionLookupRequest(intQuery.tuple, intQuery.source)));
                slc_queryCache.write(intQuery);
                slc_fsmState = LUP_RSP;
            } else if (!rxEng2sLookup_req.empty()) {
                rxEng2sLookup_req.read(query);
                intQuery.tuple.theirIp = query.tuple.srcIp;
                intQuery.tuple.theirPort = query.tuple.srcPort;
                // intQuery.tuple.myIp = query.tuple.dstIp;
                intQuery.tuple.myPort = query.tuple.dstPort;
                intQuery.allowCreation = query.allowCreation;
                intQuery.source = RX;
                sessionLookup_req.write(rtlSessionLookupRequest2axiu(rtlSessionLookupRequest(intQuery.tuple, intQuery.source)));
                slc_queryCache.write(intQuery);
                slc_fsmState = LUP_RSP;
            }
            break;
        case LUP_RSP:
            // Lookup response statue
            if (!sessionLookup_rsp.empty() && !slc_queryCache.empty()) {
                lupReply = axiu2rtlSessionLookupReply(sessionLookup_rsp.read());
                slc_queryCache.read(intQuery);
                if (!lupReply.hit && intQuery.allowCreation && !sessionIdFreeList.empty()) {
                    sessionIdFreeList.read(freeID);
                    sessionInsert_req.write(rtlSessionUpdateRequest(intQuery.tuple, freeID, INSERT, lupReply.source));
                    slc_insertTuples.write(intQuery.tuple);
                    slc_fsmState = UPD_RSP;
                } else {
                    if (lupReply.source == RX) {
                        sLookup2rxEng_rsp.write(sessionLookupReply(lupReply.sessionID, lupReply.hit));
                    } else {
                        sLookup2txApp_rsp.write(sessionLookupReply(lupReply.sessionID, lupReply.hit));
                    }
                    slc_fsmState = LUP_REQ;
                }
            }
            break;
        // case UPD_REQ:
        // break;
        case UPD_RSP:
            // Update response state
            if (!sessionInsert_rsp.empty() && !slc_insertTuples.empty()) {
                sessionInsert_rsp.read(insertReply);
                slc_insertTuples.read(tuple);
                // updateReplies.write(sessionLookupReply(insertReply.sessionID, true));
                if (insertReply.source == RX) {
                    sLookup2rxEng_rsp.write(sessionLookupReply(insertReply.sessionID, true));
                } else {
                    sLookup2txApp_rsp.write(sessionLookupReply(insertReply.sessionID, true));
                }
                reverseTableInsertFifo.write(revLupInsert(insertReply.sessionID, tuple));
                slc_fsmState = LUP_REQ;
            }
            break;
    }
}

/**
 * @ingroup session_lookup_controller
 *
 * @param[in]  sessionInsert_req
 * @param[in]  sessionDelete_req
 * @param[out] sessionUpdate_req
 * @param[out] sessionIdFinFifo
 * @param[out] regSessionCount
 */
void updateRequestSender(hls::stream<rtlSessionUpdateRequest>& sessionInsert_req,
                         hls::stream<rtlSessionUpdateRequest>& sessionDelete_req,
                         hls::stream<ap_axiu<128,0,0,0> >& sessionUpdate_req,
                         hls::stream<ap_uint<14> >& sessionIdFinFifo,
                         uint32_t& tcpCurrEstab) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static uint32_t usedSessionIDs = 0;
    rtlSessionUpdateRequest request;

    if (!sessionInsert_req.empty()) {
        sessionUpdate_req.write(rtlSessionUpdateRequest2axiu(sessionInsert_req.read()));
        usedSessionIDs++;    
    } else if (!sessionDelete_req.empty()) {
        sessionDelete_req.read(request);
        sessionUpdate_req.write(rtlSessionUpdateRequest2axiu(request));
        sessionIdFinFifo.write(request.value);
        usedSessionIDs--;
    }
    tcpCurrEstab = usedSessionIDs;    
}

/**
 * @ingroup session_lookup_controller
 *
 * Filter out everything except INSERT operations from the 
 * Update stream. 
 * 
 * @param[in]  sessionUpdate_rsp
 * @param[out] sessionInsert_rsp
 */
void updateReplyHandler(hls::stream<ap_axiu<128,0,0,0> >& sessionUpdate_rsp,
                        hls::stream<rtlSessionUpdateReply>& sessionInsert_rsp)
// stream<ap_uint<14> >&					sessionIdFinFifo)
{
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    rtlSessionUpdateReply upReply;
    threeTupleInternal tuple;

    if (!sessionUpdate_rsp.empty()) {
        upReply = axiu2rtlSessionUpdateReply(sessionUpdate_rsp.read());
        if (upReply.op == INSERT) {
            sessionInsert_rsp.write(upReply);
        }
    }
}

/**
 * @ingroup session_lookup_controller
 *
 * @param[in]  revTableInserts
 * @param[in]  stateTable2sLookup_releaseSession
 * @param[in]  txEng2sLookup_rev_req
 * @param[out] sLookup2portTable_releasePort
 * @param[out] deleteCache
 * @param[out] sLookup2txEng_rev_rsp
 * @param[in]  myIpAddress
 */
void reverseLookupTableInterface(hls::stream<revLupInsert>& revTableInserts,
                                 hls::stream<ap_uint<16> >& stateTable2sLookup_releaseSession,
                                 hls::stream<ap_uint<16> >& txEng2sLookup_rev_req,
                                 hls::stream<ap_uint<16> >& sLookup2portTable_releasePort,
                                 hls::stream<rtlSessionUpdateRequest>& deleteCache,
                                 hls::stream<fourTuple>& sLookup2txEng_rev_rsp,
                                 ap_uint<32> myIpAddress) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static threeTupleInternal reverseLookupTable[MAX_SESSIONS];
    // clang-format off 
    #pragma HLS RESOURCE variable = reverseLookupTable core = RAM_T2P_BRAM
    #pragma HLS DEPENDENCE variable = reverseLookupTable inter false
    // cang-format on
    static bool tupleValid[MAX_SESSIONS];
    // clang-format off
    #pragma HLS DEPENDENCE variable = tupleValid inter false
    // clang-format on

    revLupInsert insert;
    fourTuple toeTuple;
    threeTupleInternal releaseTuple;
    ap_uint<16> sessionID;

    if (!revTableInserts.empty()) {
        revTableInserts.read(insert);
        reverseLookupTable[insert.key] = insert.value;
        tupleValid[insert.key] = true;

    }
    // TODO check if else if necessary
    else if (!stateTable2sLookup_releaseSession.empty()) {
        stateTable2sLookup_releaseSession.read(sessionID);
        releaseTuple = reverseLookupTable[sessionID];
        if (tupleValid[sessionID]) // if valid
        {
            // Session Lookup Controller expects little endian order,
            // Tuple stores the port number in Big Endian format.
            sLookup2portTable_releasePort.write(reverse(releaseTuple.myPort));
            deleteCache.write(rtlSessionUpdateRequest(releaseTuple, sessionID, DELETE, RX));
        }
        tupleValid[sessionID] = false;
    } else if (!txEng2sLookup_rev_req.empty()) {
        txEng2sLookup_rev_req.read(sessionID);
        toeTuple.srcIp = myIpAddress;
        toeTuple.dstIp = reverseLookupTable[sessionID].theirIp;
        toeTuple.srcPort = reverseLookupTable[sessionID].myPort;
        toeTuple.dstPort = reverseLookupTable[sessionID].theirPort;
        sLookup2txEng_rev_rsp.write(toeTuple);
    }
}

/**
 * @ingroup session_lookup_controller
 *
 * @param[in]  rxEng2sLookup_req
 * @param[out] sLookup2rxEng_rsp
 * @param[in]  stateTable2sLookup_releaseSession
 * @param[out] sLookup2portTable_releasePort
 * @param[in]  txApp2sLookup_req
 * @param[out] sLookup2txApp_rsp
 * @param[in]  txEng2sLookup_rev_req
 * @param[out] sLookup2txEng_rev_rsp
 * @param[in]  sessionLookup_req
 * @param[in]  sessionLookup_rsp
 * @param[in]  sessionUpdate_req
 * @param[out] sessionUpdate_rsp
 * @param[out] regSessionCount
 * @param[in]  myIpAddress
 */
void session_lookup_controller(hls::stream<sessionLookupQuery>& rxEng2sLookup_req,
                               hls::stream<sessionLookupReply>& sLookup2rxEng_rsp,
                               hls::stream<ap_uint<16> >& stateTable2sLookup_releaseSession,
                               hls::stream<ap_uint<16> >& sLookup2portTable_releasePort,
                               hls::stream<fourTuple>& txApp2sLookup_req,
                               hls::stream<sessionLookupReply>& sLookup2txApp_rsp,
                               hls::stream<ap_uint<16> >& txEng2sLookup_rev_req,
                               hls::stream<fourTuple>& sLookup2txEng_rev_rsp,
							   hls::stream<ap_axiu<128,0,0,0> >& sessionLookup_req,
                               hls::stream<ap_axiu<128,0,0,0> >& sessionLookup_rsp,
                               hls::stream<ap_axiu<128,0,0,0> >& sessionUpdate_req,
                               hls::stream<ap_axiu<128,0,0,0> >& sessionUpdate_rsp,
                               uint32_t& tcpCurrEstab,
                               ap_uint<32> myIpAddress) {
#pragma HLS INLINE

    static hls::stream<ap_uint<14> > slc_sessionIdFreeList("slc_sessionIdFreeList");
    static hls::stream<ap_uint<14> > slc_sessionIdFinFifo("slc_sessionIdFinFifo");
    // clang-format off
    SET_FIFO_DEPTH(slc_sessionIdFreeList, MAX_SESSIONS)
    #pragma HLS stream variable = slc_sessionIdFinFifo depth = 2
    // clang-format on

    static hls::stream<rtlSessionUpdateReply> slc_sessionInsert_rsp("slc_sessionInsert_rsp");
    // clang-format off
    #pragma HLS STREAM variable = slc_sessionInsert_rsp depth = 4
    // clang-format on

    static hls::stream<rtlSessionUpdateRequest> sessionInsert_req("sessionInsert_req");
    // clang-format off
    #pragma HLS STREAM variable = sessionInsert_req depth = 4
    // clang-format on

    static hls::stream<rtlSessionUpdateRequest> sessionDelete_req("sessionDelete_req");
    // clang-format off
    #pragma HLS STREAM variable = sessionDelete_req depth = 4
    // clang-format on

    static hls::stream<revLupInsert> reverseLupInsertFifo("reverseLupInsertFifo");
    // clang-format off
    #pragma HLS STREAM variable = reverseLupInsertFifo depth = 4
    // clang-format on

    sessionIdManager(slc_sessionIdFreeList, slc_sessionIdFinFifo);

    lookupReplyHandler(sessionLookup_rsp, slc_sessionInsert_rsp, rxEng2sLookup_req, txApp2sLookup_req,
                       slc_sessionIdFreeList, sessionLookup_req, sLookup2rxEng_rsp, sLookup2txApp_rsp,
                       sessionInsert_req,
                       reverseLupInsertFifo);

    updateRequestSender(sessionInsert_req, sessionDelete_req, sessionUpdate_req, slc_sessionIdFinFifo, tcpCurrEstab);

    updateReplyHandler(sessionUpdate_rsp, slc_sessionInsert_rsp);

    reverseLookupTableInterface(reverseLupInsertFifo, stateTable2sLookup_releaseSession, txEng2sLookup_rev_req,
                                sLookup2portTable_releasePort, sessionDelete_req, sLookup2txEng_rev_rsp, myIpAddress);
}
