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

#ifndef ETHERNET_LOOPBACK_H
#define ETHERNET_LOOPBACK_H

#include <stdint.h>
#include <ap_int.h>
#include <ap_axi_sdata.h>
#include <hls_stream.h>

#define LOOPBACK_ENABLE (1<<0)

typedef ap_axis<64,0,0,0> axiWord_t;

extern "C" void loopback(ap_uint<32> &reg_control,
                         hls::stream<axiWord_t> &s_axis_rx,
                         hls::stream<axiWord_t> &m_axis_tx);

#endif
