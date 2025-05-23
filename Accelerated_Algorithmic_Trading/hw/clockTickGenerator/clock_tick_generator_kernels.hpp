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

#ifndef CLKTICKGEN_KERNELS_H
#define CLKTICKGEN_KERNELS_H

#include "clock_tick_generator.hpp"

extern "C" void clockTickGeneratorTop(clockTickGeneratorRegControl_t &regControl,
                                      clockTickGeneratorRegStatus_t &regStatus,
                                      hls::stream<clockTickGeneratorEvent_t> &eventStream00,
                                      hls::stream<clockTickGeneratorEvent_t> &eventStream01,
                                      hls::stream<clockTickGeneratorEvent_t> &eventStream02,
                                      hls::stream<clockTickGeneratorEvent_t> &eventStream03,
                                      hls::stream<clockTickGeneratorEvent_t> &eventStream04);

#endif
