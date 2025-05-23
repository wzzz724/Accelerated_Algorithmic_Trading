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

#ifndef LINEHANDLER_KERNELS_H
#define LINEHANDLER_KERNELS_H

#include "linehandler.hpp"
#include "aat_defines.hpp"
#include "aat_interfaces.hpp"

extern "C" void lineHandlerTop(lineHandlerRegControl_t &regControl,
                               lineHandlerRegStatus_t &regStatus,
                               regPortFilterContainer &regPortFilter,
                               hls::stream<axiWordExt_t> &inputDataPort0,
                               hls::stream<ipUdpMetaPackExt_t> &inputMetaPort0,
                               hls::stream<axiWordExt_t> &outputDataPort0,
                               hls::stream<ipUdpMetaPackExt_t> &outputMetaPort0,
                               hls::stream<axiWordExt_t> &inputDataPort1,
                               hls::stream<ipUdpMetaPackExt_t> &inputMetaPort1,
                               hls::stream<axiWordExt_t> &outputDataPort1,
                               hls::stream<ipUdpMetaPackExt_t> &outputMetaPort1,
                               hls::stream<axiWordExt_t> &outputArbDataFeed,
                               hls::stream<clockTickGeneratorEvent_t> &eventStream);

#endif // LINEHANDLER_KERNELS_H
