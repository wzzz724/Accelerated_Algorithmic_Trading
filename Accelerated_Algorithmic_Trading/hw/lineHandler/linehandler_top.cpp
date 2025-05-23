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

#include "linehandler_kernels.hpp"

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
                               hls::stream<clockTickGeneratorEvent_t> &eventStream)
{
#pragma HLS INTERFACE s_axilite port=regControl
#pragma HLS INTERFACE s_axilite port=regStatus
#pragma HLS INTERFACE s_axilite port=regPortFilter
#pragma HLS INTERFACE axis port=inputDataPort0
#pragma HLS INTERFACE axis port=inputMetaPort0
#pragma HLS INTERFACE axis port=outputDataPort0
#pragma HLS INTERFACE axis port=outputMetaPort0
#pragma HLS INTERFACE axis port=inputDataPort1
#pragma HLS INTERFACE axis port=inputMetaPort1
#pragma HLS INTERFACE axis port=outputDataPort1
#pragma HLS INTERFACE axis port=outputMetaPort1
#pragma HLS INTERFACE axis port=outputArbDataFeed
#pragma HLS INTERFACE axis port=eventStream
#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS DISAGGREGATE variable=regControl
#pragma HLS DISAGGREGATE variable=regStatus
#pragma HLS STABLE variable=regPortFilter
#pragma HLS DATAFLOW disable_start_propagation

    static LineHandler kernel;
    static hls::stream<axiWord_t> port0Filtered;
    static hls::stream<axiWord_t> port1Filtered;
    static hls::stream<lhSplitId_t> port0SplitId;
    static hls::stream<lhSplitId_t> port1SplitId;

#pragma HLS STREAM variable=port0Filtered depth=64
#pragma HLS STREAM variable=port1Filtered depth=64
#pragma HLS STREAM variable=port0SplitId depth=64
#pragma HLS STREAM variable=port1SplitId depth=64

    kernel.port0Filter.portFilter(regControl.controlPort0,
                                  regControl.echoAddress0,
                                  regControl.echoPort0,
                                  regPortFilter.filterAddress0,
                                  regPortFilter.filterPort0,
                                  regPortFilter.filterSplitId0,
                                  regStatus.rxWord0,
                                  regStatus.rxMeta0,
                                  regStatus.dropWord0,
                                  regStatus.debugAddress0,
                                  regStatus.debugPort0,
                                  inputDataPort0,
                                  inputMetaPort0,
                                  outputDataPort0,
                                  outputMetaPort0,
                                  port0Filtered,
                                  port0SplitId);

    kernel.port1Filter.portFilter(regControl.controlPort1,
                                  regControl.echoAddress1,
                                  regControl.echoPort1,
                                  regPortFilter.filterAddress1,
                                  regPortFilter.filterPort1,
                                  regPortFilter.filterSplitId1,
                                  regStatus.rxWord1,
                                  regStatus.rxMeta1,
                                  regStatus.dropWord1,
                                  regStatus.debugAddress1,
                                  regStatus.debugPort1,
                                  inputDataPort1,
                                  inputMetaPort1,
                                  outputDataPort1,
                                  outputMetaPort1,
                                  port1Filtered,
                                  port1SplitId);

    kernel.lineArbitrator(regControl.controlArb,
                          regControl.resetTimerInterval,
                          regStatus.totalSent,
                          regStatus.totalWordSent,
                          regStatus.totalMissed,
                          regStatus.rxFeed0,
                          regStatus.rxFeed1,
                          regStatus.txFeed0,
                          regStatus.txFeed1,
                          regStatus.discarded0,
                          regStatus.discarded1,
                          port0Filtered,
                          port1Filtered,
                          port0SplitId,
                          port1SplitId,
                          outputArbDataFeed);

    kernel.eventHandler(regStatus.rxEvent,
                        eventStream);

}
