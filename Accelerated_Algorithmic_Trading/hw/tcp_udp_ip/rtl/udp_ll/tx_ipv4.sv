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
 * Generate IPv4 Packet from Tx UDP packet
 */
module tx_ipv4 (
    // System Signals
    input  wire             ap_clk,
    input  wire             ap_rst_n,

    input  wire  [31:0]     local_ipv4_addr,
    input  wire   [7:0]     protocol,

    // UDP Data
    input  wire             s_axis_udp_tvalid,
    output logic            s_axis_udp_tready,
    input  wire  [63:0]     s_axis_udp_tdata,
    input  wire   [7:0]     s_axis_udp_tkeep,
    input  wire             s_axis_udp_tlast,

    input wire              udp_meta_valid,
    output logic            udp_meta_ready,
    input wire   [47:0]     udp_meta_data,

    // IPv4 Output Packet
    output logic            m_axis_ip_data_tvalid,
    input  wire             m_axis_ip_data_tready,
    output logic [63:0]     m_axis_ip_data_tdata,
    output logic  [7:0]     m_axis_ip_data_tkeep,
    output logic            m_axis_ip_data_tuser,
    output logic            m_axis_ip_data_tlast
);

localparam VERSION = 4'h4;
localparam IHL = 4'h5; // We use a fixed length Header size of 20 bytes. No options
localparam TTL = 8'h40;
localparam HEADER_W0 = 0;
localparam HEADER_W1 = 1;
localparam HEADER_W2 = 2;

