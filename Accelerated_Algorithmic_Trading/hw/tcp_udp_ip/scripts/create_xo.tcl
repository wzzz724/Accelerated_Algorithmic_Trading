# 
# Copyright (c) 2019-2020, Xilinx, Inc.
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
# 

if {$::argc != 1} {
    puts "ERROR: Program \"$::argv0\" requires 1 argument!\n"
    puts "Usage: $::argv0 <target>\n"
    puts "Target can be one of: tcp_udp_ip_krnl.xo tcp_ip_krnl.xo udp_ip_krnl.xo\n"
    exit
}

set proj_name "tcp_ip_pack"
set part $::env(XPART)
set path_to_tmp_project "./tmp_tcp_ip_pack"
set path_to_packaged "packaged_top_level"
set ip_repo "./iprepo"

set target [lindex $::argv 0]
set kernel_name [lindex [split $target .] 0]
set top $kernel_name

switch -- $kernel_name {
    udp_ip_krnl {
        set has_udp 1
        set has_tcp 0
    }
    tcp_ip_krnl {
        set has_udp 0
        set has_tcp 1
    }
    tcp_udp_ip_krnl {
        set has_udp 1
        set has_tcp 1
    }
}

create_project -force $proj_name $path_to_tmp_project -part $part
set_property IP_REPO_PATHS $ip_repo [current_fileset]
update_ip_catalog

