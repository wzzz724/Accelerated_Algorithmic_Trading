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

#include "loopback.hpp"

void loopback(ap_uint<32> &reg_control,
              hls::stream<axiWord_t> &s_axis_rx,
              hls::stream<axiWord_t> &m_axis_tx)
{
#pragma HLS INTERFACE s_axilite port=reg_control bundle=control
#pragma HLS INTERFACE ap_none port=reg_control
#pragma HLS INTERFACE axis port=s_axis_rx
#pragma HLS INTERFACE axis port=m_axis_tx
#pragma HLS INTERFACE ap_ctrl_none port=return

    axiWord_t rx_word;

#pragma HLS PIPELINE II=1 style=flp

    // drain the rx stream
    if(!s_axis_rx.empty())
    {
        rx_word = s_axis_rx.read();

        // loop the data from rx to tx stream if control bit enabled
        if(LOOPBACK_ENABLE & reg_control)
        {
            m_axis_tx.write(rx_word);
        }
    }

}
