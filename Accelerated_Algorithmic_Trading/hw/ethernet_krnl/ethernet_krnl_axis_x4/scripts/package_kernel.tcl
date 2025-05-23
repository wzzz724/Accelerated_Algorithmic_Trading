# /*******************************************************************************
# (c) Copyright 2019 Xilinx, Inc. All rights reserved.
# This file contains confidential and proprietary information
# of Xilinx, Inc. and is protected under U.S. and
# international copyright and other intellectual property
# laws.
#
# DISCLAIMER
# This disclaimer is not a license and does not grant any
# rights to the materials distributed herewith. Except as
# otherwise provided in a valid license issued to you by
# Xilinx, and to the maximum extent permitted by applicable
# law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
# WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
# AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
# BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
# INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
# (2) Xilinx shall not be liable (whether in contract or tort,
# including negligence, or under any other theory of
# liability) for any loss or damage of any kind or nature
# related to, arising under or in connection with these
# materials, including for any direct, or any indirect,
# special, incidental, or consequential loss or damage
# (including loss of data, profits, goodwill, or any type of
# loss or damage suffered as a result of any action brought
# by a third party) even if such damage or loss was
# reasonably foreseeable or Xilinx had been advised of the
# possibility of the same.
#
# CRITICAL APPLICATIONS
# Xilinx products are not designed or intended to be fail-
# safe, or for use in any application requiring fail-safe
# performance, such as life-support or safety devices or
# systems, Class III medical devices, nuclear facilities,
# applications related to the deployment of airbags, or any
# other applications that could lead to death, personal
# injury, or severe property or environmental damage
# (individually and collectively, "Critical
# Applications"). Customer assumes the sole risk and
# liability of any use of Xilinx products in Critical
# Applications, subject only to applicable laws and
# regulations governing limitations on product liability.
#
# THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
# PART OF THIS FILE AT ALL TIMES.
#
# *******************************************************************************/
set path_to_hdl "./src"
set path_to_cmn "./../common"
set path_to_packaged "./packaged_kernel/${suffix}"
set path_to_tmp_project "./tmp_kernel_pack_${suffix}"

set words [split $device "_"]
set board [lindex $words 1]
set __intfName gt_serial_port
set __refclkIntfName gt_refclk