typedef enum logic[2:0] {IDLE            = 3'b000,
                         WAIT_RDY        = 3'b001,
                         IP_HEADER       = 3'b010,
                         PAYLOAD         = 3'b011,
                         LOAD_LAST       = 3'b100} ip_stateCoding_t;
ip_stateCoding_t state = IDLE;
ip_stateCoding_t next_state;

logic            axis_data_tvalid;
logic            axis_data_tready;
logic   [63:0]   axis_data_tdata;
logic    [7:0]   axis_data_tkeep;
logic            axis_data_tlast;

logic            udp_tvalid_r;

logic    [1:0]   hdr_cnt;
logic    [1:0]   hdr_cnt_next;

logic   [15:0]   ip_length;
logic   [15:0]   ip_length_r;
logic   [31:0]   dst_ip_addr_mangled = 32'd0;

logic   [95:0]   shift_ip_payload;
logic   [11:0]   shift_ip_keep;
logic   [95:0]   next_shift_ip_payload;
logic   [11:0]   next_shift_ip_keep;  

logic            eof;
logic            sop;

// Pass data through register before going to HLS IPv4 Block
udp_ll_axis_reg_slice_64 tx_ipv4_reg_slice (
  .aclk          (ap_clk),                  
  .aresetn       (ap_rst_n),             
  .s_axis_tvalid (axis_data_tvalid), 
  .s_axis_tready (axis_data_tready),
  .s_axis_tdata  (axis_data_tdata),
  .s_axis_tkeep  (axis_data_tkeep),
  .s_axis_tlast  (axis_data_tlast),
  .s_axis_tuser  (1'b0),
  .m_axis_tvalid (m_axis_ip_data_tvalid),
  .m_axis_tready (m_axis_ip_data_tready),
  .m_axis_tdata  (m_axis_ip_data_tdata),
  .m_axis_tkeep  (m_axis_ip_data_tkeep),
  .m_axis_tlast  (m_axis_ip_data_tlast),
  .m_axis_tuser  (m_axis_ip_data_tuser)
);

//just added state after reg slice on meta data
// assign udp_meta_ready = (next_state == IDLE) || (state == IDLE); 
always_ff @(posedge ap_clk) begin
    if ((state == IDLE) || (next_state == IDLE)) begin
        udp_meta_ready <= 1'b1;
    end
    else begin
        udp_meta_ready <= 1'b0;
    end
end


assign eof = s_axis_udp_tlast && s_axis_udp_tvalid && axis_data_tready;

// Adding the 20 bytes worth of Header data to UDP Length
assign ip_length = udp_meta_data[32+:16] + 20;

// dst address isn't required until the end of the IP Header
// Register IP Length to cover case where we don't send it right away
always_ff @(posedge ap_clk) begin
    if (udp_meta_valid) begin
        ip_length_r <= ip_length;
        dst_ip_addr_mangled <= {udp_meta_data[0+:8], udp_meta_data[8+:8], udp_meta_data[16+:8], udp_meta_data[24+:8]};
    end
end

// Signal will indicate the start of payload for the shift register
assign sop = (hdr_cnt == HEADER_W2) && (hdr_cnt_next == 2'h3);

// FIFO controls IP Packet generation
// FSM states:
// IDLE        : Wait for valid metadata from the Tx UDP block.
// IP_HEADER   : Generate the IP Header which has a total length
//               of 20 bytes. Once complete request payload from
//               from Tx UDP Block                 
// PAYLOAD     : Process UDP Payload
// LOAD_LAST   : Final state to send any leftover UDP payload
always_comb
begin
    // Avoid any possible latches
    next_state = state;
    case(state)
        IDLE: begin
            if (udp_meta_valid) begin
                if (axis_data_tready) begin
                    next_state = IP_HEADER;
                end
                else begin
                    next_state = WAIT_RDY;
                end
            end
        end

        WAIT_RDY: begin
            if (axis_data_tready) begin
                next_state = IP_HEADER;
            end      
        end

        IP_HEADER: begin
            if (hdr_cnt_next == 2'h3 && s_axis_udp_tvalid && axis_data_tready) begin
                if (s_axis_udp_tlast) begin
                    // We have no payload only UDP Header
                    // next_state = IDLE;
                    next_state = LOAD_LAST;
                end
                else begin
                    next_state = PAYLOAD;
                end
            end
        end

        PAYLOAD: begin
            if (eof) begin
                if (s_axis_udp_tkeep[4] == 0) begin
                    next_state = IDLE;
                end
                else begin
                    next_state = LOAD_LAST;
                end
            end
        end

        LOAD_LAST: begin
            if (axis_data_tready) begin
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
    case (next_state)
        IDLE: begin
            hdr_cnt <= 2'b00;
        end    
        IP_HEADER: begin
            if (m_axis_ip_data_tready) begin
                hdr_cnt <= hdr_cnt_next;
            end
        end
        PAYLOAD: begin
            hdr_cnt <= 2'b11;
        end
        LOAD_LAST: begin
            hdr_cnt <= 2'b11;
        end        
        default: hdr_cnt <= 2'b00;
    endcase    
end

assign hdr_cnt_next = hdr_cnt + 'd1;


// ---------------------------------------------------------------------- //
// Align payload
// We use a 96 bit register that shifts right by 64 on every valid word. This ensures that
// the bottom 32-bits of the previous word are cut off.

// <-------------------------- 96 bits -------------------------->
// <------------- 64 bits -----------------><------ 32 bits ----->
// |-------------------------------------------------------------|
// |           cur word                    |    prev word        |
// |-------------------------------------------------------------|
//                       <---------- 64 bit output -------------->

always_ff @(posedge ap_clk) begin
    // Initialize the shift register at start of UDP payload. This is to ensure I don't
    // lose the first bytes if m_axis_ip_data_tready is low at this point.
    if (sop) begin
        shift_ip_payload <= {s_axis_udp_tdata, 32'd0};
        shift_ip_keep    <= {s_axis_udp_tkeep, 4'd0};
        udp_tvalid_r     <= s_axis_udp_tvalid;
    end
    // Only update if data is valid and UDP block can accept new data
    else if (s_axis_udp_tvalid && axis_data_tready) begin
        shift_ip_payload <= next_shift_ip_payload;
        shift_ip_keep    <= next_shift_ip_keep;
        udp_tvalid_r     <= 1'b1;
    end
end

// Shift UDP payload to align IP payload output
assign next_shift_ip_payload = {s_axis_udp_tdata, shift_ip_payload[64+:32]};
assign next_shift_ip_keep    = {s_axis_udp_tkeep, shift_ip_keep[8+:4]};

// ---------------------------------------------------------------------- //
// ------------------- IP Data to MAC IP Block -------------------------- //
// ---------------------------------------------------------------------- //
always_comb begin
    axis_data_tdata   = 32'd0;
    axis_data_tkeep   = 4'h0;
    s_axis_udp_tready = 1'b0;
    axis_data_tvalid  = 1'b0;       
    case (state)
        IDLE: begin
            axis_data_tdata   = {32'h00000000, ip_length[0+:8], ip_length[8+:8], 8'h00, VERSION, IHL};
            axis_data_tkeep   = 8'hFF;
            s_axis_udp_tready = 1'b0;
            axis_data_tvalid  =  udp_meta_valid;           
        end

        WAIT_RDY: begin
            axis_data_tdata   = {32'h00000000, ip_length_r[0+:8], ip_length_r[8+:8], 8'h00, VERSION, IHL};
            axis_data_tkeep   = 8'hFF;
            s_axis_udp_tready = 1'b0;
            axis_data_tvalid  =  1'b1;              
        end

        IP_HEADER: begin
            case (hdr_cnt)
                HEADER_W1: begin
                    axis_data_tdata   = {local_ipv4_addr, 16'h0000, protocol, TTL};
                    axis_data_tkeep   = 8'hFF;
                    s_axis_udp_tready = 1'b0;
                    axis_data_tvalid  =  1'b1;
                end
                HEADER_W2: begin
                    axis_data_tdata = {s_axis_udp_tdata[0+:32], dst_ip_addr_mangled};
                    axis_data_tkeep[0+:4] = 4'hF;
                    axis_data_tkeep[4+:4] = s_axis_udp_tkeep[0+:4];
                    s_axis_udp_tready     = axis_data_tready; 
                    axis_data_tvalid      = s_axis_udp_tvalid;
                end
            endcase
        end

        PAYLOAD: begin
            axis_data_tdata   = next_shift_ip_payload[0+:64];
            axis_data_tkeep   = next_shift_ip_keep[0+:8];
            s_axis_udp_tready = axis_data_tready;
            axis_data_tvalid  = udp_tvalid_r && s_axis_udp_tvalid; // valid depends on previous and current valid
        end

        LOAD_LAST: begin
            axis_data_tdata[0+:32]  = shift_ip_payload[64+:32];
            axis_data_tdata[32+:32] = 32'h00000000;
            axis_data_tkeep[0+:4]   = shift_ip_keep[8+:4];
            axis_data_tkeep[4+:4]   = 4'h0;
            s_axis_udp_tready       = 1'b0; //Don't request new data here
            axis_data_tvalid        = udp_tvalid_r; // We depend on the previous tvalid
        end        
        default: begin
            axis_data_tdata   = 32'd0;
            axis_data_tkeep   = 4'h0;
            s_axis_udp_tready = 1'b0;
            axis_data_tvalid  = 1'b0;          
        end
    endcase
end

assign axis_data_tlast = (state != IDLE) && (next_state == IDLE);

endmodule