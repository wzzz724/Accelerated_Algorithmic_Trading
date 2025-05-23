/************************************************
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

#include "port_table.hpp"

/** @ingroup port_table
 *  
 *  @ref rx_engine and @ref tx_app_if access the "listen" port
 *  table:
 *
 *  Block           | Access
 *  ----------------|--------------
 *  @ref rx_engine  | read
 *  @ref tx_app_if  | read -> write

 *  If read and write operation on same address occur at the same time,
 *  read should get the old value, either way it doesn't matter.
 *
 *  @param[in]      rxApp2portTable_listen_req      Request to open a port for listening
 *  @param[in]      pt_portCheckListening_req_fifo  
 *  @param[out]     portTable2rxApp_listen_rsp
 *  @param[out]     pt_portCheckListening_rsp_fifo
 *
 *  @todo make sure currPort is not equal in 2 consecutive cycles - why?
 */
void listening_port_table(hls::stream<ap_uint<16> >& rxApp2portTable_listen_req,
                          hls::stream<ap_uint<15> >& pt_portCheckListening_req_fifo,
                          hls::stream<bool>&         portTable2rxApp_listen_rsp,
                          hls::stream<bool>&         pt_portCheckListening_rsp_fifo) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static bool listeningPortTable[32768];
    // clang-format off
    #pragma HLS RESOURCE variable=listeningPortTable core=RAM_T2P_BRAM
    #pragma HLS DEPENDENCE variable=listeningPortTable inter false
    // clang-format on

    ap_uint<16> currPort;

    // check range, TODO make sure currPort is not equal in 2 consecutive cycles
    if (!rxApp2portTable_listen_req.empty()) 
    {
        rxApp2portTable_listen_req.read(currPort);
        if (!listeningPortTable[currPort(14, 0)] && currPort < 32768) {
            // If port not set for listening, set it for listening
            listeningPortTable[currPort] = true;
            // then confirm it is opened.
            portTable2rxApp_listen_rsp.write(true);
        } else {
            // otherwise deny the request to open for listening
            portTable2rxApp_listen_rsp.write(false);
        }
    } else if (!pt_portCheckListening_req_fifo.empty()) {
        // send the current listening status of the port being queried
        pt_portCheckListening_rsp_fifo.write(listeningPortTable[pt_portCheckListening_req_fifo.read()]);
    }
}

/** @ingroup port_table
 *
 *  @ref rx_engine and @ref session_lookup_controller access the free port table.
 *
 *  Assumption: We are never going to run out of free ports, since 32 sessions <<< 32K ports
 *
 *  Block                          | Access
 *  -------------------------------|---------
 *  @ref rx_engine                 | read
 *  @ref session_lookup_controller | write
 *
 *  If a free port is found it is written into @portTable2txApp_port_rsp and 
 *  cached until @ref tx_app_stream_if reads it out.
 * 
 *  @param[in]		sLookup2portTable_releasePort
 *  @param[in]		pt_portCheckUsed_req_fifo
 *  @param[out]		pt_portCheckUsed_rsp_fifo
 *  @param[out]		portTable2txApp_port_rsp
 *
 * @todo Figure out what's special about port numbers above 32768 here - top
 *       bit being set signifies something to the @ref tx_app_if.
 */
void free_port_table(hls::stream<ap_uint<16> >& sLookup2portTable_releasePort,
                     hls::stream<ap_uint<15> >& pt_portCheckUsed_req_fifo,
                     hls::stream<bool>&         pt_portCheckUsed_rsp_fifo,
                     hls::stream<ap_uint<16> >& portTable2txApp_port_rsp) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static bool freePortTable[32768];
    // clang-format off
	#pragma HLS RESOURCE variable=freePortTable core=RAM_T2P_BRAM
	#pragma HLS DEPENDENCE variable=freePortTable inter false
    // clang-format on

    // pt_cursor is an index into the free port table that increments every clock cycle
    static ap_uint<15> pt_cursor = 0; // index into the free port table

    ap_uint<16> currPort;
    ap_uint<16> freePort;

    // check range, TODO make sure no accesses to same location in 2 consecutive cycles
    if (!sLookup2portTable_releasePort.empty()) 
    {
        sLookup2portTable_releasePort.read(currPort);
        if (currPort >= 32768) {
            freePortTable[currPort(14, 0)] = false;
        }
    } else if (!pt_portCheckUsed_req_fifo.empty()) {
        pt_portCheckUsed_rsp_fifo.write(freePortTable[pt_portCheckUsed_req_fifo.read()]);
    } else {
        if (!freePortTable[pt_cursor] && !portTable2txApp_port_rsp.full())
        {
            freePort(14, 0) = pt_cursor;
            freePort[15] = 1;
            freePortTable[pt_cursor] = true;
            portTable2txApp_port_rsp.write(freePort);
        }
    }
    pt_cursor++;
}

