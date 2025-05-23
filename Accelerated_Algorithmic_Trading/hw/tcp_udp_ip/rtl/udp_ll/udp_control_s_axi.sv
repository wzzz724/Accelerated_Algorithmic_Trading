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

module udp_control_s_axi
#(parameter
    C_S_AXI_ADDR_WIDTH = 14,
    C_S_AXI_DATA_WIDTH = 32
)(
    input  wire                          ACLK,
    input  wire                          ARESET,
    input  wire                          ACLK_EN,
    input  wire [C_S_AXI_ADDR_WIDTH-1:0] AWADDR,
    input  wire                          AWVALID,
    output wire                          AWREADY,
    input  wire [C_S_AXI_DATA_WIDTH-1:0] WDATA,
    input  wire [C_S_AXI_DATA_WIDTH/8-1:0] WSTRB,
    input  wire                          WVALID,
    output wire                          WREADY,
    output wire [1:0]                    BRESP,
    output wire                          BVALID,
    input  wire                          BREADY,
    input  wire [C_S_AXI_ADDR_WIDTH-1:0] ARADDR,
    input  wire                          ARVALID,
    output wire                          ARREADY,
    output wire [C_S_AXI_DATA_WIDTH-1:0] RDATA,
    output wire [1:0]                    RRESP,
    output wire                          RVALID,
    input  wire                          RREADY,
    input  wire [31:0]                   datagrams_transmitted,
    input  wire [31:0]                   datagrams_recv,
    input  wire [31:0]                   datagrams_recv_invalid_port,
    input  wire [12:0]                   arrPorts_address0,
    input  wire                          arrPorts_ce0,
    output wire                          arrPorts_vld,
    output wire [7:0]                    arrPorts_q0
);
//------------------------Address Info-------------------
// 0x0000 : reserved
// 0x0004 : reserved
// 0x0008 : reserved
// 0x000c : reserved
// 0x0010 : Data signal of datagrams_transmitted
//          bit 31~0 - datagrams_transmitted[31:0] (Read)
// 0x0014 : Control signal of datagrams_transmitted
//          bit 0  - datagrams_transmitted_ap_vld (Read/COR)
//          others - reserved
// 0x0020 : Data signal of datagrams_recv
//          bit 31~0 - datagrams_recv[31:0] (Read)
// 0x0024 : Control signal of datagrams_recv
//          bit 0  - datagrams_recv_ap_vld (Read/COR)
//          others - reserved
// 0x0030 : Data signal of datagrams_recv_invalid_port
//          bit 31~0 - datagrams_recv_invalid_port[31:0] (Read)
// 0x0034 : Control signal of datagrams_recv_invalid_port
//          bit 0  - datagrams_recv_invalid_port_ap_vld (Read/COR)
//          others - reserved
// 0x2000 ~
// 0x3fff : Memory 'arrPorts' (8192 * 8b)
//          Word n : bit [ 7: 0] - arrPorts[4n]
//                   bit [15: 8] - arrPorts[4n+1]
//                   bit [23:16] - arrPorts[4n+2]
//                   bit [31:24] - arrPorts[4n+3]
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

//------------------------Parameter----------------------
localparam
    ADDR_DATAGRAMS_TRANSMITTED_DATA_0       = 14'h0010,
    ADDR_DATAGRAMS_TRANSMITTED_CTRL         = 14'h0014,
    ADDR_DATAGRAMS_RECV_DATA_0              = 14'h0020,
    ADDR_DATAGRAMS_RECV_CTRL                = 14'h0024,
    ADDR_DATAGRAMS_RECV_INVALID_PORT_DATA_0 = 14'h0030,
    ADDR_DATAGRAMS_RECV_INVALID_PORT_CTRL   = 14'h0034,
    ADDR_ARRPORTS_BASE                      = 14'h2000,
    ADDR_ARRPORTS_HIGH                      = 14'h3fff,
    WRIDLE                                  = 2'd0,
    WRDATA                                  = 2'd1,
    WRRESP                                  = 2'd2,
    WRRESET                                 = 2'd3,
    RDIDLE                                  = 2'd0,
    RDDATA                                  = 2'd1,
    RDRESET                                 = 2'd2,
    ADDR_BITS                = 14;

