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
/*
 * UDP Top Level
 */
module udp_top (
    // System Signals
    input  wire             ap_clk,
    input  wire             ap_rst_n,

    // Destination Port Lookup result
    input  wire  [7:0]     dst_port_lookup,
    input  wire            dst_port_vld,
    input  wire  [7:0]     expectedBit,

    // ----------- RX ------------- //
    // Incoming Data from IP Handler
    input  wire            s_axis_iph_data_tvalid,
    output wire            s_axis_iph_data_tready,
    input  wire [63:0]     s_axis_iph_data_tdata,
    input  wire  [7:0]     s_axis_iph_data_tkeep,
    input  wire            s_axis_udp_data_tuser,
    input  wire            s_axis_iph_data_tlast,
    input  wire            s_axis_iph_metadata_tvalid,
    input  wire [79:0]     s_axis_iph_metadata_tdata,

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

    output wire             m_axis_tx_meta_tvalid,
    input  wire             m_axis_tx_meta_tready,
    output wire  [63:0]     m_axis_tx_meta_tdata,

    output wire             m_axis_tx_data_tvalid,
    input  wire             m_axis_tx_data_tready,
    output wire   [63:0]    m_axis_tx_data_tdata,
    output wire    [7:0]    m_axis_tx_data_tkeep,
    output wire             m_axis_tx_data_tlast,

    // ----------- Stat Counters ------------- //
    output logic [31:0]     datagrams_recv_invalid,
    output logic [31:0]     datagrams_recv,
    output logic [31:0]     datagrams_transmitted
);

udp_rx udp_rx_i (
    // System Signals
    .ap_clk                       (ap_clk),
    .ap_rst_n                     (ap_rst_n),

    // Destination Port Lookup result
    .dst_port_lookup              (dst_port_lookup),
    .dst_port_vld                 (dst_port_vld),
    .expectedBit                  (expectedBit),

    // Incoming Data from IP Handler
    .s_axis_iph_tvalid            (s_axis_iph_data_tvalid),
    .s_axis_iph_tready            (s_axis_iph_data_tready),
    .s_axis_iph_tdata             (s_axis_iph_data_tdata),
    .s_axis_iph_tkeep             (s_axis_iph_data_tkeep),
    .s_axis_iph_tuser             (s_axis_udp_data_tuser),
    .s_axis_iph_tlast             (s_axis_iph_data_tlast),
    .s_axis_iph_metadata_tvalid   (s_axis_iph_metadata_tvalid),
    .s_axis_iph_metadata_tdata    (s_axis_iph_metadata_tdata),

    // UDP Application Interface
    .m_axis_udp_tvalid            (m_axis_udp_data_tvalid),
    .m_axis_udp_tready            (m_axis_udp_data_tready),
    .m_axis_udp_tdata             (m_axis_udp_data_tdata),
    .m_axis_udp_tkeep             (m_axis_udp_data_tkeep),
    .m_axis_udp_tlast             (m_axis_udp_data_tlast),
    .m_axis_udp_data_tstrb        (m_axis_udp_data_tstrb),
    .m_axis_udp_metadata_tvalid   (m_axis_udp_metadata_tvalid),
    .m_axis_udp_metadata_tready   (m_axis_udp_metadata_tready),
    .m_axis_udp_metadata_tdata    (m_axis_udp_metadata_tdata),
    .m_axis_udp_metadata_tkeep    (m_axis_udp_metadata_tkeep),
    .m_axis_udp_metadata_tlast    (m_axis_udp_metadata_tlast),
    .m_axis_udp_metadata_tstrb    (m_axis_udp_metadata_tstrb),

    // Stat Counters
    .datagrams_recv_invalid       (datagrams_recv_invalid),
    .datagrams_recv               (datagrams_recv) 
);

udp_tx udp_tx_i(
    // System Signals
    .ap_clk                       (ap_clk),
    .ap_rst_n                     (ap_rst_n),

    // Incoming UDP Data from User App
    .s_axis_tx_data_tvalid        (s_axis_tx_data_tvalid),
    .s_axis_tx_data_tready        (s_axis_tx_data_tready),
    .s_axis_tx_data_tdata         (s_axis_tx_data_tdata),
    .s_axis_tx_data_tkeep         (s_axis_tx_data_tkeep),
    .s_axis_tx_data_tstrb         (s_axis_tx_data_tstrb),
    .s_axis_tx_data_tlast         (s_axis_tx_data_tlast),

    // Incoming UDP Meta Data from User App
    .s_axis_tx_meta_tvalid        (s_axis_tx_meta_tvalid),
    .s_axis_tx_meta_tready        (s_axis_tx_meta_tready),
    .s_axis_tx_meta_tdata         (s_axis_tx_meta_tdata),
    .s_axis_tx_meta_tkeep         (s_axis_tx_meta_tkeep),
    .s_axis_tx_meta_tstrb         (s_axis_tx_meta_tstrb),
    .s_axis_tx_meta_tlast         (s_axis_tx_meta_tlast),

    // Output IP Data
    .m_axis_iph_data_tvalid       (m_axis_tx_data_tvalid),
    .m_axis_iph_data_tready       (m_axis_tx_data_tready),
    .m_axis_iph_data_tdata        (m_axis_tx_data_tdata),
    .m_axis_iph_data_tkeep        (m_axis_tx_data_tkeep),
    .m_axis_iph_data_tlast        (m_axis_tx_data_tlast),

    // IP Meta Data
    .m_axis_iph_meta_tvalid       (m_axis_tx_meta_tvalid),
    .m_axis_iph_meta_tready       (m_axis_tx_meta_tready),
    .m_axis_iph_meta_tdata        (m_axis_tx_meta_tdata), 

    // Stat Counter
    .datagrams_transmitted        (datagrams_transmitted)
);

endmodule