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
 * Tx UDP Block. Generates IP data from incoming UDP data
 */
module udp_tx (
    // System Signals
    input  wire           ap_clk,
    input  wire           ap_rst_n,

    // Incoming UDP Data from User App
    input  wire           s_axis_tx_data_tvalid,
    output logic          s_axis_tx_data_tready,
    input  wire   [63:0]  s_axis_tx_data_tdata,
    input  wire    [7:0]  s_axis_tx_data_tkeep,
    input  wire    [7:0]  s_axis_tx_data_tstrb,
    input  wire           s_axis_tx_data_tlast,

    // Incoming UDP Meta Data from User App
    input  wire           s_axis_tx_meta_tvalid,
    output wire           s_axis_tx_meta_tready,
    input  wire  [255:0]  s_axis_tx_meta_tdata,
    input  wire   [31:0]  s_axis_tx_meta_tkeep,
    input  wire   [31:0]  s_axis_tx_meta_tstrb,
    input  wire           s_axis_tx_meta_tlast,

    // Output IP Data
    output logic          m_axis_iph_data_tvalid,
    input  wire           m_axis_iph_data_tready,
    output logic  [63:0]  m_axis_iph_data_tdata,
    output logic   [7:0]  m_axis_iph_data_tkeep,
    output logic          m_axis_iph_data_tlast,

    // IP Meta Data
    output logic          m_axis_iph_meta_tvalid,
    input  wire           m_axis_iph_meta_tready,
    output logic  [63:0]  m_axis_iph_meta_tdata,  

    // Stat Counter
    output logic  [31:0]  datagrams_transmitted
);

