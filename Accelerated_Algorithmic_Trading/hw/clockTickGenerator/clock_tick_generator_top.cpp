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
#include "clock_tick_generator_kernels.hpp"

extern "C" void clockTickGeneratorTop(clockTickGeneratorRegControl_t &regControl,
                                      clockTickGeneratorRegStatus_t &regStatus,
                                      hls::stream<clockTickGeneratorEvent_t> &eventStream00,
                                      hls::stream<clockTickGeneratorEvent_t> &eventStream01,
                                      hls::stream<clockTickGeneratorEvent_t> &eventStream02,
                                      hls::stream<clockTickGeneratorEvent_t> &eventStream03,
                                      hls::stream<clockTickGeneratorEvent_t> &eventStream04)
{
#pragma HLS INTERFACE s_axilite port=regControl bundle=control
#pragma HLS INTERFACE s_axilite port=regStatus bundle=control
#pragma HLS INTERFACE ap_none port=regControl
#pragma HLS INTERFACE ap_none port=regStatus
#pragma HLS INTERFACE axis port=eventStream00
#pragma HLS INTERFACE axis port=eventStream01
#pragma HLS INTERFACE axis port=eventStream02
#pragma HLS INTERFACE axis port=eventStream03
#pragma HLS INTERFACE axis port=eventStream04
#pragma HLS INTERFACE ap_ctrl_none port=return

    static ClockTickGenerator kernel;

#pragma HLS STABLE variable=regControl
#pragma HLS DATAFLOW disable_start_propagation

    kernel.tickProcess(regControl.control,
                       regControl.interval00,
                       regControl.interval01,
                       regControl.interval02,
                       regControl.interval03,
                       regControl.interval04,
                       regStatus.tickCount,
                       regStatus.txCount00,
                       regStatus.txCount01,
                       regStatus.txCount02,
                       regStatus.txCount03,
                       regStatus.txCount04,
                       eventStream00,
                       eventStream01,
                       eventStream02,
                       eventStream03,
                       eventStream04);

}