/** @ingroup port_table
 *  
 *  Forward request according to port number, store table to keep order.
 *
 *  @param[in]   rxEng2portTable_check_req      Request to check from
 *                                              @ref rx_engine 
 *  @param[out]  pt_portCheckListening_req_fifo 
 *  @param[out]  pt_portCheckUsed_req_fifo
 *  @param[out]  pt_dstFifoOut
 *
 *  @todo Consider replacing LT/FT with a specialised enumerated type.
 */
void check_in_multiplexer(hls::stream<ap_uint<16> >& rxEng2portTable_check_req,
                          hls::stream<ap_uint<15> >& pt_portCheckListening_req_fifo,
                          hls::stream<ap_uint<15> >& pt_portCheckUsed_req_fifo,
                          hls::stream<bool>& pt_dstFifoOut) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    static const bool LT = true;  // Listening Table
    static const bool FT = false; // Free Table

    ap_uint<16> checkPort;

    // Forward request according to port number, store table to keep order
    if (!rxEng2portTable_check_req.empty()) {
        rxEng2portTable_check_req.read(checkPort);
        if (checkPort < 32768) {
            pt_portCheckListening_req_fifo.write(checkPort);
            pt_dstFifoOut.write(LT);
        } else {
            pt_portCheckUsed_req_fifo.write(checkPort);
            pt_dstFifoOut.write(FT);
        }
    }
}

/** @ingroup port_table
 *
 *  Read responses from listening port and free port tables in order and
 *  merge them into an output stream to the @ref rx_engine.
 * 
 *  @param[in]  pt_dstFifoIn                    Stream to select which table
 *                                              response to read from.
 *  @param[in]  pt_portCheckListening_rsp_fifo  Responses from Listen Port
 *                                              Table.
 *  @param[in]  pt_portCheckUsed_rsp_fifo       Responses from Free Port
 *                                              Table.
 *  @param[out] portTable2rxEng_check_rsp       Combined responses to the
 *                                              @ref rx_engine.
 * 
 *  @todo replace the magic numbers in the case statement.
 *  @todo Consider replacing LT/FT with a specialised enumerated type.
 */
void check_out_multiplexer(hls::stream<bool>& pt_dstFifoIn,
                           hls::stream<bool>& pt_portCheckListening_rsp_fifo,
                           hls::stream<bool>& pt_portCheckUsed_rsp_fifo,
                           hls::stream<bool>& portTable2rxEng_check_rsp) {
#pragma HLS PIPELINE II = 1
#pragma HLS INLINE off

    // enum portCheckDstType {LT, FT};
    static const bool LT = true;
    static const bool FT = false;
    // static stream<bool> pt_dstFifo("pt_dstFifo");
    //#pragma HLS STREAM variable=pt_dstFifo depth=4

    static bool dst = LT;

    // Read out responses from tables in order and merge them
    enum cmFsmStateType { READ_DST, READ_LISTENING, READ_USED };
    static cmFsmStateType cm_fsmState = READ_DST;

    switch (cm_fsmState) {
        case 0: // Replace with enum symbol
            if (!pt_dstFifoIn.empty()) {
                pt_dstFifoIn.read(dst);
                if (dst == LT) {
                    cm_fsmState = READ_LISTENING;
                } else {
                    cm_fsmState = READ_USED;
                }
            }
            break;
        case 1: // Replace with enum symbol
            if (!pt_portCheckListening_rsp_fifo.empty()) {
                portTable2rxEng_check_rsp.write(pt_portCheckListening_rsp_fifo.read());
                cm_fsmState = READ_DST;
            }
            break;
        case 2: // replace with enum symbol
            if (!pt_portCheckUsed_rsp_fifo.empty()) {
                portTable2rxEng_check_rsp.write(pt_portCheckUsed_rsp_fifo.read());
                cm_fsmState = READ_DST;
            }
            break;
    }
}