//------------------------Local signal-------------------
    logic [1:0]                    wstate = WRRESET;
    logic [1:0]                    wnext;
    logic [ADDR_BITS-1:0]          waddr;
    logic [C_S_AXI_DATA_WIDTH-1:0] wmask;
    logic                          aw_hs;
    logic                          w_hs;
    logic [1:0]                    rstate = RDRESET;
    logic [1:0]                    rnext;
    logic [C_S_AXI_DATA_WIDTH-1:0] rdata;
    logic                          ar_hs;
    logic [ADDR_BITS-1:0]          raddr;
    // internal registers
    logic  [31:0]                  int_datagrams_transmitted = 'b0;
    logic  [31:0]                  int_datagrams_recv = 'b0;
    logic  [31:0]                  int_datagrams_recv_invalid_port = 'b0;
    // memory signals
    logic [10:0]                   int_arrPorts_address0;
    logic                          int_arrPorts_ce0;
    logic                          int_arrPorts_we0;
    logic [3:0]                    int_arrPorts_be0;
    logic [31:0]                   int_arrPorts_d0;
    logic [31:0]                   int_arrPorts_q0;
    logic [10:0]                   int_arrPorts_address1;
    logic                          int_arrPorts_ce1;
    logic                          int_arrPorts_we1;
    logic [3:0]                    int_arrPorts_be1;
    logic [31:0]                   int_arrPorts_d1;
    logic [31:0]                   int_arrPorts_q1;
    logic                          int_arrPorts_read;
    logic                          int_arrPorts_write;
    logic [1:0]                    int_arrPorts_shift;

//------------------------Instantiation------------------
// int_arrPorts
udp_control_s_axi_ram #(
    .BYTES    ( 4 ),
    .DEPTH    ( 2048 )
) int_arrPorts (
    .clk0     ( ACLK ),
    .address0 ( int_arrPorts_address0 ),
    .ce0      ( int_arrPorts_ce0 ),
    .we0      ( int_arrPorts_we0 ),
    .be0      ( int_arrPorts_be0 ),
    .d0       ( int_arrPorts_d0 ),
    .q0       ( int_arrPorts_q0 ),
    .rd0_vld  (arrPorts_vld),
    .clk1     ( ACLK ),
    .address1 ( int_arrPorts_address1 ),
    .ce1      ( int_arrPorts_ce1 ),
    .we1      ( int_arrPorts_we1 ),
    .be1      ( int_arrPorts_be1 ),
    .d1       ( int_arrPorts_d1 ),
    .q1       ( int_arrPorts_q1 )
);


//------------------------AXI write fsm------------------
assign AWREADY = (wstate == WRIDLE);
assign WREADY  = (wstate == WRDATA) && (!ar_hs);
assign BRESP   = 2'b00;  // OKAY
assign BVALID  = (wstate == WRRESP);
assign wmask   = { {8{WSTRB[3]}}, {8{WSTRB[2]}}, {8{WSTRB[1]}}, {8{WSTRB[0]}} };
assign aw_hs   = AWVALID & AWREADY;
assign w_hs    = WVALID & WREADY;

// wstate
always_ff @(posedge ACLK) begin
    if (ARESET)
        wstate <= WRRESET;
    else if (ACLK_EN)
        wstate <= wnext;
end

// wnext
always_comb begin
    case (wstate)
        WRIDLE:
            if (AWVALID)
                wnext = WRDATA;
            else
                wnext = WRIDLE;
        WRDATA:
            if (w_hs)
                wnext = WRRESP;
            else
                wnext = WRDATA;
        WRRESP:
            if (BREADY)
                wnext = WRIDLE;
            else
                wnext = WRRESP;
        default:
            wnext = WRIDLE;
    endcase
end

// waddr
always_ff @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (aw_hs)
            waddr <= AWADDR[ADDR_BITS-1:0];
    end
end

//------------------------AXI read fsm-------------------
assign ARREADY = (rstate == RDIDLE);
assign RDATA   = rdata;
assign RRESP   = 2'b00;  // OKAY
assign RVALID  = (rstate == RDDATA) & !int_arrPorts_read;
assign ar_hs   = ARVALID & ARREADY;
assign raddr   = ARADDR[ADDR_BITS-1:0];

// rstate
always_ff @(posedge ACLK) begin
    if (ARESET)
        rstate <= RDRESET;
    else if (ACLK_EN)
        rstate <= rnext;
end

// rnext
always_comb begin
    case (rstate)
        RDIDLE:
            if (ARVALID)
                rnext = RDDATA;
            else
                rnext = RDIDLE;
        RDDATA:
            if (RREADY & RVALID)
                rnext = RDIDLE;
            else
                rnext = RDDATA;
        default:
            rnext = RDIDLE;
    endcase
end

// rdata
always_ff @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (ar_hs) begin
            rdata <= 'b0;
            case (raddr)
                ADDR_DATAGRAMS_TRANSMITTED_DATA_0: begin
                    rdata <= datagrams_transmitted[31:0];
                end
                ADDR_DATAGRAMS_RECV_DATA_0: begin
                    rdata <= datagrams_recv[31:0];
                end
                ADDR_DATAGRAMS_RECV_INVALID_PORT_DATA_0: begin
                    rdata <= datagrams_recv_invalid_port[31:0];
                end
            endcase
        end
        else if (int_arrPorts_read) begin
            rdata <= int_arrPorts_q1;
        end
    end
