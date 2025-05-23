// (c) Copyright 2001-2020 Xilinx, Inc. All rights reserved.
//
// This file contains confidential and proprietary information
// of Xilinx, Inc. and is protected under U.S. and
// international copyright and other intellectual property
// laws.
//
// DISCLAIMER
// This disclaimer is not a license and does not grant any
// rights to the materials distributed herewith. Except as
// otherwise provided in a valid license issued to you by
// Xilinx, and to the maximum extent permitted by applicable
// law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
// WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
// AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
// BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
// INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
// (2) Xilinx shall not be liable (whether in contract or tort,
// including negligence, or under any other theory of
// liability) for any loss or damage of any kind or nature
// related to, arising under or in connection with these
// materials, including for any direct, or any indirect,
// special, incidental, or consequential loss or damage
// (including loss of data, profits, goodwill, or any type of
// loss or damage suffered as a result of any action brought
// by a third party) even if such damage or loss was
// reasonably foreseeable or Xilinx had been advised of the
// possibility of the same.
//
// CRITICAL APPLICATIONS
// Xilinx products are not designed or intended to be fail-
// safe, or for use in any application requiring fail-safe
// performance, such as life-support or safety devices or
// systems, Class III medical devices, nuclear facilities,
// applications related to the deployment of airbags, or any
// other applications that could lead to death, personal
// injury, or severe property or environmental damage
// (individually and collectively, "Critical
// Applications"). Customer assumes the sole risk and
// liability of any use of Xilinx products in Critical
// Applications, subject only to applicable laws and
// regulations governing limitations on product liability.
//
// THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
// PART OF THIS FILE AT ALL TIMES.
//
//
//------------------------------------------------------------------------------
// Description: This is the AXI-Streaming fifo for the client loopback design
//              example of the 10 Gigabit Ethernet MAC core
//
//              The FIFO is created from Block RAMs and can be chosen to of
//              size (in 8 bytes words) 512, 1024, 2048, 4096, 8192, or 2048.
//
//              Frame data received from the write side is written into the
//              data field of the BRAM on the wr_axis_aclk. Start of Frame ,
//              End of Frame and a binary encoded strobe signal (indicating the
//              number of valid bytes in the last word of the frame) are
//              created and stored in the parity field of the BRAM
//
//              The wr_axis_tlast and wr_axis_tuser signals are used to qualify
//              the frame.  A frame for which wr_axis_tuser was not asserted
//              when wr_axis_tlast was asserted will cause the FIFO write
//              address pointer to be reset to the base address of that frame.
//              In this way the bad frame will be overwritten with the next
//              received frame and is therefore dropped from the FIFO.
//
//              When there is at least one complete frame in the FIFO,
//              the read interface will be enabled allowing data to be read
//              from the fifo.
//
//-----------------------------------------------------------------------------

