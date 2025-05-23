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

#include <iostream>
#include "clock_tick_generator.hpp"

/**
 * ClockTickGenerator Core
 */

void ClockTickGenerator::tickProcess(ap_uint<32> &regControl,
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
                                     hls::stream<clockTickGeneratorEvent_t> &eventStream04)

{
#pragma HLS PIPELINE II=1 style=flp

    clockTickGeneratorEvent_t eventTick00;
    clockTickGeneratorEvent_t eventTick01;
    clockTickGeneratorEvent_t eventTick02;
    clockTickGeneratorEvent_t eventTick03;
    clockTickGeneratorEvent_t eventTick04;

    static ap_uint<32> freeCount = 0;
    static ap_uint<32> tickCount00 = 0;
    static ap_uint<32> tickCount01 = 0;
    static ap_uint<32> tickCount02 = 0;
    static ap_uint<32> tickCount03 = 0;
    static ap_uint<32> tickCount04 = 0;
    static ap_uint<32> txCount00 = 0;
    static ap_uint<32> txCount01 = 0;
    static ap_uint<32> txCount02 = 0;
    static ap_uint<32> txCount03 = 0;
    static ap_uint<32> txCount04 = 0;

    if(TICK_ENABLE_00 & regControl)
    {
        if(tickCount00 == regInterval00)
        {
            // TODO: add enocoding for different event types?
            eventTick00.data = 1;
            eventStream00.write(eventTick00);
            tickCount00 = 0;
            ++txCount00;
        }
        else
        {
            ++tickCount00;
        }
    }
    else
    {
        tickCount00 = 0;
    }

    if(TICK_ENABLE_01 & regControl)
    {
        if(tickCount01 == regInterval01)
        {
            eventTick01.data = 1;
            eventStream01.write(eventTick01);
            tickCount01 = 0;
            ++txCount01;
        }
        else
        {
            ++tickCount01;
        }
    }
    else
    {
        tickCount01 = 0;
    }

    if(TICK_ENABLE_02 & regControl)
    {
        if(tickCount02 == regInterval02)
        {
            eventTick02.data = 1;
            eventStream02.write(eventTick02);
            tickCount02 = 0;
            ++txCount02;
        }
        else
        {
            ++tickCount02;
        }
    }
    else
    {
        tickCount02 = 0;
    }

    if(TICK_ENABLE_03 & regControl)
    {
        if(tickCount03 == regInterval03)
        {
            eventTick03.data = 1;
            eventStream03.write(eventTick03);
            tickCount03 = 0;
            ++txCount03;
        }
        else
        {
            ++tickCount03;
        }
    }
    else
    {
        tickCount03 = 0;
    }

    if(TICK_ENABLE_04 & regControl)
    {
        if(tickCount04 == regInterval03)
        {
            eventTick04.data = 1;
            eventStream04.write(eventTick04);
            tickCount04 = 0;
            ++txCount04;
        }
        else
        {
            ++tickCount04;
        }
    }
    else
    {
        tickCount04 = 0;
    }

    regTickCount = ++freeCount;
    regTxCount00 = txCount00;
    regTxCount01 = txCount01;
    regTxCount02 = txCount02;
    regTxCount03 = txCount03;
    regTxCount04 = txCount04;

    return;
}