/** @ingroup port_table
 *
 *  The @ref port_table contains an array of 65536 entries split into two banks of
 *  32768, one for each port number.
 *  It receives passive opening (listening) requests from @ref rx_app_if, requests
 *  to check if the port is open from the @ref rx_engine, and requests to allocate
 *  a free port when opening an active connection from the @ref tx_app_if.
 *
 *  @param[in]  rxEng2portTable_check_req     Stream of checks to see if port is
 *                                            passively open from @ref rx_engine.
 *  @param[in]  rxApp2portTable_listen_req    Stream of requests to open a port for
 *                                            listening from the @ref rx_app_if.
 *  @param[in]  txApp2portTable_port_req      Connection currently commented out.
 *  @param[in]  sLookup2portTable_releasePort
 *  @param[out] portTable2rxEng_check_rsp     Stream of confirmations that a port
 *                                            is accepting connections to
 *                                            @ref rx_engine.
 *  @param[out] portTable2rxApp_listen_rsp    Stream of confirmations that
 *                                            passive open has succeeded to 
 *                                            @ref rx_app_if
 *  @param[out] portTable2txApp_rsp
 *
 *  @todo Figure out why txApp2portTable_port_req is commented out as an input.
 */
void port_table(hls::stream<ap_uint<16> >& rxEng2portTable_check_req,
                hls::stream<ap_uint<16> >& rxApp2portTable_listen_req,
                // stream<ap_uint<1> >&		txApp2portTable_port_req,
                hls::stream<ap_uint<16> >& sLookup2portTable_releasePort,

                hls::stream<bool>& portTable2rxEng_check_rsp,
                hls::stream<bool>& portTable2rxApp_listen_rsp,
                hls::stream<ap_uint<16> >& portTable2txApp_port_rsp) {
#pragma HLS INLINE

    /*
     * Fifos necessary for multiplexing Check requests
     */
    static hls::stream<ap_uint<15> > pt_portCheckListening_req_fifo("pt_portCheckListening_req_fifo");
    static hls::stream<ap_uint<15> > pt_portCheckUsed_req_fifo("pt_portCheckUsed_req_fifo");
    // clang-format off
    #pragma HLS STREAM variable=pt_portCheckListening_req_fifo depth=2
    #pragma HLS STREAM variable=pt_portCheckUsed_req_fifo depth=2
    // clang-format on

    static hls::stream<bool> pt_portCheckListening_rsp_fifo("pt_portCheckListening_rsp_fifo");
    static hls::stream<bool> pt_portCheckUsed_rsp_fifo("pt_portCheckUsed_rsp_fifo");
    // clang-format off
    #pragma HLS STREAM variable=pt_portCheckListening_rsp_fifo depth=5
    #pragma HLS STREAM variable=pt_portCheckUsed_rsp_fifo depth=2
    // clang-format on

    static hls::stream<bool> pt_dstFifo("pt_dstFifo");
    // clang-format off
    #pragma HLS STREAM variable=pt_dstFifo depth=5
    // clang-format on

    /*
     * Listening PortTable
     */
    listening_port_table(rxApp2portTable_listen_req,
                         pt_portCheckListening_req_fifo,
                         portTable2rxApp_listen_rsp,
                         pt_portCheckListening_rsp_fifo);

    /*
     * Free PortTable
     */
    free_port_table(sLookup2portTable_releasePort,
                    pt_portCheckUsed_req_fifo,
                    pt_portCheckUsed_rsp_fifo,
                    portTable2txApp_port_rsp);

    /*
     * Multiplex this query
     */
    check_in_multiplexer(rxEng2portTable_check_req,
                         pt_portCheckListening_req_fifo,
                         pt_portCheckUsed_req_fifo,
                         pt_dstFifo);

    check_out_multiplexer(pt_dstFifo,
                          pt_portCheckListening_rsp_fifo,
                          pt_portCheckUsed_rsp_fifo,
                          portTable2rxEng_check_rsp);
}