`timescale 1ps / 1ps

module tx_fifo #(
    parameter                   FIFO_SIZE = 1024
) (
    // FIFO write domain
    input                       wr_axis_aresetn,
    input                       wr_axis_aclk,
    input       [63:0]          wr_axis_tdata,
    input       [7:0]           wr_axis_tkeep,
    input                       wr_axis_tvalid,
    input                       wr_axis_tlast,
    output                      wr_axis_tready,
    input                       wr_axis_tuser,

    input       [7:0]           threshold,

    // FIFO read domain
    input                       rd_axis_aresetn,
    input                       rd_axis_aclk,
    output      [63:0]          rd_axis_tdata,
    output logic [7:0]          rd_axis_tkeep,
    output logic                rd_axis_tvalid,
    output                      rd_axis_tlast,
    output logic                rd_axis_tuser,
    input                       rd_axis_tready,

    // FIFO Status Signals
    output logic [3:0]          fifo_status,
    output                      fifo_full,
    output logic [31:0]         underflow_count
);

// the address width required is a function of FIFO size
localparam ADDR_WIDTH = $clog2(FIFO_SIZE);

// write clock domain
logic       [ADDR_WIDTH-1:0]        wr_addr = '0;        // current write address

logic       [ADDR_WIDTH-1:0]        wr_rd_addr;          // rd_addr in wr domain
logic                               wr_enable;           // write enable
logic                               wr_enable_ram;
logic                               wr_fifo_full;        // fifo full
logic       [4:0]                   wr_ctrl;             // Underflow, EOF and Remainder information for the frame: stored in the parity bits of BRAM.
logic                               wr_store_frame;      // decision to keep the previous received frame
logic                               wr_store_frame_tog = 0;  // toggle everytime a frame is kept: this crosses onto the read clock domain
logic       [2:0]                   wr_rem = 3'b000;         // Number of bytes valiod in last word of frame encoded as a binary remainder
logic                               wr_eof;              // asserted with the last word of the frame
logic                               wr_underflow;
logic                               wr_underflow_r;
logic                               wr_underflow_pulse;
logic       [7:0]                   word_cnt = 8'h00;
logic       [7:0]                   word_cnt_next;
logic                               threshold_ok;
logic       [7:0]                   frame_threshold = '0;
logic                               sof_pulse;
logic                               sof_pulse_toggle=0;

// read clock domain
logic       [ADDR_WIDTH-1:0]        rd_addr = '0;        // current read address
logic       [ADDR_WIDTH-1:0]        rd_addr_last = '0;   // store last address for frame underflow
logic       [ADDR_WIDTH-1:0]        rd_addr_reg;
logic       [ADDR_WIDTH-1:0]        rd_wr_addr;          // rd/wr addr diff in read clk domain
logic       [ADDR_WIDTH-2:0]        rd_frames ='0;       // A count of the number of frames currently stored in the FIFO

logic                               rd_store_eof_sync; // register wr_store_frame_tog a 2nd time
logic                               rd_store_eof_sync_del = 0; // register wr_store_frame_tog a 2nd time
logic                               rd_store_frame;      // edge detector for wr_store_frame_tog
logic                               rd_enable_ram;       // read enable
logic       [63:0]                  rd_data;             // data word output from BRAM
logic       [4:0]                   rd_ctrl;             // data control output from BRAM parity (contains underflow, EOF and Remainder information for the frame)
logic                               rd_sof_pulse;
logic                               rd_sof_pulse_toggle;
logic                               rd_sof_pulse_toggle_r;
logic                               rd_eof;
logic                               rd_underflow;
logic                               ctrl_empty;
logic                               rd_en;

logic       [ADDR_WIDTH-1:0]        wr_addr_diff;           // the difference between read and write address
logic       [ADDR_WIDTH-1:0]        wr_addr_diff_2s_comp;   // 2s complement of read/write diff

// sync rd avail into write domain
logic                               nearly_full = 1'b0;
logic                               empty_flag;

// STATES FOR TX read FSM
typedef enum logic[1:0] {TX_IDLE      = 2'b00,
                         TX_READ      = 2'b01,
                         TX_LAST_WORD = 2'b10,
                         TX_REWIND    = 2'b11} rd_stateCoding_t;
rd_stateCoding_t read_state = TX_IDLE;
rd_stateCoding_t next_state;

// States for TX write FSM
typedef enum logic[1:0] {TX_WR_IDLE   = 2'b00,
                         TX_WR        = 2'b01,
                         TX_THRESHOLD = 2'b10} wr_stateCoding_t;
wr_stateCoding_t wr_state = TX_WR_IDLE;
wr_stateCoding_t next_wr_state;

//--------------------------------------------------------------------
// FIFO for storing SOF signals needed in write clk domain
//--------------------------------------------------------------------
tx_sof_fifo tx_sof_fifo_i (
  .clk(rd_axis_aclk),         // input wire clk
  .srst(!rd_axis_aresetn),    // input wire srst
  .din(1'b1),                 // input wire [0 : 0] din
  .wr_en(rd_sof_pulse),       // input wire wr_en
  .rd_en(rd_en),              // input wire rd_en
  .dout(),                    // output wire [0 : 0] dout
  .full(),                    // output wire full
  .empty(ctrl_empty),         // output wire empty
  .wr_rst_busy(),             // output wire wr_rst_busy
  .rd_rst_busy()              // output wire rd_rst_busy
);

//--------------------------------------------------------------------
// FIFO Read domain
//--------------------------------------------------------------------

// Edge detector to register that a new frame was written into the
// FIFO.
// NOTE: wr_store_frame_tog crosses clock domains from FIFO write
xpm_cdc_single #(
   .DEST_SYNC_FF(4),
   .INIT_SYNC_FF(0),
   .SRC_INPUT_REG(0)
) rd_store_sync (
   .dest_out(rd_store_eof_sync),
   .dest_clk(rd_axis_aclk),
   .src_clk(1'b0),
   .src_in(wr_store_frame_tog)
);

HARD_SYNC #(
   .INIT(1'b0),            // Initial values, 1'b0, 1'b1
   .IS_CLK_INVERTED(1'b0), // Programmable inversion on CLK input
   .LATENCY(2)             // 2-3
)
sof_pulse_toggle_sync (
   .DOUT(rd_sof_pulse_toggle), // 1-bit output: Data
   .CLK(rd_axis_aclk),         // 1-bit input: Clock
   .DIN(sof_pulse_toggle)      // 1-bit input: Data
);

always_ff @(posedge rd_axis_aclk) begin
    rd_sof_pulse_toggle_r    <= rd_sof_pulse_toggle;
end

assign rd_sof_pulse = rd_sof_pulse_toggle ^ rd_sof_pulse_toggle_r;

always_ff @(posedge rd_axis_aclk) begin
    rd_store_eof_sync_del    <= rd_store_eof_sync;
end

always_ff @(posedge rd_axis_aclk) begin
    if (rd_store_eof_sync ^ rd_store_eof_sync_del) begin
        rd_store_frame     <= 1'b1;
    end
    else begin
        rd_store_frame     <= 1'b0;
    end    
end

// Up/Down counter to monitor the number of frames stored within the
// the FIFO. A frame is removed when rd_eof is seen
always_ff @(posedge rd_axis_aclk) begin
    if (!rd_axis_aresetn) begin
        rd_frames            <= 0;
    end
    else begin
        // A frame has been written into the FIFO
        if (rd_store_frame == 1'b1 && (rd_eof && rd_axis_tready)) begin
            // do nothing one in one out
            rd_frames     <= rd_frames;
        end
        else if (rd_store_frame == 1'b1) begin
            if (&rd_frames != 1'b1) begin  // if we max out error!
                rd_frames     <= rd_frames + 1;
            end
        end
        else if (rd_eof && rd_axis_tready) begin  // eof is seen
            if (rd_frames > 0) begin // if we bottom out error!
                rd_frames     <= rd_frames - 1;
            end
        end
    end
end

// Record the starting address of a new frame in case of an overflow
// at the start of each frame
always_ff @(posedge rd_axis_aclk) begin
    if ( ((next_state == TX_READ) && (read_state == TX_IDLE)) || ((read_state == TX_LAST_WORD) && (next_state == TX_READ)) ) begin
        rd_addr_last       <= rd_addr;
    end
end

assign rd_eof = rd_ctrl[3];

// Remove SoF from FIFO with a single pulse when EoF is seen
assign rd_en = rd_eof && (read_state==TX_READ);

always_comb begin
    // Avoid any possible latches
    next_state = read_state;
    case(read_state)
    TX_IDLE: begin
        if (rd_sof_pulse) begin
            next_state = TX_READ;
        end
        else begin
            next_state = TX_IDLE;
        end
    end

    TX_READ: begin
        if (rd_eof) begin
            next_state = TX_LAST_WORD;
        end    
        else if (rd_underflow & rd_axis_tready) begin
            next_state = TX_REWIND;
        end
        else begin
            next_state = TX_READ;
        end
    end    

    TX_LAST_WORD: begin
        if (!ctrl_empty) begin
            next_state = TX_READ;    
        end
        else begin
            next_state = TX_IDLE;
        end
    end    

    TX_REWIND: begin
        if (rd_frames > 0) begin
            next_state = TX_READ;
        end
        else begin
            next_state   = TX_REWIND;
        end
    end

    default : next_state  = TX_IDLE;
  endcase
end

always_ff @(posedge rd_axis_aclk) begin
    if(!rd_axis_aresetn) begin
        read_state <= TX_IDLE;
    end
    else begin
        read_state <= next_state;
    end
end

// Create the Read Address Pointer
always_ff @(posedge rd_axis_aclk) begin
    // When an underflow is seen rewind the addr
    if (next_state == TX_REWIND) begin
        rd_addr           <= rd_addr_last;
    end
    else if (read_state == TX_READ && rd_axis_tready) begin
        rd_addr           <= rd_addr + 1;
    end
end

assign rd_enable_ram = (next_state == TX_READ);

// Decode the data valid signals as a binary remainder:
// wr_axis_tkeep   wr_rem
// -----------//   ------
// 0x00000001      000
// 0x00000011      001
// 0x00000111      010
// 0x00001111      011
// 0x00011111      100
// 0x00111111      101
// 0x01111111      110
// 0x11111111      111
always_comb begin
    case (rd_ctrl[2:0])
    3'b000 :
        rd_axis_tkeep   = 8'b00000001;
    3'b001 :
        rd_axis_tkeep   = 8'b00000011;
    3'b010 :
        rd_axis_tkeep   = 8'b00000111;
    3'b011 :
        rd_axis_tkeep   = 8'b00001111;
    3'b100 :
        rd_axis_tkeep   = 8'b00011111;
    3'b101 :
        rd_axis_tkeep   = 8'b00111111;
    3'b110 :
        rd_axis_tkeep   = 8'b01111111;
    3'b111 :
        rd_axis_tkeep   = 8'b11111111;
    default:
        rd_axis_tkeep   = 8'b00000000;
   endcase
end

// tlast will be asserted when an underflow is seen
assign rd_axis_tlast  = (rd_eof) || next_state == TX_REWIND;
assign rd_axis_tdata  = rd_data[63:0];
// Assert tuser when frame is invalid due to underflow
assign rd_axis_tuser  = (next_state == TX_REWIND);

// assign rd_axis_tvalid = (next_state == TX_READ) || (read_state == TX_READ) || (read_state == TX_LAST_WORD);
assign rd_axis_tvalid = (read_state == TX_READ);

// register the Read Address Pointer gray code
always_ff @(posedge rd_axis_aclk) begin
    rd_addr_reg  <= rd_addr;
end

xpm_cdc_gray #(
    .DEST_SYNC_FF(4),
    .INIT_SYNC_FF(0),
    .REG_OUTPUT(0),
    .SIM_ASSERT_CHK(0),
    .SIM_LOSSLESS_GRAY_CHK(0),
    .WIDTH(ADDR_WIDTH)
)
sync_addr_diff_gray (
    .dest_out_bin (rd_wr_addr),
    .dest_clk     (rd_axis_aclk),
    .src_clk      (wr_axis_aclk),
    .src_in_bin   (wr_addr)
);

// Assign underflow wire
assign rd_underflow = rd_ctrl[4];

// Flag to indicate that the FIFO is empty
assign empty_flag = (rd_addr == rd_wr_addr);

//--------------------------------------------------------------------
// FIFO Write Domain
//--------------------------------------------------------------------

// Resync the Read Address Pointer grey code onto the write clock
xpm_cdc_gray #(
    .DEST_SYNC_FF(4),
    .INIT_SYNC_FF(0),
    .REG_OUTPUT(0),
    .SIM_ASSERT_CHK(0),
    .SIM_LOSSLESS_GRAY_CHK(0),
    .WIDTH(ADDR_WIDTH)
)
sync_gray_addr (
    .dest_out_bin(wr_rd_addr),
    .dest_clk(wr_axis_aclk),
    .src_clk(rd_axis_aclk),
    .src_in_bin(rd_addr_reg)
);

// Obtain the difference between write and read pointers
always_ff @(posedge wr_axis_aclk) begin
    wr_addr_diff     <= wr_rd_addr - wr_addr;
end

always_ff @(posedge wr_axis_aclk) begin
    if ( (wr_axis_tlast== 1'b1 && wr_fifo_full == 1'b1) ||
       (wr_axis_tlast== 1'b1 && wr_axis_tuser==1'b1)) begin
        nearly_full      <= 0;
    end
    // if within 16 of full then start checking lower bits
    else if (wr_addr_diff[ADDR_WIDTH-1:4] == 0) begin
        if (wr_addr_diff[3]) begin
            nearly_full  <= 1;
        end
    end
    else begin
        nearly_full      <= 0;
    end
end

// Detect when the FIFO is full
assign wr_fifo_full = nearly_full;

always_ff @(posedge wr_axis_aclk)
begin
    if(!wr_axis_aresetn) begin
        underflow_count   <= 0;
    end
    else begin
        if (wr_underflow_pulse) begin
            underflow_count <= underflow_count + 1;
        end
    end
end

//--------------------------------------------------------------------
// Create FIFO Status Signals in the Read Domain
//--------------------------------------------------------------------

// The FIFO status signal is four bits which represents the occupancy
// of the FIFO in 16'ths.  To generate this signal take the 2's
// complement of the difference between the read and write address
// pointers and take the top 4 bits.

assign wr_addr_diff_2s_comp         = (~(wr_addr_diff) + 1);

// Register the top 4 bits to create the fifo status
always_ff @(posedge wr_axis_aclk) begin
    fifo_status   <= wr_addr_diff_2s_comp[ADDR_WIDTH-1:ADDR_WIDTH-4];
end

// if nearly_full and frames are available then deassert tready
assign fifo_full         = wr_fifo_full;
assign wr_axis_tready    = !wr_fifo_full;

// Create the Write Address Pointer
always_ff @(posedge wr_axis_aclk) begin
    // increment write pointer as frame is written.
    if (wr_enable) begin
        wr_addr  <= wr_addr + 1;
    end
end

// ---- Underflow logic ----
// We flag an underflow if Cut-through is enabled and tvalid goes low 
// mid-packet.
assign wr_underflow = (next_wr_state == TX_THRESHOLD & !wr_axis_tvalid);

// Create an underflow pusle to be used as a write enable to the RAM
always_ff @(posedge wr_axis_aclk) begin
    wr_underflow_r <= wr_underflow;
end

assign wr_underflow_pulse = (wr_underflow && !wr_underflow_r);

// Write Enable signal
assign wr_enable  = (wr_axis_tvalid && !wr_fifo_full);

// Allow a bad frame to be written into the RAM so read side can see underflow
// Note that when we write due to wr_underflow_pulse we do NOT increment the 
// write address
assign wr_enable_ram  = wr_enable || wr_underflow_pulse;

// Indicate that the entire frame has been written into the memory
assign wr_store_frame = (wr_axis_tlast && wr_axis_tvalid && !wr_fifo_full);

// count the number of words in the frame
always_ff @(posedge wr_axis_aclk) begin
    if (wr_store_frame) begin
        word_cnt <= 8'h00;
    end
    else if (wr_enable) begin
        word_cnt <= word_cnt_next;
    end
end

assign word_cnt_next = word_cnt + 'd1;

// Write state machine is used to trigger start of frame
// to read side. Threshold value is updated only in IDLE
// to ensure this doesn't change mid frame. SOF triggers 
// when leaving TX_WR
always_comb begin
    next_wr_state = wr_state;
    case(wr_state)
    TX_WR_IDLE: begin
        if (wr_axis_tvalid) begin
            if (threshold_ok) begin
                // This will cover the case where threshold=1
                next_wr_state = TX_THRESHOLD;
            end
            else begin
                next_wr_state = TX_WR;
            end            
        end
        else begin
            next_wr_state = TX_WR_IDLE;
        end

    end

    TX_WR: begin
        // EOF takes priority if threshold = frame size
        if (wr_store_frame) begin
            next_wr_state = TX_WR_IDLE;
        end
        else if (threshold_ok) begin
            next_wr_state = TX_THRESHOLD;
        end
        else begin
            next_wr_state = TX_WR;
        end 
    end

    TX_THRESHOLD: begin
        if (wr_store_frame) begin
            next_wr_state = TX_WR_IDLE;
        end
        else begin
            next_wr_state = TX_THRESHOLD;
        end
    end

    default : next_wr_state  = TX_WR_IDLE;
  endcase
end

always_ff @(posedge wr_axis_aclk) begin
    if(!wr_axis_aresetn) begin
        wr_state <= TX_WR_IDLE;
    end
    else begin
        wr_state <= next_wr_state;
    end
end

// Only update the threshold value at the start of a new frame
always_ff @(posedge wr_axis_aclk) begin
    if (wr_state == TX_WR_IDLE) begin
        frame_threshold <= threshold;
    end
end

// Signal indicates that the threshold has been met. Need to cover case where threshold = 1 with wr_enable
assign threshold_ok = ((word_cnt_next == frame_threshold && wr_enable) || (word_cnt_next > frame_threshold)) && frame_threshold != 0;

// Pulse will indicate SOF
// Cover special case where threshold=1
assign sof_pulse = ( ((wr_state == TX_WR) && ((next_wr_state == TX_WR_IDLE) || (next_wr_state == TX_THRESHOLD))) || ((wr_state == TX_WR_IDLE) && (next_wr_state == TX_THRESHOLD)) );

// This signal notifies that a full frame has been writen into the RAM.
always_ff @(posedge wr_axis_aclk) begin
    if (sof_pulse) begin
        sof_pulse_toggle            <= !sof_pulse_toggle;
    end
end

// This signal notifies that a full frame has been writen into the RAM.
always_ff @(posedge wr_axis_aclk) begin
    if (wr_store_frame) begin
        wr_store_frame_tog            <= !wr_store_frame_tog;
    end
end

always_comb begin
    wr_rem[2]                = wr_axis_tkeep[4];
        
    case (wr_axis_tkeep)
    8'b00000001, 8'b00011111 :
        wr_rem[1:0]          = 2'b00;
    8'b00000011, 8'b00111111 :
        wr_rem[1:0]          = 2'b01;
    8'b00000111, 8'b01111111 :
        wr_rem[1:0]          = 2'b10;
    default:
        wr_rem[1:0]          = 2'b11;
    endcase
end

assign wr_eof = wr_axis_tlast & wr_axis_tvalid;

// This signal, stored in the parity bits of the BRAM, contains
// EOF and Remainder information for the stored frame:
// wr_ctrl[4]    = underflow
// wr_ctrl[3]    = EOF
// wr_ctrl([2:0] = remainder
// Note that remainder is only valid when EOF is asserted.

assign wr_ctrl  =  {wr_underflow,wr_eof,wr_rem};

//--------------------------------------------------------------------
// Instantiate BRAMs to produce the dual port memory
//--------------------------------------------------------------------
fifo_ram #(ADDR_WIDTH) fifo_ram_inst (
    .wr_clk        (wr_axis_aclk),
    .wr_addr       (wr_addr),
    .data_in       (wr_axis_tdata),
    .ctrl_in       (wr_ctrl),
    .wr_allow      (wr_enable_ram),
    .rd_clk        (rd_axis_aclk),
    .rd_sreset     (!rd_axis_aresetn),
    .rd_addr       (rd_addr),
    .data_out      (rd_data),
    .ctrl_out      (rd_ctrl),
    .rd_allow      (rd_enable_ram)
);

endmodule