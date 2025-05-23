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

set build $::env(HLSBUILD)
if {$build eq "UDP_ONLY"} {
    set top ip_handler_udp    
} elseif {$build eq "TCP_ONLY"} {
    set top ip_handler_tcp
} else {
    set top ip_handler_tcp_udp
}

set cflags "-D $::env(HLSBUILD)"

open_project ${top}_prj
set_top $top
add_files ../axi_utils.hpp
add_files ../axi_utils.cpp
add_files ip_handler.cpp -cflags $cflags

open_solution "solution1"
set_part $::env(XPART)
create_clock -period $::env(KERNEL_PERIOD) -name default

config_rtl -module_auto_prefix
config_export -ipname $top
csynth_design
export_design -format ip_catalog -display_name "IP Handler for 10G TCP Offload Engine" -vendor "com.xilinx.dcg.fintech" -version "1.0"

exit

