/*
 * Copyright (c) 2019-2020, Xilinx, Inc.
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

#include "arp_server_subnet.hpp"


/** @ingroup arp_server
 *  Maintains a table of all active ARP Requests sent by block. Request is
 *  kept in table for 0.5sec then is removed and marked as an unanswered request.
 *  Table is also updated when receiving a reply.
 *  @param[in]      tick100ms
 *  @param[in]      arpRequestMetaFifo
 *  @param[in]      arpReplyFifo
 *  @param[out]     arpRequestMetaFilteredFifo
 *  @param[out]     reqLost
 */
void arp_server_request_filter(hls::stream<bool>& tick100ms,
                               hls::stream<ap_uint<32> >& arpRequestMetaFifo,
                               hls::stream<ap_uint<32> >& arpReplyFifo,
                               hls::stream<ap_uint<32> >& arpRequestMetaFilteredFifo,
                               hls::stream<ap_uint<1> >& reqLost) {
    #define MAX_ARP_TIMERS 256
    #pragma HLS PIPELINE II = 1

    static arpTimerEntry arpTimerTable[MAX_ARP_TIMERS];
    #pragma HLS RESOURCE variable = arpTimerTable core = RAM_T2P_BRAM
    #pragma HLS DEPENDENCE variable = arpTimerTable inter false

    enum arpTimerStateType { IDLE, CHECK_TIMERS, DECREMENT_TIMERS };
    static arpTimerStateType arpTimerState = IDLE;
    enum arpTableStateType { WAIT, INSERT_ENTRY, REMOVE_ENTRY, UPDATE_TIME };
    static arpTableStateType arpTableState = WAIT;

    static ap_uint<32> currReq;
    static ap_uint<32> currReply;
    static ap_uint<8> index;
    static uint8_t nCount = 0;
    static bool hasData = false;

    if (!arpRequestMetaFifo.empty() || !arpReplyFifo.empty() || hasData) {
        switch (arpTableState) {
            case WAIT:
                if (!arpRequestMetaFifo.empty()) {
                    arpRequestMetaFifo.read(currReq);
                    index = currReq.range(31, 24);
                    arpTableState = INSERT_ENTRY;
                    hasData = true;
                } else if (!arpReplyFifo.empty()) {
                    arpReplyFifo.read(currReply);
                    index = currReply.range(31, 24);
                    arpTableState = REMOVE_ENTRY;
                    hasData = true;
                }
                break;

            case INSERT_ENTRY:
                if (arpTimerTable[index].fActive) {
                    // We need to check if the incoming request is already in the table
                    // While active we don't send out another request for the same
                    // addr. Do we want to count the dropped requests?
                } else {
                    arpTimerTable[index].fActive = true;
                    arpTimerTable[index].time = MAX_WAIT;
                    arpRequestMetaFilteredFifo.write(currReq);
                }
                hasData = false;
                arpTableState = WAIT;
                break;

            case REMOVE_ENTRY:
                // Remove entry from table based on received reply
                if (arpTimerTable[index].fActive) {
                    arpTimerTable[index].fActive = false;
                }
                hasData = false;
                arpTableState = WAIT;
                break;

            default:
                arpTableState = WAIT;
                hasData = false;
                break;
        }
    } else {
        switch (arpTimerState) {
            case IDLE:
                if (!tick100ms.empty()) {
                    tick100ms.read();
                    nCount = 0;
                    arpTimerState = DECREMENT_TIMERS;
                }
                break;

            case DECREMENT_TIMERS:
                if (arpTimerTable[nCount].fActive) {
                    arpTimerTable[nCount].time -= 1;
                }
                nCount++;
                if (nCount == MAX_ARP_TIMERS-1) {
                    nCount = 0;
                    arpTimerState = CHECK_TIMERS;
                }
                break;

            case CHECK_TIMERS:
                // If timer has reached 0 and the entry is still active it gets disabled and
                // marked as a lost request.
                if (arpTimerTable[nCount].fActive && arpTimerTable[nCount].time == 0) {
                    arpTimerTable[nCount].fActive = false;
                    reqLost.write(1);
                }
                nCount++;
                if (nCount == MAX_ARP_TIMERS-1) {
                    arpTimerState = IDLE;
                }
                break;

            default:
                arpTimerState = IDLE;
                break;
        }
    }
}
