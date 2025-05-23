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
 * IP ethernet frame receiver (Ethernet frame in, UDP frame out)
 */
module rx_ip_handler (
    // System Signals
    input  wire             ap_clk,
    input  wire             ap_rst_n,

    // IP Addr used by UDP
    input  wire  [31:0]     my_ip_addr,

    // Signals to memory
    output logic [12:0]     mem_addr,
    output logic            mem_ce,
    output logic  [7:0]     expectedBit,

    // Raw Ethernet Data Input
    input  wire             s_axis_line_tvalid,
    output wire             s_axis_line_tready,
    input  wire  [63:0]     s_axis_line_tdata,
    input  wire   [7:0]     s_axis_line_tkeep,
    input  wire             s_axis_line_tlast,

    // UDP Application Interface
    // I don't need to actually use AXI-S here
    // this is still intenal going to new UDP module...
    output logic            m_axis_udp_data_tvalid,
    input  wire             m_axis_udp_data_tready,
    output logic [63:0]     m_axis_udp_data_tdata,
    output logic  [7:0]     m_axis_udp_data_tkeep,
    output logic            m_axis_udp_data_tuser,
    output logic            m_axis_udp_data_tlast,
    output logic            m_axis_udp_metadata_tvalid,
    output logic [79:0]     m_axis_udp_metadata_tdata
);

// I need to remember we have network order.
// Don't waste cycles on reversing if not needed
localparam IPv4 = 16'h0008;
localparam UDP  = 8'h11;
localparam ETH_HDR_DONE = 1;
localparam IP_HDR_W0 = 2;
localparam IP_HDR_W1 = 3;
localparam IP_HDR_W2 = 4;
localparam IP_HDR_W3 = 5;

