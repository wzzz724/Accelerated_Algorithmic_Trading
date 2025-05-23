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

module mac_ip_encode_top #(
    parameter integer C_S_AXI_ADDR_WIDTH = 6,
    parameter integer C_S_AXI_DATA_WIDTH = 32
)(
    input  wire [C_S_AXI_ADDR_WIDTH-1:0]   s_axi_control_awaddr,
    input  wire                            s_axi_control_awvalid,
    output wire                            s_axi_control_awready,
    input  wire [C_S_AXI_DATA_WIDTH-1:0]   s_axi_control_wdata,
    input  wire [C_S_AXI_DATA_WIDTH/8-1:0] s_axi_control_wstrb,
    input  wire                            s_axi_control_wvalid,
    output wire                            s_axi_control_wready,
    output wire [1:0]                      s_axi_control_bresp,
    output wire                            s_axi_control_bvalid,
    input  wire                            s_axi_control_bready,
    input  wire [C_S_AXI_ADDR_WIDTH-1:0]   s_axi_control_araddr,
    input  wire                            s_axi_control_arvalid,
    output wire                            s_axi_control_arready,
    output wire [C_S_AXI_DATA_WIDTH-1:0]   s_axi_control_rdata,
    output wire [1:0]                      s_axi_control_rresp,
    output wire                            s_axi_control_rvalid,
    input  wire                            s_axi_control_rready,

    // System Signals
    input  wire          ap_clk,
    input  wire          ap_rst_n,

    input  wire          s_axis_ip_tvalid,
    output wire          s_axis_ip_tready,
    input  wire  [63:0]  s_axis_ip_tdata,
    input  wire   [7:0]  s_axis_ip_tkeep,
    input  wire          s_axis_ip_tlast,

    input  wire          s_axis_arp_lookup_reply_tvalid,
    output wire          s_axis_arp_lookup_reply_tready,
    input  wire  [71:0]  s_axis_arp_lookup_reply_tdata,

    output wire          m_axis_ip_tvalid,
    input  wire          m_axis_ip_tready,
    output wire  [63:0]  m_axis_ip_tdata,
    output wire   [7:0]  m_axis_ip_tkeep,
    output wire          m_axis_ip_tlast,

    output logic         m_axis_arp_lookup_request_tvalid,
    input  wire          m_axis_arp_lookup_request_tready,
    output logic [31:0]  m_axis_arp_lookup_request_tdata,

    input  wire  [47:0]  myMacAddress,
    input  wire  [31:0]  regSubNetMask,    
    input  wire  [31:0]  regDefaultGateway 
);

logic         ap_rst;
logic [31:0]  ipv4_packets_sent;
logic [31:0]  droppedPkts;

always_ff @(posedge ap_clk) begin
    ap_rst <= !ap_rst_n;    
end

mac_ip_encode_control_s_axi #(
    .C_S_AXI_ADDR_WIDTH( C_S_AXI_ADDR_WIDTH ),
    .C_S_AXI_DATA_WIDTH( C_S_AXI_DATA_WIDTH )
)
mac_control_s_axi_i(
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
    .ARESET                        (ap_rst),
    .ACLK_EN                       (1'b1),
    .ipv4_packets_sent             (ipv4_packets_sent),
    .packets_dropped               (droppedPkts)
);

mac_ip_encode mac_ip_encode_i (
    // System Signals
    .ap_clk                          (ap_clk),
    .ap_rst_n                        (ap_rst_n),

    .s_axis_ip_tvalid                (s_axis_ip_tvalid),
    .s_axis_ip_tready                (s_axis_ip_tready),
    .s_axis_ip_tdata                 (s_axis_ip_tdata),
    .s_axis_ip_tkeep                 (s_axis_ip_tkeep),
    .s_axis_ip_tlast                 (s_axis_ip_tlast),

    .s_axis_arp_lookup_reply_tvalid  (s_axis_arp_lookup_reply_tvalid),
    .s_axis_arp_lookup_reply_tready  (s_axis_arp_lookup_reply_tready),
    .s_axis_arp_lookup_reply_tdata   (s_axis_arp_lookup_reply_tdata),

    .m_axis_ip_tvalid                (m_axis_ip_tvalid),
    .m_axis_ip_tready                (m_axis_ip_tready),
    .m_axis_ip_tdata                 (m_axis_ip_tdata),
    .m_axis_ip_tkeep                 (m_axis_ip_tkeep),
    .m_axis_ip_tlast                 (m_axis_ip_tlast),

    .m_axis_arp_lookup_request_tvalid(m_axis_arp_lookup_request_tvalid),
    .m_axis_arp_lookup_request_tready(m_axis_arp_lookup_request_tready),
    .m_axis_arp_lookup_request_tdata (m_axis_arp_lookup_request_tdata),

    .myMacAddress                    (myMacAddress),
    .regSubNetMask                   (regSubNetMask),    
    .regDefaultGateway               (regDefaultGateway),

    .ipv4_packets_sent               (ipv4_packets_sent),
    .droppedPkts                     (droppedPkts)
);

endmodule