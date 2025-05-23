/************************************************
 * Copyright (c) 2019, Systems Group, ETH Zurich
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

#include "../toe_internals.hpp"

/** @ingroup session_lookup_controller
 *
 */
enum lookupSource { RX=0, TX_APP=1 };

/** @ingroup session_lookup_controller
 *
 */
enum lookupOp { INSERT=0, DELETE=1 };


/** @ingroup session_lookup_controller
 *
 */
struct slupRouting {
    bool isUpdate;
    lookupSource source;
    slupRouting() {}
    slupRouting(bool isUpdate, lookupSource src) : isUpdate(isUpdate), source(src) {}
};

/** @ingroup session_lookup_controller
 *
 *  This struct defines the internal storage format of the IP tuple instead of destination and source,
 *  my and their is used. When a tuple is sent or received from the tx/rx path it is mapped to the fourTuple struct.
 */
struct threeTupleInternal {
    ap_uint<32> theirIp;
    ap_uint<16> myPort;
    ap_uint<16> theirPort;
    threeTupleInternal() {}
    threeTupleInternal(ap_uint<32> theirIp, ap_uint<16> myPort, ap_uint<16> theirPort)
        : theirIp(theirIp), myPort(myPort), theirPort(theirPort) {}

    /**
     * Comparison function used in c++ testing code when threeTupleInternal
     * is inserted into a std::map. Sort ordering is done in the following
     * order:
     * 
     * - theirIP
     * - myPort
     * - theirPort
     *
     * @param[in] other Instance to be compared against
     * @return    true if this instance is 'below' the other instance
     *
     */
    bool operator<(const threeTupleInternal& other) const {
        if (theirIp < other.theirIp) {
            return true;
        } else if (theirIp == other.theirIp) {
            if (myPort < other.myPort) {
                return true;
            } else if (myPort == other.myPort) {
                if (theirPort < other.theirPort) {
                    return true;
                }
            }
        }
        return false;
    }
};

/** @ingroup session_lookup_controller
 *
 */
struct sessionLookupQueryInternal {
    threeTupleInternal tuple;
    bool allowCreation;
    lookupSource source;
    sessionLookupQueryInternal() {}
    sessionLookupQueryInternal(threeTupleInternal tuple, bool allowCreation, lookupSource src)
        : tuple(tuple), allowCreation(allowCreation), source(src) {}
};

/** @ingroup session_lookup_controller
 *
 */
struct rtlSessionLookupRequest {
    threeTupleInternal key;
    lookupSource source;
    rtlSessionLookupRequest() {}
    rtlSessionLookupRequest(threeTupleInternal tuple, lookupSource src) : key(tuple), source(src) {}
};

/** @ingroup session_lookup_controller
 *
 */
struct rtlSessionUpdateRequest {
    lookupOp op;
    threeTupleInternal key;
    ap_uint<16> value;
    lookupSource source;
    rtlSessionUpdateRequest() {}
    rtlSessionUpdateRequest(threeTupleInternal key, ap_uint<16> value, lookupOp op, lookupSource src)
        : key(key), value(value), op(op), source(src) {}
};

/** @ingroup session_lookup_controller
 *
 */
struct rtlSessionLookupReply {
    threeTupleInternal key;
    ap_uint<16> sessionID;
    bool hit;
    lookupSource source;
    rtlSessionLookupReply() {}
    rtlSessionLookupReply(bool hit, lookupSource src) : hit(hit), sessionID(0), source(src) {}
    rtlSessionLookupReply(bool hit, ap_uint<16> id, lookupSource src) : hit(hit), sessionID(id), source(src) {}
};

/** @ingroup session_lookup_controller
 *
 */
struct rtlSessionUpdateReply {
    lookupOp op; 
    threeTupleInternal key;
    ap_uint<16> sessionID;
    bool success;
    lookupSource source;
    rtlSessionUpdateReply() {}
    rtlSessionUpdateReply(lookupOp op, lookupSource src) : op(op), source(src) {}
    rtlSessionUpdateReply(ap_uint<16> id, lookupOp op, lookupSource src) : sessionID(id), op(op), source(src) {}
};

struct revLupInsert {
    ap_uint<16> key;
    threeTupleInternal value;
    revLupInsert(){};
    revLupInsert(ap_uint<16> key, threeTupleInternal value) : key(key), value(value) {}
};

/** @defgroup session_lookup_controller Session Lookup Controller
 *  @ingroup tcp_module
 */
void session_lookup_controller(hls::stream<sessionLookupQuery>&      rxEng2sLookup_req,
                               hls::stream<sessionLookupReply>&      sLookup2rxEng_rsp,
                               hls::stream<ap_uint<16> >&            stateTable2sLookup_releaseSession,
                               hls::stream<ap_uint<16> >&            sLookup2portTable_releasePort,
                               hls::stream<fourTuple>&               txApp2sLookup_req,
                               hls::stream<sessionLookupReply>&      sLookup2txApp_rsp,
                               hls::stream<ap_uint<16> >&            txEng2sLookup_rev_req,
                               hls::stream<fourTuple>&               sLookup2txEng_rev_rsp,
                               hls::stream<ap_axiu<128,0,0,0> >&     sessionLookup_req,
                               hls::stream<ap_axiu<128,0,0,0> >&     sessionLookup_rsp,
                               hls::stream<ap_axiu<128,0,0,0> >&     sessionUpdate_req,
                               hls::stream<ap_axiu<128,0,0,0> >&     sessionUpdate_rsp,
                               uint32_t&                             tcpCurrEstab,
                               ap_uint<32>                           myIpAddress);
