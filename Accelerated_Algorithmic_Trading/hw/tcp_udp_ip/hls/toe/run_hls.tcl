# Copyright (c) 2019 Xilinx, Inc.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are 
# met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its
#    contributors may be used to endorse or promote products derived from this
#    software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE#
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# THE POSSIBILITY OF SUCH DAMAGE.
open_project toe_prj

set_top toe

add_files ack_delay/ack_delay.cpp
add_files close_timer/close_timer.cpp
add_files event_engine/event_engine.cpp
add_files port_table/port_table.cpp
add_files probe_timer/probe_timer.cpp
add_files retransmit_timer/retransmit_timer.cpp
add_files rx_app_if/rx_app_if.cpp
add_files rx_app_stream_if/rx_app_stream_if.cpp
add_files rx_engine/rx_engine.cpp -cflags -std=c++11
add_files rx_sar_table/rx_sar_table.cpp
add_files session_lookup_controller/session_lookup_controller.cpp
#add_files session_lookup_controller/session_lookup_controller/stub_session_lookup.cpp
add_files state_table/state_table.cpp
add_files tx_app_if/tx_app_if.cpp
add_files tx_app_stream_if/tx_app_stream_if.cpp
add_files tx_engine/tx_engine.cpp
add_files tx_sar_table/tx_sar_table.cpp
add_files tx_app_interface/tx_app_interface.cpp
add_files ../axi_utils.cpp
add_files toe.cpp
add_files -tb toe_tb.cpp

open_solution "solution1"
set_part $::env(XPART)
create_clock -period $::env(KERNEL_PERIOD) -name default

config_rtl -module_auto_prefix
csynth_design
set dispName "10G TCP Offload Engine"
export_design -format ip_catalog -display_name $dispName -description "TCP Offload Engine supporting 10Gbps line rate, up to 10K concurrent sessions." -vendor "com.xilinx.dcg.fintech" -version "1.0"
exit