end


//------------------------Register logic-----------------

always_ff @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_DATAGRAMS_TRANSMITTED_DATA_0) begin
            int_datagrams_transmitted <= (WDATA[0] & wmask[0]) | (datagrams_transmitted & ~wmask);
        end
        else if (w_hs && waddr == ADDR_DATAGRAMS_RECV_DATA_0) begin
            int_datagrams_recv <= (WDATA & wmask) | (datagrams_recv & ~wmask);
        end
        else if (w_hs && waddr == ADDR_DATAGRAMS_RECV_INVALID_PORT_DATA_0) begin
            int_datagrams_recv_invalid_port <= (WDATA & wmask) | (datagrams_recv_invalid_port & ~wmask);
        end
    end
end

//------------------------Memory logic-------------------
// arrPorts
assign int_arrPorts_address0 = arrPorts_address0 >> 2;
assign int_arrPorts_ce0      = arrPorts_ce0;
assign int_arrPorts_we0      = 1'b0;
assign int_arrPorts_be0      = 1'b0;
assign int_arrPorts_d0       = 1'b0;
assign arrPorts_q0           = int_arrPorts_q0 >> (int_arrPorts_shift * 8);
assign int_arrPorts_address1 = ar_hs? raddr[12:2] : waddr[12:2];
assign int_arrPorts_ce1      = ar_hs | (int_arrPorts_write & WVALID);
assign int_arrPorts_we1      = int_arrPorts_write & w_hs;
assign int_arrPorts_be1      = WSTRB;
assign int_arrPorts_d1       = WDATA;
// int_arrPorts_read
always_ff @(posedge ACLK) begin
    if (ARESET)
        int_arrPorts_read <= 1'b0;
    else if (ACLK_EN) begin
        if (ar_hs && raddr >= ADDR_ARRPORTS_BASE && raddr <= ADDR_ARRPORTS_HIGH)
            int_arrPorts_read <= 1'b1;
        else
            int_arrPorts_read <= 1'b0;
    end
end

// int_arrPorts_write
always_ff @(posedge ACLK) begin
    if (ARESET)
        int_arrPorts_write <= 1'b0;
    else if (ACLK_EN) begin
        if (aw_hs && AWADDR[ADDR_BITS-1:0] >= ADDR_ARRPORTS_BASE && AWADDR[ADDR_BITS-1:0] <= ADDR_ARRPORTS_HIGH)
            int_arrPorts_write <= 1'b1;
        else if (w_hs)
            int_arrPorts_write <= 1'b0;
    end
end

// int_arrPorts_shift
always_ff @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (arrPorts_ce0)
            int_arrPorts_shift <= arrPorts_address0[1:0];
    end
end

endmodule


`timescale 1ns/1ps

module udp_control_s_axi_ram
#(parameter
    BYTES  = 4,
    DEPTH  = 256,
    AWIDTH = log2(DEPTH)
) (
    input  wire               clk0,
    input  wire [AWIDTH-1:0]  address0,
    input  wire               ce0,
    input  wire               we0,
    input  wire [BYTES-1:0]   be0,
    input  wire [BYTES*8-1:0] d0,
    output reg  [BYTES*8-1:0] q0,
    output logic              rd0_vld,
    input  wire               clk1,
    input  wire [AWIDTH-1:0]  address1,
    input  wire               ce1,
    input  wire               we1,
    input  wire [BYTES-1:0]   be1,
    input  wire [BYTES*8-1:0] d1,
    output reg  [BYTES*8-1:0] q1
);
//------------------------Local signal-------------------
reg  [BYTES*8-1:0] mem[0:DEPTH-1];
//------------------------Task and function--------------
function integer log2;
    input integer x;
    integer n, m;
begin
    n = 1;
    m = 2;
    while (m < x) begin
        n = n + 1;
        m = m * 2;
    end
    log2 = n;
end
endfunction
//------------------------Body---------------------------
// read port 0
always_ff @(posedge clk0) begin
    if (ce0) begin
        q0 <= mem[address0];
        rd0_vld <= 1'b1;
    end
    else begin
        rd0_vld <= 1'b0;
    end
end

// read port 1
always_ff @(posedge clk1) begin
    if (ce1) q1 <= mem[address1];
end

genvar i;
generate
    for (i = 0; i < BYTES; i = i + 1) begin : gen_write
        // write port 0
        always_ff @(posedge clk0) begin
            if (ce0 & we0 & be0[i]) begin
                mem[address0][8*i+7:8*i] <= d0[8*i+7:8*i];
            end
        end
        // write port 1
        always_ff @(posedge clk1) begin
            if (ce1 & we1 & be1[i]) begin
                mem[address1][8*i+7:8*i] <= d1[8*i+7:8*i];
            end
        end
    end
endgenerate

endmodule