// States for FSM
typedef enum logic[1:0] {IDLE           = 2'b00,
                         WAIT_RDY       = 2'b01,
                         PROCESS_DATA   = 2'b10} udp_stateCoding_t;
udp_stateCoding_t state = IDLE;
udp_stateCoding_t next_state;

logic             eof;

logic  [31:0]     dst_addr;
logic  [15:0]     dst_port;
logic  [31:0]     src_addr;
logic  [15:0]     src_port;
logic  [15:0]     length;

logic  [63:0]     store_header;

logic             axis_data_tvalid;
logic             axis_data_tready;
logic  [63:0]     axis_data_tdata;
logic   [7:0]     axis_data_tkeep;
logic             axis_data_tlast;

logic             axis_tx_meta_tready;
logic             axis_tx_meta_tvalid;
logic [255:0]     axis_tx_meta_tdata;

// Register slice for incoming data
udp_ll_axis_reg_slice_64 tx_data_reg_slice (
  .aclk          (ap_clk),                  
  .aresetn       (ap_rst_n),             
  .s_axis_tvalid (s_axis_tx_data_tvalid), 
  .s_axis_tready (s_axis_tx_data_tready),
  .s_axis_tdata  (s_axis_tx_data_tdata),
  .s_axis_tkeep  (s_axis_tx_data_tkeep),
  .s_axis_tlast  (s_axis_tx_data_tlast),
  .s_axis_tuser  (1'b0),
  .m_axis_tvalid (axis_data_tvalid),
  .m_axis_tready (axis_data_tready),
  .m_axis_tdata  (axis_data_tdata),
  .m_axis_tkeep  (axis_data_tkeep),
  .m_axis_tlast  (axis_data_tlast),
  .m_axis_tuser  ()
);

udp_ll_reg_slice_256 tx_meta_reg_slice (
  .aclk          (ap_clk),                  
  .aresetn       (ap_rst_n),             
  .s_axis_tvalid (s_axis_tx_meta_tvalid), 
  .s_axis_tready (s_axis_tx_meta_tready),
  .s_axis_tdata  (s_axis_tx_meta_tdata),
  .s_axis_tkeep  (s_axis_tx_meta_tkeep),
  .s_axis_tlast  (s_axis_tx_meta_tlast),
  .s_axis_tuser  (1'b0),
  .m_axis_tvalid (axis_tx_meta_tvalid),
  .m_axis_tready (axis_tx_meta_tready),
  .m_axis_tdata  (axis_tx_meta_tdata),
  .m_axis_tkeep  (),
  .m_axis_tlast  (),
  .m_axis_tuser  ()
);

assign eof = axis_data_tlast && axis_data_tvalid && m_axis_iph_data_tready;

// I've got a case where next_state=IDLE I read metadata from
// PROCESS Data and then I get stuck in IDLE having already read metadata
assign axis_tx_meta_tready = (state == IDLE) && m_axis_iph_meta_tready;

always_comb begin
    if (state == PROCESS_DATA) begin
        axis_data_tready = m_axis_iph_data_tready;
    end    
    else begin
        // I don't want to receive payload until I have the metadata
        axis_data_tready = 1'b0;
    end
end

// FIFO control for UDP packet generation
// FSM states:
// IDLE             : Wait in this state until we see valid metadata from User App.
//                    Create UDP Header based on metadata.
//                    Depending on state of IPv4 block we either need to wait till 
//                    it can accecpt data or proceed to processing UDP data.
// WAIT_RDY         : Wait here until IPv4 Block can accept new data. UDP header has been
//                    registered for later use and metadata has been passed on         
// PROCESS_DATA     : Process UDP Payload
always_comb
begin
    // Avoid any possible latches
    next_state = state;
    case(state)
        IDLE: begin
            if (axis_tx_meta_tvalid && m_axis_iph_meta_tready) begin
                if (m_axis_iph_data_tready) begin
                    // This should never be true since the IP handler need 2 cycles
                    // to add and forward the IP Header before it can accept new data 
                    next_state = PROCESS_DATA;
                end
                else begin
                    next_state = WAIT_RDY;
                end
            end
        end

        WAIT_RDY: begin
            if ((store_header[32+:16] == 16'h0800) && m_axis_iph_data_tready) begin
                // Zero Payload case
                next_state = IDLE;
            end            
            else if (axis_data_tvalid && m_axis_iph_data_tready) begin
                next_state = PROCESS_DATA;
            end
        end

        PROCESS_DATA: begin
            if (eof) begin
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

assign dst_addr = axis_tx_meta_tdata[0+:32];
assign dst_port = {axis_tx_meta_tdata[32+:8], axis_tx_meta_tdata[40+:8]};
assign src_addr = axis_tx_meta_tdata[48+:32];
assign src_port = {axis_tx_meta_tdata[80+:8], axis_tx_meta_tdata[88+:8]};
// Adding the UDP Header of 8 Bytes
assign length   = axis_tx_meta_tdata[96+:16] + 8;


// ---------------------------------------------------------------------- //
// ------------------------- IP Meta Data ------------------------------- //
// ---------------------------------------------------------------------- //
assign m_axis_iph_meta_tdata  = {length, dst_addr};
assign m_axis_iph_meta_tvalid = axis_tx_meta_tvalid;

// ---------------------------------------------------------------------- //
// -------------------- UDP Data to IPv4 Block -------------------------- //
// ---------------------------------------------------------------------- //
always_ff @(posedge ap_clk) begin
    if (axis_tx_meta_tvalid) begin
        store_header <= {16'h0000, length[0+:8], length[8+:8], dst_port, src_port};
    end
end

always_comb
begin
    m_axis_iph_data_tdata = 32'd0;
    case (state)
        IDLE: begin
            // UDP Header
            m_axis_iph_data_tdata = {16'h0000, length[0+:8], length[8+:8], dst_port, src_port};            
        end

        WAIT_RDY: begin
            // Registered UDP Header
            m_axis_iph_data_tdata = store_header;        
        end

        PROCESS_DATA: begin
            m_axis_iph_data_tdata = axis_data_tdata;
        end

        default: m_axis_iph_data_tdata = 32'd0;
    endcase
end


//  in rdy I should have vld high because I have data ready to send
always_comb begin
    if (state == IDLE && next_state == IDLE) begin
        // Nothing to forward in this case
        m_axis_iph_data_tvalid = 1'b0;
        m_axis_iph_data_tkeep  = '0;        
    end
    else if (state == IDLE && (next_state == WAIT_RDY || next_state == PROCESS_DATA)) begin
        // UDP Header is valid
        m_axis_iph_data_tvalid = 1'b1;
        m_axis_iph_data_tkeep  = '1;
    end
    else if (state == WAIT_RDY) begin
        // Registered UDP header is valid
        m_axis_iph_data_tvalid = 1'b1;
        m_axis_iph_data_tkeep  = '1;
    end   
    else begin
        m_axis_iph_data_tvalid = axis_data_tvalid;
        m_axis_iph_data_tkeep  = axis_data_tkeep;
    end
end

always_comb begin
    if (state != IDLE && next_state == IDLE) begin
        m_axis_iph_data_tlast = 1'b1;
    end
    else begin
        m_axis_iph_data_tlast = 1'b0;
    end
end

// ---------------------------------------------------------------------- //
// -------------------------- Stat Counters ----------------------------- //
// ---------------------------------------------------------------------- //
always_ff @(posedge ap_clk) begin
    if(!ap_rst_n) begin
        datagrams_transmitted <= 32'd0;
    end
    else if (state != IDLE && next_state == IDLE) begin
        datagrams_transmitted <= datagrams_transmitted + 1;
    end
end

endmodule
