/*
 * Copyright (c) 2016, 2019-2020, Xilinx, Inc.
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
`timescale 1ns / 1ps

`default_nettype none

//`define RX_DDR_BYPASS
//`define DEBUG

module tcp_udp_ip_krnl #(
    parameter integer C_S_AXI_CONTROL_ADDR_WIDTH = 16,
    parameter integer C_S_AXI_CONTROL_DATA_WIDTH = 32
)(
    input wire           ap_clk,
    input wire           ap_rst_n,
    // AXI4-Lite slave interface
    input  wire                                    s_axi_control_awvalid,
    output wire                                    s_axi_control_awready,
    input  wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   s_axi_control_awaddr ,
    input  wire                                    s_axi_control_wvalid ,
    output wire                                    s_axi_control_wready ,
    input  wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   s_axi_control_wdata  ,
    input  wire [C_S_AXI_CONTROL_DATA_WIDTH/8-1:0] s_axi_control_wstrb  ,
    input  wire                                    s_axi_control_arvalid,
    output wire                                    s_axi_control_arready,
    input  wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   s_axi_control_araddr ,
    output wire                                    s_axi_control_rvalid ,
    input  wire                                    s_axi_control_rready ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   s_axi_control_rdata  ,
    output wire [2-1:0]                            s_axi_control_rresp  ,
    output wire                                    s_axi_control_bvalid ,
    input  wire                                    s_axi_control_bready ,
    output wire [2-1:0]                            s_axi_control_bresp  ,

    // network interface streams
    output wire          m_axis_line_TVALID,
    input wire           m_axis_line_TREADY,
    output wire[63:0]    m_axis_line_TDATA,
    output wire[7:0]     m_axis_line_TKEEP,
    output wire          m_axis_line_TLAST,

    input wire           s_axis_line_TVALID,
    output wire          s_axis_line_TREADY,
    input wire[63:0]     s_axis_line_TDATA,
    input wire[7:0]      s_axis_line_TKEEP,
    input wire           s_axis_line_TLAST,

    //application interface streams
    output wire          m_axis_listen_port_status_TVALID,
    input wire           m_axis_listen_port_status_TREADY,
    output wire[7:0]     m_axis_listen_port_status_TDATA,
    output wire          m_axis_listen_port_status_TKEEP,
    output wire          m_axis_listen_port_status_TLAST,   
    output wire          m_axis_notifications_TVALID,
    input wire           m_axis_notifications_TREADY,
    output wire[127:0]   m_axis_notifications_TDATA,
    output wire[15:0]    m_axis_notifications_TKEEP,
    output wire          m_axis_notifications_TLAST,    
    output wire          m_axis_open_status_TVALID,
    input wire           m_axis_open_status_TREADY,
    output wire[31:0]    m_axis_open_status_TDATA,
    output wire [3:0]    m_axis_open_status_TKEEP,
    output wire          m_axis_open_status_TLAST, 
    output wire          m_axis_rx_data_TVALID,
    input wire           m_axis_rx_data_TREADY,
    output wire[63:0]    m_axis_rx_data_TDATA,
    output wire[7:0]     m_axis_rx_data_TSTRB,
    output wire[7:0]     m_axis_rx_data_TKEEP,
    output wire          m_axis_rx_data_TLAST,
    output wire          m_axis_rx_metadata_TVALID,
    input wire           m_axis_rx_metadata_TREADY,
    output wire[15:0]    m_axis_rx_metadata_TDATA,
    output wire [1:0]    m_axis_rx_metadata_TSTRB,
    output wire [1:0]    m_axis_rx_metadata_TKEEP,
    output wire          m_axis_rx_metadata_TLAST,    
    output wire          m_axis_tx_status_TVALID,
    input wire           m_axis_tx_status_TREADY,
    output wire[63:0]    m_axis_tx_status_TDATA,
    input wire           s_axis_listen_port_TVALID,
    output wire          s_axis_listen_port_TREADY,
    input wire[15:0]     s_axis_listen_port_TDATA,
    input wire [1:0]     s_axis_listen_port_TKEEP,
    input wire [1:0]     s_axis_listen_port_TSTRB,
    input wire           s_axis_listen_port_TLAST,
    //input wire           s_axis_close_port_TVALID,
    //output wire          s_axis_close_port_TREADY,
    //input wire[15:0]     s_axis_close_port_TDATA,
    input wire           s_axis_close_connection_TVALID,
    output wire          s_axis_close_connection_TREADY,
    input wire[15:0]     s_axis_close_connection_TDATA,
    input wire [1:0]     s_axis_close_connection_TKEEP,
    input wire           s_axis_close_connection_TLAST,
    input wire           s_axis_open_connection_TVALID,
    output wire          s_axis_open_connection_TREADY,
    input wire[63:0]     s_axis_open_connection_TDATA,
    input wire[7:0]      s_axis_open_connection_TKEEP,
    input wire           s_axis_open_connection_TLAST,    
    input wire           s_axis_read_package_TVALID,
    output wire          s_axis_read_package_TREADY,
    input wire[31:0]     s_axis_read_package_TDATA,
    input wire [1:0]     s_axis_read_package_TKEEP,
    input wire [1:0]     s_axis_read_package_TSTRB,
    input wire           s_axis_read_package_TLAST,
    input wire           s_axis_tx_data_TVALID,
    output wire          s_axis_tx_data_TREADY,
    input wire[63:0]     s_axis_tx_data_TDATA,
    input wire[7:0]      s_axis_tx_data_TSTRB,
    input wire[7:0]      s_axis_tx_data_TKEEP,
    input wire           s_axis_tx_data_TLAST,
    input wire[7:0]      s_axis_tx_metadata_TSTRB,
    input wire[7:0]      s_axis_tx_metadata_TKEEP,
    input wire           s_axis_tx_metadata_TLAST,   
    input wire           s_axis_tx_metadata_TVALID,
    output wire          s_axis_tx_metadata_TREADY,
    input wire[63:0]    s_axis_tx_metadata_TDATA, 
    output wire          m_axis_udp_data_TVALID,
    input wire           m_axis_udp_data_TREADY,
    output wire[63:0]    m_axis_udp_data_TDATA,
    output wire[7:0]     m_axis_udp_data_TKEEP,
    output wire          m_axis_udp_data_TLAST,
    output wire[7 : 0]   m_axis_udp_data_TSTRB,    
    output wire          m_axis_udp_metadata_TVALID,
    input wire           m_axis_udp_metadata_TREADY,
    output wire[255:0]   m_axis_udp_metadata_TDATA,
    output wire[31 : 0]  m_axis_udp_metadata_TKEEP,
    output wire          m_axis_udp_metadata_TLAST,
    output wire[31 : 0]  m_axis_udp_metadata_TSTRB,  
    input wire           s_axis_udp_data_TVALID,
    output wire          s_axis_udp_data_TREADY,
    input wire[63:0]     s_axis_udp_data_TDATA,
    input wire[7:0]      s_axis_udp_data_TKEEP,
    input wire[7 : 0]    s_axis_udp_data_TSTRB,    
    input wire           s_axis_udp_data_TLAST,   
    input wire           s_axis_udp_metadata_TVALID,
    output wire          s_axis_udp_metadata_TREADY,
    input wire[255:0]    s_axis_udp_metadata_TDATA,
    input wire[31 : 0]   s_axis_udp_metadata_TKEEP,
    input wire[31 : 0]   s_axis_udp_metadata_TSTRB,
    input wire           s_axis_udp_metadata_TLAST
    );

    localparam integer C_S_AXI_CONTROL_CONTROL_ADDR_WIDTH = 6;
    localparam integer C_S_AXI_CONTROL_CONTROL_DATA_WIDTH = 32;
    localparam integer C_S_AXI_CONTROL_IGMP_ADDR_WIDTH = 7;
    localparam integer C_S_AXI_CONTROL_IGMP_DATA_WIDTH = 32;
    localparam integer C_S_AXI_CONTROL_IPH_ADDR_WIDTH = 7;
    localparam integer C_S_AXI_CONTROL_IPH_DATA_WIDTH = 32;
    localparam integer C_S_AXI_CONTROL_MIE_ADDR_WIDTH = 6;
    localparam integer C_S_AXI_CONTROL_MIE_DATA_WIDTH = 32;
    localparam integer C_S_AXI_CONTROL_UDP_ADDR_WIDTH = 14;
    localparam integer C_S_AXI_CONTROL_UDP_DATA_WIDTH = 32;
    localparam integer C_S_AXI_CONTROL_ARP_SERVER_ADDR_WIDTH = 8;
    localparam integer C_S_AXI_CONTROL_ARP_SERVER_DATA_WIDTH = 32;
    localparam integer C_S_AXI_CONTROL_ICMP_SERVER_ADDR_WIDTH = 6;
    localparam integer C_S_AXI_CONTROL_ICMP_SERVER_DATA_WIDTH = 32;
    localparam integer C_S_AXI_CONTROL_TOE_ADDR_WIDTH = 8;
    localparam integer C_S_AXI_CONTROL_TOE_DATA_WIDTH = 32;    
    wire [47:0] mac_address;                // 00:0A:35:02:9D:E5 = 48'h000A35029DE5
    wire [47:0] mac_address_mangled;        // 00:0A:35:02:9D:E5 = 48'hE59D02350A00
    wire [31:0] ip_address;                 // 10.1.212.209 = 32'h0A01D4D1
    wire [31:0] ip_address_mangled;         // 10.1.212.209 = 32'hD1D4010A
    wire [31:0] ip_subnet_mask;             // 255.255.255.0 = 32'hFFFFFF00
    wire [31:0] ip_subnet_mask_mangled;     // 255.255.255.0 = 32'h00FFFFFF;
    wire [31:0] ip_default_gateway;         // 10.1.212.1 = 32'h0A01D401;
    wire [31:0] ip_default_gateway_mangled; // 10.1.212.1 = 32'h01D4010A;


    // IP Handler Outputs
    wire            axi_iph_to_arp_slice_tvalid;
    wire            axi_iph_to_arp_slice_tready;
    wire[63:0]      axi_iph_to_arp_slice_tdata;
    wire[7:0]       axi_iph_to_arp_slice_tkeep;
    wire            axi_iph_to_arp_slice_tlast;
    wire            axi_iph_to_icmp_slice_tvalid;
    wire            axi_iph_to_icmp_slice_tready;
    wire[63:0]      axi_iph_to_icmp_slice_tdata;
    wire[7:0]       axi_iph_to_icmp_slice_tkeep;
    wire            axi_iph_to_icmp_slice_tlast;

    //Slice connections on RX path
    wire            axi_arp_slice_to_arp_tvalid;
    wire            axi_arp_slice_to_arp_tready;
    wire[63:0]      axi_arp_slice_to_arp_tdata;
    wire[7:0]       axi_arp_slice_to_arp_tkeep;
    wire            axi_arp_slice_to_arp_tlast;
    wire            axi_icmp_slice_to_icmp_tvalid;
    wire            axi_icmp_slice_to_icmp_tready;
    wire[63:0]      axi_icmp_slice_to_icmp_tdata;
    wire[7:0]       axi_icmp_slice_to_icmp_tkeep;
    wire            axi_icmp_slice_to_icmp_tlast;


    // MAC-IP Encode Inputs
    wire            axi_intercon_to_mie_tvalid;
    wire            axi_intercon_to_mie_tready;
    wire[63:0]      axi_intercon_to_mie_tdata;
    wire[7:0]       axi_intercon_to_mie_tkeep;
    wire            axi_intercon_to_mie_tlast;
    wire            axi_mie_to_intercon_tvalid;
    wire            axi_mie_to_intercon_tready;
    wire[63:0]      axi_mie_to_intercon_tdata;
    wire[7:0]       axi_mie_to_intercon_tkeep;
    wire            axi_mie_to_intercon_tlast;
    //Slice connections on RX path
    wire            axi_arp_to_mac_merger_tvalid;
    wire            axi_arp_to_mac_merger_tready;
    wire[63:0]      axi_arp_to_mac_merger_tdata;
    wire[7:0]       axi_arp_to_mac_merger_tkeep;
    wire            axi_arp_to_mac_merger_tlast;
    wire            axi_icmp_to_merge_tvalid;
    wire            axi_icmp_to_merge_tready;
    wire[63:0]      axi_icmp_to_merge_tdata;
    wire[7:0]       axi_icmp_to_merge_tkeep;
    wire            axi_icmp_to_merge_tlast;

    wire            axi_iph_to_toe_slice_tvalid;
    wire            axi_iph_to_toe_slice_tready;
    wire[63:0]      axi_iph_to_toe_slice_tdata;
    wire[7:0]       axi_iph_to_toe_slice_tkeep;
    wire            axi_iph_to_toe_slice_tlast;
    wire            axis_broadcast_to_iph_tvalid;
    wire            axis_broadcast_to_iph_tready;
    wire[63:0]      axis_broadcast_to_iph_tdata;
    wire[7:0]       axis_broadcast_to_iph_tkeep;
    wire            axis_broadcast_to_iph_tlast;
    
    wire            axi_iph_to_igmp_slice_tvalid;
    wire            axi_iph_to_igmp_slice_tready;
    wire[63:0]      axi_iph_to_igmp_slice_tdata;
    wire[7:0]       axi_iph_to_igmp_slice_tkeep;
    wire            axi_iph_to_igmp_slice_tlast;

    wire            axi_igmp_slice_to_igmp_tvalid;
    wire            axi_igmp_slice_to_igmp_tready;
    wire[63:0]      axi_igmp_slice_to_igmp_tdata;
    wire[7:0]       axi_igmp_slice_to_igmp_tkeep;
    wire            axi_igmp_slice_to_igmp_tlast;

    wire            axi_igmp_to_merge_tvalid;
    wire            axi_igmp_to_merge_tready;
    wire[63:0]      axi_igmp_to_merge_tdata;
    wire[7:0]       axi_igmp_to_merge_tkeep;
    wire            axi_igmp_to_merge_tlast;

    wire            axi_toe_slice_to_toe_tvalid;
    wire            axi_toe_slice_to_toe_tready;
    wire[63:0]      axi_toe_slice_to_toe_tdata;
    wire[7:0]       axi_toe_slice_to_toe_tkeep;
    wire            axi_toe_slice_to_toe_tlast;
    wire        axi_udp_to_merge_tvalid;
    wire        axi_udp_to_merge_tready;
    wire[63:0]  axi_udp_to_merge_tdata;
    wire[7:0]   axi_udp_to_merge_tkeep;
    wire        axi_udp_to_merge_tlast;
    wire        axi_iph_to_udp_slice_tvalid;
    wire        axi_iph_to_udp_slice_tready;
    wire[63:0]  axi_iph_to_udp_slice_tdata;
    wire[7:0]   axi_iph_to_udp_slice_tkeep;
    wire        axi_iph_to_udp_slice_tlast;

    wire        axi_udp_slice_to_udp_tvalid;
    wire        axi_udp_slice_to_udp_tready;
    wire[63:0]  axi_udp_slice_to_udp_tdata;
    wire[7:0]   axi_udp_slice_to_udp_tkeep;
    wire        axi_udp_slice_to_udp_tlast;

    wire        s_axis_line_udp_TREADY;
    wire        s_axis_line_iph_TREADY;

logic  [1 : 0]   m_axis_broadcast_tvalid;
logic  [1 : 0]   m_axis_broadcast_tready;
logic  [127 : 0] m_axis_broadcast_tdata;
logic  [15 : 0]  m_axis_broadcast_tkeep;
logic  [1 : 0]   m_axis_broadcast_tlast;

logic          axis_broadcast_to_udp_tvalid;
logic          axis_broadcast_to_udp_tready;
logic  [63:0]  axis_broadcast_to_udp_tdata;
logic   [7:0]  axis_broadcast_to_udp_tkeep;
logic          axis_broadcast_to_udp_tlast; 

    wire        axi_toe_to_merge_tvalid;
    wire        axi_toe_to_merge_tready;
    wire[63:0]  axi_toe_to_merge_tdata;
    wire[7:0]   axi_toe_to_merge_tkeep;
    wire        axi_toe_to_merge_tlast;


    // TCP Offload SmartCAM signals //
    wire        upd_req_TVALID;
    wire        upd_req_TREADY;
    wire[127:0] upd_req_TDATA;
    wire[15:0]  upd_req_TKEEP;
    wire[15:0]  upd_req_TSTRB;
    wire        upd_req_TLAST;    
    wire        upd_rsp_TVALID;
    wire        upd_rsp_TREADY;
    wire[127:0] upd_rsp_TDATA;
    wire[15:0]  upd_rsp_TKEEP;
    wire[15:0]  upd_rsp_TSTRB;
    wire        upd_rsp_TLAST;     

    // Had to remove this "if" block from the
    // coditional as elaboration was still trying
    // to find the connection when !(HAS_TCP).
    wire        lup_req_TVALID;
    wire        lup_req_TREADY;
    wire[127:0] lup_req_TDATA;
    wire[15:0]  lup_req_TKEEP;
    wire[15:0]  lup_req_TSTRB;
    wire        lup_req_TLAST;        
    wire        lup_rsp_TVALID;
    wire        lup_rsp_TREADY;
    wire[127:0] lup_rsp_TDATA;
    wire[15:0]  lup_rsp_TKEEP;
    wire[15:0]  lup_rsp_TSTRB;
    wire        lup_rsp_TLAST;        
    //RX Buffer bypass data streams
    wire axis_rxbuffer2app_tvalid;
    wire axis_rxbuffer2app_tready;
    wire[63:0] axis_rxbuffer2app_tdata;
    wire[7:0] axis_rxbuffer2app_tkeep;
    wire axis_rxbuffer2app_tlast;

    wire axis_tcp2rxbuffer_tvalid;
    wire axis_tcp2rxbuffer_tready;
    wire[63:0] axis_tcp2rxbuffer_tdata;
    wire[7:0] axis_tcp2rxbuffer_tkeep;
    wire axis_tcp2rxbuffer_tlast;

    wire[31:0] rx_buffer_data_count;
    // Tx URAM Datamover
    wire         m_axis_txread_cmd_TVALID;
    wire         m_axis_txread_cmd_TREADY;
    wire [127:0] m_axis_txread_cmd_TDATA;
    wire  [15:0] m_axis_txread_cmd_TKEEP;
    wire         m_axis_txread_cmd_TLAST;    
    wire         m_axis_txwrite_cmd_TVALID;
    wire         m_axis_txwrite_cmd_TREADY;
    wire [127:0] m_axis_txwrite_cmd_TDATA;
    wire  [15:0] m_axis_txwrite_cmd_TKEEP;
    wire   [0:0] m_axis_txwrite_cmd_TLAST;    
    wire         s_axis_txwrite_sts_TVALID;
    wire         s_axis_txwrite_sts_TREADY;    
    wire   [7:0] s_axis_txwrite_sts_TDATA;
    wire         s_axis_txwrite_sts_TKEEP;
    wire         s_axis_txwrite_sts_TLAST;      
    wire         s_axis_txread_data_TVALID;
    wire         s_axis_txread_data_TREADY;
    wire  [63:0] s_axis_txread_data_TDATA;
    wire   [7:0] s_axis_txread_data_TSTRB;
    wire   [7:0] s_axis_txread_data_TKEEP;
    wire         s_axis_txread_data_TLAST;
    wire         m_axis_txwrite_data_TVALID;
    wire         m_axis_txwrite_data_TREADY;
    wire  [63:0] m_axis_txwrite_data_TDATA;
    wire   [7:0] m_axis_txwrite_data_TSTRB;
    wire   [7:0] m_axis_txwrite_data_TKEEP;
    wire         m_axis_txwrite_data_TLAST;
    wire [31:0] scalar00;

    // Register and distribute ip address
    wire[31:0]  dhcp_ip_address;
    wire        dhcp_ip_address_en;
    reg[47:0]   mie_mac_address;
    reg[47:0]   arp_mac_address;
    reg[31:0]   iph_ip_address;
    reg[31:0]   arp_ip_address;
    reg[31:0]   toe_ip_address;
    reg[31:0]   udp_ip_address;
    reg[31:0]   mie_ip_subnet_mask;
    reg[31:0]   mie_ip_default_gateway;

    always @(posedge ap_clk)
    begin
        if (ap_rst_n == 0) begin
            mie_mac_address <= 48'h000000000000;
            arp_mac_address <= 48'h000000000000;
            iph_ip_address <= 32'h00000000;
            arp_ip_address <= 32'h00000000;
            toe_ip_address <= 32'h00000000;
            udp_ip_address <= 32'h00000000;
            mie_ip_subnet_mask <= 32'h00000000;
            mie_ip_default_gateway <= 32'h00000000;
        end
        else begin
            mie_mac_address <= mac_address_mangled;
            arp_mac_address <= mac_address_mangled;
            iph_ip_address <= ip_address_mangled;
            arp_ip_address <= ip_address_mangled;
            toe_ip_address <= ip_address_mangled;
            udp_ip_address <= ip_address_mangled;
            mie_ip_subnet_mask <= ip_subnet_mask_mangled;
            mie_ip_default_gateway <= ip_default_gateway_mangled;
        end
    end

    wire [C_S_AXI_CONTROL_TOE_ADDR_WIDTH-1:0] s_axi_toe_i_awaddr;
    wire        s_axi_toe_i_awvalid;
    wire        s_axi_toe_i_awready;
    wire [C_S_AXI_CONTROL_TOE_DATA_WIDTH-1:0] s_axi_toe_i_wdata;
    wire [ 3:0] s_axi_toe_i_wstrb;
    wire        s_axi_toe_i_wvalid;
    wire        s_axi_toe_i_wready;
    wire [ 1:0] s_axi_toe_i_bresp;
    wire        s_axi_toe_i_bvalid;
    wire        s_axi_toe_i_bready; 
    wire [C_S_AXI_CONTROL_TOE_ADDR_WIDTH-1:0] s_axi_toe_i_araddr;
    wire        s_axi_toe_i_arvalid;
    wire        s_axi_toe_i_arready;
    wire [C_S_AXI_CONTROL_TOE_DATA_WIDTH-1:0] s_axi_toe_i_rdata;
    wire [ 1:0] s_axi_toe_i_rresp;
    wire        s_axi_toe_i_rvalid;
    wire        s_axi_toe_i_rready;

    // Tie off unused TSTRB (but present for connectivity)
    assign m_axis_rx_data_TSTRB = m_axis_rx_data_TKEEP;

    toe_ip toe_inst (
        .s_axi_control_AWVALID(s_axi_toe_i_awvalid),
        .s_axi_control_AWREADY(s_axi_toe_i_awready),
        .s_axi_control_AWADDR(s_axi_toe_i_awaddr),
        .s_axi_control_WVALID(s_axi_toe_i_wvalid),
        .s_axi_control_WREADY(s_axi_toe_i_wready),
        .s_axi_control_WDATA(s_axi_toe_i_wdata),
        .s_axi_control_WSTRB(s_axi_toe_i_wstrb),
        .s_axi_control_ARVALID(s_axi_toe_i_arvalid),
        .s_axi_control_ARREADY(s_axi_toe_i_arready),
        .s_axi_control_ARADDR(s_axi_toe_i_araddr),
        .s_axi_control_RVALID(s_axi_toe_i_rvalid),
        .s_axi_control_RREADY(s_axi_toe_i_rready),
        .s_axi_control_RDATA(s_axi_toe_i_rdata),
        .s_axi_control_RRESP(s_axi_toe_i_rresp),
        .s_axi_control_BVALID(s_axi_toe_i_bvalid),
        .s_axi_control_BREADY(s_axi_toe_i_bready),
        .s_axi_control_BRESP(s_axi_toe_i_bresp),         
        // Data output
        .m_axis_tcp_data_TVALID(axi_toe_to_merge_tvalid),
        .m_axis_tcp_data_TREADY(axi_toe_to_merge_tready),
        .m_axis_tcp_data_TDATA(axi_toe_to_merge_tdata),
        .m_axis_tcp_data_TSTRB(),
        .m_axis_tcp_data_TKEEP(axi_toe_to_merge_tkeep),
        .m_axis_tcp_data_TLAST(axi_toe_to_merge_tlast),
        // Data input
        .s_axis_tcp_data_TVALID(axi_toe_slice_to_toe_tvalid),
        .s_axis_tcp_data_TREADY(axi_toe_slice_to_toe_tready),
        .s_axis_tcp_data_TDATA(axi_toe_slice_to_toe_tdata),
        .s_axis_tcp_data_TSTRB('1),
        .s_axis_tcp_data_TKEEP(axi_toe_slice_to_toe_tkeep),
        .s_axis_tcp_data_TLAST(axi_toe_slice_to_toe_tlast),
        // rx buffer read path
        .s_axis_rxread_data_TVALID(axis_rxbuffer2app_tvalid),
        .s_axis_rxread_data_TREADY(axis_rxbuffer2app_tready),
        .s_axis_rxread_data_TDATA(axis_rxbuffer2app_tdata),
        .s_axis_rxread_data_TKEEP(axis_rxbuffer2app_tkeep),
        .s_axis_rxread_data_TLAST(axis_rxbuffer2app_tlast),
        .s_axis_rxread_data_TSTRB('1),
        // rx buffer write path
        .m_axis_rxwrite_data_TVALID(axis_tcp2rxbuffer_tvalid),
        .m_axis_rxwrite_data_TREADY(axis_tcp2rxbuffer_tready),
        .m_axis_rxwrite_data_TDATA(axis_tcp2rxbuffer_tdata),
        .m_axis_rxwrite_data_TKEEP(axis_tcp2rxbuffer_tkeep),
        .m_axis_rxwrite_data_TLAST(axis_tcp2rxbuffer_tlast),
        .m_axis_rxwrite_data_TSTRB(),        
        .axis_data_count(rx_buffer_data_count),
        .axis_max_data_count(32'd2048),
        // tx read commands
        .m_axis_txread_cmd_TVALID(m_axis_txread_cmd_TVALID),
        .m_axis_txread_cmd_TREADY(m_axis_txread_cmd_TREADY),
        .m_axis_txread_cmd_TDATA(m_axis_txread_cmd_TDATA),
        .m_axis_txread_cmd_TKEEP(m_axis_txread_cmd_TKEEP),
        .m_axis_txread_cmd_TSTRB(),
        .m_axis_txread_cmd_TLAST(m_axis_txread_cmd_TLAST),        
        //tx write commands
        .m_axis_txwrite_cmd_TVALID(m_axis_txwrite_cmd_TVALID),
        .m_axis_txwrite_cmd_TREADY(m_axis_txwrite_cmd_TREADY),
        .m_axis_txwrite_cmd_TDATA(m_axis_txwrite_cmd_TDATA),
        .m_axis_txwrite_cmd_TSTRB(),
        .m_axis_txwrite_cmd_TKEEP(m_axis_txwrite_cmd_TKEEP),
        .m_axis_txwrite_cmd_TLAST(m_axis_txwrite_cmd_TLAST),        
        // tx write status
        .s_axis_txwrite_sts_TVALID(s_axis_txwrite_sts_TVALID),
        .s_axis_txwrite_sts_TREADY(s_axis_txwrite_sts_TREADY),
        .s_axis_txwrite_sts_TDATA(s_axis_txwrite_sts_TDATA),
        .s_axis_txwrite_sts_TSTRB('1),
        .s_axis_txwrite_sts_TKEEP(s_axis_txwrite_sts_TKEEP),
        .s_axis_txwrite_sts_TLAST(s_axis_txwrite_sts_TLAST),        
        // tx read path
        .s_axis_txread_data_TVALID(s_axis_txread_data_TVALID),
        .s_axis_txread_data_TREADY(s_axis_txread_data_TREADY),
        .s_axis_txread_data_TDATA(s_axis_txread_data_TDATA),
        .s_axis_txread_data_TSTRB(s_axis_txread_data_TSTRB),
        .s_axis_txread_data_TKEEP(s_axis_txread_data_TKEEP),
        .s_axis_txread_data_TLAST(s_axis_txread_data_TLAST),
        // tx write path
        .m_axis_txwrite_data_TVALID(m_axis_txwrite_data_TVALID),
        .m_axis_txwrite_data_TREADY(m_axis_txwrite_data_TREADY),
        .m_axis_txwrite_data_TDATA(m_axis_txwrite_data_TDATA),
        .m_axis_txwrite_data_TSTRB(m_axis_txwrite_data_TSTRB),
        .m_axis_txwrite_data_TKEEP(m_axis_txwrite_data_TKEEP),
        .m_axis_txwrite_data_TLAST(m_axis_txwrite_data_TLAST),

        .m_axis_session_upd_req_TVALID(upd_req_TVALID),
        .m_axis_session_upd_req_TREADY(upd_req_TREADY),
        .m_axis_session_upd_req_TDATA(upd_req_TDATA),
        .m_axis_session_upd_req_TKEEP(upd_req_TKEEP),
        .m_axis_session_upd_req_TSTRB(upd_req_TSTRB),
        .m_axis_session_upd_req_TLAST(upd_req_TLAST),        

        .s_axis_session_upd_rsp_TVALID(upd_rsp_TVALID),
        .s_axis_session_upd_rsp_TREADY(upd_rsp_TREADY),
        .s_axis_session_upd_rsp_TDATA(upd_rsp_TDATA),
        .s_axis_session_upd_rsp_TKEEP(upd_rsp_TKEEP),
        .s_axis_session_upd_rsp_TSTRB(upd_rsp_TSTRB),
        .s_axis_session_upd_rsp_TLAST(upd_rsp_TLAST),         

        .m_axis_session_lup_req_TVALID(lup_req_TVALID),
        .m_axis_session_lup_req_TREADY(lup_req_TREADY),
        .m_axis_session_lup_req_TDATA(lup_req_TDATA),
        .m_axis_session_lup_req_TKEEP(lup_req_TKEEP),
        .m_axis_session_lup_req_TSTRB(lup_req_TSTRB),
        .m_axis_session_lup_req_TLAST(lup_req_TLAST),         
        .s_axis_session_lup_rsp_TVALID(lup_rsp_TVALID),
        .s_axis_session_lup_rsp_TREADY(lup_rsp_TREADY),
        .s_axis_session_lup_rsp_TDATA(lup_rsp_TDATA),
        .s_axis_session_lup_rsp_TKEEP(lup_rsp_TKEEP),
        .s_axis_session_lup_rsp_TSTRB(lup_rsp_TSTRB),
        .s_axis_session_lup_rsp_TLAST(lup_rsp_TLAST),        

        /* Application Interface */
        // listen&close port
        .s_axis_listen_port_req_TVALID(s_axis_listen_port_TVALID),
        .s_axis_listen_port_req_TREADY(s_axis_listen_port_TREADY),
        .s_axis_listen_port_req_TDATA(s_axis_listen_port_TDATA),
        .s_axis_listen_port_req_TKEEP(s_axis_listen_port_TKEEP),
        .s_axis_listen_port_req_TSTRB(s_axis_listen_port_TSTRB),
        .s_axis_listen_port_req_TLAST(s_axis_listen_port_TLAST),         
        .m_axis_listen_port_rsp_TVALID(m_axis_listen_port_status_TVALID),
        .m_axis_listen_port_rsp_TREADY(m_axis_listen_port_status_TREADY),
        .m_axis_listen_port_rsp_TDATA(m_axis_listen_port_status_TDATA),
        .m_axis_listen_port_rsp_TKEEP(m_axis_listen_port_status_TKEEP),
        .m_axis_listen_port_rsp_TSTRB(),
        .m_axis_listen_port_rsp_TLAST(m_axis_listen_port_status_TLAST),         

        // notification & read request
        .m_axis_notification_TVALID(m_axis_notifications_TVALID),
        .m_axis_notification_TREADY(m_axis_notifications_TREADY),
        .m_axis_notification_TDATA(m_axis_notifications_TDATA),
        .m_axis_notification_TKEEP(m_axis_notifications_TKEEP),
        .m_axis_notification_TSTRB(),
        .m_axis_notification_TLAST(m_axis_notifications_TLAST),        
        .s_axis_rx_data_req_TVALID(s_axis_read_package_TVALID),
        .s_axis_rx_data_req_TREADY(s_axis_read_package_TREADY),
        .s_axis_rx_data_req_TDATA(s_axis_read_package_TDATA),
        .s_axis_rx_data_req_TKEEP(s_axis_read_package_TKEEP),
        .s_axis_rx_data_req_TSTRB(s_axis_read_package_TSTRB),
        .s_axis_rx_data_req_TLAST(s_axis_read_package_TLAST),         

        // open&close connection
        .s_axis_open_conn_req_TVALID(s_axis_open_connection_TVALID),
        .s_axis_open_conn_req_TREADY(s_axis_open_connection_TREADY),
        .s_axis_open_conn_req_TDATA(s_axis_open_connection_TDATA),
        .s_axis_open_conn_req_TKEEP(s_axis_open_connection_TKEEP),
        .s_axis_open_conn_req_TSTRB('1),
        .s_axis_open_conn_req_TLAST(s_axis_open_connection_TLAST),        
        .m_axis_open_conn_rsp_TVALID(m_axis_open_status_TVALID),
        .m_axis_open_conn_rsp_TREADY(m_axis_open_status_TREADY),
        .m_axis_open_conn_rsp_TDATA(m_axis_open_status_TDATA),
        .m_axis_open_conn_rsp_TSTRB(),
        .m_axis_open_conn_rsp_TKEEP(m_axis_open_status_TKEEP),
        .m_axis_open_conn_rsp_TLAST(m_axis_open_status_TLAST),        
        .s_axis_close_conn_req_TVALID(s_axis_close_connection_TVALID),
        .s_axis_close_conn_req_TREADY(s_axis_close_connection_TREADY),
        .s_axis_close_conn_req_TDATA(s_axis_close_connection_TDATA),
        .s_axis_close_conn_req_TSTRB('1),
        .s_axis_close_conn_req_TKEEP(s_axis_close_connection_TKEEP),
        .s_axis_close_conn_req_TLAST(s_axis_close_connection_TLAST),        

        // rx data
        .m_axis_rx_data_rsp_metadata_TVALID(m_axis_rx_metadata_TVALID),
        .m_axis_rx_data_rsp_metadata_TREADY(m_axis_rx_metadata_TREADY),
        .m_axis_rx_data_rsp_metadata_TDATA(m_axis_rx_metadata_TDATA),
        .m_axis_rx_data_rsp_metadata_TSTRB(m_axis_rx_metadata_TSTRB),
        .m_axis_rx_data_rsp_metadata_TKEEP(m_axis_rx_metadata_TKEEP),
        .m_axis_rx_data_rsp_metadata_TLAST(m_axis_rx_metadata_TLAST),        
        .m_axis_rx_data_rsp_TVALID(m_axis_rx_data_TVALID),
        .m_axis_rx_data_rsp_TREADY(m_axis_rx_data_TREADY),
        .m_axis_rx_data_rsp_TDATA(m_axis_rx_data_TDATA),
        .m_axis_rx_data_rsp_TSTRB(),
        .m_axis_rx_data_rsp_TKEEP(m_axis_rx_data_TKEEP),
        .m_axis_rx_data_rsp_TLAST(m_axis_rx_data_TLAST),

        // tx data
        .s_axis_tx_data_req_metadata_TVALID(s_axis_tx_metadata_TVALID),
        .s_axis_tx_data_req_metadata_TREADY(s_axis_tx_metadata_TREADY),
        .s_axis_tx_data_req_metadata_TDATA(s_axis_tx_metadata_TDATA),
        .s_axis_tx_data_req_metadata_TSTRB(s_axis_tx_metadata_TSTRB),
        .s_axis_tx_data_req_metadata_TKEEP(s_axis_tx_metadata_TKEEP),
        .s_axis_tx_data_req_metadata_TLAST(s_axis_tx_metadata_TLAST),        
        .s_axis_tx_data_req_TVALID(s_axis_tx_data_TVALID),
        .s_axis_tx_data_req_TREADY(s_axis_tx_data_TREADY),
        .s_axis_tx_data_req_TDATA(s_axis_tx_data_TDATA),
        .s_axis_tx_data_req_TSTRB(s_axis_tx_data_TSTRB),
        .s_axis_tx_data_req_TKEEP(s_axis_tx_data_TKEEP),
        .s_axis_tx_data_req_TLAST(s_axis_tx_data_TLAST),
        .m_axis_tx_data_rsp_TVALID(m_axis_tx_status_TVALID),
        .m_axis_tx_data_rsp_TREADY(m_axis_tx_status_TREADY),
        .m_axis_tx_data_rsp_TDATA(m_axis_tx_status_TDATA),
        .m_axis_tx_data_rsp_TKEEP(),
        .m_axis_tx_data_rsp_TSTRB(),        
        .m_axis_tx_data_rsp_TLAST(),

        .myIpAddress(toe_ip_address),
        .ap_clk(ap_clk),
        .ap_rst_n(ap_rst_n)
    );
    //RX BUFFER FIFO
    axis_data_fifo_64_d2048 rx_buffer_fifo (
        .s_axis_aresetn(ap_rst_n),          // input wire s_axis_aresetn
        .s_axis_aclk(ap_clk),                // input wire s_axis_aclk
        .s_axis_tvalid(axis_tcp2rxbuffer_tvalid),
        .s_axis_tready(axis_tcp2rxbuffer_tready),
        .s_axis_tdata(axis_tcp2rxbuffer_tdata),
        .s_axis_tkeep(axis_tcp2rxbuffer_tkeep),
        .s_axis_tlast(axis_tcp2rxbuffer_tlast),
        .m_axis_tvalid(axis_rxbuffer2app_tvalid),
        .m_axis_tready(axis_rxbuffer2app_tready),
        .m_axis_tdata(axis_rxbuffer2app_tdata),
        .m_axis_tkeep(axis_rxbuffer2app_tkeep),
        .m_axis_tlast(axis_rxbuffer2app_tlast),
        .axis_wr_data_count(rx_buffer_data_count[11:0])
    );
    assign rx_buffer_data_count[31:12] = 20'h0;
    hash_table_ip hash_table_inst (
        .ap_clk(ap_clk),
        .ap_rst_n(ap_rst_n),
        .s_axis_lup_req_TVALID(lup_req_TVALID),
        .s_axis_lup_req_TREADY(lup_req_TREADY),
        .s_axis_lup_req_TDATA(lup_req_TDATA),
        .s_axis_lup_req_TKEEP(lup_req_TKEEP),
        .s_axis_lup_req_TSTRB(lup_req_TSTRB),
        .s_axis_lup_req_TLAST(lup_req_TLAST),        
        .m_axis_lup_rsp_TVALID(lup_rsp_TVALID),
        .m_axis_lup_rsp_TREADY(lup_rsp_TREADY),
        .m_axis_lup_rsp_TDATA(lup_rsp_TDATA),
        .m_axis_lup_rsp_TKEEP(lup_rsp_TKEEP),
        .m_axis_lup_rsp_TSTRB(lup_rsp_TSTRB),
        .m_axis_lup_rsp_TLAST(lup_rsp_TLAST),        
        .s_axis_upd_req_TVALID(upd_req_TVALID),
        .s_axis_upd_req_TREADY(upd_req_TREADY),
        .s_axis_upd_req_TDATA(upd_req_TDATA),
        .s_axis_upd_req_TKEEP(upd_req_TKEEP),
        .s_axis_upd_req_TSTRB(upd_req_TSTRB),
        .s_axis_upd_req_TLAST(upd_req_TLAST),
        .m_axis_upd_rsp_TVALID(upd_rsp_TVALID),
        .m_axis_upd_rsp_TREADY(upd_rsp_TREADY),
        .m_axis_upd_rsp_TDATA(upd_rsp_TDATA),
        .m_axis_upd_rsp_TKEEP(upd_rsp_TKEEP),
        .m_axis_upd_rsp_TSTRB(upd_rsp_TSTRB),
        .m_axis_upd_rsp_TLAST(upd_rsp_TLAST),        
        .regInsertFailureCount_ap_vld(),
        .regInsertFailureCount()
        );    
// URAM Tx Memory Datamover
uram_datamover_ip tx_buffer_inst (
    .ap_clk,
    .ap_rst_n,
    .s_axis_cmdRead_TDATA( m_axis_txread_cmd_TDATA),
    .s_axis_cmdRead_TVALID(m_axis_txread_cmd_TVALID),
    .s_axis_cmdRead_TREADY(m_axis_txread_cmd_TREADY),
    .s_axis_cmdRead_TKEEP(m_axis_txread_cmd_TKEEP),
    .s_axis_cmdRead_TSTRB('1),
    .s_axis_cmdRead_TLAST(m_axis_txread_cmd_TLAST),     
    .s_axis_cmdWrite_TDATA(m_axis_txwrite_cmd_TDATA),
    .s_axis_cmdWrite_TVALID(m_axis_txwrite_cmd_TVALID),
    .s_axis_cmdWrite_TREADY(m_axis_txwrite_cmd_TREADY),
    .s_axis_cmdWrite_TKEEP(m_axis_txwrite_cmd_TKEEP),
    .s_axis_cmdWrite_TSTRB('1),
    .s_axis_cmdWrite_TLAST(m_axis_txwrite_cmd_TLAST),    
    .s_axis_data_TDATA(m_axis_txwrite_data_TDATA),
    .s_axis_data_TVALID(m_axis_txwrite_data_TVALID),
    .s_axis_data_TREADY(m_axis_txwrite_data_TREADY),
    .s_axis_data_TKEEP(m_axis_txwrite_data_TKEEP),
    .s_axis_data_TSTRB(m_axis_txwrite_data_TSTRB),
    .s_axis_data_TLAST(m_axis_txwrite_data_TLAST),
    .m_axis_writeStatus_TDATA(s_axis_txwrite_sts_TDATA),
    .m_axis_writeStatus_TVALID(s_axis_txwrite_sts_TVALID),
    .m_axis_writeStatus_TREADY(s_axis_txwrite_sts_TREADY),
    .m_axis_writeStatus_TKEEP(s_axis_txwrite_sts_TKEEP),
    .m_axis_writeStatus_TSTRB(),
    .m_axis_writeStatus_TLAST(s_axis_txwrite_sts_TLAST),    
    .m_axis_data_TDATA(s_axis_txread_data_TDATA),
    .m_axis_data_TVALID(s_axis_txread_data_TVALID),
    .m_axis_data_TREADY(s_axis_txread_data_TREADY),
    .m_axis_data_TKEEP(s_axis_txread_data_TKEEP),
    .m_axis_data_TSTRB(s_axis_txread_data_TSTRB),
    .m_axis_data_TLAST(s_axis_txread_data_TLAST)
);
    wire        axis_ip_to_slice_meta_tvalid;
    wire        axis_ip_to_slice_meta_tready;
    wire[47:0]  axis_ip_to_slice_meta_tdata;

    wire        axis_slice_to_udp_meta_tvalid;
    wire        axis_slice_to_udp_meta_tready;
    wire[63:0]  axis_slice_to_udp_meta_tdata;
    wire [7:0]  axis_slice_to_udp_meta_tkeep;
    wire        axis_slice_to_udp_meta_tlast;

    wire        axis_ip_to_udp_data_tvalid;
    wire        axis_ip_to_udp_data_tready;
    wire[63:0]  axis_ip_to_udp_data_tdata;
    wire[7:0]   axis_ip_to_udp_data_tkeep;
    wire        axis_ip_to_udp_data_tlast;

    wire        axis_udp_to_slice_meta_tvalid;
    wire        axis_udp_to_slice_meta_tready;
    wire[63:0]  axis_udp_to_slice_meta_tdata;
    wire [7:0]  axis_udp_to_slice_meta_tkeep;
    wire        axis_udp_to_slice_meta_tlast;

    wire        axis_slice_to_ip_meta_tvalid;
    wire        axis_slice_to_ip_meta_tready;
    wire[47:0]  axis_slice_to_ip_meta_tdata;

    wire        axis_udp_to_ip_data_tvalid;
    wire        axis_udp_to_ip_data_tready;
    wire[63:0]  axis_udp_to_ip_data_tdata;
    wire[7:0]   axis_udp_to_ip_data_tkeep;
    wire        axis_udp_to_ip_data_tlast;

    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_udp_i_awaddr;
    wire        s_axi_udp_i_awvalid;
    wire        s_axi_udp_i_awready;
    wire [31:0] s_axi_udp_i_wdata;
    wire [ 3:0] s_axi_udp_i_wstrb;
    wire        s_axi_udp_i_wvalid;
    wire        s_axi_udp_i_wready;
    wire [ 1:0] s_axi_udp_i_bresp;
    wire        s_axi_udp_i_bvalid;
    wire        s_axi_udp_i_bready; 
    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_udp_i_araddr;
    wire        s_axi_udp_i_arvalid;
    wire        s_axi_udp_i_arready;
    wire [31:0] s_axi_udp_i_rdata;
    wire [ 1:0] s_axi_udp_i_rresp;
    wire        s_axi_udp_i_rvalid;
    wire        s_axi_udp_i_rready;

    logic m_axis_udp_rxdata_to_slice_TVALID;
    logic m_axis_udp_rxdata_to_slice_TREADY;
    logic [63:0] m_axis_udp_rxdata_to_slice_TDATA;
    logic [7:0] m_axis_udp_rxdata_to_slice_TKEEP;
    logic m_axis_udp_rxdata_to_slice_TLAST; 

    assign m_axis_udp_data_TSTRB = m_axis_udp_data_TKEEP;

    assign axis_broadcast_to_udp_tvalid = m_axis_broadcast_tvalid[0:0];
    assign m_axis_broadcast_tready[0:0] = axis_broadcast_to_udp_tready;
    assign axis_broadcast_to_udp_tdata = m_axis_broadcast_tdata[0+:64];
    assign axis_broadcast_to_udp_tkeep = m_axis_broadcast_tkeep[0+:8];
    assign axis_broadcast_to_udp_tlast = m_axis_broadcast_tlast[0:0];

    assign axis_broadcast_to_iph_tvalid = m_axis_broadcast_tvalid[1:1];
    assign m_axis_broadcast_tready[1:1] = axis_broadcast_to_iph_tready;
    assign axis_broadcast_to_iph_tdata = m_axis_broadcast_tdata[64+:64];
    assign axis_broadcast_to_iph_tkeep = m_axis_broadcast_tkeep[8+:8];
    assign axis_broadcast_to_iph_tlast = m_axis_broadcast_tlast[1:1];

    udp_ll_axis_broadcaster udp_ll_broadcaster_i (
        .aclk           (ap_clk),                    
        .aresetn        (ap_rst_n),              
        .s_axis_tvalid  (s_axis_line_TVALID),  // input wire s_axis_tvalid
        .s_axis_tready  (s_axis_line_TREADY),  // output wire s_axis_tready
        .s_axis_tdata   (s_axis_line_TDATA),    // input wire [63 : 0] s_axis_tdata
        .s_axis_tkeep   (s_axis_line_TKEEP),    // input wire [7 : 0] s_axis_tkeep
        .s_axis_tlast   (s_axis_line_TLAST),    // input wire s_axis_tlast
        .m_axis_tvalid  (m_axis_broadcast_tvalid),  // output wire [1 : 0] m_axis_tvalid
        .m_axis_tready  (m_axis_broadcast_tready),  // input wire [1 : 0] m_axis_tready
        .m_axis_tdata   (m_axis_broadcast_tdata),    // output wire [127 : 0] m_axis_tdata
        .m_axis_tkeep   (m_axis_broadcast_tkeep),    // output wire [15 : 0] m_axis_tkeep
        .m_axis_tlast   (m_axis_broadcast_tlast)    // output wire [1 : 0] m_axis_tlast
    );
 
    udp_stack udp_stack_i (
        // AXI4-Lite slave interface
        .s_axi_control_awvalid(s_axi_udp_i_awvalid),
        .s_axi_control_awready(s_axi_udp_i_awready),
        .s_axi_control_awaddr(s_axi_udp_i_awaddr[C_S_AXI_CONTROL_UDP_ADDR_WIDTH-1:0]),
        .s_axi_control_wvalid(s_axi_udp_i_wvalid),
        .s_axi_control_wready(s_axi_udp_i_wready),
        .s_axi_control_wdata(s_axi_udp_i_wdata),
        .s_axi_control_wstrb(s_axi_udp_i_wstrb),
        .s_axi_control_arvalid(s_axi_udp_i_arvalid),
        .s_axi_control_arready(s_axi_udp_i_arready),
        .s_axi_control_araddr(s_axi_udp_i_araddr[C_S_AXI_CONTROL_UDP_ADDR_WIDTH-1:0]),
        .s_axi_control_rvalid(s_axi_udp_i_rvalid),
        .s_axi_control_rready(s_axi_udp_i_rready),
        .s_axi_control_rdata(s_axi_udp_i_rdata),
        .s_axi_control_rresp(s_axi_udp_i_rresp),
        .s_axi_control_bvalid(s_axi_udp_i_bvalid),
        .s_axi_control_bready(s_axi_udp_i_bready),
        .s_axi_control_bresp(s_axi_udp_i_bresp),  

        // System Signals
        .ap_clk     (ap_clk),
        .ap_rst_n   (ap_rst_n),

        .my_ip_addr  (iph_ip_address),
        .ip_protocol (8'h11), //UDP_PROTOCOL        

        // Raw Ethernet Data Input
        .s_axis_line_tvalid (axis_broadcast_to_udp_tvalid),
        .s_axis_line_tready (axis_broadcast_to_udp_tready),
        .s_axis_line_tdata (axis_broadcast_to_udp_tdata),
        .s_axis_line_tkeep (axis_broadcast_to_udp_tkeep),
        .s_axis_line_tlast (axis_broadcast_to_udp_tlast),

         // UDP Application Interface
         .m_axis_udp_data_tvalid (m_axis_udp_data_TVALID),
         .m_axis_udp_data_tready (m_axis_udp_data_TREADY),
         .m_axis_udp_data_tdata  (m_axis_udp_data_TDATA),
         .m_axis_udp_data_tkeep  (m_axis_udp_data_TKEEP),
         .m_axis_udp_data_tlast  (m_axis_udp_data_TLAST),
         .m_axis_udp_data_tstrb  (),    
         .m_axis_udp_metadata_tvalid (m_axis_udp_metadata_TVALID),
         .m_axis_udp_metadata_tready (m_axis_udp_metadata_TREADY),
         .m_axis_udp_metadata_tdata  (m_axis_udp_metadata_TDATA),
         .m_axis_udp_metadata_tkeep  (m_axis_udp_metadata_TKEEP),
         .m_axis_udp_metadata_tlast  (m_axis_udp_metadata_TLAST),
         .m_axis_udp_metadata_tstrb  (m_axis_udp_metadata_TSTRB),  

         // ----------- TX ------------- //
         .s_axis_tx_meta_tvalid (s_axis_udp_metadata_TVALID),
         .s_axis_tx_meta_tready (s_axis_udp_metadata_TREADY),
         .s_axis_tx_meta_tdata (s_axis_udp_metadata_TDATA),
         .s_axis_tx_meta_tkeep (s_axis_udp_metadata_TKEEP),
         .s_axis_tx_meta_tlast (s_axis_udp_metadata_TLAST),
         .s_axis_tx_meta_tstrb (s_axis_udp_metadata_TSTRB),    
    
         .s_axis_tx_data_tvalid (s_axis_udp_data_TVALID),
         .s_axis_tx_data_tready (s_axis_udp_data_TREADY),
         .s_axis_tx_data_tdata (s_axis_udp_data_TDATA),
         .s_axis_tx_data_tkeep (s_axis_udp_data_TKEEP),
         .s_axis_tx_data_tlast (s_axis_udp_data_TLAST),
         .s_axis_tx_data_tstrb (s_axis_udp_data_TSTRB),

         .m_axis_tx_data_tvalid (axi_udp_to_merge_tvalid),
         .m_axis_tx_data_tready (axi_udp_to_merge_tready),
         .m_axis_tx_data_tdata  (axi_udp_to_merge_tdata),
         .m_axis_tx_data_tkeep  (axi_udp_to_merge_tkeep),
         .m_axis_tx_data_tlast  (axi_udp_to_merge_tlast)
);

    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_igmp_i_awaddr;
    wire        s_axi_igmp_i_awvalid;
    wire        s_axi_igmp_i_awready;
    wire [31:0] s_axi_igmp_i_wdata;
    wire [ 3:0] s_axi_igmp_i_wstrb;
    wire        s_axi_igmp_i_wvalid;
    wire        s_axi_igmp_i_wready;
    wire [ 1:0] s_axi_igmp_i_bresp;
    wire        s_axi_igmp_i_bvalid;
    wire        s_axi_igmp_i_bready; 
    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_igmp_i_araddr;
    wire        s_axi_igmp_i_arvalid;
    wire        s_axi_igmp_i_arready;
    wire [31:0] s_axi_igmp_i_rdata;
    wire [ 1:0] s_axi_igmp_i_rresp;
    wire        s_axi_igmp_i_rvalid;
    wire        s_axi_igmp_i_rready;


    // IGMP Offload Engine
    igmp_ip igmp_inst (
        .srcIpAddr(udp_ip_address),
        .s_axi_control_AWVALID(s_axi_igmp_i_awvalid),
        .s_axi_control_AWREADY(s_axi_igmp_i_awready),
        .s_axi_control_AWADDR(s_axi_igmp_i_awaddr[C_S_AXI_CONTROL_IGMP_ADDR_WIDTH-1:0]),
        .s_axi_control_WVALID(s_axi_igmp_i_wvalid),
        .s_axi_control_WREADY(s_axi_igmp_i_wready),
        .s_axi_control_WDATA(s_axi_igmp_i_wdata),
        .s_axi_control_WSTRB(s_axi_igmp_i_wstrb),
        .s_axi_control_ARVALID(s_axi_igmp_i_arvalid),
        .s_axi_control_ARREADY(s_axi_igmp_i_arready),
        .s_axi_control_ARADDR(s_axi_igmp_i_araddr[C_S_AXI_CONTROL_IGMP_ADDR_WIDTH-1:0]),
        .s_axi_control_RVALID(s_axi_igmp_i_rvalid),
        .s_axi_control_RREADY(s_axi_igmp_i_rready),
        .s_axi_control_RDATA(s_axi_igmp_i_rdata),
        .s_axi_control_RRESP(s_axi_igmp_i_rresp),
        .s_axi_control_BVALID(s_axi_igmp_i_bvalid),
        .s_axi_control_BREADY(s_axi_igmp_i_bready),
        .s_axi_control_BRESP(s_axi_igmp_i_bresp),

        .s_axis_data_TDATA(axi_igmp_slice_to_igmp_tdata),
        .s_axis_data_TKEEP(axi_igmp_slice_to_igmp_tkeep),
        .s_axis_data_TSTRB(8'hFF),
        .s_axis_data_TLAST(axi_igmp_slice_to_igmp_tlast),
        .s_axis_data_TVALID(axi_igmp_slice_to_igmp_tvalid),
        .s_axis_data_TREADY(axi_igmp_slice_to_igmp_tready),

        .m_axis_data_TDATA(axi_igmp_to_merge_tdata),
        .m_axis_data_TKEEP(axi_igmp_to_merge_tkeep),
        .m_axis_data_TSTRB(),
        .m_axis_data_TLAST(axi_igmp_to_merge_tlast),
        .m_axis_data_TVALID(axi_igmp_to_merge_tvalid),
        .m_axis_data_TREADY(axi_igmp_to_merge_tready),

        .ap_clk(ap_clk),
        .ap_rst_n(ap_rst_n)
    );

    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_iph_i_awaddr;
    wire        s_axi_iph_i_awvalid;
    wire        s_axi_iph_i_awready;
    wire [31:0] s_axi_iph_i_wdata;
    wire [ 3:0] s_axi_iph_i_wstrb;
    wire        s_axi_iph_i_wvalid;
    wire        s_axi_iph_i_wready;
    wire [ 1:0] s_axi_iph_i_bresp;
    wire        s_axi_iph_i_bvalid;
    wire        s_axi_iph_i_bready; 
    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_iph_i_araddr;
    wire        s_axi_iph_i_arvalid;
    wire        s_axi_iph_i_arready;
    wire [31:0] s_axi_iph_i_rdata;
    wire [ 1:0] s_axi_iph_i_rresp;
    wire        s_axi_iph_i_rvalid;
    wire        s_axi_iph_i_rready;

    ip_handler_tcp_udp_ip ip_handler_inst (
        .s_axi_control_AWVALID(s_axi_iph_i_awvalid),
        .s_axi_control_AWREADY(s_axi_iph_i_awready),
        .s_axi_control_AWADDR(s_axi_iph_i_awaddr[C_S_AXI_CONTROL_IPH_ADDR_WIDTH-1:0]),
        .s_axi_control_WVALID(s_axi_iph_i_wvalid),
        .s_axi_control_WREADY(s_axi_iph_i_wready),
        .s_axi_control_WDATA(s_axi_iph_i_wdata),
        .s_axi_control_WSTRB(s_axi_iph_i_wstrb),
        .s_axi_control_ARVALID(s_axi_iph_i_arvalid),
        .s_axi_control_ARREADY(s_axi_iph_i_arready),
        .s_axi_control_ARADDR(s_axi_iph_i_araddr[C_S_AXI_CONTROL_IPH_ADDR_WIDTH-1:0]),
        .s_axi_control_RVALID(s_axi_iph_i_rvalid),
        .s_axi_control_RREADY(s_axi_iph_i_rready),
        .s_axi_control_RDATA(s_axi_iph_i_rdata),
        .s_axi_control_RRESP(s_axi_iph_i_rresp),
        .s_axi_control_BVALID(s_axi_iph_i_bvalid),
        .s_axi_control_BREADY(s_axi_iph_i_bready),
        .s_axi_control_BRESP(s_axi_iph_i_bresp),
                
        .m_axis_ARP_TVALID(axi_iph_to_arp_slice_tvalid), // output AXI4Stream_M_TVALID
        .m_axis_ARP_TREADY(axi_iph_to_arp_slice_tready), // input AXI4Stream_M_TREADY
        .m_axis_ARP_TDATA(axi_iph_to_arp_slice_tdata), // output [63 : 0] AXI4Stream_M_TDATA
        .m_axis_ARP_TSTRB(),
        .m_axis_ARP_TKEEP(axi_iph_to_arp_slice_tkeep), // output [7 : 0] AXI4Stream_M_TSTRB
        .m_axis_ARP_TLAST(axi_iph_to_arp_slice_tlast), // output [0 : 0] AXI4Stream_M_TLAST

        .m_axis_ICMP_TVALID(axi_iph_to_icmp_slice_tvalid), // output AXI4Stream_M_TVALID
        .m_axis_ICMP_TREADY(axi_iph_to_icmp_slice_tready), // input AXI4Stream_M_TREADY
        .m_axis_ICMP_TDATA(axi_iph_to_icmp_slice_tdata), // output [63 : 0] AXI4Stream_M_TDATA
        .m_axis_ICMP_TSTRB(),
        .m_axis_ICMP_TKEEP(axi_iph_to_icmp_slice_tkeep), // output [7 : 0] AXI4Stream_M_TSTRB
        .m_axis_ICMP_TLAST(axi_iph_to_icmp_slice_tlast), // output [0 : 0] AXI4Stream_M_TLAST
       
        .m_axis_TCP_TVALID(axi_iph_to_toe_slice_tvalid), // output AXI4Stream_M_TVALID
        .m_axis_TCP_TREADY(axi_iph_to_toe_slice_tready), // input AXI4Stream_M_TREADY
        .m_axis_TCP_TDATA(axi_iph_to_toe_slice_tdata), // output [63 : 0] AXI4Stream_M_TDATA
        .m_axis_TCP_TSTRB(),
        .m_axis_TCP_TKEEP(axi_iph_to_toe_slice_tkeep), // output [7 : 0] AXI4Stream_M_TSTRB
        .m_axis_TCP_TLAST(axi_iph_to_toe_slice_tlast), // output [0 : 0] AXI4Stream_M_TLAST

        .m_axis_IGMP_TVALID(axi_iph_to_igmp_slice_tvalid), // output AXI4Stream_M_TVALID
        .m_axis_IGMP_TREADY(axi_iph_to_igmp_slice_tready), // input AXI4Stream_M_TREADY
        .m_axis_IGMP_TDATA(axi_iph_to_igmp_slice_tdata), // output [63 : 0] AXI4Stream_M_TDATA
        .m_axis_IGMP_TSTRB(),
        .m_axis_IGMP_TKEEP(axi_iph_to_igmp_slice_tkeep), // output [7 : 0] AXI4Stream_M_TSTRB
        .m_axis_IGMP_TLAST(axi_iph_to_igmp_slice_tlast), // output [0 : 0] AXI4Stream_M_TLAST

        .s_axis_raw_TVALID(axis_broadcast_to_iph_tvalid), // input AXI4Stream_S_TVALID
        .s_axis_raw_TREADY(axis_broadcast_to_iph_tready), // output AXI4Stream_S_TREADY
        .s_axis_raw_TDATA(axis_broadcast_to_iph_tdata), // input [63 : 0] AXI4Stream_S_TDATA
        .s_axis_raw_TSTRB('1),
        .s_axis_raw_TKEEP(axis_broadcast_to_iph_tkeep), // input [7 : 0] AXI4Stream_S_TSTRB
        .s_axis_raw_TLAST(axis_broadcast_to_iph_tlast), // input [0 : 0] AXI4Stream_S_TLAST

        .myIpAddress(iph_ip_address),

        .ap_clk(ap_clk), // input ap_clk
        .ap_rst_n(ap_rst_n) // input ap_rst_n
    );

    // ARP lookup
    wire        axis_arp_lookup_request_TVALID;
    wire        axis_arp_lookup_request_TREADY;
    wire[31:0]  axis_arp_lookup_request_TDATA;
    wire        axis_arp_lookup_reply_TVALID;
    wire        axis_arp_lookup_reply_TREADY;
    wire[71:0]  axis_arp_lookup_reply_TDATA;

    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_mie_i_awaddr;
    wire        s_axi_mie_i_awvalid;
    wire        s_axi_mie_i_awready;
    wire [31:0] s_axi_mie_i_wdata;
    wire [ 3:0] s_axi_mie_i_wstrb;
    wire        s_axi_mie_i_wvalid;
    wire        s_axi_mie_i_wready;
    wire [ 1:0] s_axi_mie_i_bresp;
    wire        s_axi_mie_i_bvalid;
    wire        s_axi_mie_i_bready; 
    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_mie_i_araddr;
    wire        s_axi_mie_i_arvalid;
    wire        s_axi_mie_i_arready;
    wire [31:0] s_axi_mie_i_rdata;
    wire [ 1:0] s_axi_mie_i_rresp;
    wire        s_axi_mie_i_rvalid;
    wire        s_axi_mie_i_rready;

 mac_ip_encode_top mac_ip_encode_inst(
    .s_axi_control_awvalid(s_axi_mie_i_awvalid),
    .s_axi_control_awready(s_axi_mie_i_awready),
    .s_axi_control_awaddr(s_axi_mie_i_awaddr[C_S_AXI_CONTROL_MIE_ADDR_WIDTH-1:0]),
    .s_axi_control_wvalid(s_axi_mie_i_wvalid),
    .s_axi_control_wready(s_axi_mie_i_wready),
    .s_axi_control_wdata(s_axi_mie_i_wdata),
    .s_axi_control_wstrb(s_axi_mie_i_wstrb),
    .s_axi_control_arvalid(s_axi_mie_i_arvalid),
    .s_axi_control_arready(s_axi_mie_i_arready),
    .s_axi_control_araddr(s_axi_mie_i_araddr[C_S_AXI_CONTROL_MIE_ADDR_WIDTH-1:0]),
    .s_axi_control_rvalid(s_axi_mie_i_rvalid),
    .s_axi_control_rready(s_axi_mie_i_rready),
    .s_axi_control_rdata(s_axi_mie_i_rdata),
    .s_axi_control_rresp(s_axi_mie_i_rresp),
    .s_axi_control_bvalid(s_axi_mie_i_bvalid),
    .s_axi_control_bready(s_axi_mie_i_bready),
    .s_axi_control_bresp(s_axi_mie_i_bresp),

    // System Signals
    .ap_clk(ap_clk),
    .ap_rst_n(ap_rst_n),

    .s_axis_ip_tvalid(axi_intercon_to_mie_tvalid),
    .s_axis_ip_tready(axi_intercon_to_mie_tready),
    .s_axis_ip_tdata(axi_intercon_to_mie_tdata),
    .s_axis_ip_tkeep(axi_intercon_to_mie_tkeep),
    .s_axis_ip_tlast(axi_intercon_to_mie_tlast),

    .s_axis_arp_lookup_reply_tvalid(axis_arp_lookup_reply_TVALID),
    .s_axis_arp_lookup_reply_tready(axis_arp_lookup_reply_TREADY),
    .s_axis_arp_lookup_reply_tdata(axis_arp_lookup_reply_TDATA),

    .m_axis_ip_tvalid(axi_mie_to_intercon_tvalid),
    .m_axis_ip_tready(axi_mie_to_intercon_tready),
    .m_axis_ip_tdata(axi_mie_to_intercon_tdata),
    .m_axis_ip_tkeep(axi_mie_to_intercon_tkeep),
    .m_axis_ip_tlast(axi_mie_to_intercon_tlast),

    .m_axis_arp_lookup_request_tvalid(axis_arp_lookup_request_TVALID),
    .m_axis_arp_lookup_request_tready(axis_arp_lookup_request_TREADY),
    .m_axis_arp_lookup_request_tdata(axis_arp_lookup_request_TDATA),

    .myMacAddress(mie_mac_address),
    .regSubNetMask(mie_ip_subnet_mask),    
    .regDefaultGateway(mie_ip_default_gateway) 
);    


    // merges ICMP, UDP, TCP and IGMP
    axis_interconnect_4to1 ip_merger (
        .ACLK(ap_clk),                                  // input wire ACLK
        .ARESETN(ap_rst_n),                            // input wire ARESETN

        .S00_AXIS_ACLK(ap_clk),                // input wire S00_AXIS_ACLK
        .S00_AXIS_ARESETN(ap_rst_n),          // input wire S00_AXIS_ARESETN
        .S00_AXIS_TVALID(axi_icmp_to_merge_tvalid),            // input wire S00_AXIS_TVALID
        .S00_AXIS_TREADY(axi_icmp_to_merge_tready),            // output wire S00_AXIS_TREADY
        .S00_AXIS_TDATA(axi_icmp_to_merge_tdata),              // input wire [63 : 0] S00_AXIS_TDATA
        .S00_AXIS_TKEEP(axi_icmp_to_merge_tkeep),              // input wire [7 : 0] S00_AXIS_TKEEP
        .S00_AXIS_TLAST(axi_icmp_to_merge_tlast),              // input wire S00_AXIS_TLAST

        .S01_AXIS_ACLK(ap_clk),                // input wire S01_AXIS_ACLK
        .S01_AXIS_ARESETN(ap_rst_n),          // input wire S01_AXIS_ARESETN
        .S01_AXIS_TVALID(axi_udp_to_merge_tvalid),            // input wire S01_AXIS_TVALID
        .S01_AXIS_TREADY(axi_udp_to_merge_tready),            // output wire S01_AXIS_TREADY
        .S01_AXIS_TDATA(axi_udp_to_merge_tdata),              // input wire [63 : 0] S01_AXIS_TDATA
        .S01_AXIS_TKEEP(axi_udp_to_merge_tkeep),              // input wire [7 : 0] S01_AXIS_TKEEP
        .S01_AXIS_TLAST(axi_udp_to_merge_tlast),              // input wire S01_AXIS_TLAST

        .S02_AXIS_ACLK(ap_clk),                // input wire S02_AXIS_ACLK
        .S02_AXIS_ARESETN(ap_rst_n),          // input wire S02_AXIS_ARESETN
        .S02_AXIS_TVALID(axi_toe_to_merge_tvalid),            // input wire S02_AXIS_TVALID
        .S02_AXIS_TREADY(axi_toe_to_merge_tready),            // output wire S02_AXIS_TREADY
        .S02_AXIS_TDATA(axi_toe_to_merge_tdata),              // input wire [63 : 0] S02_AXIS_TDATA
        .S02_AXIS_TKEEP(axi_toe_to_merge_tkeep),              // input wire [7 : 0] S02_AXIS_TKEEP
        .S02_AXIS_TLAST(axi_toe_to_merge_tlast),              // input wire S02_AXIS_TLAST

        .S03_AXIS_ACLK(ap_clk),
        .S03_AXIS_ARESETN(ap_rst_n),
        .S03_AXIS_TVALID(axi_igmp_to_merge_tvalid),
        .S03_AXIS_TREADY(axi_igmp_to_merge_tready),
        .S03_AXIS_TDATA(axi_igmp_to_merge_tdata),
        .S03_AXIS_TKEEP(axi_igmp_to_merge_tkeep),
        .S03_AXIS_TLAST(axi_igmp_to_merge_tlast),

        .M00_AXIS_ACLK(ap_clk),                // input wire M00_AXIS_ACLK
        .M00_AXIS_ARESETN(ap_rst_n),          // input wire M00_AXIS_ARESETN
        .M00_AXIS_TVALID(axi_intercon_to_mie_tvalid),            // output wire M00_AXIS_TVALID
        .M00_AXIS_TREADY(axi_intercon_to_mie_tready),            // input wire M00_AXIS_TREADY
        .M00_AXIS_TDATA(axi_intercon_to_mie_tdata),              // output wire [63 : 0] M00_AXIS_TDATA
        .M00_AXIS_TKEEP(axi_intercon_to_mie_tkeep),              // output wire [7 : 0] M00_AXIS_TKEEP
        .M00_AXIS_TLAST(axi_intercon_to_mie_tlast),              // output wire M00_AXIS_TLAST

        .S00_ARB_REQ_SUPPRESS(1'b0),  // input wire S00_ARB_REQ_SUPPRESS
        .S01_ARB_REQ_SUPPRESS(1'b0),  // input wire S01_ARB_REQ_SUPPRESS
        .S02_ARB_REQ_SUPPRESS(1'b0),  // input wire S02_ARB_REQ_SUPPRESS
        .S03_ARB_REQ_SUPPRESS(1'b0)
    );

// merges ip and arp

    axis_interconnect_2to1 mac_merger (
        .ACLK(ap_clk), // input ACLK
        .ARESETN(ap_rst_n), // input ARESETN
        .S00_AXIS_ACLK(ap_clk), // input S00_AXIS_ACLK
        .S01_AXIS_ACLK(ap_clk), // input S01_AXIS_ACLK
        .S00_AXIS_ARESETN(ap_rst_n), // input S00_AXIS_ARESETN
        .S01_AXIS_ARESETN(ap_rst_n), // input S01_AXIS_ARESETN
        .S00_AXIS_TVALID(axi_arp_to_mac_merger_tvalid), // input S00_AXIS_TVALID
        .S01_AXIS_TVALID(axi_mie_to_intercon_tvalid), // input S01_AXIS_TVALID
        .S00_AXIS_TREADY(axi_arp_to_mac_merger_tready), // output S00_AXIS_TREADY
        .S01_AXIS_TREADY(axi_mie_to_intercon_tready), // output S01_AXIS_TREADY
        .S00_AXIS_TDATA(axi_arp_to_mac_merger_tdata), // input [63 : 0] S00_AXIS_TDATA
        .S01_AXIS_TDATA(axi_mie_to_intercon_tdata), // input [63 : 0] S01_AXIS_TDATA
        .S00_AXIS_TKEEP(axi_arp_to_mac_merger_tkeep), // input [7 : 0] S00_AXIS_TKEEP
        .S01_AXIS_TKEEP(axi_mie_to_intercon_tkeep), // input [7 : 0] S01_AXIS_TKEEP
        .S00_AXIS_TLAST(axi_arp_to_mac_merger_tlast), // input S00_AXIS_TLAST
        .S01_AXIS_TLAST(axi_mie_to_intercon_tlast), // input S01_AXIS_TLAST
        .M00_AXIS_ACLK(ap_clk), // input M00_AXIS_ACLK
        .M00_AXIS_ARESETN(ap_rst_n), // input M00_AXIS_ARESETN
        .M00_AXIS_TVALID(m_axis_line_TVALID), // output M00_AXIS_TVALID
        .M00_AXIS_TREADY(m_axis_line_TREADY), // input M00_AXIS_TREADY
        .M00_AXIS_TDATA(m_axis_line_TDATA), // output [63 : 0] M00_AXIS_TDATA
        .M00_AXIS_TKEEP(m_axis_line_TKEEP), // output [7 : 0] M00_AXIS_TKEEP
        .M00_AXIS_TLAST(m_axis_line_TLAST), // output M00_AXIS_TLAST
        .S00_ARB_REQ_SUPPRESS(1'b0), // input S00_ARB_REQ_SUPPRESS
        .S01_ARB_REQ_SUPPRESS(1'b0) // input S01_ARB_REQ_SUPPRESS
    );

    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_arp_server_i_awaddr;
    wire        s_axi_arp_server_i_awvalid;
    wire        s_axi_arp_server_i_awready;
    wire [31:0] s_axi_arp_server_i_wdata;
    wire [ 3:0] s_axi_arp_server_i_wstrb;
    wire        s_axi_arp_server_i_wvalid;
    wire        s_axi_arp_server_i_wready;
    wire [ 1:0] s_axi_arp_server_i_bresp;
    wire        s_axi_arp_server_i_bvalid;
    wire        s_axi_arp_server_i_bready; 
    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_arp_server_i_araddr;
    wire        s_axi_arp_server_i_arvalid;
    wire        s_axi_arp_server_i_arready;
    wire [31:0] s_axi_arp_server_i_rdata;
    wire [ 1:0] s_axi_arp_server_i_rresp;
    wire        s_axi_arp_server_i_rvalid;
    wire        s_axi_arp_server_i_rready;

    arp_server_subnet_ip arp_server_inst(
        .s_axi_control_AWVALID(s_axi_arp_server_i_awvalid),
        .s_axi_control_AWREADY(s_axi_arp_server_i_awready),
        .s_axi_control_AWADDR(s_axi_arp_server_i_awaddr[C_S_AXI_CONTROL_ARP_SERVER_ADDR_WIDTH-1:0]),
        .s_axi_control_WVALID(s_axi_arp_server_i_wvalid),
        .s_axi_control_WREADY(s_axi_arp_server_i_wready),
        .s_axi_control_WDATA(s_axi_arp_server_i_wdata),
        .s_axi_control_WSTRB(s_axi_arp_server_i_wstrb),
        .s_axi_control_ARVALID(s_axi_arp_server_i_arvalid),
        .s_axi_control_ARREADY(s_axi_arp_server_i_arready),
        .s_axi_control_ARADDR(s_axi_arp_server_i_araddr[C_S_AXI_CONTROL_ARP_SERVER_ADDR_WIDTH-1:0]),
        .s_axi_control_RVALID(s_axi_arp_server_i_rvalid),
        .s_axi_control_RREADY(s_axi_arp_server_i_rready),
        .s_axi_control_RDATA(s_axi_arp_server_i_rdata),
        .s_axi_control_RRESP(s_axi_arp_server_i_rresp),
        .s_axi_control_BVALID(s_axi_arp_server_i_bvalid),
        .s_axi_control_BREADY(s_axi_arp_server_i_bready),
        .s_axi_control_BRESP(s_axi_arp_server_i_bresp), 

        .m_axis_arpDataOut_TVALID(axi_arp_to_mac_merger_tvalid),
        .m_axis_arpDataOut_TREADY(axi_arp_to_mac_merger_tready),
        .m_axis_arpDataOut_TDATA(axi_arp_to_mac_merger_tdata),
        .m_axis_arpDataOut_TSTRB(),
        .m_axis_arpDataOut_TKEEP(axi_arp_to_mac_merger_tkeep),
        .m_axis_arpDataOut_TLAST(axi_arp_to_mac_merger_tlast),
        .m_axis_arp_lookup_reply_V_TVALID(axis_arp_lookup_reply_TVALID),
        .m_axis_arp_lookup_reply_V_TREADY(axis_arp_lookup_reply_TREADY),
        .m_axis_arp_lookup_reply_V_TDATA(axis_arp_lookup_reply_TDATA),
        .s_axis_arpDataIn_TVALID(axi_arp_slice_to_arp_tvalid),
        .s_axis_arpDataIn_TREADY(axi_arp_slice_to_arp_tready),
        .s_axis_arpDataIn_TDATA(axi_arp_slice_to_arp_tdata),
        .s_axis_arpDataIn_TSTRB('1),
        .s_axis_arpDataIn_TKEEP(axi_arp_slice_to_arp_tkeep),
        .s_axis_arpDataIn_TLAST(axi_arp_slice_to_arp_tlast),
        .s_axis_arp_lookup_request_V_TVALID(axis_arp_lookup_request_TVALID),
        .s_axis_arp_lookup_request_V_TREADY(axis_arp_lookup_request_TREADY),
        .s_axis_arp_lookup_request_V_TDATA(axis_arp_lookup_request_TDATA),

        .myMacAddress(arp_mac_address),
        .myIpAddress(arp_ip_address),

        .ap_clk(ap_clk), // input ap_clk
        .ap_rst_n(ap_rst_n) // input ap_rst_n
    );

`ifdef DEBUG
    // Probe input and output
    ila_64 ila_arp_egress (
        .clk(ap_clk), // input wire clk

        .probe0(axi_arp_to_mac_merger_tready), // input wire [0:0] probe0  
        .probe1(axi_arp_to_mac_merger_tdata), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axi_arp_to_mac_merger_tvalid), // input wire [0:0]  probe3 
        .probe4(axi_arp_to_mac_merger_tlast), // input wire [0:0]  probe4 
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6(axi_arp_to_mac_merger_tkeep), // input wire [7:0]  probe6 
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );

    ila_64 ila_arp_ingress (
        .clk(ap_clk), // input wire clk

        .probe0(axi_arp_slice_to_arp_tready), // input wire [0:0] probe0  
        .probe1(axi_arp_slice_to_arp_tdata), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axi_arp_slice_to_arp_tvalid), // input wire [0:0]  probe3 
        .probe4(axi_arp_slice_to_arp_tlast), // input wire [0:0]  probe4 
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6(axi_arp_slice_to_arp_tkeep), // input wire [7:0]  probe6 
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );

    ila_64 ila_arp_lookup_req (
        .clk(ap_clk), // input wire clk

        .probe0(axis_arp_lookup_request_TREADY), // input wire [0:0] probe0  
        .probe1({32'd0,axis_arp_lookup_request_TDATA}), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axis_arp_lookup_request_TVALID), // input wire [0:0]  probe3 
        .probe4(1'b0), // input wire [0:0]  probe4 TLAST
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6('b0), // input wire [7:0]  probe6 TKEEP
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );

    ila_64 ila_arp_lookup_rsp (
        .clk(ap_clk), // input wire clk

        .probe0(axis_arp_lookup_reply_TREADY), // input wire [0:0] probe0  
        .probe1({axis_arp_lookup_reply_TDATA[71:64], axis_arp_lookup_reply_TDATA[47:0]}), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axis_arp_lookup_reply_TVALID), // input wire [0:0]  probe3 
        .probe4(1'b0), // input wire [0:0]  probe4 TLAST
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6('b0), // input wire [7:0]  probe6 TKEEP
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );

    ila_64 ila_icmp_ingress (
        .clk(ap_clk), // input wire clk

        .probe0(axi_iph_to_icmp_slice_tready), // input wire [0:0] probe0  
        .probe1(axi_iph_to_icmp_slice_tdata), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axi_iph_to_icmp_slice_tvalid), // input wire [0:0]  probe3 
        .probe4(axi_iph_to_icmp_slice_tlast), // input wire [0:0]  probe4 
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6(axi_iph_to_icmp_slice_tkeep), // input wire [7:0]  probe6 
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );

    ila_64 ila_icmp_egress (
        .clk(ap_clk), // input wire clk

        .probe0(axi_icmp_to_merge_tready), // input wire [0:0] probe0  
        .probe1(axi_icmp_to_merge_tdata), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axi_icmp_to_merge_tvalid), // input wire [0:0]  probe3 
        .probe4(axi_icmp_to_merge_tlast), // input wire [0:0]  probe4 
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6(axi_icmp_to_merge_tkeep), // input wire [7:0]  probe6 
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );
    ila_64 ila_ipv4_ingress (
        .clk(ap_clk), // input wire clk

        .probe0(axi_udp_slice_to_udp_tready), // input wire [0:0] probe0  
        .probe1(axi_udp_slice_to_udp_tdata), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axi_udp_slice_to_udp_tvalid), // input wire [0:0]  probe3 
        .probe4(axi_udp_slice_to_udp_tlast), // input wire [0:0]  probe4 
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6(axi_udp_slice_to_udp_tkeep), // input wire [7:0]  probe6 
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );

    ila_64 ila_ipv4_egress (
        .clk(ap_clk), // input wire clk

        .probe0(axi_udp_to_merge_tready), // input wire [0:0] probe0  
        .probe1(axi_udp_to_merge_tdata), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axi_udp_to_merge_tvalid), // input wire [0:0]  probe3 
        .probe4(axi_udp_to_merge_tlast), // input wire [0:0]  probe4 
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6(axi_udp_to_merge_tkeep), // input wire [7:0]  probe6 
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );

    ila_64 ila_udp_data_ingress (
        .clk(ap_clk), // input wire clk

        .probe0(axis_ip_to_udp_data_tready), // input wire [0:0] probe0  
        .probe1(axis_ip_to_udp_data_tdata), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axis_ip_to_udp_data_tvalid), // input wire [0:0]  probe3 
        .probe4(axis_ip_to_udp_data_tlast), // input wire [0:0]  probe4 
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6(axis_ip_to_udp_data_tkeep), // input wire [7:0]  probe6 
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );

    ila_64 ila_udp_meta_ingress (
        .clk(ap_clk), // input wire clk

        .probe0(axis_slice_to_udp_meta_tready), // input wire [0:0] probe0  
        .probe1(axis_slice_to_udp_meta_tdata), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axis_slice_to_udp_meta_tvalid), // input wire [0:0]  probe3 
        .probe4(1'b0), // input wire [0:0]  probe4 
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6(8'h00), // input wire [7:0]  probe6 
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );

    ila_64 ila_udp_data_egress (
        .clk(ap_clk), // input wire clk

        .probe0(axis_udp_to_ip_data_tready), // input wire [0:0] probe0  
        .probe1(axis_udp_to_ip_data_tdata), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axis_udp_to_ip_data_tvalid), // input wire [0:0]  probe3 
        .probe4(axis_udp_to_ip_data_tlast), // input wire [0:0]  probe4 
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6(axis_udp_to_ip_data_tkeep), // input wire [7:0]  probe6 
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );

    ila_64 ila_udp_meta_egress (
        .clk(ap_clk), // input wire clk

        .probe0(axis_udp_to_slice_meta_tready), // input wire [0:0] probe0  
        .probe1(axis_udp_to_slice_meta_tdata), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axis_udp_to_slice_meta_tvalid), // input wire [0:0]  probe3 
        .probe4(1'b0), // input wire [0:0]  probe4 
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6(8'h00), // input wire [7:0]  probe6 
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );
    ila_64 ila_igmp_egress (
        .clk(ap_clk), // input wire clk

        .probe0(axi_igmp_to_merge_tready), // input wire [0:0] probe0  
        .probe1(axi_igmp_to_merge_tdata), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axi_igmp_to_merge_tvalid), // input wire [0:0]  probe3 
        .probe4(axi_igmp_to_merge_tlast), // input wire [0:0]  probe4 
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6(axi_igmp_to_merge_tkeep), // input wire [7:0]  probe6 
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );

    ila_64 ila_tcp_line_ingress (
        .clk(ap_clk), // input wire clk
        .probe0(axi_toe_slice_to_toe_tready), // input wire [0:0] probe0  
        .probe1(axi_toe_slice_to_toe_tdata), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axi_toe_slice_to_toe_tvalid), // input wire [0:0]  probe3 
        .probe4(axi_toe_slice_to_toe_tlast), // input wire [0:0]  probe4 
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6(axi_toe_slice_to_toe_tkeep), // input wire [7:0]  probe6 
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    ); 

    ila_64 ila_tcp_line_egress (
        .clk(ap_clk), // input wire clk
        .probe0(axi_toe_to_merge_tready), // input wire [0:0] probe0  
        .probe1(axi_toe_to_merge_tdata), // input wire [63:0]  probe1 
        .probe2('0), // input wire [7:0]  probe2 
        .probe3(axi_toe_to_merge_tvalid), // input wire [0:0]  probe3 
        .probe4(axi_toe_to_merge_tlast), // input wire [0:0]  probe4 
        .probe5(1'b0), // input wire [0:0]  probe5 
        .probe6(axi_toe_to_merge_tkeep), // input wire [7:0]  probe6 
        .probe7(1'b0), // input wire [0:0]  probe7  
        .probe8(1'b0) // input wire [0:0]  probe8
    );         
`endif

    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_icmp_server_i_awaddr;
    wire        s_axi_icmp_server_i_awvalid;
    wire        s_axi_icmp_server_i_awready;
    wire [31:0] s_axi_icmp_server_i_wdata;
    wire [ 3:0] s_axi_icmp_server_i_wstrb;
    wire        s_axi_icmp_server_i_wvalid;
    wire        s_axi_icmp_server_i_wready;
    wire [ 1:0] s_axi_icmp_server_i_bresp;
    wire        s_axi_icmp_server_i_bvalid;
    wire        s_axi_icmp_server_i_bready; 
    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_icmp_server_i_araddr;
    wire        s_axi_icmp_server_i_arvalid;
    wire        s_axi_icmp_server_i_arready;
    wire [31:0] s_axi_icmp_server_i_rdata;
    wire [ 1:0] s_axi_icmp_server_i_rresp;
    wire        s_axi_icmp_server_i_rvalid;
    wire        s_axi_icmp_server_i_rready;

    icmp_server_ip icmp_server_inst (
        .s_axi_control_AWVALID(s_axi_icmp_server_i_awvalid),
        .s_axi_control_AWREADY(s_axi_icmp_server_i_awready),
        .s_axi_control_AWADDR(s_axi_icmp_server_i_awaddr[C_S_AXI_CONTROL_ICMP_SERVER_ADDR_WIDTH-1:0]),
        .s_axi_control_WVALID(s_axi_icmp_server_i_wvalid),
        .s_axi_control_WREADY(s_axi_icmp_server_i_wready),
        .s_axi_control_WDATA(s_axi_icmp_server_i_wdata),
        .s_axi_control_WSTRB(s_axi_icmp_server_i_wstrb),
        .s_axi_control_ARVALID(s_axi_icmp_server_i_arvalid),
        .s_axi_control_ARREADY(s_axi_icmp_server_i_arready),
        .s_axi_control_ARADDR(s_axi_icmp_server_i_araddr[C_S_AXI_CONTROL_ICMP_SERVER_ADDR_WIDTH-1:0]),
        .s_axi_control_RVALID(s_axi_icmp_server_i_rvalid),
        .s_axi_control_RREADY(s_axi_icmp_server_i_rready),
        .s_axi_control_RDATA(s_axi_icmp_server_i_rdata),
        .s_axi_control_RRESP(s_axi_icmp_server_i_rresp),
        .s_axi_control_BVALID(s_axi_icmp_server_i_bvalid),
        .s_axi_control_BREADY(s_axi_icmp_server_i_bready),
        .s_axi_control_BRESP(s_axi_icmp_server_i_bresp),         
        .dataIn_TVALID(axi_icmp_slice_to_icmp_tvalid),    // input wire dataIn_TVALID
        .dataIn_TREADY(axi_icmp_slice_to_icmp_tready),    // output wire dataIn_TREADY
        .dataIn_TDATA(axi_icmp_slice_to_icmp_tdata),      // input wire [63 : 0] dataIn_TDATA
        .dataIn_TSTRB('1),
        .dataIn_TKEEP(axi_icmp_slice_to_icmp_tkeep),      // input wire [7 : 0] dataIn_TKEEP
        .dataIn_TLAST(axi_icmp_slice_to_icmp_tlast),      // input wire [0 : 0] dataIn_TLAST
        .dataOut_TVALID(axi_icmp_to_merge_tvalid),   // output wire dataOut_TVALID
        .dataOut_TREADY(axi_icmp_to_merge_tready),   // input wire dataOut_TREADY
        .dataOut_TDATA(axi_icmp_to_merge_tdata),     // output wire [63 : 0] dataOut_TDATA
        .dataOut_TSTRB(),
        .dataOut_TKEEP(axi_icmp_to_merge_tkeep),     // output wire [7 : 0] dataOut_TKEEP
        .dataOut_TLAST(axi_icmp_to_merge_tlast),     // output wire [0 : 0] dataOut_TLAST
        .ap_clk(ap_clk),                                    // input wire ap_clk
        .ap_rst_n(ap_rst_n)                                // input wire ap_rst_n
    );

    /*
    * Slices
    */
    // ARP Input Slice
    axis_register_slice_64 axis_register_arp_in_slice(
        .aclk(ap_clk),
        .aresetn(ap_rst_n),
        .s_axis_tvalid(axi_iph_to_arp_slice_tvalid),
        .s_axis_tready(axi_iph_to_arp_slice_tready),
        .s_axis_tdata(axi_iph_to_arp_slice_tdata),
        .s_axis_tkeep(axi_iph_to_arp_slice_tkeep),
        .s_axis_tlast(axi_iph_to_arp_slice_tlast),
        .m_axis_tvalid(axi_arp_slice_to_arp_tvalid),
        .m_axis_tready(axi_arp_slice_to_arp_tready),
        .m_axis_tdata(axi_arp_slice_to_arp_tdata),
        .m_axis_tkeep(axi_arp_slice_to_arp_tkeep),
        .m_axis_tlast(axi_arp_slice_to_arp_tlast)
    );
    // ICMP Input Slice
    axis_register_slice_64 axis_register_icmp_in_slice(
        .aclk(ap_clk),
        .aresetn(ap_rst_n),
        .s_axis_tvalid(axi_iph_to_icmp_slice_tvalid),
        .s_axis_tready(axi_iph_to_icmp_slice_tready),
        .s_axis_tdata(axi_iph_to_icmp_slice_tdata),
        .s_axis_tkeep(axi_iph_to_icmp_slice_tkeep),
        .s_axis_tlast(axi_iph_to_icmp_slice_tlast),
        .m_axis_tvalid(axi_icmp_slice_to_icmp_tvalid),
        .m_axis_tready(axi_icmp_slice_to_icmp_tready),
        .m_axis_tdata(axi_icmp_slice_to_icmp_tdata),
        .m_axis_tkeep(axi_icmp_slice_to_icmp_tkeep),
        .m_axis_tlast(axi_icmp_slice_to_icmp_tlast)
    );

    axis_register_slice_64 axis_register_igmp_in_slice(
        .aclk(ap_clk),
        .aresetn(ap_rst_n),
        .s_axis_tvalid(axi_iph_to_igmp_slice_tvalid),
        .s_axis_tready(axi_iph_to_igmp_slice_tready),
        .s_axis_tdata(axi_iph_to_igmp_slice_tdata),
        .s_axis_tkeep(axi_iph_to_igmp_slice_tkeep),
        .s_axis_tlast(axi_iph_to_igmp_slice_tlast),
        .m_axis_tvalid(axi_igmp_slice_to_igmp_tvalid),
        .m_axis_tready(axi_igmp_slice_to_igmp_tready),
        .m_axis_tdata(axi_igmp_slice_to_igmp_tdata),
        .m_axis_tkeep(axi_igmp_slice_to_igmp_tkeep),
        .m_axis_tlast(axi_igmp_slice_to_igmp_tlast)
    );

    // UDP Input Slice
    // axis_register_slice_64 axis_register_udp_in_slice(
    //     .aclk(ap_clk),
    //     .aresetn(ap_rst_n),
    //     .s_axis_tvalid(axi_iph_to_udp_slice_tvalid),
    //     .s_axis_tready(axi_iph_to_udp_slice_tready),
    //     .s_axis_tdata(axi_iph_to_udp_slice_tdata),
    //     .s_axis_tkeep(axi_iph_to_udp_slice_tkeep),
    //     .s_axis_tlast(axi_iph_to_udp_slice_tlast),
    //     .m_axis_tvalid(axi_udp_slice_to_udp_tvalid),
    //     .m_axis_tready(axi_udp_slice_to_udp_tready),
    //     .m_axis_tdata(axi_udp_slice_to_udp_tdata),
    //     .m_axis_tkeep(axi_udp_slice_to_udp_tkeep),
    //     .m_axis_tlast(axi_udp_slice_to_udp_tlast)
    // );    
    // TOE Input Slice
    axis_register_slice_64 axis_register_toe_in_slice(
        .aclk(ap_clk),
        .aresetn(ap_rst_n),
        .s_axis_tvalid(axi_iph_to_toe_slice_tvalid),
        .s_axis_tready(axi_iph_to_toe_slice_tready),
        .s_axis_tdata(axi_iph_to_toe_slice_tdata),
        .s_axis_tkeep(axi_iph_to_toe_slice_tkeep),
        .s_axis_tlast(axi_iph_to_toe_slice_tlast),
        .m_axis_tvalid(axi_toe_slice_to_toe_tvalid),
        .m_axis_tready(axi_toe_slice_to_toe_tready),
        .m_axis_tdata(axi_toe_slice_to_toe_tdata),
        .m_axis_tkeep(axi_toe_slice_to_toe_tkeep),
        .m_axis_tlast(axi_toe_slice_to_toe_tlast)
    );
    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_control_i_awaddr;
    wire        s_axi_control_i_awvalid;
    wire        s_axi_control_i_awready;
    wire [31:0] s_axi_control_i_wdata;
    wire [ 3:0] s_axi_control_i_wstrb;
    wire        s_axi_control_i_wvalid;
    wire        s_axi_control_i_wready;
    wire [ 1:0] s_axi_control_i_bresp;
    wire        s_axi_control_i_bvalid;
    wire        s_axi_control_i_bready;
    wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0] s_axi_control_i_araddr;
    wire        s_axi_control_i_arvalid;
    wire        s_axi_control_i_arready;
    wire [31:0] s_axi_control_i_rdata;
    wire [ 1:0] s_axi_control_i_rresp;
    wire        s_axi_control_i_rvalid;
    wire        s_axi_control_i_rready;


    tcp_ip_control_s_axi #(
        .C_S_AXI_ADDR_WIDTH(C_S_AXI_CONTROL_CONTROL_ADDR_WIDTH),
        .C_S_AXI_DATA_WIDTH(C_S_AXI_CONTROL_CONTROL_DATA_WIDTH)
    )
    control_s_i (
        .ACLK        (ap_clk),
        .ARESET      (1'b0),
        .ACLK_EN     (1'b1),
        .AWADDR      (s_axi_control_i_awaddr),
        .AWVALID     (s_axi_control_i_awvalid),
        .AWREADY     (s_axi_control_i_awready),
        .WDATA       (s_axi_control_i_wdata),
        .WSTRB       (s_axi_control_i_wstrb),
        .WVALID      (s_axi_control_i_wvalid),
        .WREADY      (s_axi_control_i_wready),
        .BRESP       (s_axi_control_i_bresp),
        .BVALID      (s_axi_control_i_bvalid),
        .BREADY      (s_axi_control_i_bready),
        .ARADDR      (s_axi_control_i_araddr),
        .ARVALID     (s_axi_control_i_arvalid),
        .ARREADY     (s_axi_control_i_arready),
        .RDATA       (s_axi_control_i_rdata),
        .RRESP       (s_axi_control_i_rresp),
        .RVALID      (s_axi_control_i_rvalid),
        .RREADY      (s_axi_control_i_rready),
        .mac_address (mac_address),
        .ip_address  (ip_address),
        .ip_subnet_mask(ip_subnet_mask),
        .ip_default_gateway(ip_default_gateway)
    );
    assign mac_address_mangled = { mac_address[7:0], mac_address[15:8], mac_address[23:16], mac_address[31:24], mac_address[39:32], mac_address[47:40] };
    assign ip_address_mangled = { ip_address[7:0], ip_address[15:8], ip_address[23:16], ip_address[31:24] };
    assign ip_subnet_mask_mangled = { ip_subnet_mask[7:0], ip_subnet_mask[15:8], ip_subnet_mask[23:16], ip_subnet_mask[31:24] };
    assign ip_default_gateway_mangled = { ip_default_gateway[7:0], ip_default_gateway[15:8], ip_default_gateway[23:16], ip_default_gateway[31:24] };

    axi_lite_crossbar_tcp_udp crossbar_i (
        .ap_clk(ap_clk),                    
        .ap_rst_n(ap_rst_n),

        .s_axi_awaddr(s_axi_control_awaddr),
        .s_axi_awvalid(s_axi_control_awvalid), 
        .s_axi_awready(s_axi_control_awready), 
        .s_axi_wdata(s_axi_control_wdata),     
        .s_axi_wstrb(s_axi_control_wstrb),     
        .s_axi_wvalid(s_axi_control_wvalid),   
        .s_axi_wready(s_axi_control_wready),   
        .s_axi_bresp(s_axi_control_bresp),     
        .s_axi_bvalid(s_axi_control_bvalid),   
        .s_axi_bready(s_axi_control_bready),   
        .s_axi_araddr(s_axi_control_araddr),   
        .s_axi_arvalid(s_axi_control_arvalid), 
        .s_axi_arready(s_axi_control_arready), 
        .s_axi_rdata(s_axi_control_rdata),     
        .s_axi_rresp(s_axi_control_rresp),     
        .s_axi_rvalid(s_axi_control_rvalid),   
        .s_axi_rready(s_axi_control_rready),   

        .m00_axi_awvalid(s_axi_control_i_awvalid),
        .m00_axi_awready(s_axi_control_i_awready),
        .m00_axi_awaddr(s_axi_control_i_awaddr),
        .m00_axi_wvalid(s_axi_control_i_wvalid),
        .m00_axi_wready(s_axi_control_i_wready),
        .m00_axi_wdata(s_axi_control_i_wdata),
        .m00_axi_wstrb(s_axi_control_i_wstrb),
        .m00_axi_arvalid(s_axi_control_i_arvalid),
        .m00_axi_arready(s_axi_control_i_arready),
        .m00_axi_araddr(s_axi_control_i_araddr),
        .m00_axi_rvalid(s_axi_control_i_rvalid),
        .m00_axi_rready(s_axi_control_i_rready),
        .m00_axi_rdata(s_axi_control_i_rdata),
        .m00_axi_rresp(s_axi_control_i_rresp),
        .m00_axi_bvalid(s_axi_control_i_bvalid),
        .m00_axi_bready(s_axi_control_i_bready),
        .m00_axi_bresp(s_axi_control_i_bresp),

        .m01_axi_awvalid(s_axi_igmp_i_awvalid),
        .m01_axi_awready(s_axi_igmp_i_awready),
        .m01_axi_awaddr(s_axi_igmp_i_awaddr),
        .m01_axi_wvalid(s_axi_igmp_i_wvalid),
        .m01_axi_wready(s_axi_igmp_i_wready),
        .m01_axi_wdata(s_axi_igmp_i_wdata),
        .m01_axi_wstrb(s_axi_igmp_i_wstrb),
        .m01_axi_arvalid(s_axi_igmp_i_arvalid),
        .m01_axi_arready(s_axi_igmp_i_arready),
        .m01_axi_araddr(s_axi_igmp_i_araddr),
        .m01_axi_rvalid(s_axi_igmp_i_rvalid),
        .m01_axi_rready(s_axi_igmp_i_rready),
        .m01_axi_rdata(s_axi_igmp_i_rdata),
        .m01_axi_rresp(s_axi_igmp_i_rresp),
        .m01_axi_bvalid(s_axi_igmp_i_bvalid),
        .m01_axi_bready(s_axi_igmp_i_bready),
        .m01_axi_bresp(s_axi_igmp_i_bresp),

        .m02_axi_awvalid(s_axi_arp_server_i_awvalid),
        .m02_axi_awready(s_axi_arp_server_i_awready),
        .m02_axi_awaddr(s_axi_arp_server_i_awaddr),
        .m02_axi_wvalid(s_axi_arp_server_i_wvalid),
        .m02_axi_wready(s_axi_arp_server_i_wready),
        .m02_axi_wdata(s_axi_arp_server_i_wdata),
        .m02_axi_wstrb(s_axi_arp_server_i_wstrb),
        .m02_axi_arvalid(s_axi_arp_server_i_arvalid),
        .m02_axi_arready(s_axi_arp_server_i_arready),
        .m02_axi_araddr(s_axi_arp_server_i_araddr),
        .m02_axi_rvalid(s_axi_arp_server_i_rvalid),
        .m02_axi_rready(s_axi_arp_server_i_rready),
        .m02_axi_rdata(s_axi_arp_server_i_rdata),
        .m02_axi_rresp(s_axi_arp_server_i_rresp),
        .m02_axi_bvalid(s_axi_arp_server_i_bvalid),
        .m02_axi_bready(s_axi_arp_server_i_bready),
        .m02_axi_bresp(s_axi_arp_server_i_bresp),

        .m03_axi_awvalid(s_axi_icmp_server_i_awvalid),
        .m03_axi_awready(s_axi_icmp_server_i_awready),
        .m03_axi_awaddr(s_axi_icmp_server_i_awaddr),
        .m03_axi_wvalid(s_axi_icmp_server_i_wvalid),
        .m03_axi_wready(s_axi_icmp_server_i_wready),
        .m03_axi_wdata(s_axi_icmp_server_i_wdata),
        .m03_axi_wstrb(s_axi_icmp_server_i_wstrb),
        .m03_axi_arvalid(s_axi_icmp_server_i_arvalid),
        .m03_axi_arready(s_axi_icmp_server_i_arready),
        .m03_axi_araddr(s_axi_icmp_server_i_araddr),
        .m03_axi_rvalid(s_axi_icmp_server_i_rvalid),
        .m03_axi_rready(s_axi_icmp_server_i_rready),
        .m03_axi_rdata(s_axi_icmp_server_i_rdata),
        .m03_axi_rresp(s_axi_icmp_server_i_rresp),
        .m03_axi_bvalid(s_axi_icmp_server_i_bvalid),
        .m03_axi_bready(s_axi_icmp_server_i_bready),
        .m03_axi_bresp(s_axi_icmp_server_i_bresp),

        .m04_axi_awvalid(s_axi_iph_i_awvalid),
        .m04_axi_awready(s_axi_iph_i_awready),
        .m04_axi_awaddr(s_axi_iph_i_awaddr),
        .m04_axi_wvalid(s_axi_iph_i_wvalid),
        .m04_axi_wready(s_axi_iph_i_wready),
        .m04_axi_wdata(s_axi_iph_i_wdata),
        .m04_axi_wstrb(s_axi_iph_i_wstrb),
        .m04_axi_arvalid(s_axi_iph_i_arvalid),
        .m04_axi_arready(s_axi_iph_i_arready),
        .m04_axi_araddr(s_axi_iph_i_araddr),
        .m04_axi_rvalid(s_axi_iph_i_rvalid),
        .m04_axi_rready(s_axi_iph_i_rready),
        .m04_axi_rdata(s_axi_iph_i_rdata),
        .m04_axi_rresp(s_axi_iph_i_rresp),
        .m04_axi_bvalid(s_axi_iph_i_bvalid),
        .m04_axi_bready(s_axi_iph_i_bready),
        .m04_axi_bresp(s_axi_iph_i_bresp),

        .m05_axi_awvalid(s_axi_mie_i_awvalid),
        .m05_axi_awready(s_axi_mie_i_awready),
        .m05_axi_awaddr(s_axi_mie_i_awaddr),
        .m05_axi_wvalid(s_axi_mie_i_wvalid),
        .m05_axi_wready(s_axi_mie_i_wready),
        .m05_axi_wdata(s_axi_mie_i_wdata),
        .m05_axi_wstrb(s_axi_mie_i_wstrb),
        .m05_axi_arvalid(s_axi_mie_i_arvalid),
        .m05_axi_arready(s_axi_mie_i_arready),
        .m05_axi_araddr(s_axi_mie_i_araddr),
        .m05_axi_rvalid(s_axi_mie_i_rvalid),
        .m05_axi_rready(s_axi_mie_i_rready),
        .m05_axi_rdata(s_axi_mie_i_rdata),
        .m05_axi_rresp(s_axi_mie_i_rresp),
        .m05_axi_bvalid(s_axi_mie_i_bvalid),
        .m05_axi_bready(s_axi_mie_i_bready),
        .m05_axi_bresp(s_axi_mie_i_bresp),
        .m06_axi_awvalid(s_axi_udp_i_awvalid),
        .m06_axi_awready(s_axi_udp_i_awready),
        .m06_axi_awaddr(s_axi_udp_i_awaddr),
        .m06_axi_wvalid(s_axi_udp_i_wvalid),
        .m06_axi_wready(s_axi_udp_i_wready),
        .m06_axi_wdata(s_axi_udp_i_wdata),
        .m06_axi_wstrb(s_axi_udp_i_wstrb),
        .m06_axi_arvalid(s_axi_udp_i_arvalid),
        .m06_axi_arready(s_axi_udp_i_arready),
        .m06_axi_araddr(s_axi_udp_i_araddr),
        .m06_axi_rvalid(s_axi_udp_i_rvalid),
        .m06_axi_rready(s_axi_udp_i_rready),
        .m06_axi_rdata(s_axi_udp_i_rdata),
        .m06_axi_rresp(s_axi_udp_i_rresp),
        .m06_axi_bvalid(s_axi_udp_i_bvalid),
        .m06_axi_bready(s_axi_udp_i_bready),
        .m06_axi_bresp(s_axi_udp_i_bresp),
        .m07_axi_awvalid(s_axi_toe_i_awvalid),
        .m07_axi_awready(s_axi_toe_i_awready),
        .m07_axi_awaddr(s_axi_toe_i_awaddr),
        .m07_axi_wvalid(s_axi_toe_i_wvalid),
        .m07_axi_wready(s_axi_toe_i_wready),
        .m07_axi_wdata(s_axi_toe_i_wdata),
        .m07_axi_wstrb(s_axi_toe_i_wstrb),
        .m07_axi_arvalid(s_axi_toe_i_arvalid),
        .m07_axi_arready(s_axi_toe_i_arready),
        .m07_axi_araddr(s_axi_toe_i_araddr),
        .m07_axi_rvalid(s_axi_toe_i_rvalid),
        .m07_axi_rready(s_axi_toe_i_rready),
        .m07_axi_rdata(s_axi_toe_i_rdata),
        .m07_axi_rresp(s_axi_toe_i_rresp),
        .m07_axi_bvalid(s_axi_toe_i_bvalid),
        .m07_axi_bready(s_axi_toe_i_bready),
        .m07_axi_bresp(s_axi_toe_i_bresp)
    );

endmodule : tcp_udp_ip_krnl

`default_nettype wire