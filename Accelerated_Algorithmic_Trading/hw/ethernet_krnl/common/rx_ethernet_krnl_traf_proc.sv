/* (c) Copyright 2019 Xilinx, Inc. All rights reserved.
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
module rx_ethernet_krnl_axis_traf_proc
(
    // System Signals
    input  wire         ap_clk,

    input  wire         fifo_resetn,
    input  wire         rx_resetn,
    input  wire         areset,
    input  wire         rx_reset,

    input  wire         rx_clk_in,

    // control signals
    input  wire  [47:0] unicast_mac_addr,
    input  wire         unicast_promiscuous_en,
    input  wire         multicast_promiscuous_en,
    input  wire         cutthrough_en,
    input  wire         reset_drop_frame_count,
    output wire   [1:0] rx_overflow,
    output logic [31:0] drop_unicast_count = 32'd0,
    output logic [31:0] drop_multicast_count = 32'd0,
    output logic [31:0] drop_fifo_count = 32'd0,
    output logic [31:0] oversize_frame_count = 32'd0,

    // receive stream passed to upstream kernel
    output wire         rx0_axis_tvalid_from_fsm,
    output wire  [63:0] rx0_axis_tdata,
    output wire         rx0_axis_tlast,
    output wire   [7:0] rx0_axis_tkeep,
    input  wire         rx0_axis_tready,
    output wire         rx0_axis_tuser,

    // receive stream from xxv_ethernet block
    input  wire         axi_str_rxd_tvalid,
    output wire         axi_str_rxd_tready,
    input  wire         axi_str_rxd_tlast,
    input  wire   [3:0] axi_str_rxd_tkeep,
    input  wire  [31:0] axi_str_rxd_tdata,
    input  wire         axi_str_rxd_tuser
);

///////////////////////////////////////////////////////////////////////////////
// Wires and Variables
///////////////////////////////////////////////////////////////////////////////
logic         areset_n = 1'b1; 
logic [3:0]   axi_str_rxd_tuser_int;

assign axi_str_rxd_tuser_int[0] = axi_str_rxd_tuser;
assign axi_str_rxd_tuser_int[3:1] = 0;

logic         conv_rx0_axis_tvalid;
logic         conv_rx0_axis_tready;
logic         conv_rx0_axis_tlast;
logic [7:0]   conv_rx0_axis_tkeep;
logic [63:0]  conv_rx0_axis_tdata;
logic [7:0]   conv_rx0_axis_tuser;

logic         sync_rx0_axis_tvalid;
logic         sync_rx0_axis_tready;
logic         sync_rx0_axis_tlast;
logic [7:0]   sync_rx0_axis_tkeep;
logic [63:0]  sync_rx0_axis_tdata;
logic [7:0]   sync_rx0_axis_tuser;

logic [63:0]  rx0_axis_tdata_i;
logic         rx0_axis_tlast_i;
logic [7:0]   rx0_axis_tkeep_i;
logic         axis_rd_tready_i;
logic         rx0_axis_tuser_i;

// extra logic signals associated with Frame processing.
logic         axis_wr_tvalid;

logic         axis_wr_tlast;
logic [7:0]   axis_wr_tkeep;
logic [63:0]  axis_wr_tdata;
logic         axis_wr_tuser;

logic [47:0]  unicast_mac_addr_mangled;

logic         valid_cmd;
logic         bad_crc;
logic         pass_frame_in;
logic         pass_frame_out;
logic         wr_en;
logic         rd_en;
logic         axis_rd_tready='d0 ;
logic         axis_rd_tvalid_from_fsm=1'b0;
logic         rx0_axis_tvalid;
logic         eof;
logic         async_fifo_tready;

logic         cmd_fifo_full;
logic         cmd_fifo_empty;

logic         force_tlast_to_fifo='d0;

logic         wr_reached_threshold;
logic         fifos_full;

// States for FSM
typedef enum logic[1:0] {IDLE_WR     = 2'b00,
                         WRITE       = 2'b01,
                         DROP_FRAME  = 2'b10} wr_stateCoding_t;
wr_stateCoding_t state_wr = IDLE_WR;
wr_stateCoding_t next_state;

typedef enum logic {IDLE_RD     = 1'b0,
                    BEGIN_READ  = 1'b1} rd_stateCoding_t;
rd_stateCoding_t state_rd = IDLE_RD;
rd_stateCoding_t next_state_rd;

logic         broadcast_detect;
logic         broadcast_detect_r;
logic         da_match;
logic         multicast_detect;
logic         address_chk_en;
logic         force_drop;
logic         force_drop_r;
logic         unicast_drop = 'd0;
logic         multicast_drop = 'd0;
logic         new_cmd = 'd0;
logic [7:0]   frame_size ='d3;
logic         oversize_frame;
logic         sof_pulse;

always_ff @(posedge ap_clk) begin
    areset_n <= !areset;    
end

rx_axis_data_fifo_0 rx0_fifo (
    .s_axis_aresetn(areset_n),
    .s_axis_aclk(ap_clk),
    .s_axis_tvalid(axis_wr_tvalid),
    .s_axis_tready(sync_rx0_axis_tready),
    .s_axis_tdata(axis_wr_tdata),
    .s_axis_tkeep(axis_wr_tkeep),
    .s_axis_tlast(axis_wr_tlast),
    .s_axis_tuser(axis_wr_tuser),
    .m_axis_tvalid(rx0_axis_tvalid),
    .m_axis_tready(axis_rd_tready),
    .m_axis_tdata(rx0_axis_tdata_i),
    .m_axis_tkeep(rx0_axis_tkeep_i),
    .m_axis_tlast(rx0_axis_tlast_i),
    .m_axis_tuser(rx0_axis_tuser_i),
    .almost_full(wr_reached_threshold)
);

axis_register_slice_0 rx0_reg_slice (
  .aclk(ap_clk),                  
  .aresetn(areset_n),             
  .s_axis_tvalid(axis_rd_tvalid_from_fsm), 
  .s_axis_tready(axis_rd_tready_i),
  .s_axis_tdata(rx0_axis_tdata_i),
  .s_axis_tkeep(rx0_axis_tkeep_i),
  .s_axis_tlast(rx0_axis_tlast_i),
  .s_axis_tuser(rx0_axis_tuser_i),
  .m_axis_tvalid(rx0_axis_tvalid_from_fsm),
  .m_axis_tready(rx0_axis_tready),
  .m_axis_tdata(rx0_axis_tdata),
  .m_axis_tkeep(rx0_axis_tkeep),
  .m_axis_tlast(rx0_axis_tlast),
  .m_axis_tuser(rx0_axis_tuser)
);

axis_dwidth_converter_rx rx_32_64_dwidth_converter (
    .aclk(rx_clk_in),
    .aresetn(rx_resetn),
    .s_axis_tvalid(axi_str_rxd_tvalid),
    .s_axis_tready(axi_str_rxd_tready),
    .s_axis_tdata(axi_str_rxd_tdata),
    .s_axis_tkeep(axi_str_rxd_tkeep),
    .s_axis_tlast(axi_str_rxd_tlast),
    .s_axis_tuser(axi_str_rxd_tuser_int),
    .m_axis_tvalid(conv_rx0_axis_tvalid),
    .m_axis_tready(conv_rx0_axis_tready),
    .m_axis_tdata(conv_rx0_axis_tdata),
    .m_axis_tkeep(conv_rx0_axis_tkeep),
    .m_axis_tlast(conv_rx0_axis_tlast),
    .m_axis_tuser(conv_rx0_axis_tuser)
);

axis_data_fifo_1 rx0_async_fifo (
    .s_axis_aresetn(rx_resetn),
    .s_axis_aclk(rx_clk_in),
    .s_axis_tvalid(conv_rx0_axis_tvalid),
    .s_axis_tready(conv_rx0_axis_tready),
    .s_axis_tdata(conv_rx0_axis_tdata),
    .s_axis_tkeep(conv_rx0_axis_tkeep),
    .s_axis_tlast(conv_rx0_axis_tlast),
    .s_axis_tuser(conv_rx0_axis_tuser),
    .m_axis_aclk(ap_clk),
    .m_axis_tvalid(sync_rx0_axis_tvalid),
    .m_axis_tready(async_fifo_tready),
    .m_axis_tdata(sync_rx0_axis_tdata),
    .m_axis_tkeep(sync_rx0_axis_tkeep),
    .m_axis_tlast(sync_rx0_axis_tlast),
    .m_axis_tuser(sync_rx0_axis_tuser)
);

//command FIFO interface for controlling the read side interface
rx_fifo_generator_0 cmd_fifo_inst (
    .clk          (ap_clk),
    .srst         (areset),
    .din          (pass_frame_in),
    .wr_en        (wr_en      ),
    .rd_en        (rd_en      ),
    .dout         (pass_frame_out),
    .full         (cmd_fifo_full),
    .empty        (cmd_fifo_empty),
    .wr_rst_busy  (),
    .rd_rst_busy  ()
);

// Signal indicates the end of a frame
assign eof = (sync_rx0_axis_tlast & sync_rx0_axis_tvalid & async_fifo_tready);

assign broadcast_detect = (sync_rx0_axis_tdata[47:0]== {48{1'b1}});

assign unicast_mac_addr_mangled = {unicast_mac_addr[0+:8], unicast_mac_addr[8+:8], unicast_mac_addr[16+:8], unicast_mac_addr[24+:8], unicast_mac_addr[32+:8], unicast_mac_addr[40+:8]};
assign da_match = (sync_rx0_axis_tdata[47:0] == unicast_mac_addr_mangled);

assign multicast_detect = sync_rx0_axis_tdata[0];

assign fifos_full = cmd_fifo_full | wr_reached_threshold;

// We need to continue to read from the ASYNC FIFO while in DROP FRAME
// This will cover the case where the FIFO is full and de-asserts tready.
assign async_fifo_tready = ((state_wr == DROP_FRAME)) ? 1'b1 : sync_rx0_axis_tready;

// We only write to the data FIFO in WRITE state
// We force tvalid high when we need to force a drop
assign axis_wr_tvalid = (state_wr==DROP_FRAME) ? 1'b0 : (sync_rx0_axis_tvalid | (next_state == DROP_FRAME));

// tlast needs to be forced high when going to DROP_STATE
assign axis_wr_tlast  = (next_state == DROP_FRAME) || sync_rx0_axis_tlast ;
assign axis_wr_tkeep  = sync_rx0_axis_tkeep;
assign axis_wr_tdata  = sync_rx0_axis_tdata;
assign axis_wr_tuser  = (!valid_cmd && (state_wr != IDLE_WR)) || (sync_rx0_axis_tuser[0] && sync_rx0_axis_tkeep[0] || sync_rx0_axis_tuser[4] && sync_rx0_axis_tkeep[4]);

assign rx_overflow = {cmd_fifo_full, force_tlast_to_fifo};

// the force_tlast_to_fifo signal goes high when the data fifo is compltely full and 
// we haven't seen tlast.
// I don't like (next_state != IDLE_WR) but almost full starts off high for some reason in simulations
always_ff @(posedge ap_clk)
begin
    if(force_tlast_to_fifo & eof)
        force_tlast_to_fifo <= 1'b0;
    else if((wr_reached_threshold && (next_state != IDLE_WR) || oversize_frame) && (!(sync_rx0_axis_tlast & sync_rx0_axis_tvalid)))
        force_tlast_to_fifo <= 1'b1;
end

// Calculate size of frame. frame size starts at 3 counts up to 192. The 2
// MSBs are used to flag an oversized frame. 
always_ff @(posedge ap_clk)
begin
    if(eof)
        frame_size <= 'd3;
    else if(sync_rx0_axis_tvalid & sync_rx0_axis_tready & !(&frame_size[7:6]))
        frame_size <= frame_size + 1;
end

// We only want to use the sof if cut-through is enabled.
always_ff @(posedge ap_clk) begin
    sof_pulse <= (state_wr == IDLE_WR) && (next_state == WRITE);
end

// Using axis_wr_tvalid will ensure that it ony triggers once
assign oversize_frame = &{frame_size[7:6],sync_rx0_axis_tkeep[6], axis_wr_tvalid};

always_ff @(posedge ap_clk)
begin
    if(address_chk_en) begin
        broadcast_detect_r <= broadcast_detect;
    end
    else if(eof) begin
        broadcast_detect_r <= '0;
    end
end

// When unicast_promiscuous_en set all unicast traffic will be passed
// When disabled, only unicast traffic matching the configured Ethernet MAC 
// address will pass. Multicast frames are ignored.
// Signal is cleared at the the end of the frame
always_ff @(posedge ap_clk)
begin
    if((unicast_drop & eof) || unicast_promiscuous_en)
        unicast_drop <= 1'b0;
    else if(address_chk_en && ~multicast_detect)
        unicast_drop <= ~da_match;
end

// When set all multicast traffic will be passed
// When disabled, no multicast traffic is passed
// Unicast traffic is ignored.
// Signal is cleared at the the end of the frame
always_ff @(posedge ap_clk)
begin
    if((multicast_drop & eof) || multicast_promiscuous_en)
        multicast_drop <= 1'b0;
    else if(address_chk_en && multicast_detect)
        multicast_drop <= 1'b1;
end

assign force_drop = (unicast_drop || multicast_drop) & ~broadcast_detect_r;

always_ff @(posedge ap_clk)
    force_drop_r <= force_drop;

// The new_cmd signal will notify the cmd FIFO that a new frame has
// started going into the data fifo. This will ensure we only
// write a new command if data is written to the data fifo
always_ff @(posedge ap_clk)
begin
    if(eof)
        new_cmd <= 1'b0;
    else if(axis_wr_tvalid)
        new_cmd <= 1'b1;
end

//Two FIFOs are implemented: one for Ethernet data(data FIFO) and the other for controlling
//read side command(command FIFO).
//Write FSM: 4 states control the entire write operation
//cmd_in is an input to the command FIFO and controls the read side command
//FSM states:
//IDLE_WR   : Wait in this state until valid is received from xxv_Ethernet_0. If the
//            data FIFO is full or tready is de-asserted from FIFO interface upstream
//            it drops the current frame from xxv_Ethernet_0
//WRITE     : The FSM continues to write data into data FIFO until tlast from xxv_ethernet_0 is hit.
//            FSM transitions to CHECK_ERROR state if tlast has arrived
//DROP_FRAME: The FSM enters into this state if the data FIFO is full or tready from data FIFO is de-asserted
//            In this state, tvalid to FIFO is de-asserted
//            The state machine should wait until the FIFO has drained back to zero ?
always_comb
begin
    case(state_wr)
    IDLE_WR: begin
        if(sync_rx0_axis_tvalid & fifos_full) begin
            next_state  =  DROP_FRAME;
        end
        else if(sync_rx0_axis_tvalid) begin
            next_state  =  WRITE;
        end
        else begin
            next_state  =  IDLE_WR;
        end
    end
    WRITE : begin
        if(force_tlast_to_fifo || force_drop_r) begin
            next_state = DROP_FRAME;
        end
        else if(eof) begin
            next_state = IDLE_WR;
        end
        else begin
            next_state = WRITE;
        end
    end
    DROP_FRAME : begin
        if(eof) begin
            next_state = IDLE_WR;
        end
        else
            next_state = DROP_FRAME;
    end
    default :   next_state  = IDLE_WR;
    endcase
end

always_ff @(posedge ap_clk) begin
    if(areset) begin
        state_wr  <= IDLE_WR;
    end
    else begin
        state_wr <= next_state;
    end
end

// Only write into the cmd fifo at the end of the frame if cut-through not enabled 
always_comb begin
    if (cutthrough_en)
        wr_en = sof_pulse;
    else
        wr_en = eof && new_cmd;
end

always_comb begin
    // Addresses need to be checked on the first packet of the frame.
    address_chk_en = (next_state == WRITE && state_wr == IDLE_WR);
    // We consider a frame to be valid as long as we remain in the WRITE state
    // We also need to cover the case where we have an oversize frame and we see eof
    valid_cmd   = (state_wr == WRITE) && !(oversize_frame & eof) && ((next_state != DROP_FRAME) && !force_drop);
    bad_crc     = sync_rx0_axis_tlast & (sync_rx0_axis_tuser[0] && sync_rx0_axis_tkeep[0] || sync_rx0_axis_tuser[4] && sync_rx0_axis_tkeep[4]);
end

// Frame is valid when both CRC and valid cmd are high
assign pass_frame_in = valid_cmd & ~bad_crc;

always_ff @(posedge ap_clk)
begin
    if(reset_drop_frame_count) begin
        drop_unicast_count   <= 0;
        drop_multicast_count <= 0;
        drop_fifo_count      <= 0;
    end
    else begin
        if (next_state == DROP_FRAME && state_wr != DROP_FRAME) begin
            if (unicast_drop) begin
                drop_unicast_count <= drop_unicast_count + 1;
            end
            if  (multicast_drop) begin
                drop_multicast_count <= drop_multicast_count + 1;
            end
            if (wr_reached_threshold) begin
                drop_fifo_count <= drop_fifo_count + 1;
            end
        end
    end
end

always_ff @(posedge ap_clk)
begin
    if(reset_drop_frame_count) begin
        oversize_frame_count <= 0;
    end
    else begin
        if (oversize_frame && next_state != DROP_FRAME) begin
            oversize_frame_count <= oversize_frame_count + 1;
        end
    end
end

//Read FSM reads out the data from data FIFO and present it to the packet FIFO interface
//The read FSM starts reading data from the data FIFO as soon as it decodes a valid command
//from the command FIFO. Various state transitions are basically controlled by the command FIFO
//empty flag and tready assertion from packet FIFO interface
//FSM states
//IDLE_RD: The FSM stays in this state until command FIFO empty is de-asserted and tready from packet
//         FIFO interface is active low.
//BEGIN_READ: In this state, the FSM reads data until tlast is read from the fifo

always_comb
begin
    case(state_rd)
        IDLE_RD : begin
            if(!cmd_fifo_empty) begin
                next_state_rd   = BEGIN_READ;
            end
            else begin
                next_state_rd   = IDLE_RD;
            end
        end
        BEGIN_READ : begin
            //Continue reading data until tlast from data fifo is seen
            if((rx0_axis_tlast_i & rx0_axis_tvalid & axis_rd_tready)) begin
                next_state_rd   = IDLE_RD;
            end
            else begin
                next_state_rd   = BEGIN_READ;
            end
        end
        default:   next_state_rd             = IDLE_RD;
    endcase
end

always_ff @(posedge ap_clk) begin
    if(areset) begin
        state_rd  <= IDLE_RD;
    end
    else begin
        state_rd <= next_state_rd;
    end
end

always_comb
begin
    rd_en = (next_state_rd == BEGIN_READ) && (state_rd  == IDLE_RD);
end

always_comb
begin
    if(state_rd==BEGIN_READ) begin
        if (pass_frame_out) begin
            axis_rd_tready          = axis_rd_tready_i ;
            axis_rd_tvalid_from_fsm = rx0_axis_tvalid;
        end
        else begin
            axis_rd_tready          = 1'b1;
            axis_rd_tvalid_from_fsm = 1'b0;
        end
    end
    else begin
        axis_rd_tready          = 1'b0;
        axis_rd_tvalid_from_fsm = 1'b0;
    end
end

endmodule
`default_nettype wire
