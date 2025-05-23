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
module mac_ip_encode (
    // System Signals
    input  wire          ap_clk,
    input  wire          ap_rst_n,

    input  wire          s_axis_ip_tvalid,
    output logic         s_axis_ip_tready,
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
    input  wire  [31:0]  regDefaultGateway,

    output logic [31:0]  ipv4_packets_sent = '0,
    output logic [31:0]  droppedPkts = '0
);

localparam ETH_TYPE = 16'h0008;

// States for FSM
typedef enum logic[1:0] {IDLE        = 2'b00,
                         HEADER      = 2'b01,
                         DATA        = 2'b10,
                         REMAINDER   = 2'b11} stateCoding_t;
stateCoding_t state = IDLE;
stateCoding_t next_state;

typedef enum logic[1:0] {RD_IDLE     = 2'b00,
                         ETH_HEADER  = 2'b01,
                         ETH_DATA    = 2'b10,
                         DROP        = 2'b11} rd_stateCoding_t;
rd_stateCoding_t rd_state = RD_IDLE;
rd_stateCoding_t rd_next_state;

typedef enum logic[1:0] {REQ_IDLE  = 2'b00,
                         REQ_WAIT  = 2'b01,
                         REQ_SENT  = 2'b10} req_stateCoding_t;
req_stateCoding_t req_state = REQ_IDLE;
req_stateCoding_t req_next_state;

logic           ap_rst;

logic   [63:0]  tdata_mangled;
logic           is_multicast;
logic           vld_ip;

logic    [3:0]  remaining_ip_header = 4'd5;
logic    [3:0]  remaining_ip_header_next;

logic   [19:0]  hdr_sum_high_reg = 19'd0;
logic   [19:0]  hdr_sum_low_reg = 19'd0;
logic   [19:0]  chk_sum;
logic   [15:0]  chk_sum_r;
logic   [15:0]  chk_sum_out;
logic           chk_sum_vld;
logic           chk_sum_vld_r;
logic           chk_sum_vld_pulse;
logic           rd_chksum;
logic           fifo_empty;

logic  [111:0]  shift_tdata = 112'd0;
logic  [111:0]  next_shift_tdata;
logic   [13:0]  shift_tkeep = 14'd0;
logic   [13:0]  next_shift_tkeep;
logic    [1:0]  shift_tvalid = 2'b00;
logic    [1:0]  next_shift_tvalid;

logic    [2:0]  hdr_cnt;
logic    [2:0]  hdr_cnt_next;
logic           hdr_done;
logic           eof;
logic           rd_eof;

logic          data_fifo_tvalid;
logic          data_fifo_tready;
logic  [63:0]  data_fifo_tdata;
logic   [7:0]  data_fifo_tkeep;
logic          data_fifo_tlast;

logic          wr_axis_ip_tvalid;
logic          wr_axis_ip_tready;
logic  [63:0]  wr_axis_ip_tdata;
logic   [7:0]  wr_axis_ip_tkeep;
logic          wr_axis_ip_tlast;

logic          rd_axis_ip_tvalid;
logic          rd_axis_ip_tready;
logic          rd_axis_ip_tready_fifo;
logic  [63:0]  rd_axis_ip_tdata;
logic   [7:0]  rd_axis_ip_tkeep;
logic          rd_axis_ip_tlast;

logic          lookup_reply_valid;
logic          lookup_reply_ready;
logic  [71:0]  lookup_reply_data;

logic   [2:0]  rd_cnt = 3'b100;

logic         arp_lookup_request_valid;
logic         m_axis_arp_lookup_request_tvalid_i;
logic         m_axis_arp_lookup_request_tready_i;
logic [31:0]  m_axis_arp_lookup_request_tdata_i;

always_ff @(posedge ap_clk) begin
    ap_rst <= !ap_rst_n;    
end

mac_ip_axis_data_fifo_0 mac_ip_fifo (
    .s_axis_aresetn(ap_rst_n),
    .s_axis_aclk(ap_clk),
    .s_axis_tvalid(data_fifo_tvalid),
    .s_axis_tready(data_fifo_tready),
    .s_axis_tdata(data_fifo_tdata),
    .s_axis_tkeep(data_fifo_tkeep),
    .s_axis_tlast(data_fifo_tlast),
    .m_axis_tvalid(wr_axis_ip_tvalid),
    .m_axis_tready(rd_axis_ip_tready_fifo),
    .m_axis_tdata(wr_axis_ip_tdata),
    .m_axis_tkeep(wr_axis_ip_tkeep),
    .m_axis_tlast(wr_axis_ip_tlast)
);

axis_register_slice_64 axis_register_arp_in_slice(
    .aclk(ap_clk),
    .aresetn(ap_rst_n),
    .s_axis_tvalid(rd_axis_ip_tvalid),
    .s_axis_tready(rd_axis_ip_tready),
    .s_axis_tdata(rd_axis_ip_tdata),
    .s_axis_tkeep(rd_axis_ip_tkeep),
    .s_axis_tlast(rd_axis_ip_tlast),
    .m_axis_tvalid(m_axis_ip_tvalid),
    .m_axis_tready(m_axis_ip_tready),
    .m_axis_tdata(m_axis_ip_tdata),
    .m_axis_tkeep(m_axis_ip_tkeep),
    .m_axis_tlast(m_axis_ip_tlast)
);

mac_ip_chksum_fifo mac_ip_chksum_fifo_i (
  .clk(ap_clk),
  .srst(ap_rst),
  .din(chk_sum_r),
  .wr_en(chk_sum_vld_pulse),
  .rd_en(rd_chksum),
  .dout(chk_sum_out),
  .full(),
  .empty(fifo_empty),
  .wr_rst_busy(),
  .rd_rst_busy()
);

axis_reg_slice_32 mac_ip_slice_req(
    .aclk(ap_clk),
    .aresetn(ap_rst_n),
    .s_axis_tvalid(m_axis_arp_lookup_request_tvalid_i),
    .s_axis_tready(m_axis_arp_lookup_request_tready_i),
    .s_axis_tdata(m_axis_arp_lookup_request_tdata_i),
    .m_axis_tvalid(m_axis_arp_lookup_request_tvalid),
    .m_axis_tready(m_axis_arp_lookup_request_tready),
    .m_axis_tdata(m_axis_arp_lookup_request_tdata)
);

axis_reg_slice_72 mac_ip_slice_reply(
    .aclk(ap_clk),
    .aresetn(ap_rst_n),
    .s_axis_tvalid(s_axis_arp_lookup_reply_tvalid),
    .s_axis_tready(s_axis_arp_lookup_reply_tready),
    .s_axis_tdata(s_axis_arp_lookup_reply_tdata),
    .m_axis_tvalid(lookup_reply_valid),
    .m_axis_tready(lookup_reply_ready),
    .m_axis_tdata(lookup_reply_data)
);

assign tdata_mangled = {s_axis_ip_tdata[48+:8], s_axis_ip_tdata[56+:8], s_axis_ip_tdata[32+:8],
                        s_axis_ip_tdata[40+:8], s_axis_ip_tdata[16+:8], s_axis_ip_tdata[24+:8],
                        s_axis_ip_tdata[0+:8], s_axis_ip_tdata[8+:8]};   

assign eof              = s_axis_ip_tvalid && s_axis_ip_tlast && data_fifo_tready;
assign rd_eof           = wr_axis_ip_tlast && wr_axis_ip_tvalid && rd_axis_ip_tready;
assign data_fifo_tlast  = (state != IDLE && next_state == IDLE);

always_ff @(posedge ap_clk) begin
    if(next_state == REMAINDER) begin
        s_axis_ip_tready <= 1'b0;
    end
    else begin
        s_axis_ip_tready = data_fifo_tready;
    end
end

// FIFO controls writing the incoming packet to the data FIFO 
// FSM states:
// IDLE        : Wait for valid data
// HEADER      : Process IPv4 Header                
// DATA        : Process payload
// REMAINDER   : Write any remaining bytes to FIFO due to shifting of data
always_comb
begin
    // Avoid any possible latches
    next_state = state;
    case(state)
        IDLE: begin
            if (s_axis_ip_tvalid && data_fifo_tready) begin
                next_state = HEADER;
            end
        end   

        HEADER: begin
            if (eof) begin                
                next_state = IDLE;
            end
            else if (s_axis_ip_tvalid && data_fifo_tready) begin
                if (hdr_done) begin
                    next_state = DATA;
                end 
                else begin
                    next_state = HEADER;
                end
            end                  
        end

        DATA: begin
            if (eof && s_axis_ip_tkeep[2]) begin                
                next_state = REMAINDER;
            end
            else if (eof) begin
                next_state = IDLE;
            end  
        end

        REMAINDER: begin
            if (data_fifo_tready) begin
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

// Count the number of words in the IP Header
always_ff @(posedge ap_clk) begin
    if ((next_state == HEADER) && s_axis_ip_tvalid && !hdr_done) begin
        hdr_cnt <= hdr_cnt_next;
    end
    else if (next_state != HEADER) begin
        hdr_cnt <= 3'b000;
    end
end

assign hdr_cnt_next = hdr_cnt + 'd1;

// count the number of remaining words in the IPv4 Header to cover the optional fields
// Currently on the Tx side we have a fixed length of 5
always_ff @(posedge ap_clk) begin
    if (state == IDLE && next_state == HEADER) begin
        remaining_ip_header <= s_axis_ip_tdata[0+:4];
    end
    else if (next_state == HEADER && s_axis_ip_tvalid && !hdr_done) begin
        remaining_ip_header <= remaining_ip_header_next;
    end
end

// The IP header length field counts 32-bit words
assign remaining_ip_header_next = remaining_ip_header - 'd2;
assign hdr_done = (remaining_ip_header_next <= 2 && state == HEADER);
// Need to fix the counter for this to work
// assign hdr_done = (remaining_ip_header_next <= 2);

// ---------------------------------------------------------------------- //
// ------------------- Create request for MAC addr ---------------------- //
// ---------------------------------------------------------------------- //
assign is_multicast = (s_axis_ip_tdata[7:4] == 4'hE);
assign vld_ip       = ((s_axis_ip_tdata[0+:32] & regSubNetMask) == (regDefaultGateway & regSubNetMask));

always_comb
begin
    if (hdr_cnt_next == 3 && s_axis_ip_tvalid) begin
        arp_lookup_request_valid = 1'b1;
        if (vld_ip || is_multicast) begin
            m_axis_arp_lookup_request_tdata_i = s_axis_ip_tdata[0+:32];
        end
        else begin
            m_axis_arp_lookup_request_tdata_i = regDefaultGateway;
        end        
    end
    else begin
        m_axis_arp_lookup_request_tdata_i  = 32'h00000000;
        arp_lookup_request_valid = 1'b0;
    end
end

always_comb
begin
    // Avoid any possible latches
    req_next_state = req_state;
    case(req_state)
        REQ_IDLE: begin
            if (arp_lookup_request_valid && m_axis_arp_lookup_request_tready_i) begin
                req_next_state = REQ_SENT;
            end
            else if (arp_lookup_request_valid) begin
                req_next_state = REQ_WAIT;
            end
        end   

        REQ_WAIT: begin
            if (m_axis_arp_lookup_request_tready_i) begin                
                req_next_state = REQ_SENT;
            end               
        end

        REQ_SENT: begin
            if (eof) begin                
                req_next_state = REQ_IDLE;
            end         
        end                    

    default : req_next_state  = REQ_IDLE;
  endcase         
end

always_ff @(posedge ap_clk) begin
    if(!ap_rst_n) begin
        req_state <= REQ_IDLE;
    end
    else begin
        req_state <= req_next_state;
    end
end

always_comb
begin
    // Avoid any possible latches
    case(req_state)
        REQ_IDLE: begin
            m_axis_arp_lookup_request_tvalid_i = arp_lookup_request_valid;
        end
        REQ_WAIT: begin
            m_axis_arp_lookup_request_tvalid_i = 1'b1;
        end
        REQ_SENT: begin
            m_axis_arp_lookup_request_tvalid_i = 1'b0;
        end        
    endcase
end

// ---------------------------------------------------------------------- //
// ---------------------- Checksum Gen Logic ---------------------------- //
// ---------------------------------------------------------------------- //
// Calculate sums of header
always_ff @(posedge ap_clk) begin
    if (next_state == IDLE) begin
        hdr_sum_low_reg  <= 19'd0;
        hdr_sum_high_reg <= 19'd0;
    end       
    else if (s_axis_ip_tvalid && data_fifo_tready) begin 
        if (next_state == HEADER && !hdr_done) begin
            if (hdr_cnt == 1) begin
                // We need to skip the checksum bits in this case
                hdr_sum_low_reg  <= hdr_sum_low_reg[15:0] + hdr_sum_low_reg[19:16] + tdata_mangled[15:0] + tdata_mangled[47:32];
                hdr_sum_high_reg <= hdr_sum_high_reg[15:0] + hdr_sum_high_reg[19:16] + tdata_mangled[63:48];                
            end
            else begin
                hdr_sum_low_reg  <= hdr_sum_low_reg[15:0] + hdr_sum_low_reg[19:16] + tdata_mangled[15:0] + tdata_mangled[47:32];
                hdr_sum_high_reg <= hdr_sum_high_reg[15:0] + hdr_sum_high_reg[19:16] + tdata_mangled[31:16] + tdata_mangled[63:48];
            end
        end
        else if (state == HEADER && next_state == DATA) begin
            if (remaining_ip_header_next == 'd1) begin
                hdr_sum_low_reg  <= hdr_sum_low_reg[15:0] + hdr_sum_low_reg[19:16] + tdata_mangled[15:0];
                hdr_sum_high_reg <= hdr_sum_high_reg[15:0] + hdr_sum_high_reg[19:16] + tdata_mangled[31:16];
            end
            else begin
                hdr_sum_low_reg  <= hdr_sum_low_reg[15:0] + hdr_sum_low_reg[19:16] + tdata_mangled[15:0] + tdata_mangled[47:32];
                hdr_sum_high_reg <= hdr_sum_high_reg[15:0] + hdr_sum_high_reg[19:16] + tdata_mangled[31:16] + tdata_mangled[63:48];        
            end
        end
    end 
end

assign chk_sum = hdr_sum_low_reg[15:0] + hdr_sum_high_reg[15:0] + hdr_sum_low_reg[19:16] + hdr_sum_high_reg[19:16];

// Register checksum result to help timing
always_ff @(posedge ap_clk) begin
    // Adding the carry to the LSB of the checksum
    chk_sum_r <= ~(chk_sum[15:0] + chk_sum[19:16]);
    if (state == DATA) begin
       chk_sum_vld <= 1'b1; 
    end
    else begin
       chk_sum_vld <= 1'b0;         
    end
end

always_ff @(posedge ap_clk) begin
    chk_sum_vld_r  <= chk_sum_vld;
end
// Create a pulse to write the checksum value into the FIFO.
assign chk_sum_vld_pulse = chk_sum_vld && !chk_sum_vld_r;

// We use a 112 bit register that shifts right by 64 on every valid word. This ensures that
// the bottom 16-bits of the previous word are cut off.
// <------------------------- 112 bits -------------------------->
// <------------- 64 bits -----------------><------ 48 bits ----->
// |-------------------------------------------------------------|
// |           cur word                    |    prev word        |
// |-------------------------------------------------------------|
//                       <---------- 64 bit output -------------->
always_ff @(posedge ap_clk) begin
    // Initialize the shift register to take into account the Ethernet Header.
    // We need an extra 48-bits which at this point is being set to 0
    if (s_axis_ip_tvalid && data_fifo_tready) begin
        shift_tdata   <= next_shift_tdata;
        shift_tkeep   <= next_shift_tkeep;
        shift_tvalid  <= next_shift_tvalid;
    end
end

// Shift data to align
assign next_shift_tdata  = {s_axis_ip_tdata, shift_tdata[64+:48]};
assign next_shift_tkeep  = {s_axis_ip_tkeep, shift_tkeep[8+:6]};
assign next_shift_tvalid = {s_axis_ip_tvalid, shift_tvalid[1]};

always_comb
begin
    if (state == IDLE && next_state == HEADER) begin
        // Load the first word since I have enough data
        // taking the 0's for the ethernet header into account
        data_fifo_tvalid        = next_shift_tvalid[1];
        data_fifo_tkeep[0+:6]   = 6'b111111;
        data_fifo_tkeep[6+:2]   = next_shift_tkeep[6+:2];
        data_fifo_tdata[0+:48]  = 48'd0;
        data_fifo_tdata[48+:16] = next_shift_tdata[48+:16];         
    end
    else if (state == REMAINDER) begin
        data_fifo_tvalid        = shift_tvalid[1];
        data_fifo_tkeep[0+:6]   = shift_tkeep[8+:6];
        data_fifo_tkeep[6+:2]   = 2'b00;
        data_fifo_tdata[0+:48]  = shift_tdata[64+:48];
        data_fifo_tdata[48+:16] = 16'h0000;        
    end
    else begin
        // data_fifo_tvalid = &next_shift_tvalid;
        data_fifo_tvalid = next_shift_tvalid[1];
        data_fifo_tkeep  = next_shift_tkeep[0+:8];
        data_fifo_tdata  = next_shift_tdata[0+:64];    
    end    
end

// ---------------------------------------------------------------------- //
// -------------------------- Read Logic -------------------------------- //
// ---------------------------------------------------------------------- //
// FIFO controls reading data from FIFO 
// FSM states:
// RD_IDLE     : Wait for valid responce from ARP block. If there was a hit
//               insert Ethernet Header else drop packet from FIFO. We don't
//               check valid here since we don't need the data just yet. Ethenert 
//               header is 112-bits.
// ETH_HEADER  : Insert Ethernet Header to output data                
// ETH_DATA    : Read the data from the fifo which can be passed to the output
// DROP        : Drop any packets due to missed ARP requests
always_comb
begin
    // Avoid any possible latches
    rd_next_state = rd_state;
    case(rd_state)
        RD_IDLE: begin
            if (lookup_reply_valid && rd_axis_ip_tready) begin
                if (lookup_reply_data[64] == 1'b1) begin
                   rd_next_state = ETH_HEADER; 
                end
                else begin
                    rd_next_state = DROP; 
                end
            end
        end   

        ETH_HEADER: begin
            if (wr_axis_ip_tvalid && rd_axis_ip_tready) begin                
                rd_next_state = ETH_DATA;
            end               
        end

        ETH_DATA: begin
            if (rd_eof) begin                
                rd_next_state = RD_IDLE;
            end         
        end

        DROP: begin
            if (rd_eof) begin 
                rd_next_state = RD_IDLE;
            end
        end                       

    default : rd_next_state  = RD_IDLE;
  endcase         
end

always_ff @(posedge ap_clk) begin
    if(!ap_rst_n) begin
        rd_state <= RD_IDLE;
    end
    else begin
        rd_state <= rd_next_state;
    end
end

// Ready should stay high only while next state is IDLE 
// so we don't pop more than 1 item out of the FIFO
always_comb 
begin
    if (rd_next_state == RD_IDLE) begin
       lookup_reply_ready = 1'b1; 
    end
    else begin
        lookup_reply_ready = 1'b0;
    end
end

// rd_cnt[0] indicates the location where I need to insert the checksum
always_ff @(posedge ap_clk) begin
    if (rd_state == RD_IDLE) begin
        rd_cnt <= 3'b100;
    end
    else if (wr_axis_ip_tvalid && rd_axis_ip_tready) begin
        rd_cnt <= rd_cnt >> 1;
    end
end

// In current design IHL is tied to 5, by the time the checksum is
// required it should already be in the FIFO. 
always_ff @(posedge ap_clk) begin
    if (rd_cnt[0] == 1'b1 && rd_state == ETH_DATA && !fifo_empty) begin
        rd_chksum <= 1'b1; 
    end
    else if (rd_state == DROP && rd_next_state == RD_IDLE && !fifo_empty) begin
        // We still need to remove the checksum for the missed packet
        rd_chksum <= 1'b1;
    end
    else begin
        rd_chksum <= 1'b0; 
    end
end

// Assign output data based on rd_state
always_comb
begin
    case(rd_state)
        RD_IDLE: begin
            if (lookup_reply_valid) begin
                rd_axis_ip_tvalid        = lookup_reply_data[64];
                rd_axis_ip_tdata[0+:48]  = lookup_reply_data[0+:48];
                rd_axis_ip_tdata[48+:16] = myMacAddress[0+:16];
                rd_axis_ip_tkeep         = 8'hFF;
            end
            else begin
                rd_axis_ip_tvalid = 1'b0;
                rd_axis_ip_tdata  = 64'd0;
                rd_axis_ip_tkeep  = 8'h00; 
            end
            rd_axis_ip_tready_fifo = 1'b0;
            rd_axis_ip_tlast       = 1'b0;            
        end

        ETH_HEADER: begin
            rd_axis_ip_tvalid        = wr_axis_ip_tvalid;
            rd_axis_ip_tdata[0+:32]  = myMacAddress[16+:32];
            rd_axis_ip_tdata[32+:16] = ETH_TYPE;
            rd_axis_ip_tdata[48+:16] = wr_axis_ip_tdata[48+:16];
            rd_axis_ip_tready_fifo   = rd_axis_ip_tready;
            rd_axis_ip_tkeep         = wr_axis_ip_tkeep;
            rd_axis_ip_tlast         = wr_axis_ip_tlast;
        end

        ETH_DATA: begin
            rd_axis_ip_tvalid      = wr_axis_ip_tvalid;
            rd_axis_ip_tlast       = wr_axis_ip_tlast;
            rd_axis_ip_tkeep       = wr_axis_ip_tkeep;
            rd_axis_ip_tready_fifo = rd_axis_ip_tready;
            if (rd_cnt[0] == 1'b1) begin
                rd_axis_ip_tdata[0+:16]  = {chk_sum_out[0+:8], chk_sum_out[8+:8]};
                rd_axis_ip_tdata[16+:48] = wr_axis_ip_tdata[16+:48];
            end
            else begin
                rd_axis_ip_tdata = wr_axis_ip_tdata;
            end
        end

        DROP: begin
            rd_axis_ip_tready_fifo = 1'b1;
            rd_axis_ip_tvalid      = 1'b0;
            rd_axis_ip_tdata       = 64'd0;
            rd_axis_ip_tkeep       = 8'h00;
            rd_axis_ip_tlast       = 1'b0;       
        end

        default: begin
            rd_axis_ip_tvalid      = 1'b0;
            rd_axis_ip_tdata       = 64'd0;
            rd_axis_ip_tlast       = 1'b0;
            rd_axis_ip_tkeep       = 8'h00;
            rd_axis_ip_tready_fifo = 1'b0;
        end
    endcase
end

// ---------------------------------------------------------------------- //
// ----------------------------- Stats ---------------------------------- //
// ---------------------------------------------------------------------- //
always_ff @(posedge ap_clk) begin
    if (rd_state == DROP && rd_next_state == RD_IDLE) begin
        droppedPkts <= droppedPkts + 1;
    end
end

always_ff @(posedge ap_clk) begin
    if (rd_state == ETH_DATA && rd_next_state == RD_IDLE) begin
        ipv4_packets_sent <= ipv4_packets_sent + 1;
    end
end

endmodule