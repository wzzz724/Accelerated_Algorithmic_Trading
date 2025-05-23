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

#include "igmp.hpp"

/** @ingroup igmp
 *  Processes query requests and adds them to the table. Decrements the timers every 0.1s
 *  on timer expiry emits a report.
 *	@param[in]		tick100ms
 *	@param[in]		is_v3
 *  @param[in]      inIgmpQuery
 *	@param[out]		outIgmpReport
 */
void igmp_process_timer(hls::stream<bool>& tick100ms,
                        hls::stream<bool>& is_v3,
                        hls::stream<igmpQuery>& inIgmpQuery,
                        hls::stream<igmpQuery>& outIgmpReport) {
#define MAX_IGMP_TIMERS 16
#pragma HLS PIPELINE II = 1

    static igmpTimerEntry igmpTimerTable[MAX_IGMP_TIMERS];
#pragma HLS RESOURCE variable = igmpTimerTable core = RAM_T2P_BRAM
#pragma HLS DEPENDENCE variable = igmpTimerTable inter false

    enum igmpTimerStateType { IDLE, CHECK_TIMERS, DECREMENT_TIMERS, INSERT_ENTRY };
    static igmpTimerStateType igmpTimerState = IDLE;
    static igmpQuery currQuery;
    static bool fExisting = false;
    static bool v3_query = true;
    static uint8_t nCount = 0;

    ap_uint<16> checkID;

    switch (igmpTimerState) {
        case IDLE:
            if(!is_v3.empty()){
                is_v3.read(v3_query);
            } else if (!inIgmpQuery.empty()) {
                inIgmpQuery.read(currQuery);
                if (v3_query==true && (currQuery.maxRespTime > 128)) {
                    uint32_t mant;
                    uint32_t exponent;
                    mant = currQuery.maxRespTime & 0xF;
                    exponent = (currQuery.maxRespTime & 0x70) >> 4;
                    currQuery.maxRespTime = ((mant | 0x10) << (exponent + 3));
                }
                fExisting = false;
                nCount = 0;
                igmpTimerState = CHECK_TIMERS;
            } else if (!tick100ms.empty()) {
                tick100ms.read();
                nCount = 0;
                igmpTimerState = DECREMENT_TIMERS;
            }
            break;
        case DECREMENT_TIMERS:
            // Increment the entry pointer
            // Look in the table at the current entry
            // If it has expired, issue a report event.
            // Otherwise, decrement the time

            // If Active and timer == 0 expire the timer, otherwise decrement the timer
            // If inactive, and we have an insertion request - activate a timer
            if (igmpTimerTable[nCount].fActive) {
                if (igmpTimerTable[nCount].time == 0) {
                    igmpTimerTable[nCount].fActive = false;
                    outIgmpReport.write(igmpQuery(igmpTimerTable[nCount].groupAddr));
                } else {
                    igmpTimerTable[nCount].time -= 1;
                }
            }
            nCount++;
            if (nCount == MAX_IGMP_TIMERS) {
                igmpTimerState = IDLE;
            }
            break;
        case CHECK_TIMERS:
            // Iterate over our timers. See if there already is a matching entry
            if (igmpTimerTable[nCount].fActive && (igmpTimerTable[nCount].groupAddr == currQuery.groupAddr)) {
                // We have a match, if the new query has a lower time - use that time
                if (igmpTimerTable[nCount].time > currQuery.maxRespTime) {
                    igmpTimerTable[nCount].time = currQuery.maxRespTime;
                }
                fExisting = true;
            }
            nCount++;
            if (nCount == MAX_IGMP_TIMERS) {
                if (!fExisting) {
                    nCount = 0;
                    igmpTimerState = INSERT_ENTRY;
                } else {
                    igmpTimerState = IDLE;
                }
            }
            break;
        case INSERT_ENTRY:
            if (!igmpTimerTable[nCount].fActive) {
                igmpTimerTable[nCount].fActive = true;
                igmpTimerTable[nCount].groupAddr = currQuery.groupAddr;
                igmpTimerTable[nCount].time = currQuery.maxRespTime;
                igmpTimerState = IDLE;
            }
            nCount++;
            // Check for the case that there are no free timer slots.
            if (nCount == MAX_IGMP_TIMERS) {
                igmpTimerState = IDLE;
            }
            break;
    }
}