add_files rtl/tcp_ip_control_s_axi.sv
if {$has_tcp && $has_udp} {
    add_files rtl/axi_lite_crossbar_tcp_udp.sv
    add_files -norecurse [glob rtl/udp_ll/*.v rtl/udp_ll/*.sv]
} elseif {$has_tcp} {
    add_files rtl/axi_lite_crossbar_tcp.sv
    add_files -norecurse [glob rtl/udp_ll/mac_ip*.v rtl/udp_ll/mac_ip*.sv]
} else {
    add_files rtl/axi_lite_crossbar_udp.sv
    add_files -norecurse [glob rtl/udp_ll/*.v rtl/udp_ll/*.sv]
}
add_files rtl/${kernel_name}.sv

set_property top $top [current_fileset]

# HLS IP Cores

if {$has_tcp && $has_udp} {
    create_ip -name ip_handler_tcp_udp -vendor com.xilinx.dcg.fintech -library hls -version 1.0 -module_name ip_handler_tcp_udp_ip
} elseif {$has_tcp} {
    create_ip -name ip_handler_tcp -vendor com.xilinx.dcg.fintech -library hls -version 1.0 -module_name ip_handler_tcp_ip
} else {
    create_ip -name ip_handler_udp -vendor com.xilinx.dcg.fintech -library hls -version 1.0 -module_name ip_handler_udp_ip
}
# create_ip -name mac_ip_encode -vendor com.xilinx.dcg.fintech -library hls -version 1.0 -module_name mac_ip_encode_ip
create_ip -name icmp_server -vendor com.xilinx.dcg.fintech -library hls -version 1.0 -module_name icmp_server_ip
create_ip -name igmp -vendor com.xilinx.dcg.fintech -library hls -version 1.0 -module_name igmp_ip
create_ip -name arp_server_subnet -vendor com.xilinx.dcg.fintech -library hls -version 1.0 -module_name arp_server_subnet_ip
# if {$has_udp} {
    # create_ip -name ipv4 -vendor com.xilinx.dcg.fintech -library hls -version 1.0 -module_name ipv4_ip
    # create_ip -name udp -vendor com.xilinx.dcg.fintech -library hls -version 1.0 -module_name udp_ip
# }
if {$has_tcp} {
    create_ip -name toe -vendor com.xilinx.dcg.fintech -library hls -version 1.0 -module_name toe_ip
    create_ip -name uram_datamover -vendor com.xilinx.dcg.fintech -library hls -version 1.0 -module_name uram_datamover_ip
    create_ip -name hash_table -vendor com.xilinx.dcg.fintech -library hls -version 1.0 -module_name hash_table_ip
    create_ip -name axis_data_fifo -vendor xilinx.com -library ip -version 2.0 -module_name axis_data_fifo_64_d2048
    set_property -dict [list CONFIG.TDATA_NUM_BYTES {64} CONFIG.FIFO_DEPTH {2048} CONFIG.HAS_TKEEP {1} CONFIG.HAS_TLAST {1} CONFIG.HAS_WR_DATA_COUNT {1} CONFIG.HAS_RD_DATA_COUNT {0}] [get_ips axis_data_fifo_64_d2048]
}

# Interconnects

create_ip -name axis_interconnect -vendor xilinx.com -library ip -version 1.1 -module_name axis_interconnect_4to1
set_property -dict [list CONFIG.C_NUM_SI_SLOTS {4} CONFIG.SWITCH_TDATA_NUM_BYTES {8} CONFIG.HAS_TSTRB {false} CONFIG.HAS_TID {false} CONFIG.HAS_TDEST {false} CONFIG.SWITCH_PACKET_MODE {true} CONFIG.C_S00_AXIS_REG_CONFIG {1} CONFIG.C_S01_AXIS_REG_CONFIG {1} CONFIG.C_S02_AXIS_REG_CONFIG {1} CONFIG.C_S03_AXIS_REG_CONFIG {1} CONFIG.C_SWITCH_MAX_XFERS_PER_ARB {0} CONFIG.C_SWITCH_NUM_CYCLES_TIMEOUT {0} CONFIG.M00_AXIS_TDATA_NUM_BYTES {8} CONFIG.S00_AXIS_TDATA_NUM_BYTES {8} CONFIG.S01_AXIS_TDATA_NUM_BYTES {8} CONFIG.S02_AXIS_TDATA_NUM_BYTES {8} CONFIG.S03_AXIS_TDATA_NUM_BYTES {8} CONFIG.M00_S01_CONNECTIVITY {true} CONFIG.M00_S02_CONNECTIVITY {true} CONFIG.M00_S03_CONNECTIVITY {true}] [get_ips axis_interconnect_4to1]

create_ip -name axis_interconnect -vendor xilinx.com -library ip -version 1.1 -module_name axis_interconnect_2to1
set_property -dict [list CONFIG.C_NUM_SI_SLOTS {2} CONFIG.SWITCH_TDATA_NUM_BYTES {8} CONFIG.HAS_TSTRB {false} CONFIG.HAS_TID {false} CONFIG.HAS_TDEST {false} CONFIG.SWITCH_PACKET_MODE {true} CONFIG.C_SWITCH_MAX_XFERS_PER_ARB {0} CONFIG.C_M00_AXIS_REG_CONFIG {1} CONFIG.C_S00_AXIS_REG_CONFIG {1} CONFIG.C_S01_AXIS_REG_CONFIG {1} CONFIG.C_SWITCH_NUM_CYCLES_TIMEOUT {0} CONFIG.M00_AXIS_TDATA_NUM_BYTES {8} CONFIG.S00_AXIS_TDATA_NUM_BYTES {8} CONFIG.S01_AXIS_TDATA_NUM_BYTES {8} CONFIG.M00_S01_CONNECTIVITY {true}] [get_ips axis_interconnect_2to1]

create_ip -name axi_crossbar -vendor xilinx.com -library ip -version 2.1 -module_name axi_crossbar_10
set_property -dict [list CONFIG.NUM_MI {10} CONFIG.ADDR_WIDTH {16} CONFIG.PROTOCOL {AXI4LITE} CONFIG.CONNECTIVITY_MODE {SASD} CONFIG.R_REGISTER {1} CONFIG.S00_WRITE_ACCEPTANCE {1} CONFIG.S01_WRITE_ACCEPTANCE {1} CONFIG.S02_WRITE_ACCEPTANCE {1} CONFIG.S03_WRITE_ACCEPTANCE {1} CONFIG.S04_WRITE_ACCEPTANCE {1} CONFIG.S05_WRITE_ACCEPTANCE {1} CONFIG.S06_WRITE_ACCEPTANCE {1} CONFIG.S07_WRITE_ACCEPTANCE {1} CONFIG.S08_WRITE_ACCEPTANCE {1} CONFIG.S09_WRITE_ACCEPTANCE {1}  CONFIG.S00_READ_ACCEPTANCE {1} CONFIG.S01_READ_ACCEPTANCE {1} CONFIG.S02_READ_ACCEPTANCE {1} CONFIG.S03_READ_ACCEPTANCE {1} CONFIG.S04_READ_ACCEPTANCE {1} CONFIG.S05_READ_ACCEPTANCE {1} CONFIG.S06_READ_ACCEPTANCE {1} CONFIG.S07_READ_ACCEPTANCE {1} CONFIG.S08_READ_ACCEPTANCE {1} CONFIG.S09_READ_ACCEPTANCE {1} CONFIG.M00_WRITE_ISSUING {1} CONFIG.M01_WRITE_ISSUING {1} CONFIG.M02_WRITE_ISSUING {1} CONFIG.M03_WRITE_ISSUING {1} CONFIG.M04_WRITE_ISSUING {1} CONFIG.M05_WRITE_ISSUING {1} CONFIG.M06_WRITE_ISSUING {1} CONFIG.M07_WRITE_ISSUING {1} CONFIG.M08_WRITE_ISSUING {1} CONFIG.M09_WRITE_ISSUING {1} CONFIG.M00_READ_ISSUING {1} CONFIG.M01_READ_ISSUING {1} CONFIG.M02_READ_ISSUING {1} CONFIG.M03_READ_ISSUING {1} CONFIG.M04_READ_ISSUING {1} CONFIG.M05_READ_ISSUING {1} CONFIG.M06_READ_ISSUING {1} CONFIG.M07_READ_ISSUING {1} CONFIG.M08_READ_ISSUING {1} CONFIG.M09_READ_ISSUING {1} CONFIG.S00_SINGLE_THREAD {1} CONFIG.M01_A00_BASE_ADDR {0x0000000000001000} CONFIG.M02_A00_BASE_ADDR {0x0000000000002000} CONFIG.M03_A00_BASE_ADDR {0x0000000000003000} CONFIG.M04_A00_BASE_ADDR {0x0000000000004000} CONFIG.M05_A00_BASE_ADDR {0x0000000000005000} CONFIG.M06_A00_BASE_ADDR {0x0000000000008000} CONFIG.M07_A00_BASE_ADDR {0x000000000000C000} CONFIG.M08_A00_BASE_ADDR {0x000000000000D000} CONFIG.M09_A00_BASE_ADDR {0x000000000000E000} CONFIG.M00_A00_ADDR_WIDTH {12} CONFIG.M01_A00_ADDR_WIDTH {12} CONFIG.M02_A00_ADDR_WIDTH {12} CONFIG.M03_A00_ADDR_WIDTH {12} CONFIG.M04_A00_ADDR_WIDTH {12} CONFIG.M05_A00_ADDR_WIDTH {12} CONFIG.M06_A00_ADDR_WIDTH {14} CONFIG.M07_A00_ADDR_WIDTH {12} CONFIG.M08_A00_ADDR_WIDTH {12} CONFIG.M09_A00_ADDR_WIDTH {12}] [get_ips axi_crossbar_10]
# Register slices

create_ip -name axis_register_slice -vendor xilinx.com -library ip -version 1.1 -module_name axis_register_slice_64
set_property -dict [list CONFIG.TDATA_NUM_BYTES {8} CONFIG.HAS_TKEEP {1} CONFIG.HAS_TLAST {1}] [get_ips axis_register_slice_64]

if {$has_udp} {
    create_ip -name axis_register_slice -vendor xilinx.com -library ip -version 1.1 -module_name udp_ll_axis_reg_slice_64
    set_property -dict [list CONFIG.TDATA_NUM_BYTES {8} CONFIG.HAS_TKEEP {1} CONFIG.HAS_TLAST {1} CONFIG.TUSER_WIDTH {1}] [get_ips udp_ll_axis_reg_slice_64]
    create_ip -name axis_register_slice -vendor xilinx.com -library ip -version 1.1 -module_name udp_ll_reg_slice_256
    set_property -dict [list CONFIG.TDATA_NUM_BYTES {32} CONFIG.HAS_TKEEP {1} CONFIG.HAS_TLAST {1} CONFIG.TUSER_WIDTH {1}] [get_ips udp_ll_reg_slice_256]

    create_ip -name axis_broadcaster -vendor xilinx.com -library ip -version 1.1 -module_name udp_ll_axis_broadcaster
    set_property -dict [list CONFIG.M_TDATA_NUM_BYTES {8} CONFIG.S_TDATA_NUM_BYTES {8} CONFIG.HAS_TKEEP {1} CONFIG.HAS_TLAST {1} CONFIG.M00_TDATA_REMAP {tdata[63:0]} CONFIG.M01_TDATA_REMAP {tdata[63:0]}] [get_ips udp_ll_axis_broadcaster]

    # create_ip -name ila -vendor xilinx.com -library ip -version 6.2 -module_name ila_rx
    # set_property -dict [list CONFIG.C_DATA_DEPTH {1024} CONFIG.C_PROBE9_WIDTH {2} CONFIG.C_PROBE8_WIDTH {2} CONFIG.C_PROBE7_WIDTH {1} CONFIG.C_PROBE6_WIDTH {64} CONFIG.C_PROBE2_WIDTH {64} CONFIG.C_NUM_OF_PROBES {11} CONFIG.C_ENABLE_ILA_AXI_MON {false} CONFIG.C_MONITOR_TYPE {Native}] [get_ips ila_rx]

    # create_ip -name ila -vendor xilinx.com -library ip -version 6.2 -module_name ila_iph
    # set_property -dict [list CONFIG.C_DATA_DEPTH {1024} CONFIG.C_PROBE12_WIDTH {16} CONFIG.C_PROBE11_WIDTH {16} CONFIG.C_PROBE9_WIDTH {2} CONFIG.C_PROBE8_WIDTH {3} CONFIG.C_PROBE7_WIDTH {1} CONFIG.C_PROBE6_WIDTH {64} CONFIG.C_PROBE2_WIDTH {64} CONFIG.C_NUM_OF_PROBES {13} CONFIG.C_ENABLE_ILA_AXI_MON {false} CONFIG.C_MONITOR_TYPE {Native}] [get_ips ila_iph]
}

create_ip -name axis_register_slice -vendor xilinx.com -library ip -version 1.1 -module_name axis_reg_slice_72
set_property -dict [list CONFIG.TDATA_NUM_BYTES {9}] [get_ips axis_reg_slice_72]

create_ip -name axis_register_slice -vendor xilinx.com -library ip -version 1.1 -module_name axis_reg_slice_32
set_property -dict [list CONFIG.TDATA_NUM_BYTES {9}] [get_ips axis_reg_slice_32]

create_ip -name axis_data_fifo -vendor xilinx.com -library ip -version 2.0 -module_name mac_ip_axis_data_fifo_0
set_property -dict [list CONFIG.TDATA_NUM_BYTES {8} CONFIG.FIFO_DEPTH {64} CONFIG.HAS_TKEEP {1} CONFIG.HAS_TLAST {1}] [get_ips mac_ip_axis_data_fifo_0]

create_ip -name fifo_generator -vendor xilinx.com -library ip -version 13.2 -module_name mac_ip_chksum_fifo
set_property -dict [list CONFIG.Fifo_Implementation {Common_Clock_Block_RAM} CONFIG.Performance_Options {First_Word_Fall_Through} CONFIG.Input_Data_Width {16} CONFIG.Input_Depth {16} CONFIG.Output_Data_Width {16} CONFIG.Output_Depth {16} CONFIG.Use_Embedded_Registers {false} CONFIG.Use_Extra_Logic {true} CONFIG.Data_Count_Width {5} CONFIG.Write_Data_Count_Width {5} CONFIG.Read_Data_Count_Width {5} CONFIG.Full_Threshold_Assert_Value {15} CONFIG.Full_Threshold_Negate_Value {14} CONFIG.Empty_Threshold_Assert_Value {4} CONFIG.Empty_Threshold_Negate_Value {5}] [get_ips mac_ip_chksum_fifo]

# debug IP
create_ip -name ila -vendor xilinx.com -library ip -version 6.2 -module_name ila_64
set_property -dict [list CONFIG.C_DATA_DEPTH {2048} CONFIG.C_NUM_OF_PROBES {9} CONFIG.C_EN_STRG_QUAL {1} CONFIG.C_ADV_TRIGGER {true} CONFIG.C_PROBE8_MU_CNT {2} CONFIG.C_PROBE7_MU_CNT {2} CONFIG.C_PROBE6_MU_CNT {2} CONFIG.C_PROBE5_MU_CNT {2} CONFIG.C_PROBE4_MU_CNT {2} CONFIG.C_PROBE3_MU_CNT {2} CONFIG.C_PROBE2_MU_CNT {2} CONFIG.C_PROBE1_MU_CNT {2} CONFIG.C_PROBE0_MU_CNT {2} CONFIG.ALL_PROBE_SAME_MU_CNT {2} CONFIG.C_SLOT_0_AXI_PROTOCOL {AXI4S} CONFIG.C_SLOT_0_AXIS_TDATA_WIDTH {64} CONFIG.C_SLOT_0_AXIS_TID_WIDTH {0} CONFIG.C_SLOT_0_AXIS_TUSER_WIDTH {0} CONFIG.C_SLOT_0_AXIS_TDEST_WIDTH {0} CONFIG.C_ENABLE_ILA_AXI_MON {true} CONFIG.C_MONITOR_TYPE {AXI}] [get_ips ila_64]

update_compile_order -fileset sources_1
update_compile_order -fileset sim_1

ipx::package_project -root_dir $path_to_packaged -vendor xilinx.com -library RTLKernel -taxonomy /KernelIP -import_files -set_current false
ipx::unload_core $path_to_packaged/component.xml
ipx::edit_ip_in_project -upgrade true -name tmp_edit_project -directory $path_to_packaged $path_to_packaged/component.xml
set_property core_revision 1 [ipx::current_core]
foreach up [ipx::get_user_parameters] {
    ipx::remove_user_parameter [get_property NAME $up] [ipx::current_core]
}
set_property sdx_kernel true [ipx::current_core]
set_property sdx_kernel_type rtl [ipx::current_core]
ipx::create_xgui_files [ipx::current_core]
ipx::add_bus_interface ap_clk [ipx::current_core]
set_property abstraction_type_vlnv xilinx.com:signal:clock_rtl:1.0 [ipx::get_bus_interfaces ap_clk -of_objects [ipx::current_core]]
set_property bus_type_vlnv xilinx.com:signal:clock:1.0 [ipx::get_bus_interfaces ap_clk -of_objects [ipx::current_core]]
ipx::add_port_map CLK [ipx::get_bus_interfaces ap_clk -of_objects [ipx::current_core]]
set_property physical_name ap_clk [ipx::get_port_maps CLK -of_objects [ipx::get_bus_interfaces ap_clk -of_objects [ipx::current_core]]]
ipx::associate_bus_interfaces -busif s_axi_control -clock ap_clk [ipx::current_core]

ipx::associate_bus_interfaces -busif m_axis_line        -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif s_axis_line        -clock ap_clk [ipx::current_core]

if {$has_udp} {
    ipx::associate_bus_interfaces -busif m_axis_udp_data     -clock ap_clk [ipx::current_core]
    ipx::associate_bus_interfaces -busif m_axis_udp_metadata -clock ap_clk [ipx::current_core]
    ipx::associate_bus_interfaces -busif s_axis_udp_data     -clock ap_clk [ipx::current_core]
    ipx::associate_bus_interfaces -busif s_axis_udp_metadata -clock ap_clk [ipx::current_core]
}

if {$has_tcp} {
    ipx::associate_bus_interfaces -busif m_axis_listen_port_status -clock ap_clk [ipx::current_core]
    set bifparam [ipx::add_bus_parameter TDATA_NUM_BYTES [ipx::get_bus_interfaces m_axis_listen_port_status -of_objects [ipx::current_core]]]
    set_property value 1 $bifparam
    ipx::associate_bus_interfaces -busif m_axis_notifications      -clock ap_clk [ipx::current_core]
    set bifparam [ipx::add_bus_parameter TDATA_NUM_BYTES [ipx::get_bus_interfaces m_axis_notifications -of_objects [ipx::current_core]]]
    set_property value 16 $bifparam
    ipx::associate_bus_interfaces -busif m_axis_open_status        -clock ap_clk [ipx::current_core]
    set bifparam [ipx::add_bus_parameter TDATA_NUM_BYTES [ipx::get_bus_interfaces m_axis_open_status -of_objects [ipx::current_core]]]
    set_property value 4 $bifparam
    ipx::associate_bus_interfaces -busif m_axis_rx_data            -clock ap_clk [ipx::current_core]
    set bifparam [ipx::add_bus_parameter TDATA_NUM_BYTES [ipx::get_bus_interfaces m_axis_rx_data -of_objects [ipx::current_core]]]
    set_property value 8 $bifparam
    ipx::associate_bus_interfaces -busif m_axis_rx_metadata        -clock ap_clk [ipx::current_core]
    set bifparam [ipx::add_bus_parameter TDATA_NUM_BYTES [ipx::get_bus_interfaces m_axis_rx_metadata -of_objects [ipx::current_core]]]
    set_property value 2 $bifparam
    ipx::associate_bus_interfaces -busif m_axis_tx_status          -clock ap_clk [ipx::current_core]
    set bifparam [ipx::add_bus_parameter TDATA_NUM_BYTES [ipx::get_bus_interfaces m_axis_tx_status -of_objects [ipx::current_core]]]
    set_property value 8 $bifparam
    ipx::associate_bus_interfaces -busif s_axis_listen_port        -clock ap_clk [ipx::current_core]
    set bifparam [ipx::add_bus_parameter TDATA_NUM_BYTES [ipx::get_bus_interfaces s_axis_listen_port -of_objects [ipx::current_core]]]
    set_property value 2 $bifparam
    ipx::associate_bus_interfaces -busif s_axis_close_connection   -clock ap_clk [ipx::current_core]
    set bifparam [ipx::add_bus_parameter TDATA_NUM_BYTES [ipx::get_bus_interfaces s_axis_close_connection -of_objects [ipx::current_core]]]
    set_property value 2 $bifparam
    ipx::associate_bus_interfaces -busif s_axis_open_connection    -clock ap_clk [ipx::current_core]
    set bifparam [ipx::add_bus_parameter TDATA_NUM_BYTES [ipx::get_bus_interfaces s_axis_open_connection -of_objects [ipx::current_core]]]
    set_property value 8 $bifparam
    ipx::associate_bus_interfaces -busif s_axis_read_package       -clock ap_clk [ipx::current_core]
    set bifparam [ipx::add_bus_parameter TDATA_NUM_BYTES [ipx::get_bus_interfaces s_axis_read_package -of_objects [ipx::current_core]]]
    set_property value 4 $bifparam
    ipx::associate_bus_interfaces -busif s_axis_tx_data            -clock ap_clk [ipx::current_core]
    set bifparam [ipx::add_bus_parameter TDATA_NUM_BYTES [ipx::get_bus_interfaces s_axis_tx_data -of_objects [ipx::current_core]]]
    set_property value 8 $bifparam
    ipx::associate_bus_interfaces -busif s_axis_tx_metadata        -clock ap_clk [ipx::current_core]
    set bifparam [ipx::add_bus_parameter TDATA_NUM_BYTES [ipx::get_bus_interfaces s_axis_tx_metadata -of_objects [ipx::current_core]]]
    set_property value 8 $bifparam
}

set_property xpm_libraries {XPM_CDC XPM_MEMORY XPM_FIFO} [ipx::current_core]
set_property supported_families { } [ipx::current_core]
set_property auto_family_support_level level_2 [ipx::current_core]
ipx::update_checksums [ipx::current_core]
ipx::save_core [ipx::current_core]

# For some reason, Vivado won't export all of the simulation files if the export_simulation command is executed prior to the close_project command.
# CRITICAL WARNING: [filemgmt 20-1365] Unable to generate target(s) for the following file is locked: .../arp_server_subnet_ip.xci
# Locked reason:
# IP file '.../arp_server_subnet_ip.xml' for IP 'arp_server_subnet_ip' contains stale content.

close_project -delete

# The temporary solution is to close the project as above and then execute generate_target so that the handles are available.
# Strangely, the project is still open in memory in batch mode. Hence, the quiet flag. Otherwise, it will complain that the project is already open.
open_project -quiet $path_to_tmp_project/${proj_name}.xpr

# The real solution is to skip the steps above and simply upgrade the IP so that Vivado is happy with it.
# However, it is not clear if that will give an issue in hardware as that causes some parameters to be removed.
#upgrade_ip [get_ips]

# Generate and export the simulation files
generate_target simulation [get_ips]
set_property top $top [current_fileset -simset]
set_property target_simulator Questa [current_project]
set_property compxlib.questa_compiled_library_dir /proj/xbuilds/2020.2_daily_latest/clibs/questa/2020.2/lin64/lib [current_project]
launch_simulation -scripts_only -absolute_path
close_project


# following assumes ./packaged_kernels has been populated with the UDP subcore directories
if {[file exists "./packaged_kernels/"] == 1} {
   puts "Older directory detected! Deleting...";
   file delete -force ./packaged_kernels/${kernel_name}
}
file copy -force $path_to_packaged ./packaged_kernels/
file rename -force ./packaged_kernels/$path_to_packaged ./packaged_kernels/${kernel_name}

if {[file exists "./${kernel_name}.xo"] == 1} {
   puts "Older kernel detected. Deleting...";
   file delete -force ./${kernel_name}.xo
}
package_xo -xo_path $target -kernel_name ${kernel_name} -ip_directory ./packaged_kernels -parent_ip_directory ./packaged_kernels/${kernel_name} -kernel_xml scripts/${kernel_name}.xml
