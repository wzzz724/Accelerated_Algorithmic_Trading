/* (c) Copyright 2021 Xilinx, Inc. All rights reserved.
 This file contains confidential and proprietary information
 of Xilinx, Inc. and is protected under U.S. and
 international copyright and other intellectual property
 laws.

 DISCLAIMER
 This disclaimer is not a license and does not grant any
 rights to the materials distributed herewith. Except as
 otherwise provided in a valid license issued to you by
 Xilinx, and to the maximum extent permitted by applicable
 law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
 WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
 AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
 BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
 INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
 (2) Xilinx shall not be liable (whether in contract or tort,
 including negligence, or under any other theory of
 liability) for any loss or damage of any kind or nature
 related to, arising under or in connection with these
 materials, including for any direct, or any indirect,
 special, incidental, or consequential loss or damage
 (including loss of data, profits, goodwill, or any type of
 loss or damage suffered as a result of any action brought
 by a third party) even if such damage or loss was
 reasonably foreseeable or Xilinx had been advised of the
 possibility of the same.

 CRITICAL APPLICATIONS
 Xilinx products are not designed or intended to be fail-
 safe, or for use in any application requiring fail-safe
 performance, such as life-support or safety devices or
 systems, Class III medical devices, nuclear facilities,
 applications related to the deployment of airbags, or any
 other applications that could lead to death, personal
 injury, or severe property or environmental damage
 (individually and collectively, "Critical
 Applications"). Customer assumes the sole risk and
 liability of any use of Xilinx products in Critical
 Applications, subject only to applicable laws and
 regulations governing limitations on product liability.

 THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
 PART OF THIS FILE AT ALL TIMES.
*/

////////////////////////////////////////////////////////////////////////////////
// default_nettype of none prevents implicit wire declaration.
`default_nettype none
`timescale 1 ns / 1 ps
// Top level of the kernel. Do not modify module name, parameters or ports.
module udp_stack #(
    parameter integer C_S_AXI_CONTROL_ADDR_WIDTH = 16,
    parameter integer C_S_AXI_CONTROL_DATA_WIDTH = 32
)
(
    // AXI4-Lite slave interface
    input  wire                                      s_axi_control_awvalid,
    output wire                                      s_axi_control_awready,
    input  wire    [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]  s_axi_control_awaddr,
    input  wire                                      s_axi_control_wvalid,
    output wire                                      s_axi_control_wready,
    input  wire    [C_S_AXI_CONTROL_DATA_WIDTH-1:0]  s_axi_control_wdata,
    input  wire  [C_S_AXI_CONTROL_DATA_WIDTH/8-1:0]  s_axi_control_wstrb,
    input  wire                                      s_axi_control_arvalid,
    output wire                                      s_axi_control_arready,
    input  wire    [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]  s_axi_control_araddr,
    output wire                                      s_axi_control_rvalid,
    input  wire                                      s_axi_control_rready,
    output wire    [C_S_AXI_CONTROL_DATA_WIDTH-1:0]  s_axi_control_rdata,
    output wire                             [2-1:0]  s_axi_control_rresp,
    output wire                                      s_axi_control_bvalid,
    input  wire                                      s_axi_control_bready,
    output wire                             [2-1:0]  s_axi_control_bresp,    

    // System Signals
    input  wire             ap_clk,
    input  wire             ap_rst_n,

    input  wire  [31:0]     my_ip_addr,
    input  wire   [7:0]     ip_protocol,

    // Raw Ethernet Data Input
    input  wire             s_axis_line_tvalid,
    output wire             s_axis_line_tready,
    input  wire  [63:0]     s_axis_line_tdata,
    input  wire   [7:0]     s_axis_line_tkeep,
    input  wire             s_axis_line_tlast,

    // UDP Application Interface
    output wire             m_axis_udp_data_tvalid,
    input  wire             m_axis_udp_data_tready,
    output wire  [63:0]     m_axis_udp_data_tdata,
    output wire   [7:0]     m_axis_udp_data_tkeep,
    output wire             m_axis_udp_data_tlast,
    output wire   [7:0]     m_axis_udp_data_tstrb,    
    output wire             m_axis_udp_metadata_tvalid,
    input  wire             m_axis_udp_metadata_tready,
    output wire [255:0]     m_axis_udp_metadata_tdata,
    output wire  [31:0]     m_axis_udp_metadata_tkeep,
    output wire             m_axis_udp_metadata_tlast,
    output wire  [31:0]     m_axis_udp_metadata_tstrb,  

    // ----------- TX ------------- //
    input  wire             s_axis_tx_meta_tvalid,
    output wire             s_axis_tx_meta_tready,
    input  wire [255:0]     s_axis_tx_meta_tdata,
    input  wire  [31:0]     s_axis_tx_meta_tkeep,
    input  wire             s_axis_tx_meta_tlast,
    input  wire  [31:0]     s_axis_tx_meta_tstrb,    
    
    input  wire             s_axis_tx_data_tvalid,
    output wire             s_axis_tx_data_tready,
    input  wire  [63:0]     s_axis_tx_data_tdata,
    input  wire   [7:0]     s_axis_tx_data_tkeep,
    input  wire             s_axis_tx_data_tlast,
    input  wire   [7:0]     s_axis_tx_data_tstrb,

    output wire             m_axis_tx_data_tvalid,
    input  wire             m_axis_tx_data_tready,
    output wire   [63:0]    m_axis_tx_data_tdata,
    output wire    [7:0]    m_axis_tx_data_tkeep,
    output wire             m_axis_tx_data_tlast
);


