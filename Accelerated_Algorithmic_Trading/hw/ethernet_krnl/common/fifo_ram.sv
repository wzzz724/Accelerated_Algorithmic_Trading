//--------------------------------------------------------------------------
// Description: BRAM used by tx and rx FIFOs
//--------------------------------------------------------------------------
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
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

`timescale 1ps / 1ps

module fifo_ram#(
    parameter ADDR_WIDTH = 10)
(
    input  wire                          wr_clk,
    input  wire  [(ADDR_WIDTH-1):0]      wr_addr,
    input  wire  [63:0]                  data_in,
    input  wire  [4:0]                   ctrl_in,
    input  wire                          wr_allow,
    input  wire                          rd_clk,
    input  wire                          rd_sreset,
    input  wire  [(ADDR_WIDTH-1):0]      rd_addr,
    output wire  [63:0]                  data_out,
    output wire  [4:0]                   ctrl_out,
    input  wire                          rd_allow
) ;
 
logic   [68:0]            wr_data;
logic   [68:0]            rd_data;

assign wr_data[63:0]      = data_in;
assign wr_data[68:64]     = ctrl_in;

assign data_out           = rd_data[63:0];
assign ctrl_out           = rd_data[68:64];

xpm_memory_sdpram #(
    .ADDR_WIDTH_A(ADDR_WIDTH),      
    .ADDR_WIDTH_B(ADDR_WIDTH),
    .BYTE_WRITE_WIDTH_A(69),
    .CLOCKING_MODE("independent_clock"),
    .ECC_MODE("no_ecc"),
    .MEMORY_INIT_PARAM("0"),
    .MEMORY_OPTIMIZATION("true"),
    .MEMORY_PRIMITIVE("block"),
    .MEMORY_SIZE(70656),
    .READ_DATA_WIDTH_B(69), 
    .READ_LATENCY_B(1),
    .READ_RESET_VALUE_B("0"),
    .WRITE_DATA_WIDTH_A(69)
)
xpm_memory_sdpram_inst (
    .dbiterrb(),
    .doutb(rd_data),
    .sbiterrb(),
    .addra(wr_addr),
    .addrb(rd_addr),
    .clka(wr_clk),
    .clkb(rd_clk),
    .dina(wr_data),
    .ena(wr_allow),
    .enb(rd_allow),
    .injectdbiterra(1'b0),
    .injectsbiterra(1'b0),
    .regceb(1'b1), 
    .rstb(rd_sreset),
    .sleep(1'b0),
    .wea(1'b1)
);   

endmodule