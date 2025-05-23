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
 * Main Top Level for IGMP parser and report generator. Parses incoming data for IGMP query requests
 * Sets a timer. On timer expiry issues a report generation request.
 *	@param[in]		s_axis_data
 *	@param[out]		m_axis_data
 *  @param[in]      host_op_toggle
 *  @param[in]      host_opcode
 *  @param[in]      host_ipAddr
 *  @param[in]      srcIpAddr
 *  @param[in]      host_tableAddr
 *  @param[in]      readVal
 *  @param[in]      enable
 *  @param[out]     invalid_csum
 *  @param[out]     igmp_query
 *  @param[out]     invalid_query
 *  @param[in]      igmp_ver
 */
void igmp(hls::stream<ap_axiu<64,0,0,0> >& s_axis_data,
          hls::stream<ap_axiu<64,0,0,0> >& m_axis_data,
          bool host_op_toggle,
          ap_uint<2> host_opcode,
          uint32_t host_ipAddr,
          uint32_t srcIpAddr,
          uint8_t host_tableAddr,
          uint32_t& readVal,
          bool enable,
          uint32_t &invalid_csum,
          uint32_t &igmp_query,
          uint32_t &invalid_query,
          uint8_t igmp_ver) {
#pragma HLS DATAFLOW disable_start_propagation
#pragma HLS INTERFACE axis register port = s_axis_data
#pragma HLS INTERFACE axis register port = m_axis_data
#pragma HLS INTERFACE ap_ctrl_none port = return
#pragma HLS INTERFACE s_axilite port = host_op_toggle bundle = control
#pragma HLS INTERFACE s_axilite port = host_opcode bundle = control
#pragma HLS INTERFACE s_axilite port = host_ipAddr bundle = control
#pragma HLS INTERFACE s_axilite port = host_tableAddr bundle = control
#pragma HLS INTERFACE s_axilite port = readVal bundle = control
#pragma HLS INTERFACE s_axilite port = invalid_csum bundle = control
#pragma HLS INTERFACE s_axilite port = igmp_query bundle = control
#pragma HLS INTERFACE s_axilite port = invalid_query bundle = control
#pragma HLS INTERFACE s_axilite port = enable bundle = control
#pragma HLS INTERFACE ap_none port = srcIpAddr
#pragma HLS INTERFACE s_axilite port = igmp_ver bundle = control

    static hls::stream<igmpQuery> parser2timer("igmp_parser2timer");
#pragma HLS STREAM depth = 2 variable = parser2timer

    static hls::stream<bool> counter2timer("igmp_counter2timer");
#pragma HLS STREAM depth = 2 variable = counter2timer

    static hls::stream<igmpQuery> timer2reportgen("igmp_timer2reportgen");
#pragma HLS STREAM depth = 2 variable = timer2reportgen

    static hls::stream<bool> is_v3("igmp_is_v3");
#pragma HLS STREAM depth = 2 variable = is_v3

    igmp_parser(s_axis_data, parser2timer, invalid_csum, igmp_query, invalid_query);
    igmp_100ms_count(counter2timer);
    igmp_process_timer(counter2timer, is_v3, parser2timer, timer2reportgen);
    igmp_report_gen(m_axis_data, timer2reportgen, is_v3, host_op_toggle, host_opcode, host_ipAddr, srcIpAddr, host_tableAddr,
                    igmp_ver, readVal, enable);
}