wire            ap_rst_n_inv;
// Signals to memory from ip handler
wire [12:0]     mem_lookup_addr;
wire            mem_lookup_ce;
wire            mem_rd_vld;
wire  [7:0]     mem_lookup_val;
wire  [7:0]     expectedBit;

wire [31:0]     datagrams_recv_invalid;
wire [31:0]     datagrams_recv;
wire [31:0]     datagrams_transmitted;

wire            iph2udp_data_tvalid;
wire            iph2udp_data_tready;
wire [63:0]     iph2udp_data_tdata;
wire  [7:0]     iph2udp_data_tkeep;
wire            iph2udp_data_tlast;
wire            iph2udp_data_tuser;
wire            iph2udp_metadata_tvalid;
wire [79:0]     iph2udp_metadata_tdata;

wire            tx_udp2ip_data_tvalid;
wire            tx_udp2ip_data_tready;
wire [63:0]     tx_udp2ip_data_tdata;
wire  [7:0]     tx_udp2ip_data_tkeep;
wire            tx_udp2ip_data_tlast;

wire            tx_udp2ip_meta_tvalid;
wire [63:0]     tx_udp2ip_meta_tdata;
wire            tx_udp2ip_meta_tready;


assign ap_rst_n_inv = !ap_rst_n;

udp_control_s_axi #(
    .C_S_AXI_ADDR_WIDTH( C_S_AXI_CONTROL_ADDR_WIDTH ),
    .C_S_AXI_DATA_WIDTH( C_S_AXI_CONTROL_DATA_WIDTH )
)
control_s_axi_i(
    .AWVALID                       (s_axi_control_awvalid),
    .AWREADY                       (s_axi_control_awready),
    .AWADDR                        (s_axi_control_awaddr),
    .WVALID                        (s_axi_control_wvalid),
    .WREADY                        (s_axi_control_wready),
    .WDATA                         (s_axi_control_wdata),
    .WSTRB                         (s_axi_control_wstrb),
    .ARVALID                       (s_axi_control_arvalid),
    .ARREADY                       (s_axi_control_arready),
    .ARADDR                        (s_axi_control_araddr),
    .RVALID                        (s_axi_control_rvalid),
    .RREADY                        (s_axi_control_rready),
    .RDATA                         (s_axi_control_rdata),
    .RRESP                         (s_axi_control_rresp),
    .BVALID                        (s_axi_control_bvalid),
    .BREADY                        (s_axi_control_bready),
    .BRESP                         (s_axi_control_bresp),
    .ACLK                          (ap_clk),
    .ARESET                        (ap_rst_n_inv),
    .ACLK_EN                       (1'b1),
    .datagrams_transmitted         (datagrams_transmitted),
    .datagrams_recv                (datagrams_recv),
    .datagrams_recv_invalid_port   (datagrams_recv_invalid),
    .arrPorts_address0             (mem_lookup_addr),
    .arrPorts_ce0                  (mem_lookup_ce),
    .arrPorts_vld                  (mem_rd_vld),
    .arrPorts_q0                   (mem_lookup_val)
);

rx_ip_handler rx_ip_handler_i(
    // System Signals
    .ap_clk                      (ap_clk),
    .ap_rst_n                    (ap_rst_n),

    // IP Addr used by UDP
    .my_ip_addr                  (my_ip_addr),

    // Signals to memory
    .mem_addr                    (mem_lookup_addr),
    .mem_ce                      (mem_lookup_ce),
    .expectedBit                 (expectedBit),

    // Raw Ethernet Data Input
    .s_axis_line_tvalid          (s_axis_line_tvalid),
    .s_axis_line_tready          (s_axis_line_tready),
    .s_axis_line_tdata           (s_axis_line_tdata),
    .s_axis_line_tkeep           (s_axis_line_tkeep),
    .s_axis_line_tlast           (s_axis_line_tlast),

    // Data to UDP block
    .m_axis_udp_data_tvalid      (iph2udp_data_tvalid),
    .m_axis_udp_data_tready      (iph2udp_data_tready),
    .m_axis_udp_data_tdata       (iph2udp_data_tdata),
    .m_axis_udp_data_tkeep       (iph2udp_data_tkeep),
    .m_axis_udp_data_tuser       (iph2udp_data_tuser),
    .m_axis_udp_data_tlast       (iph2udp_data_tlast),
    .m_axis_udp_metadata_tvalid  (iph2udp_metadata_tvalid),
    .m_axis_udp_metadata_tdata   (iph2udp_metadata_tdata)
);

udp_top udp_top_i(
    // System Signals
    .ap_clk                      (ap_clk),
    .ap_rst_n                    (ap_rst_n),

    // Destination Port Lookup result
    .dst_port_lookup             (mem_lookup_val),
    .dst_port_vld                (mem_rd_vld),
    .expectedBit                 (expectedBit),

    // Incoming Data from IP Handler
    .s_axis_iph_data_tvalid      (iph2udp_data_tvalid),
    .s_axis_iph_data_tready      (iph2udp_data_tready),
    .s_axis_iph_data_tdata       (iph2udp_data_tdata),
    .s_axis_iph_data_tkeep       (iph2udp_data_tkeep),
    .s_axis_udp_data_tuser       (iph2udp_data_tuser),
    .s_axis_iph_data_tlast       (iph2udp_data_tlast),
    .s_axis_iph_metadata_tvalid  (iph2udp_metadata_tvalid),
    .s_axis_iph_metadata_tdata   (iph2udp_metadata_tdata),

    // UDP Application Interface
    .m_axis_udp_data_tvalid      (m_axis_udp_data_tvalid),
    .m_axis_udp_data_tready      (m_axis_udp_data_tready),
    .m_axis_udp_data_tdata       (m_axis_udp_data_tdata),
    .m_axis_udp_data_tkeep       (m_axis_udp_data_tkeep),
    .m_axis_udp_data_tlast       (m_axis_udp_data_tlast),
    .m_axis_udp_data_tstrb       (m_axis_udp_data_tstrb),    
    .m_axis_udp_metadata_tvalid  (m_axis_udp_metadata_tvalid),
    .m_axis_udp_metadata_tready  (m_axis_udp_metadata_tready),
    .m_axis_udp_metadata_tdata   (m_axis_udp_metadata_tdata),
    .m_axis_udp_metadata_tkeep   (m_axis_udp_metadata_tkeep),
    .m_axis_udp_metadata_tlast   (m_axis_udp_metadata_tlast),
    .m_axis_udp_metadata_tstrb   (m_axis_udp_metadata_tstrb),

    // ----------- TX ------------- //
    .s_axis_tx_meta_tvalid       (s_axis_tx_meta_tvalid),
    .s_axis_tx_meta_tready       (s_axis_tx_meta_tready),
    .s_axis_tx_meta_tdata        (s_axis_tx_meta_tdata),
    .s_axis_tx_meta_tkeep        (s_axis_tx_meta_tkeep),
    .s_axis_tx_meta_tlast        (s_axis_tx_meta_tlast),
    .s_axis_tx_meta_tstrb        (s_axis_tx_meta_tstrb),    
    
    .s_axis_tx_data_tvalid       (s_axis_tx_data_tvalid),
    .s_axis_tx_data_tready       (s_axis_tx_data_tready),
    .s_axis_tx_data_tdata        (s_axis_tx_data_tdata),
    .s_axis_tx_data_tkeep        (s_axis_tx_data_tkeep),
    .s_axis_tx_data_tlast        (s_axis_tx_data_tlast),
    .s_axis_tx_data_tstrb        (s_axis_tx_data_tstrb),

    // UDP Data to IPv4 Block
    .m_axis_tx_meta_tvalid       (tx_udp2ip_meta_tvalid),
    .m_axis_tx_meta_tready       (tx_udp2ip_meta_tready),
    .m_axis_tx_meta_tdata        (tx_udp2ip_meta_tdata),

    .m_axis_tx_data_tvalid       (tx_udp2ip_data_tvalid),
    .m_axis_tx_data_tready       (tx_udp2ip_data_tready),
    .m_axis_tx_data_tdata        (tx_udp2ip_data_tdata),
    .m_axis_tx_data_tkeep        (tx_udp2ip_data_tkeep),
    .m_axis_tx_data_tlast        (tx_udp2ip_data_tlast),

    // Stat Counters
    .datagrams_recv_invalid      (datagrams_recv_invalid),
    .datagrams_recv              (datagrams_recv),
    .datagrams_transmitted       (datagrams_transmitted)
);

tx_ipv4 tx_ipv4_i (
    // System Signals
    .ap_clk                  (ap_clk),
    .ap_rst_n                (ap_rst_n),

    .local_ipv4_addr         (my_ip_addr),
    .protocol                (ip_protocol),

    // Tx UDP data
    .s_axis_udp_tvalid       (tx_udp2ip_data_tvalid),
    .s_axis_udp_tready       (tx_udp2ip_data_tready),
    .s_axis_udp_tdata        (tx_udp2ip_data_tdata),
    .s_axis_udp_tkeep        (tx_udp2ip_data_tkeep),
    .s_axis_udp_tlast        (tx_udp2ip_data_tlast),

    .udp_meta_valid          (tx_udp2ip_meta_tvalid),
    .udp_meta_ready          (tx_udp2ip_meta_tready),
    .udp_meta_data           (tx_udp2ip_meta_tdata[47:0]),

    // IPv4 Output Packet
    .m_axis_ip_data_tvalid   (m_axis_tx_data_tvalid),
    .m_axis_ip_data_tready   (m_axis_tx_data_tready),
    .m_axis_ip_data_tdata    (m_axis_tx_data_tdata),
    .m_axis_ip_data_tkeep    (m_axis_tx_data_tkeep),
    .m_axis_ip_data_tuser    (),
    .m_axis_ip_data_tlast    (m_axis_tx_data_tlast)    
);
    

endmodule