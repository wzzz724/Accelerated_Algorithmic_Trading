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

#ifndef CLKTICKGEN_H
#define CLKTICKGEN_H

#include "hls_stream.h"
#include "ap_int.h"
#include "aat_defines.hpp"
#include "aat_interfaces.hpp"

#define TICK_ENABLE_04  (1<<4)
#define TICK_ENABLE_03  (1<<3)
#define TICK_ENABLE_02  (1<<2)
#define TICK_ENABLE_01  (1<<1)
#define TICK_ENABLE_00  (1<<0)

// TODO: typically use a struct for control/status registers, this might be an
//       instance where it's better to use seperate arrays for intervals and
//       thresholds and use a single define to set the number of timers
typedef struct clockTickGeneratorRegControl_t
{
    ap_uint<32> control;
    ap_uint<32> interval00;
    ap_uint<32> interval01;
    ap_uint<32> interval02;
    ap_uint<32> interval03;
    ap_uint<32> interval04;
    ap_uint<32> reserved06;
    ap_uint<32> reserved07;
} clockTickGeneratorRegControl_t;

typedef struct clockTickGeneratorRegStatus_t
{
    ap_uint<32> status;
    ap_uint<32> tickCount;
    ap_uint<32> txCount00;
    ap_uint<32> txCount01;
    ap_uint<32> txCount02;
    ap_uint<32> txCount03;
    ap_uint<32> txCount04;
    ap_uint<32> reserved07;
} clockTickGeneratorRegStatus_t;

/**
 * ClockTickGenerator Core
 */
class ClockTickGenerator
{
public:

    void tickProcess(ap_uint<32> &regControl,
                     ap_uint<32> &regInterval00,
                     ap_uint<32> &regInterval01,
                     ap_uint<32> &regInterval02,
                     ap_uint<32> &regInterval03,
                     ap_uint<32> &regInterval04,
                     ap_uint<32> &regTickCount,
                     ap_uint<32> &regTxCount00,
                     ap_uint<32> &regTxCount01,
                     ap_uint<32> &regTxCount02,
                     ap_uint<32> &regTxCount03,
                     ap_uint<32> &regTxCount04,
                     hls::stream<clockTickGeneratorEvent_t> &eventStream00,
                     hls::stream<clockTickGeneratorEvent_t> &eventStream01,
                     hls::stream<clockTickGeneratorEvent_t> &eventStream02,
                     hls::stream<clockTickGeneratorEvent_t> &eventStream03,
                     hls::stream<clockTickGeneratorEvent_t> &eventStream04);

private:

};

#endif