if {[string compare -nocase $board "u200"] == 0} {
set projPart "xcu200-fsgd2104-2-e"
set boardPart "xilinx.com:au200:part0:1.2"
# U250_START
} elseif {[string compare -nocase $board "u250"] == 0} {
set projPart "xcu250-figd2104-2L-e"
set boardPart "xilinx.com:au250:part0:1.3"
# U50_START
} elseif {[string compare -nocase $board "u50"] == 0} {
set projPart "xcu50-fsvh2104-2L-e"
set boardPart "xilinx.com:au50:part0:1.1"
# U50_END
} elseif {[string compare -nocase $board "u280"] == 0} {
set projPart "xcu280-fsvh2892-2L-e"
set boardPart "xilinx.com:au280:part0:1.2"
} else {
	puts "Unknown Board"
	exit
}
set projName kernel_pack
create_project -force $projName $path_to_tmp_project -part $projPart
#set_property board_part $boardPart [current_project]
set __board [string tolower $board]
add_files -norecurse [glob $path_to_hdl/*.v $path_to_hdl/*.sv $path_to_cmn/*.v $path_to_cmn/*.sv]
add_files -fileset constrs_1 -norecurse [glob $path_to_hdl/*.xdc]

# create_ip -name xxv_ethernet -vendor xilinx.com -library ip -version 3.3 -module_name xxv_ethernet_x4_0
# set_property -dict [list CONFIG.CORE {Ethernet MAC+PCS/PMA 32-bit} CONFIG.NUM_OF_CORES {$channels} CONFIG.INCLUDE_USER_FIFO {0} CONFIG.LINE_RATE {10} CONFIG.BASE_R_KR {BASE-R} CONFIG.INCLUDE_AXI4_INTERFACE {0} CONFIG.ENABLE_PIPELINE_REG {1} CONFIG.ADD_GT_CNTRL_STS_PORTS {1}] [get_ips xxv_ethernet_x4_0]

create_ip -name xxv_ethernet -vendor xilinx.com -library ip -version 4.* -module_name xxv_ethernet_x4_0
set_property -dict [list CONFIG.CORE {Ethernet MAC+PCS/PMA 32-bit} CONFIG.NUM_OF_CORES {4} CONFIG.INCLUDE_USER_FIFO {0} CONFIG.LINE_RATE {10} CONFIG.BASE_R_KR {BASE-R} CONFIG.INCLUDE_AXI4_INTERFACE {0} CONFIG.ENABLE_PIPELINE_REG {1} CONFIG.ADD_GT_CNTRL_STS_PORTS {1}] [get_ips xxv_ethernet_x4_0]

if {[string compare -nocase $board "u200"] == 0} {
  set_property -dict [list CONFIG.GT_REF_CLK_FREQ {161.1328125} CONFIG.GT_GROUP_SELECT {Quad_X1Y11} CONFIG.LANE1_GT_LOC {X1Y44} CONFIG.LANE2_GT_LOC {X1Y45} CONFIG.LANE3_GT_LOC {X1Y46} CONFIG.LANE4_GT_LOC {X1Y47}] [get_ips xxv_ethernet_x4_0]
} elseif {[string compare -nocase $board "u250"] == 0} {
  set_property -dict [list CONFIG.GT_REF_CLK_FREQ {161.1328125} CONFIG.GT_GROUP_SELECT {Quad_X1Y10} CONFIG.LANE1_GT_LOC {X1Y40} CONFIG.LANE2_GT_LOC {X1Y41} CONFIG.LANE3_GT_LOC {X1Y42} CONFIG.LANE4_GT_LOC {X1Y43}] [get_ips xxv_ethernet_x4_0]
} elseif {[string compare -nocase $board "u280"] == 0} {
  set_property -dict [list CONFIG.GT_REF_CLK_FREQ {156.25} CONFIG.GT_GROUP_SELECT {Quad_X0Y10} CONFIG.LANE1_GT_LOC {X0Y40} CONFIG.LANE2_GT_LOC {X0Y41} CONFIG.GT_DRP_CLK {50}] [get_ips xxv_ethernet_x4_0]
# U50_START
} elseif {[string compare -nocase $board "u50"] == 0} {
  set_property -dict [list CONFIG.GT_REF_CLK_FREQ {161.1328125}  CONFIG.GT_GROUP_SELECT {Quad_X0Y7} CONFIG.LANE1_GT_LOC {X0Y28} CONFIG.LANE2_GT_LOC {X0Y29} CONFIG.LANE3_GT_LOC {X0Y30} CONFIG.LANE4_GT_LOC {X0Y31}] [get_ips xxv_ethernet_x4_0]
# U50_END
} else {
    puts "Unknown board $board"
    exit
}

create_ip -name axis_data_fifo -vendor xilinx.com -library ip -version 2.0 -module_name rx_axis_data_fifo_0
set_property -dict [list CONFIG.FIFO_DEPTH {1024} CONFIG.FIFO_MODE {1} CONFIG.TDATA_NUM_BYTES {8} CONFIG.TUSER_WIDTH {1} CONFIG.IS_ACLK_ASYNC {0} CONFIG.HAS_TKEEP {1} CONFIG.HAS_TLAST {1} CONFIG.HAS_WR_DATA_COUNT {0} CONFIG.HAS_RD_DATA_COUNT {0}  CONFIG.HAS_AEMPTY {0} CONFIG.HAS_PROG_EMPTY {0} CONFIG.HAS_AFULL {1} CONFIG.HAS_PROG_FULL {0}] [get_ips rx_axis_data_fifo_0]

create_ip -name axis_data_fifo -vendor xilinx.com -library ip -version 2.0 -module_name axis_data_fifo_1
set_property -dict [list CONFIG.TDATA_NUM_BYTES {8} CONFIG.TUSER_WIDTH {8} CONFIG.IS_ACLK_ASYNC {1} CONFIG.FIFO_DEPTH {16} CONFIG.HAS_TKEEP {1} CONFIG.HAS_TLAST {1}] [get_ips axis_data_fifo_1]

create_ip -name axis_dwidth_converter -vendor xilinx.com -library ip -version 1.1 -module_name axis_dwidth_converter_rx
set_property -dict [list CONFIG.S_TDATA_NUM_BYTES {4} CONFIG.M_TDATA_NUM_BYTES {8} CONFIG.TUSER_BITS_PER_BYTE {1} CONFIG.HAS_TLAST {1} CONFIG.HAS_TKEEP {1} CONFIG.HAS_MI_TKEEP {1}] [get_ips axis_dwidth_converter_rx]

create_ip -name axis_dwidth_converter -vendor xilinx.com -library ip -version 1.1 -module_name axis_dwidth_converter_tx
set_property -dict [list CONFIG.S_TDATA_NUM_BYTES {8} CONFIG.M_TDATA_NUM_BYTES {4} CONFIG.TUSER_BITS_PER_BYTE {1} CONFIG.HAS_TLAST {1} CONFIG.HAS_TKEEP {1} CONFIG.HAS_MI_TKEEP {1}] [get_ips axis_dwidth_converter_tx]

create_ip -name fifo_generator -vendor xilinx.com -library ip -version 13.2 -module_name rx_fifo_generator_0
set_property -dict [list CONFIG.Fifo_Implementation {Common_Clock_Distributed_RAM} CONFIG.Input_Data_Width {1} CONFIG.Input_Depth {128} CONFIG.Output_Data_Width {2} CONFIG.Output_Depth {16} CONFIG.Use_Embedded_Registers {false} CONFIG.Reset_Type {Synchronous_Reset} CONFIG.Full_Flags_Reset_Value {0} CONFIG.Data_Count_Width {4} CONFIG.Write_Data_Count_Width {4} CONFIG.Read_Data_Count_Width {4}] [get_ips rx_fifo_generator_0]

create_ip -name fifo_generator -vendor xilinx.com -library ip -version 13.2 -module_name tx_sof_fifo
set_property -dict [list CONFIG.Component_Name {tx_sof_fifo} CONFIG.Performance_Options {First_Word_Fall_Through} CONFIG.Input_Data_Width {1} CONFIG.Input_Depth {512} CONFIG.Output_Data_Width {1} CONFIG.Output_Depth {512} CONFIG.Data_Count_Width {9} CONFIG.Write_Data_Count_Width {9} CONFIG.Read_Data_Count_Width {9} CONFIG.Full_Threshold_Assert_Value {511} CONFIG.Full_Threshold_Negate_Value {510} CONFIG.Empty_Threshold_Assert_Value {4} CONFIG.Empty_Threshold_Negate_Value {5}] [get_ips tx_sof_fifo]

create_ip -name axis_register_slice -vendor xilinx.com -library ip -version 1.1 -module_name axis_register_slice_0
set_property -dict [list CONFIG.TDATA_NUM_BYTES {8} CONFIG.HAS_TKEEP {1} CONFIG.HAS_TLAST {1} CONFIG.TUSER_WIDTH {1}] [get_ips axis_register_slice_0]

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
ipx::add_bus_interface $__intfName [ipx::current_core]
set_property interface_mode master [ipx::get_bus_interfaces $__intfName -of_objects [ipx::current_core]]
set_property abstraction_type_vlnv xilinx.com:interface:gt_rtl:1.0 [ipx::get_bus_interfaces $__intfName -of_objects [ipx::current_core]]
set_property bus_type_vlnv xilinx.com:interface:gt:1.0 [ipx::get_bus_interfaces $__intfName -of_objects [ipx::current_core]]
ipx::add_port_map GRX_P [ipx::get_bus_interfaces $__intfName -of_objects [ipx::current_core]]
set_property physical_name gt_rxp_in [ipx::get_port_maps GRX_P -of_objects [ipx::get_bus_interfaces $__intfName -of_objects [ipx::current_core]]]
ipx::add_port_map GTX_N [ipx::get_bus_interfaces $__intfName -of_objects [ipx::current_core]]
set_property physical_name gt_txn_out [ipx::get_port_maps GTX_N -of_objects [ipx::get_bus_interfaces $__intfName -of_objects [ipx::current_core]]]
ipx::add_port_map GRX_N [ipx::get_bus_interfaces $__intfName -of_objects [ipx::current_core]]
set_property physical_name gt_rxn_in [ipx::get_port_maps GRX_N -of_objects [ipx::get_bus_interfaces $__intfName -of_objects [ipx::current_core]]]
ipx::add_port_map GTX_P [ipx::get_bus_interfaces $__intfName -of_objects [ipx::current_core]]
set_property physical_name gt_txp_out [ipx::get_port_maps GTX_P -of_objects [ipx::get_bus_interfaces $__intfName -of_objects [ipx::current_core]]]

# GT Differential Reference Clock

ipx::add_bus_interface $__refclkIntfName [ipx::current_core]
set_property abstraction_type_vlnv xilinx.com:interface:diff_clock_rtl:1.0 [ipx::get_bus_interfaces $__refclkIntfName -of_objects [ipx::current_core]]
set_property bus_type_vlnv xilinx.com:interface:diff_clock:1.0 [ipx::get_bus_interfaces $__refclkIntfName -of_objects [ipx::current_core]]
ipx::add_port_map CLK_P [ipx::get_bus_interfaces $__refclkIntfName -of_objects [ipx::current_core]]
set_property physical_name ${__refclkIntfName}_p [ipx::get_port_maps CLK_P -of_objects [ipx::get_bus_interfaces $__refclkIntfName -of_objects [ipx::current_core]]]
ipx::add_port_map CLK_N [ipx::get_bus_interfaces $__refclkIntfName -of_objects [ipx::current_core]]
set_property physical_name ${__refclkIntfName}_n [ipx::get_port_maps CLK_N -of_objects [ipx::get_bus_interfaces $__refclkIntfName -of_objects [ipx::current_core]]]
ipx::associate_bus_interfaces -busif rx0_axis -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif rx1_axis -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif rx2_axis -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif rx3_axis -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif tx0_axis -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif tx1_axis -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif tx2_axis -clock ap_clk [ipx::current_core]
ipx::associate_bus_interfaces -busif tx3_axis -clock ap_clk [ipx::current_core]

set_property xpm_libraries {XPM_CDC XPM_MEMORY XPM_FIFO} [ipx::current_core]
set_property supported_families { } [ipx::current_core]
set_property auto_family_support_level level_2 [ipx::current_core]
ipx::update_checksums [ipx::current_core]
ipx::save_core [ipx::current_core]
close_project -delete
