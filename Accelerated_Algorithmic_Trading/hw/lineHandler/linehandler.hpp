/*
 * Copyright 2021 Xilinx, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LINEHANDLER_H
#define LINEHANDLER_H

#include <hls_stream.h>
#include <ap_int.h>
#include "aat_defines.hpp"
#include "aat_interfaces.hpp"

#ifndef __SYNTHESIS__
#define _LH_DEBUG_EN 1
#endif

#if _LH_DEBUG_EN == 1
#define LH_DBG(msg) do { std::cout << msg << std::endl; } while (0);
#else
#define LH_DBG(msg)
#endif

// Max number of port filter entries for port 0 and 1
#define NUM_FILTERS (16)
// Max number of arbitrable splits in the packet arbitrator
#define NUM_SPLITS (8)
// Number of bits in sequence number field
#define SEQ_NUM_WIDTH (32)

// Port control
#define LH_FILTER_DISABLE   (1<<4)
#define LH_ECHO_ENABLE      (1<<3)
#define LH_RESET_COUNT      (1<<2)
#define LH_RESET_DATA       (1<<1)
#define LH_HALT             (1<<0)

// Arbitration control
#define LH_RESET_SEQ_NUM    (1<<0)

typedef ap_uint<3> lhSplitId_t;

typedef struct lineHandlerRegControl
{
    // Port filter 0
    ap_uint<32> controlPort0;
    ap_uint<32> echoAddress0;
    ap_uint<32> echoPort0;
    // Port filter 1
    ap_uint<32> controlPort1;
    ap_uint<32> echoAddress1;
    ap_uint<32> echoPort1;
    // Line Arb
    ap_uint<32> controlArb;
    ap_uint<32> resetTimerInterval;
} lineHandlerRegControl_t;

typedef struct lineHandlerRegStatus
{
    // Port filter 0
    ap_uint<32> rxWord0;
    ap_uint<32> rxMeta0;
    ap_uint<32> dropWord0;
    ap_uint<32> debugAddress0;
    ap_uint<32> debugPort0;
    // Port filter 1
    ap_uint<32> rxWord1;
    ap_uint<32> rxMeta1;
    ap_uint<32> dropWord1;
    ap_uint<32> debugAddress1;
    ap_uint<32> debugPort1;
    // Line Arb
    ap_uint<32> totalSent;
    ap_uint<32> totalWordSent;
    ap_uint<32> totalMissed;
    ap_uint<32> rxFeed0;
    ap_uint<32> rxFeed1;
    ap_uint<32> txFeed0;
    ap_uint<32> txFeed1;
    ap_uint<32> discarded0;
    ap_uint<32> discarded1;
    // Clock Event
    ap_uint<32> rxEvent;
} lineHandlerRegStatus_t;

typedef struct regPortFilterContainer
{
    ap_uint<32> filterAddress0[NUM_FILTERS];
    ap_uint<32> filterPort0[NUM_FILTERS];
    ap_uint<32> filterSplitId0[NUM_FILTERS];
    ap_uint<32> filterAddress1[NUM_FILTERS];
    ap_uint<32> filterPort1[NUM_FILTERS];
    ap_uint<32> filterSplitId1[NUM_FILTERS];

    inline regPortFilterContainer()
    {
#pragma HLS INLINE
#pragma HLS ARRAY_PARTITION variable=filterAddress0
#pragma HLS ARRAY_PARTITION variable=filterPort0
#pragma HLS ARRAY_PARTITION variable=filterSplitId0
#pragma HLS ARRAY_PARTITION variable=filterAddress1
#pragma HLS ARRAY_PARTITION variable=filterPort1
#pragma HLS ARRAY_PARTITION variable=filterSplitId1
    }
} regPortFilterContainer_t;

class LineFilter
{
    enum iid_StateType {GET_VALID, FWD, DROP};

    iid_StateType iid_state=GET_VALID;
    ap_uint<32> countRxWord=0;
    ap_uint<32> countRxMeta=0;
    ap_uint<32> countDropWord=0;

  public:
    void portFilter(ap_uint<32> regControl,
                    ap_uint<32> regEchoAddress,
                    ap_uint<32> regEchoPort,
                    ap_uint<32> regFilterAddress[NUM_FILTERS],
                    ap_uint<32> regFilterPort[NUM_FILTERS],
                    ap_uint<32> regFilterSplitIdx[NUM_FILTERS],
                    ap_uint<32> &regRxWord,
                    ap_uint<32> &regRxMeta,
                    ap_uint<32> &regDropWord,
                    ap_uint<32> &regDebugAddress,
                    ap_uint<32> &regDebugPort,
                    hls::stream<axiWordExt_t> &inputStream,
                    hls::stream<ipUdpMetaPackExt_t> &inputMetaStream,
                    hls::stream<axiWordExt_t> &echoStream,
                    hls::stream<ipUdpMetaPackExt_t> &echoMetaStream,
                    hls::stream<axiWord_t> &outputStream,
                    hls::stream<lhSplitId_t> &splitIdStream);
};

class LineHandler
{
    typedef ap_uint<SEQ_NUM_WIDTH> seqNum_t;

  public:
    LineFilter port0Filter;
    LineFilter port1Filter;

    void lineArbitrator(ap_uint<32> regControlArb,
                        ap_uint<32> regResetTimerInterval,
                        ap_uint<32> &regTotalSent,
                        ap_uint<32> &regTotalWordSent,
                        ap_uint<32> &regTotalMissed,
                        ap_uint<32> &regRxFeed0,
                        ap_uint<32> &regRxFeed1,
                        ap_uint<32> &regTxFeed0,
                        ap_uint<32> &regTxFeed1,
                        ap_uint<32> &regDiscarded0,
                        ap_uint<32> &regDiscarded1,
                        hls::stream<axiWord_t> &port0Strm,
                        hls::stream<axiWord_t> &port1Strm,
                        hls::stream<lhSplitId_t> &splitIdStrm0,
                        hls::stream<lhSplitId_t> &splitIdStrm1,
                        hls::stream<axiWordExt_t> &outputFeed);

    void eventHandler(ap_uint<32> &regRxEvent,
                      hls::stream<clockTickGeneratorEvent_t> &eventStream);

};

#endif // LINEHANDLER_H
