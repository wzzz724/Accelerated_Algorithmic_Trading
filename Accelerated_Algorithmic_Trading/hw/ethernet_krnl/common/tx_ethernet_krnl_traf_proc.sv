/* (c) Copyright 2020 Xilinx, Inc. All rights reserved.
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
module tx_ethernet_krnl_axis_traf_proc
(
    // System Signals
    input  wire         ap_clk,

    input  wire         fifo_resetn,
    input  wire         tx_resetn,
    input  wire         areset,

    input  wire         tx_clk_in,
    input  wire         tx_reset,

    input  wire  [7:0]  tx_threshold,
    output wire         tx_fifo_full,
    output logic [31:0] underflow_count,

    output wire         tx0_axis_tready,
    input  wire         tx0_axis_tvalid,
    input  wire  [63:0] tx0_axis_tdata,
    input  wire         tx0_axis_tlast,
    input  wire   [7:0] tx0_axis_tkeep,

    output wire         axi_str_txd_tvalid,
    input  wire         axi_str_txd_tready,
    output wire         axi_str_txd_tlast,
    output wire   [3:0] axi_str_txd_tkeep,
    output wire   [3:0] axi_str_txd_tuser,
    output wire  [31:0] axi_str_txd_tdata
);

///////////////////////////////////////////////////////////////////////////////
// Wires and Variables
///////////////////////////////////////////////////////////////////////////////
// STATES FOR TX write FSM
typedef enum logic[1:0] {TX_IDLE = 2'b00,
                         TX_LOAD = 2'b01,
                         TX_PAD  = 2'b10} stateCoding_t;
stateCoding_t tx_state_wr = TX_IDLE;
stateCoding_t next_state;

logic         areset_n = 1'b1; 

logic         tx0_axis_tvalid_pad;
logic [63:0]  tx0_axis_tdata_pad;
logic         tx0_axis_tlast_pad;
logic [7:0]   tx0_axis_tkeep_pad;
logic         tx0_axis_tready_pad;

logic         fifo_out_tvalid;
logic [63:0]  fifo_out_tdata;
logic         fifo_out_tlast;
logic [7:0]   fifo_out_tkeep;
logic         fifo_in_tready;
logic         fifo_out_tuser;
logic         reg_slice_eof;

logic [7:0]   min_frame_ok ='d1;
logic         min_frame_r;
logic         frame_ok_pulse;

/////////////////////////////////////////////////////////////

// end of registes and wires for TX PATH
always_ff @(posedge ap_clk) begin
    areset_n <= !areset;    
end

axis_dwidth_converter_tx tx_64_32_dwidth_converter (
    .aclk(tx_clk_in),
    .aresetn(tx_resetn),
    .s_axis_tvalid(tx0_axis_tvalid_pad),
    .s_axis_tready(tx0_axis_tready_pad),
    .s_axis_tdata(tx0_axis_tdata_pad),
    .s_axis_tkeep(tx0_axis_tkeep_pad),
    .s_axis_tlast(tx0_axis_tlast_pad),
    .s_axis_tuser({3'b0,fifo_out_tuser,3'b0,fifo_out_tuser}),
    .m_axis_tvalid(axi_str_txd_tvalid),
    .m_axis_tready(axi_str_txd_tready),
    .m_axis_tdata(axi_str_txd_tdata),
    .m_axis_tkeep(axi_str_txd_tkeep),
    .m_axis_tlast(axi_str_txd_tlast),
    .m_axis_tuser(axi_str_txd_tuser)
);

tx_fifo tx_fifo_inst (
   // FIFO write domain
   .wr_axis_aresetn (areset_n),
   .wr_axis_aclk    (ap_clk),
   .wr_axis_tdata   (tx0_axis_tdata),
   .wr_axis_tkeep   (tx0_axis_tkeep),
   .wr_axis_tvalid  (tx0_axis_tvalid),
   .wr_axis_tlast   (tx0_axis_tlast),
   .wr_axis_tready  (tx0_axis_tready),
   .wr_axis_tuser   ('0),

   .threshold       (tx_threshold),

   // FIFO read domain                  
   .rd_axis_aresetn (tx_resetn),
   .rd_axis_aclk    (tx_clk_in),
   .rd_axis_tdata   (fifo_out_tdata),
   .rd_axis_tkeep   (fifo_out_tkeep),
   .rd_axis_tvalid  (fifo_out_tvalid),
   .rd_axis_tlast   (fifo_out_tlast),
   .rd_axis_tuser   (fifo_out_tuser),
   .rd_axis_tready  (fifo_in_tready),   
      
   // FIFO Status Signals
   .fifo_status     (),
   .fifo_full       (),
   .underflow_count (underflow_count)
);

// Creating a signal for the end of frame for the data coming out of the async fifo
assign reg_slice_eof = fifo_out_tlast & fifo_out_tvalid & tx0_axis_tready_pad;

// When padding is required tready to async FIFO must be driven low to stop accepting data
assign fifo_in_tready = (tx_state_wr == TX_PAD) ? 1'b0 : tx0_axis_tready_pad;

// When padding is required tvalid to data FIFO must be driven high for padded data
assign tx0_axis_tvalid_pad = (tx_state_wr == TX_PAD) ? 1'b1 : fifo_out_tvalid;

// slave tready from the data fifo will indicate if it is full or not 
assign tx_fifo_full = !tx0_axis_tready;

// For each valid word we shift a 1 through min_frame_ok. The MSB indicates 
// if we have seen the minimum required words
always_ff @(posedge tx_clk_in) begin
    if ((tx_state_wr == TX_PAD && next_state == TX_LOAD) || next_state == TX_IDLE) begin
        min_frame_ok <= 'd1;
    end
    else if (tx0_axis_tready_pad && tx0_axis_tvalid_pad && !min_frame_ok[7]) begin
        min_frame_ok <= min_frame_ok << 1;
    end
end

always_ff @(posedge tx_clk_in) begin
    if (tx0_axis_tready_pad) begin
        min_frame_r <= min_frame_ok[7];
    end
end

assign frame_ok_pulse = min_frame_ok[7] & !min_frame_r;

// FIFO controls the padding logic to ensure Ethernet frame meets minimum requirment of 60 bytes 
// FSM states:
// TX_IDLE   : Wait in this state until valid is received from the reister slice connected to
//             the upstream kernel.
// TX_LOAD   : In this state we pass on the data to the Tx Data FIFO. If at the end of the frame we have reached
//             the minimum requirement of 60 bytes we move to TX_IDLE else we move to TX_PAD to pad the data.
// TX_PAD    : The FSM remains in this state until we have reached 60 bytes. While in this state we stop 
//             receiving new data from the register slice. When we reach 60 bytes if we have a new frame waiting 
//             from the register slce we move to TX_LOAD else IDLE.
always_comb
begin
    next_state = tx_state_wr;
    case(tx_state_wr)
    TX_IDLE: begin
        if (fifo_out_tvalid) begin
            next_state = TX_LOAD;
        end
        else begin
            next_state = TX_IDLE;
        end
    end

    TX_LOAD: begin
        if (reg_slice_eof && (!min_frame_ok[7])) begin
            next_state =  TX_PAD;
        end
        else if (reg_slice_eof) begin
            next_state = TX_IDLE;
        end
        else begin
            next_state = TX_LOAD;
        end
    end

    TX_PAD: begin
        if (!min_frame_ok[7]) begin
            next_state = TX_PAD;
        end
        else if (fifo_out_tvalid && tx0_axis_tready_pad) begin
            next_state = TX_LOAD;
        end 
        else if (tx0_axis_tready_pad) begin
            next_state = TX_IDLE;
        end
    end
    default : next_state  = TX_IDLE;
    endcase
end

always_ff @(posedge tx_clk_in) begin
    if(tx_reset) begin
        tx_state_wr <= TX_IDLE;
    end
    else begin
        tx_state_wr <= next_state;
    end
end

always_comb begin
    if (tx_state_wr != TX_IDLE && !min_frame_ok[7]) begin
        tx0_axis_tkeep_pad = 8'hFF;
    end
    else if (tx_state_wr == TX_PAD && min_frame_ok[7]) begin
        tx0_axis_tkeep_pad = 8'h0F;
    end
    // Covers case where we have seen enough words and need to check tkeep.
    else if (reg_slice_eof && frame_ok_pulse) begin
        tx0_axis_tkeep_pad = {fifo_out_tkeep[7:4], 4'hF};
    end    
    else begin
        tx0_axis_tkeep_pad = fifo_out_tkeep;
    end
end
 
always_comb begin
    if (tx_state_wr == TX_LOAD && min_frame_ok[7]) begin
        tx0_axis_tlast_pad = fifo_out_tlast;
    end
    else if (tx_state_wr == TX_PAD && next_state != TX_PAD) begin
        tx0_axis_tlast_pad = 1'b1;
    end
    else begin
        tx0_axis_tlast_pad = 1'b0;
    end
end

always_comb begin
    if (tx_state_wr != TX_PAD) begin
        casez (fifo_out_tkeep)
        8'b1zzzzzzz : tx0_axis_tdata_pad  = fifo_out_tdata;
        8'b01zzzzzz : tx0_axis_tdata_pad  = {{8{1'b0}},fifo_out_tdata[0+:56]};
        8'b001zzzzz : tx0_axis_tdata_pad  = {{16{1'b0}},fifo_out_tdata[0+:48]};
        8'b0001zzzz : tx0_axis_tdata_pad  = {{24{1'b0}},fifo_out_tdata[0+:40]};
        8'b00001zzz : tx0_axis_tdata_pad  = {{32{1'b0}},fifo_out_tdata[0+:32]};
        8'b000001zz : tx0_axis_tdata_pad  = {{40{1'b0}},fifo_out_tdata[0+:24]};
        8'b0000001z : tx0_axis_tdata_pad  = {{48{1'b0}},fifo_out_tdata[0+:16]};
        8'b00000001 : tx0_axis_tdata_pad  = {{56{1'b0}},fifo_out_tdata[0+:8]};
        default : tx0_axis_tdata_pad  = fifo_out_tdata;
        endcase
    end
    else begin
        tx0_axis_tdata_pad = 64'd0;
    end
end

endmodule
`default_nettype wire