// States for FSM
typedef enum logic[2:0] {IDLE            = 3'b000,
                         READ_ETH_HEADER = 3'b001,
                         READ_IP_HEADER  = 3'b010,
                         READ_PAYLOAD    = 3'b011,
                         DROP            = 3'b100,
                         LOAD_LAST       = 3'b101} iph_stateCoding_t;
iph_stateCoding_t state = IDLE;
iph_stateCoding_t next_state;

logic  [63:0]     tdata_mangled;

logic   [3:0]     hdr_cnt;
logic   [3:0]     hdr_cnt_next;
logic  [15:0]     eth_type = 16'd0;
logic   [3:0]     ip_header_len = 4'h0;
logic   [3:0]     ip_ver = 4'h0;
logic  [15:0]     ip_tot_len = 16'd0;
logic   [7:0]     ip_protocol = 8'h00;
logic  [15:0]     ip_checksum = 16'd0;
logic  [31:0]     src_addr = 32'd0;
logic  [31:0]     dst_addr = 32'd0;

logic   [7:0]     ip_protocol_i;
logic  [31:0]     dst_addr_i;

logic             ip_protocol_vld = 1'b0;
logic             dst_addr_vld_1 = 1'b0;
logic             dst_addr_vld_2 = 1'b0;
logic             is_multicast = 1'b0;
logic             valid_ip;

logic             store_eth_header;
logic             store_ip_header0;
logic             store_ip_header1;
logic             store_ip_header2;

logic   [3:0]     remaining_ip_header = 4'd5;
logic   [3:0]     remaining_ip_header_next;

logic             eof;
logic             udp_payload_sof;
logic             udp_payload_last;
logic [111:0]     shift_udp_payload = 112'd0;
logic [111:0]     next_shift_udp_payload;
logic  [13:0]     shift_udp_keep = 14'd0;
logic  [13:0]     next_shift_udp_keep; 

logic  [19:0]     hdr_sum_high_reg = 19'd0;
logic  [19:0]     hdr_sum_low_reg = 19'd0;
logic             chk_sum_vld_r;
logic  [19:0]     chk_sum_tmp;
logic             hdr_vld;

logic [15:0]      byte_cnt = 16'd2;
logic [15:0]      byte_cnt_next;
logic  [7:0]      udp_tkeep_i;
logic             force_tlast = 1'b0;
logic [3:0]       bytes2keep;
logic [3:0]       bytes2keep_r;

logic             axis_tvalid_i;
logic             axis_tready_i;
logic  [63:0]     axis_tdata_i;
logic   [7:0]     axis_tkeep_i;
logic             axis_tlast_i;

logic             remaining_bits;
logic             data_sel = 1'b0;
logic  [63:0]     udp_data_tdata;
logic  [63:0]     udp_data_tdata_even;
logic  [80:0]     shift_udp_payload_even = '0;
logic   [9:0]     shift_udp_keep_even ='0;
logic  [80:0]     next_shift_udp_payload_even;
logic   [9:0]     next_shift_udp_keep_even;
logic   [7:0]     udp_data_tkeep;
logic   [7:0]     udp_data_tkeep_even;
logic   [1:0]     shift_udp_valid = 1'b0;
logic   [1:0]     next_shift_udp_valid;


logic             dst_port_seen = 1'b0;
logic             hdr_done;
logic             ap_rst;

function [7:0] count2keep;
    input [3:0] k;
    case (k)
        4'd1: count2keep = 8'h01;
        4'd2: count2keep = 8'h03;
        4'd3: count2keep = 8'h07;
        4'd4: count2keep = 8'h0F;
        4'd5: count2keep = 8'h1F;
        4'd6: count2keep = 8'h3F;
        4'd7: count2keep = 8'h7F;
        4'd8: count2keep = 8'hFF;
        default: count2keep = 8'hFF;
    endcase
endfunction

assign ap_rst = !ap_rst_n;

ll_udp_skidbuffer #(
    .REG_OUTPUT(1),
    .DATA_WIDTH(64)
) rx_ip_handler_reg_slice_i (
    .i_clk    (ap_clk),
    .i_reset  (ap_rst),
    .i_valid  (s_axis_line_tvalid),
    .o_ready  (s_axis_line_tready),
    .i_data   (s_axis_line_tdata),
	.i_keep   (s_axis_line_tkeep),
	.i_last   (s_axis_line_tlast),
	.i_user   (1'b0),
    .o_valid  (axis_tvalid_i),
	.i_ready  (axis_tready_i),
    .o_data   (axis_tdata_i),
	.o_keep   (axis_tkeep_i),
	.o_last   (axis_tlast_i),
	.o_user   ()
);

always_comb begin
    axis_tready_i = 1'b0;
    case(state)
        IDLE: begin
            // We always want to receive header data
            axis_tready_i = 1'b1;
        end
        
        READ_ETH_HEADER: begin
            // We always want to receive header data
            axis_tready_i = 1'b1;
        end
        READ_IP_HEADER: begin
            // // We always want to receive header data
            if (next_state == READ_PAYLOAD || hdr_done) begin
                axis_tready_i = m_axis_udp_data_tready;
            end
            else begin
                axis_tready_i = 1'b1;
            end
        end                
        READ_PAYLOAD: begin
            axis_tready_i = m_axis_udp_data_tready;
        end  
        LOAD_LAST: begin
            if (next_state == IDLE || next_state == DROP) begin
                axis_tready_i = 1'b1;
            end     
            else begin
                axis_tready_i = m_axis_udp_data_tready;
            end
        end
        DROP: begin
            // Always receive
            axis_tready_i = 1'b1;
        end
        default: axis_tready_i = 1'b0;
    endcase
end


assign eof = axis_tlast_i && axis_tvalid_i && m_axis_udp_data_tready;

assign tdata_mangled = {axis_tdata_i[48+:8], axis_tdata_i[56+:8], axis_tdata_i[32+:8],
                        axis_tdata_i[40+:8], axis_tdata_i[16+:8], axis_tdata_i[24+:8],
                        axis_tdata_i[0+:8], axis_tdata_i[8+:8]};     
                   
// ---------------------------------------------------------------------- //
// IP Frame
//  Field                       Length
//  Destination MAC address     6 octets
//  Source MAC address          6 octets
//  Ethertype (0x0800)          2 octets
//  Version (4)                 4 bits
//  IHL (5-15)                  4 bits
//  DSCP (0)                    6 bits
//  ECN (0)                     2 bits
//  length                      2 octets
//  identification (0)          2 octets
//  flags (010)                 3 bits
//  fragment offset (0)         13 bits
//  time to live (64?)          1 octet
//  protocol                    1 octet
//  header checksum             2 octets
//  source IP                   4 octets
//  destination IP              4 octets
//  options                     (IHL-5)*4 octets
//  payload                     length octets
// ---------------------------------------------------------------------- //

// FIFO controls the handling of incoming data from Ethernet 
// FSM states:
// IDLE             : Wait in this state until valid is received from the Ethernet side
// READ_ETH_HEADER  : Start processing the ethernet Header fields of the incoming data. Total length is 112 bits
//                 
// READ_IP_HEADER   : Start processing the IPv4 Header fields of the incoming data. Total length is between 20 and
//                    60 bytes if header includes options. Valid values for remaining_ip_header are 1 (odd options value)
//                    and 2 (even options value)
// READ_PAYLOAD     : Start sending UDP payload to the UDP block.
// DROP             : Drop all non-UDP or invalid packets

always_comb
begin
    // Avoid any possible latches
    next_state = state;
    case(state)
        IDLE: begin
            // Guarding against seeing a rogue tlast in the stream
            if (axis_tvalid_i && !axis_tlast_i) begin
                next_state = READ_ETH_HEADER;
            end
        end

        READ_ETH_HEADER: begin
            // Not checking m_axis_udp_data_tready becasue we aLways process valid header data
            if (axis_tvalid_i && axis_tlast_i) begin
                // Received a corrupt frame return to IDLE
                next_state = IDLE;
            end
            else if (axis_tvalid_i && hdr_cnt_next == IP_HDR_W0) begin 
                next_state = READ_IP_HEADER;
            end
        end

        // I already have the first 16 bits of the header
        // from the previous state with IHL. If >5 header
        // contains options
        READ_IP_HEADER: begin
            if (eth_type != IPv4 || ip_ver != 4'd4) begin
                next_state = DROP;
            end
            else begin
                if (eof) begin
                    next_state = IDLE;
                end
                else if (axis_tvalid_i) begin
                    if (remaining_ip_header > 2) begin
                        next_state = READ_IP_HEADER;
                    end 
                    else begin
                        if (ip_protocol_vld) begin
                            if (m_axis_udp_data_tready) begin
                                next_state = READ_PAYLOAD;
                            end
                        end
                        else begin
                            next_state = DROP;
                        end
                    end
                end
            end
        end
        
        READ_PAYLOAD: begin
            if (eof && remaining_bits) begin
                next_state = LOAD_LAST;
            end
            else if (eof) begin
                next_state = IDLE;
            end             
            else if (force_tlast && m_axis_udp_data_tready && axis_tvalid_i && remaining_bits) begin
                next_state = LOAD_LAST;
            end            
            else if (force_tlast && m_axis_udp_data_tready && axis_tvalid_i) begin
                next_state = DROP;
            end
        end

        LOAD_LAST: begin
            if (force_tlast) begin
                if (eof) begin
                    next_state = IDLE;
                end
                else if (m_axis_udp_data_tready) begin //&& axis_tvalid_i
                    next_state = DROP;
                end
            end
            else begin
                if (m_axis_udp_data_tready && axis_tvalid_i) begin
                    next_state = READ_ETH_HEADER;
                end
                else if (m_axis_udp_data_tready) begin
                    next_state = IDLE;
                end
            end
        end           

        DROP: begin
            // We don't care about ready here as we are just dropping data
            if (axis_tvalid_i && axis_tlast_i) begin
                next_state = IDLE;
            end
        end

    default : next_state  = IDLE;
  endcase         
end

always_ff @(posedge ap_clk) begin
    if(!ap_rst_n) begin
        state <= IDLE;
    end
    else begin
        state <= next_state;
    end
end

// Count the number of words in the Ethernet Header and IP Header
always_ff @(posedge ap_clk) begin
    if ((next_state == READ_ETH_HEADER || next_state == READ_IP_HEADER) && axis_tvalid_i && !hdr_done) begin
        hdr_cnt <= hdr_cnt_next;
    end
    else if (next_state == READ_PAYLOAD || next_state == IDLE || next_state == DROP || next_state == LOAD_LAST) begin //added load_last
        hdr_cnt <= 3'b000;
    end
end

assign hdr_cnt_next = hdr_cnt + 'd1;

// count the number of remaining words in the IPv4 Header to cover the optional fields
always_ff @(posedge ap_clk) begin
    if (store_eth_header) begin
        remaining_ip_header <= axis_tdata_i[51:48];
    end
    else if (next_state == READ_IP_HEADER && axis_tvalid_i && !hdr_done) begin
        remaining_ip_header <= remaining_ip_header_next;
    end
end

// The IP header length field counts 32-bit words
assign remaining_ip_header_next = remaining_ip_header - 'd2;
assign hdr_done = (remaining_ip_header <= 2 && state == READ_IP_HEADER);

// count the number of bytes in the IP packet for trimming
// Subtracting the 2 bytes that are in the IP Header
assign bytes2keep = (ip_tot_len-2) % 8;

always_ff @(posedge ap_clk) begin
    bytes2keep_r <= bytes2keep;
end

always_ff @(posedge ap_clk) begin
    // We always process the Header regardless of m_axis_udp_data_tready.
    if (next_state == READ_IP_HEADER && axis_tvalid_i && !hdr_done) begin
        byte_cnt <= byte_cnt_next;
    end
    // We also need to check if we can send a word before incrementing.
    else if (next_state == READ_PAYLOAD && axis_tvalid_i && m_axis_udp_data_tready) begin
        byte_cnt <= byte_cnt_next;
    end    
    else if ((state != IDLE && next_state == IDLE) || (state == LOAD_LAST && next_state == READ_ETH_HEADER)) begin
        // I've seen 2 bytes while reading the Ethernet Header
        byte_cnt <= 16'h2;
    end
end

assign byte_cnt_next = byte_cnt + 'd8;

// If trimming is required we need to force tlast for the UDP payload
always_ff @(posedge ap_clk) begin
    if (force_tlast && axis_tlast_i && axis_tvalid_i) begin
        force_tlast <= 1'b0;
    end
    else if ((byte_cnt_next >= ip_tot_len) && m_axis_udp_data_tready && axis_tvalid_i && next_state == READ_PAYLOAD) begin
        force_tlast <= 1'b1;
    end
end

// Update tkeep if we are trimming the IP Payload to the proper length
assign udp_tkeep_i = (force_tlast) ? count2keep(bytes2keep_r) : axis_tkeep_i;

always_comb
begin
    store_eth_header = (hdr_cnt == ETH_HDR_DONE);
    store_ip_header0 = (hdr_cnt == IP_HDR_W0);
    store_ip_header1 = (hdr_cnt == IP_HDR_W1);
    store_ip_header2 = (hdr_cnt == IP_HDR_W2);
end

// Assign data fields based on word count
always_ff @(posedge ap_clk) begin
    if (store_eth_header) begin
        eth_type      <= axis_tdata_i[47:32];
        // Start of IP Header
        ip_header_len <= axis_tdata_i[51:48];
        ip_ver        <= axis_tdata_i[55:52];
        //[63:56] -> {ECS,DSCP}
    end
    else if (store_ip_header0) begin
        ip_tot_len[0+:8] <= axis_tdata_i[8+:8];
        ip_tot_len[8+:8] <= axis_tdata_i[0+:8];
        // [31:16] -> Identification
        // [47:32] -> flags, fragment offset
        ip_protocol      <= axis_tdata_i[63:56];
    end
    else if (store_ip_header1) begin
        ip_checksum      <= axis_tdata_i[0+:16];
        src_addr[24+:8]  <= axis_tdata_i[16+:8];
        src_addr[16+:8]  <= axis_tdata_i[24+:8];
        src_addr[8+:8]   <= axis_tdata_i[32+:8];
        src_addr[0+:8]   <= axis_tdata_i[40+:8];
        dst_addr[24+:8]  <= axis_tdata_i[48+:8];
        dst_addr[16+:8]  <= axis_tdata_i[56+:8]; 
    end
    else if (store_ip_header2) begin
        dst_addr[8+:8]  <= axis_tdata_i[0+:8];
        dst_addr[0+:8]  <= axis_tdata_i[8+:8];        
    end            
end

// Register indicates if we have an extra 16 or 32 bits of data in the
// final word of the header. 
// 0: IHL is even include 32-bits
// 1: IHL is odd include 16-bits
always_ff @(posedge ap_clk) begin
    data_sel <= ip_header_len % 2;
end

// ---------------------------------------------------------------------- //
// ------------------------ Checksum Logic ------------------------------ //
// ---------------------------------------------------------------------- //

// Calculate sums of header
always_ff @(posedge ap_clk) begin
    if (axis_tvalid_i) begin 
        if (store_eth_header) begin
            // Fist lower 16 bits for checksum calculation
            hdr_sum_low_reg <= tdata_mangled[63:48];
            hdr_sum_high_reg <= 19'd0;
        end
        else if (next_state == READ_IP_HEADER && !hdr_done) begin
            hdr_sum_low_reg  <= hdr_sum_low_reg + tdata_mangled[31:16] + tdata_mangled[63:48];
            hdr_sum_high_reg <= hdr_sum_high_reg + tdata_mangled[15:0] + tdata_mangled[47:32];        
        end
        else if (state == READ_IP_HEADER && next_state == READ_PAYLOAD) begin
            if (data_sel) begin
                // There are 16 bits of header in the frame with udp payload 
                hdr_sum_high_reg <= hdr_sum_high_reg + tdata_mangled[15:0];
            end
            else begin
                // IHL is even so we have an extra 32 bits to calculate
                hdr_sum_low_reg  <= hdr_sum_low_reg + tdata_mangled[31:16];
                hdr_sum_high_reg <= hdr_sum_high_reg + tdata_mangled[15:0] + tdata_mangled[47:32];
            end
        end
    end
end

assign chk_sum_tmp = hdr_sum_low_reg[15:0] + hdr_sum_high_reg[15:0] + hdr_sum_low_reg[19:16] + hdr_sum_high_reg[19:16];
// Register checksum result to help timing
always_ff @(posedge ap_clk) begin
    if (chk_sum_tmp == 19'h0ffff || chk_sum_tmp == 19'h1fffe) begin
        chk_sum_vld_r <= 1'b1;
    end
    else begin
        chk_sum_vld_r <= 1'b0;
    end
end

// ---------------------------------------------------------------------- //

////////////////////////////////////////////////////////////////////////////

always_comb begin
    ip_protocol_i     = axis_tdata_i[63:56];
    // The IP Addr input has already been mangled to the correct order
    dst_addr_i[0+:8]  = axis_tdata_i[48+:8];
    dst_addr_i[8+:8]  = axis_tdata_i[56+:8];
    dst_addr_i[16+:8] = axis_tdata_i[0+:8];
    dst_addr_i[24+:8] = axis_tdata_i[8+:8];      
end

// Register bits that will be used to decided if packet
// is valid. They are not needed until I reach UDP payload
always_ff @(posedge ap_clk) begin
    if (store_ip_header0) begin
        ip_protocol_vld <= (ip_protocol_i == UDP);
    end
    else if (store_ip_header1) begin
        dst_addr_vld_1 <= (dst_addr_i[15:0] == my_ip_addr[15:0]) || (dst_addr_i[15:0] == {16{1'b1}});
        is_multicast   <= (dst_addr_i[7:4] == 4'hE);
    end
    else if (store_ip_header2) begin
        dst_addr_vld_2 <= (dst_addr_i[31:16] == my_ip_addr[31:16]) || (dst_addr_i[15:0] == {16{1'b1}});
    end    
    // else if (state == IDLE) begin
    else if (next_state == READ_ETH_HEADER) begin
        ip_protocol_vld <= 1'b0;
        dst_addr_vld_1  <= 1'b0;
        dst_addr_vld_2  <= 1'b0;
        is_multicast    <= 1'b0;
    end          
end

always_ff @(posedge ap_clk) begin
    // We have an extra cycle before the checksum is ready
    valid_ip = (dst_addr_vld_1 && dst_addr_vld_2) || is_multicast;
end
assign hdr_vld =  valid_ip && chk_sum_vld_r;
////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------- //
// ----------------------- UDP PAYLOAD Logic ---------------------------- //
// ---------------------------------------------------------------------- //
// We don't need to check for tready here as we only have a partial UDP word
assign udp_payload_sof = (state == READ_IP_HEADER && next_state == READ_PAYLOAD);
assign remaining_bits = (data_sel) ? udp_tkeep_i[2] : udp_tkeep_i[6];
assign udp_payload_last = (state == READ_PAYLOAD && (next_state == IDLE || next_state == DROP)) || state == LOAD_LAST;

// ---------------------------------------------------------------------- //

// Align UDP payload
// We use a 112 bit register that shifts right by 64 on every valid word. This ensures that
// the bottom 16-bits of the previous word are cut off.
// Odd options values shifter
// <------------------------- 112 bits -------------------------->
// <------------- 64 bits -----------------><------ 48 bits ----->
// |-------------------------------------------------------------|
// |           cur word                    |    prev word        |
// |-------------------------------------------------------------|
//                       <---------- 64 bit output -------------->
always_ff @(posedge ap_clk) begin
    // Initialize the shift register at udp sof. This is to ensure I don't
    // loose the first bytes if m_axis_udp_data_tready is low at this point.
    if (udp_payload_sof) begin
        shift_udp_payload <= {axis_tdata_i, axis_tdata_i[63:16]};
        shift_udp_keep    <= {udp_tkeep_i, udp_tkeep_i[7:2]};
        shift_udp_valid   <= {axis_tvalid_i, axis_tvalid_i};
    end
    // Only update if data is valid and UDP block can accept new data
    else if (state == READ_PAYLOAD && axis_tvalid_i && m_axis_udp_data_tready) begin
        shift_udp_payload <= next_shift_udp_payload;
        shift_udp_keep    <= next_shift_udp_keep;
        shift_udp_valid   <= next_shift_udp_valid;
    end
end

// The header has an extra 16 bits in the payload. Shift data to align
// UDP payload
assign next_shift_udp_payload = {axis_tdata_i, shift_udp_payload[64+:48]};
assign next_shift_udp_keep    = {udp_tkeep_i, shift_udp_keep[8+:6]};
assign next_shift_udp_valid   = {axis_tvalid_i, shift_udp_valid[1]};

always_comb begin
    if (state == LOAD_LAST) begin
        udp_data_tdata[0+:48]  = shift_udp_payload[64+:48];
        udp_data_tdata[48+:16] = 16'h0000;
    end    
    else begin
        udp_data_tdata = next_shift_udp_payload[0+:64];
    end
end

always_comb begin
    if (state == LOAD_LAST) begin
        udp_data_tkeep[0+:6]  = shift_udp_keep[8+:6];
        udp_data_tkeep[6+:2]  = 2'b00;
    end
    else if (state == READ_PAYLOAD && axis_tvalid_i) begin
        udp_data_tkeep  = next_shift_udp_keep[0+:8];
    end
    else begin
        udp_data_tkeep  = 8'h00;
    end
end
// -------------------------------------------------------------------------------------

// Even IHL shifter
// <-------------------------- 80 bits -------------------------->
// <------------- 64 bits -----------------><------ 16 bits ----->
// |-------------------------------------------------------------|
// |           cur word                    |    prev word        |
// |-------------------------------------------------------------|
//                       <---------- 64 bit output -------------->
always_ff @(posedge ap_clk) begin
    if (udp_payload_sof) begin
        shift_udp_payload_even <= {axis_tdata_i, axis_tdata_i[63:48]};
        shift_udp_keep_even    <= {udp_tkeep_i, udp_tkeep_i[7:6]};        
    end
    else if (state == READ_PAYLOAD && axis_tvalid_i && m_axis_udp_data_tready) begin
        shift_udp_payload_even <= next_shift_udp_payload_even;
        shift_udp_keep_even    <= next_shift_udp_keep_even;
    end
end

assign next_shift_udp_payload_even = {axis_tdata_i, shift_udp_payload_even[64+:16]};
assign next_shift_udp_keep_even    = {udp_tkeep_i, shift_udp_keep_even[8+:2]};

always_comb begin
    if (state == LOAD_LAST) begin
        udp_data_tdata_even[0+:16]  = shift_udp_payload_even[64+:16];
        udp_data_tdata_even[16+:48] = 48'h000000;
    end    
    else begin
        udp_data_tdata_even = next_shift_udp_payload_even[0+:64];
    end
end

always_comb begin
    if (state == LOAD_LAST) begin
        udp_data_tkeep_even[0+:2]  = shift_udp_keep_even[8+:2];
        udp_data_tkeep_even[2+:6]  = 6'b000000;
    end
    else if (state == READ_PAYLOAD && axis_tvalid_i) begin
        udp_data_tkeep_even  = next_shift_udp_keep_even[0+:8];
    end
    else begin
        udp_data_tkeep_even  = 8'h00;
    end
end
// -------------------------------------------------------------------------------------

always_comb begin
    // Avoid any possible latches
    m_axis_udp_data_tdata  = next_shift_udp_payload[0+:64];
    case(data_sel)
        0: begin
            m_axis_udp_data_tdata = udp_data_tdata_even;
            m_axis_udp_data_tkeep = udp_data_tkeep_even;
        end
        1: begin
            m_axis_udp_data_tdata = udp_data_tdata;
            m_axis_udp_data_tkeep = udp_data_tkeep;
        end
        default: begin
            m_axis_udp_data_tdata = udp_data_tdata;
            m_axis_udp_data_tkeep = udp_data_tkeep;
        end
    endcase
end
// Assign output data going to Rx UDP
// No need to register here as there is an AXI Slice in the Rx UDP Block
assign m_axis_udp_data_tlast  = udp_payload_last;
assign m_axis_udp_data_tuser  = !hdr_vld;

always_comb begin
    if (state == LOAD_LAST) begin
        m_axis_udp_data_tvalid = shift_udp_valid[1];
    end    
    else if (state == READ_PAYLOAD) begin
        m_axis_udp_data_tvalid = &next_shift_udp_valid;
    end
    else begin
        m_axis_udp_data_tvalid = 1'b0;
    end
end

assign m_axis_udp_metadata_tdata  = {ip_tot_len, dst_addr, src_addr};

logic seen = 1'b0;
// I want a single pulse valid for metadata
always_ff @(posedge ap_clk) begin
    if (next_state == READ_PAYLOAD && !seen) begin
        m_axis_udp_metadata_tvalid <= 1'b1;
        seen <= 1'b1;  
    end
    else if (next_state == READ_ETH_HEADER) begin
        m_axis_udp_metadata_tvalid <= 1'b0;
        seen <= 1'b0;
    end
    else begin
        m_axis_udp_metadata_tvalid <= 1'b0;
    end
end

// ---------------------------------------------------------------------- //
always_ff @(posedge ap_clk) begin
    case(data_sel)
        0: begin
            // Dst port is in the next word in this case. Note we will always go to state payload
            if (state == READ_PAYLOAD && axis_tvalid_i && !dst_port_seen) begin
                // Convert the dst port to a byte location in memory. Read will take 2 cycles
                // so sending to memory from here
                mem_addr      <= tdata_mangled[0+:16]/8;
                expectedBit   <= 1 << (tdata_mangled[0+:16] % 8);
                mem_ce        <= 1'b1;
                dst_port_seen <= 1'b1;
            end
            else if (udp_payload_last) begin
                mem_addr <= 13'h0000;
                mem_ce <= 1'b0;
                dst_port_seen <= 1'b0;
            end
            else begin
                mem_addr <= 13'h0000;
                mem_ce <= 1'b0;                
            end
        end
        1: begin
            if (udp_payload_sof) begin
                // Convert the dst port to a byte location in memory. Read will take 2 cycles
                // so sending to memory from here
                mem_addr    <= tdata_mangled[32+:16]/8;
                expectedBit <= 1 << (tdata_mangled[32+:16] % 8);
                mem_ce <= 1'b1;
            end
            else begin
                mem_addr <= 13'h0000;
                mem_ce <= 1'b0;
            end
        end
        default: begin
            mem_addr <= 13'h0000;
            mem_ce <= 1'b0;            
        end       
    endcase
end

    // Probe input and output
    // ila_iph udp_iph (
    //     .clk(ap_clk), // input wire clk

    //     .probe0(axis_tvalid_i), // input wire [0:0] probe0  
    //     .probe1(axis_tready_i), // input wire [63:0]  probe1 
    //     .probe2(axis_tdata_i), // input wire [7:0]  probe2 
    //     .probe3(axis_tlast_i), // input wire [0:0]  probe3 
    //     .probe4(m_axis_udp_data_tvalid), // input wire [0:0]  probe4 
    //     .probe5(m_axis_udp_data_tready), // input wire [0:0]  probe5 
    //     .probe6(m_axis_udp_data_tdata), // input wire [7:0]  probe6 
    //     .probe7(m_axis_udp_data_tlast), // input wire [0:0]  probe7  
    //     .probe8(state), // input wire [0:0]  probe8
    //     .probe9({1'b0, mem_ce}), // input wire [0:0]  probe8
    //     .probe10(force_tlast),
    //     .probe11(byte_cnt),
    //     .probe12(ip_tot_len)
    // );

endmodule