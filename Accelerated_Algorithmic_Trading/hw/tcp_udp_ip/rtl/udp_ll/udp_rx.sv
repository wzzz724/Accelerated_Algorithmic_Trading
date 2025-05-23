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
module udp_rx 
(
    // System Signals
    input  wire             ap_clk,
    input  wire             ap_rst_n,

    // Destination Port Lookup result
    input  wire  [7:0]     dst_port_lookup,
    input  wire            dst_port_vld,
    input  wire  [7:0]     expectedBit,

    // Incoming Data from IP Handler
    input  wire            s_axis_iph_tvalid,
    output logic           s_axis_iph_tready,
    input  wire [63:0]     s_axis_iph_tdata,
    input  wire  [7:0]     s_axis_iph_tkeep,
    input  wire            s_axis_iph_tuser,
    input  wire            s_axis_iph_tlast,
    input  wire            s_axis_iph_metadata_tvalid,
    input  wire [79:0]     s_axis_iph_metadata_tdata,

    // UDP Application Interface
    output wire             m_axis_udp_tvalid,
    input  wire             m_axis_udp_tready,
    output wire  [63:0]     m_axis_udp_tdata,
    output wire   [7:0]     m_axis_udp_tkeep,
    output wire             m_axis_udp_tlast,
    output wire   [7:0]     m_axis_udp_data_tstrb,
    output wire             m_axis_udp_metadata_tvalid,
    input  wire             m_axis_udp_metadata_tready,
    output wire [255:0]     m_axis_udp_metadata_tdata,
    output wire  [31:0]     m_axis_udp_metadata_tkeep,
    output wire             m_axis_udp_metadata_tlast,
    output logic [31:0]     m_axis_udp_metadata_tstrb,

    // Stat Counters
    output logic [31:0]     datagrams_recv_invalid = '0,
    output logic [31:0]     datagrams_recv = '0
);

// States for FSM
typedef enum logic[1:0] {IDLE        = 2'b00,
                         DATA        = 2'b01,
                         WAIT_MEM    = 2'b10,
                         INVALID_PKT = 2'b11} udp_stateCoding_t;
udp_stateCoding_t state = IDLE;
udp_stateCoding_t next_state;

typedef enum logic[1:0] {WAIT_VLD       = 2'b00,
                         WAIT_DATA      = 2'b01,
                         SEND_META      = 2'b10,
                         HOLD_META      = 2'b11} meta_stateCoding_t;
meta_stateCoding_t meta_state = WAIT_VLD;
meta_stateCoding_t next_meta_state;

logic             axis_tvalid_i;
logic             axis_tready_i;
logic  [63:0]     axis_tdata_i;
logic   [7:0]     axis_tkeep_i;
logic             axis_tlast_i;
logic             iph_axis_tvalid_i;
logic             iph_axis_tready_i;
logic  [63:0]     iph_axis_tdata_i;
logic   [7:0]     iph_axis_tkeep_i;
logic             iph_axis_tlast_i;
logic             iph_axis_tuser_i;
logic             axis_meta_tvalid_i;
logic             axis_meta_tready_i;
logic [255:0]     axis_meta_tdata_i;
logic  [31:0]     axis_meta_tkeep_i;
logic             axis_meta_tlast_i;
logic             eof;

logic  [15:0]     srcPort;
logic  [15:0]     dstPort;
logic  [15:0]     length;
logic  [15:0]     srcPort_r = 16'h0000;
logic  [15:0]     dstPort_r = 16'h0000;
logic  [15:0]     length_r  = 16'h0000;
logic             fPortMatch;
logic             fPortMatch_r = 1'b0;
logic             rd_vld = 1'b0;

logic [31:0]      their_addr = 32'd0;
logic [31:0]      dst_addr = 32'd0;

udp_ll_axis_reg_slice_64 iph2udp_reg_slice (
  .aclk          (ap_clk),                  
  .aresetn       (ap_rst_n),             
  .s_axis_tvalid (s_axis_iph_tvalid), 
  .s_axis_tready (s_axis_iph_tready),
  .s_axis_tdata  (s_axis_iph_tdata),
  .s_axis_tkeep  (s_axis_iph_tkeep),
  .s_axis_tlast  (s_axis_iph_tlast),
  .s_axis_tuser  (s_axis_iph_tuser),
  .m_axis_tvalid (iph_axis_tvalid_i),
  .m_axis_tready (iph_axis_tready_i),
  .m_axis_tdata  (iph_axis_tdata_i),
  .m_axis_tkeep  (iph_axis_tkeep_i),
  .m_axis_tlast  (iph_axis_tlast_i),
  .m_axis_tuser  (iph_axis_tuser_i)
);

// Pass data through register before going to user app
udp_ll_axis_reg_slice_64 rx_reg_slice (
  .aclk          (ap_clk),                  
  .aresetn       (ap_rst_n),             
  .s_axis_tvalid (axis_tvalid_i), 
  .s_axis_tready (axis_tready_i),
  .s_axis_tdata  (axis_tdata_i),
  .s_axis_tkeep  (axis_tkeep_i),
  .s_axis_tlast  (axis_tlast_i),
  .s_axis_tuser  (1'b0),
  .m_axis_tvalid (m_axis_udp_tvalid),
  .m_axis_tready (m_axis_udp_tready),
  .m_axis_tdata  (m_axis_udp_tdata),
  .m_axis_tkeep  (m_axis_udp_tkeep),
  .m_axis_tlast  (m_axis_udp_tlast),
  .m_axis_tuser  ()
);

udp_ll_reg_slice_256 rx_meta_reg_slice (
  .aclk          (ap_clk),                  
  .aresetn       (ap_rst_n),             
  .s_axis_tvalid (axis_meta_tvalid_i), 
  .s_axis_tready (axis_meta_tready_i),
  .s_axis_tdata  (axis_meta_tdata_i),
  .s_axis_tkeep  (axis_meta_tkeep_i),
  .s_axis_tlast  (axis_meta_tlast_i),
  .s_axis_tuser  (1'b0),
  .m_axis_tvalid (m_axis_udp_metadata_tvalid),
  .m_axis_tready (m_axis_udp_metadata_tready),
  .m_axis_tdata  (m_axis_udp_metadata_tdata),
  .m_axis_tkeep  (m_axis_udp_metadata_tkeep),
  .m_axis_tlast  (m_axis_udp_metadata_tlast),
  .m_axis_tuser  ()
);

assign eof = iph_axis_tvalid_i && iph_axis_tlast_i && axis_tready_i;

// UDP data to register slice
assign axis_tdata_i          = iph_axis_tdata_i;
assign axis_tlast_i          = iph_axis_tlast_i;
assign axis_tkeep_i          = iph_axis_tkeep_i;
assign m_axis_udp_data_tstrb = '1;

// Register incoming metadata to be used when required
always_ff @(posedge ap_clk) begin
    if (s_axis_iph_metadata_tvalid) begin
        their_addr <= s_axis_iph_metadata_tdata[0+:32];
        dst_addr   <= s_axis_iph_metadata_tdata[32+:32];
    end
end

// Extract required information for metadata from UDP header and register for later use
assign srcPort = {iph_axis_tdata_i[0+:8], iph_axis_tdata_i[8+:8]};
assign dstPort = {iph_axis_tdata_i[16+:8], iph_axis_tdata_i[24+:8]};
assign length  = {iph_axis_tdata_i[32+:8], iph_axis_tdata_i[40+:8]} - 8;

always_ff @(posedge ap_clk) begin
    if(state == IDLE && next_state != IDLE) begin
        srcPort_r <= srcPort;
        dstPort_r <= dstPort;
        length_r  <= length;
    end
end

assign axis_meta_tdata_i  = {144'd0, length_r, dstPort_r, dst_addr, srcPort_r, their_addr};
assign axis_meta_tkeep_i  = 32'h3FFF;
assign m_axis_udp_metadata_tstrb = 32'h3FFF;

// dst_port_lookup is already registered from the axi control block
assign fPortMatch = ((dst_port_lookup & expectedBit) != 8'h00);

// dst_port_vld arrives early so I need to register the value
always_ff @(posedge ap_clk) begin
    if (dst_port_vld && ((state == IDLE && iph_axis_tvalid_i == 1'b0) || state == DATA)) begin
        fPortMatch_r  <= fPortMatch;
        rd_vld <= 1'b1;
    end
    // reset the values once we leave IDLE since it has been seen
    else if (state == IDLE && next_state != IDLE) begin
        rd_vld <= 1'b0;
        fPortMatch_r <= 1'b0;   
    end
end

// ------------------------------------- //
// IP Frame
//  Field                       Length
//  Source Port                 2 bytes
//  Destination Port            2 bytes
//  Length                      2 bytes
//  Checksum                    2 bytes
//  payload                     
// ------------------------------------- //
// FIFO controls the handling of incoming data from Ethernet 
// FSM states:
// IDLE             : Wait for valid UDP data and drop 1st word 
//                    which is the header we don't need
// WAIT_MEM         : We enter this state if we haven't received a response
//                    from the memory regarding the Port Adrress. We have the 1 word of payload 
//                    and once response is recieved we transition out of this state
// DATA             : Process UDP payload
// INVALID_PKT      : Drop incoming UDP payload

always_comb
begin
    // Avoid any possible latches
    next_state = state;
    case(state)
        IDLE: begin
            if (iph_axis_tvalid_i) begin
                if (rd_vld) begin
                    if (!fPortMatch_r) begin                 
                        next_state = INVALID_PKT;
                    end
                    else begin
                        next_state = DATA;
                    end
                end                
                else if (dst_port_vld) begin
                    if (!fPortMatch) begin                 
                        next_state = INVALID_PKT;
                    end
                    else begin
                        next_state = DATA;
                    end
                end
                else begin
                    next_state = WAIT_MEM;
                end
            end
        end

        WAIT_MEM: begin
            if (dst_port_vld) begin
                if (eof) begin                
                    next_state = IDLE;
                end
                else if (!fPortMatch || iph_axis_tuser_i) begin
                    next_state = INVALID_PKT;
                end                        
                else if (iph_axis_tvalid_i) begin
                    next_state = DATA;
                end           
            end              
        end        

        DATA: begin
            if (eof) begin                
                next_state = IDLE;
            end
            else if (iph_axis_tuser_i) begin
                next_state = INVALID_PKT;
            end            
        end

        INVALID_PKT: begin
            if (iph_axis_tvalid_i && iph_axis_tlast_i) begin // dropping don't care about ready...
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

always_comb begin
    axis_tvalid_i = 1'b0;
    case(state)
        IDLE: begin
            axis_tvalid_i = 1'b0;
        end

        WAIT_MEM: begin
            // We need to cover the case where we have a single word and return to IDLE 
            // by checking fPortMatch
            axis_tvalid_i = iph_axis_tvalid_i && !iph_axis_tuser_i && fPortMatch;
        end

        DATA: begin
            // We don't care about fPortMatch since it has already been checked
            axis_tvalid_i = iph_axis_tvalid_i && !iph_axis_tuser_i;
        end        

        INVALID_PKT: begin
            axis_tvalid_i = 1'b0;
        end

        default: axis_tvalid_i = 1'b0;
    endcase
end


always_comb begin
    iph_axis_tready_i = 1'b0;
    case(state)
        IDLE: begin
            // We always want to receive the first word of payload
            iph_axis_tready_i = 1'b1;
        end
        WAIT_MEM: begin
            if (next_state == IDLE || next_state == INVALID_PKT) begin
                iph_axis_tready_i = 1'b1;
            end
            else if (next_state == DATA) begin
                iph_axis_tready_i = axis_tready_i;
            end
            else begin
                iph_axis_tready_i = 1'b0;
            end
        end
        DATA: begin
            if (next_state == IDLE) begin
                iph_axis_tready_i = 1'b1;
            end
            else begin
                iph_axis_tready_i = axis_tready_i;
            end
        end  
        INVALID_PKT: begin
            iph_axis_tready_i = 1'b1;
        end
        default: iph_axis_tready_i = 1'b0;
    endcase
end

always_comb begin
    next_meta_state = meta_state;
    case(meta_state)
        WAIT_VLD: begin
            if (s_axis_iph_metadata_tvalid && state == IDLE) begin
                next_meta_state = WAIT_DATA;
            end
            else if (s_axis_iph_metadata_tvalid && state != IDLE) begin
                next_meta_state = HOLD_META;
            end
        end
        HOLD_META: begin
            // We need to wait until the previous packet has completed
            if (state == IDLE) begin
                next_meta_state = WAIT_DATA;
            end
        end     
        WAIT_DATA: begin
            // I need to skip the UDP Header. Wait until we are in state DATA or WAIT MEM
            // tvalid is there to ensure that we actually have a valid payload
            if (axis_meta_tready_i && axis_tvalid_i && (state == DATA || state == WAIT_MEM)) begin
                next_meta_state = SEND_META;
            end
            else if (state == INVALID_PKT && next_state == IDLE) begin
                // Data was dropped and I don't see eof in this case since
                // it is not checked in INVALID_PKT
                next_meta_state = WAIT_VLD;
            end            
            else if (eof) begin
                // Data was dropped since we never saw valid udp payload
                next_meta_state = WAIT_VLD;
            end
        end
        SEND_META: begin
                next_meta_state = WAIT_VLD;
        end
        default: next_meta_state = WAIT_VLD;
    endcase
end

always_ff @(posedge ap_clk) begin
    if(!ap_rst_n) begin
        meta_state <= WAIT_VLD;
    end
    else begin
        meta_state <= next_meta_state;
    end
end

always_comb begin
    if (meta_state == WAIT_DATA && next_meta_state == SEND_META) begin
        axis_meta_tvalid_i = 1'b1;
        axis_meta_tlast_i = 1'b1;
    end   
    else begin
        axis_meta_tvalid_i = 1'b0;
        axis_meta_tlast_i = 1'b0;
    end
end
// ------------------------------------------------------------------------------------------------------------------

    // // Probe input and output
    // ila_rx udp_rx (
    //     .clk(ap_clk), // input wire clk

    //     .probe0(s_axis_iph_tvalid), // input wire [0:0] probe0  
    //     .probe1(s_axis_iph_tready), // input wire [63:0]  probe1 
    //     .probe2(s_axis_iph_tdata), // input wire [7:0]  probe2 
    //     .probe3(s_axis_iph_tlast), // input wire [0:0]  probe3 
    //     .probe4(m_axis_udp_tvalid), // input wire [0:0]  probe4 
    //     .probe5(m_axis_udp_tready), // input wire [0:0]  probe5 
    //     .probe6(m_axis_udp_tdata), // input wire [7:0]  probe6 
    //     .probe7(m_axis_udp_tlast), // input wire [0:0]  probe7  
    //     .probe8(state), // input wire [0:0]  probe8
    //     .probe9(meta_state), // input wire [0:0]  probe8
    //     .probe10(dst_port_vld)
    // );

// ---------------------------------------------------------------------- //
// ---------------------- Stat Counter Logic ---------------------------- //
// ---------------------------------------------------------------------- //
// We only want to count packets with an incorrect Port Number not all dropped packets
always_ff @(posedge ap_clk) begin
    if (!iph_axis_tuser_i) begin
        if (state == INVALID_PKT && next_state == IDLE) begin
            datagrams_recv_invalid <= datagrams_recv_invalid + 1;
        end
        else if (!fPortMatch && state == WAIT_MEM && next_state == IDLE) begin
            datagrams_recv_invalid <= datagrams_recv_invalid + 1;
        end
    end 
end

always_ff @(posedge ap_clk) begin
    if (state != IDLE && next_state == IDLE && !iph_axis_tuser_i) begin
        datagrams_recv <= datagrams_recv + 1;
    end    
end

endmodule