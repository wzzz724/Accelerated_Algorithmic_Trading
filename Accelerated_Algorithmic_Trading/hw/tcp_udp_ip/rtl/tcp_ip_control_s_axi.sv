/*
 * Copyright (c) 2019-2020, Xilinx, Inc.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are 
 * met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE#
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
`timescale 1ns/1ps
`default_nettype none
module tcp_ip_control_s_axi
#(parameter
    C_S_AXI_ADDR_WIDTH = 6,
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
    output wire [47:0]                   mac_address,
    output wire [31:0]                   ip_address,
    output wire [31:0]                   ip_subnet_mask,
    output wire [31:0]                   ip_default_gateway
);
//------------------------Address Info-------------------
// 0x00 : reserved
// 0x04 : reserved
// 0x08 : reserved
// 0x0c : reserved
// 0x10 : MAC address - first four octets
// 
// 0x14 : MAC address - last two octets
//
// For MAC address AA:BB:CC:DD:EE:FF, 0x10 is 0xAABBCCDD and 0x14 is 0xEEFF0000
//
// 0x18 : IP address - 192.168.0.4 would be 0xC0A80004
// 
// (SC = Self Clear, COR = Clear on Read, TOW = Toggle on Write, COH = Clear on Handshake)

//------------------------Parameter----------------------
localparam
    ADDR_MAC_ADDRESS_0_DATA      = 6'h10,
    ADDR_MAC_ADDRESS_1_DATA      = 6'h14,
    ADDR_IP_ADDRESS_DATA         = 6'h18,
    ADDR_IP_SUBNET_MASK_DATA     = 6'h1C,
    ADDR_IP_DEFAULT_GATEWAY_DATA = 6'h20,
    WRIDLE                       = 2'd0,
    WRDATA                       = 2'd1,
    WRRESP                       = 2'd2,
    WRRESET                      = 2'd3,
    RDIDLE                       = 2'd0,
    RDDATA                       = 2'd1,
    RDRESET                      = 2'd2,
    
    ADDR_BITS                    = 6;

//------------------------Local signal-------------------
    reg  [1:0]                    wstate = WRRESET;
    reg  [1:0]                    wnext;
    reg  [ADDR_BITS-1:0]          waddr;
    wire [31:0]                   wmask;
    wire                          aw_hs;
    wire                          w_hs;
    reg  [1:0]                    rstate = RDRESET;
    reg  [1:0]                    rnext;
    reg  [31:0]                   rdata;
    wire                          ar_hs;
    wire [ADDR_BITS-1:0]          raddr;
    // internal registers
    reg  [31:0]                   int_scalar00 = 'b0;

    reg  [31:0]                   int_mac_address0       = 32'h35029DE5;
    reg  [31:0]                   int_mac_address1       = 32'h0000000A;
    reg  [31:0]                   int_ip_address         = 32'h0A0164C8;
    reg  [31:0]                   int_ip_subnet_mask     = 32'hFFFFFF00;
    reg  [31:0]                   int_ip_default_gateway = 32'h0A016401;

//------------------------Instantiation------------------

//------------------------AXI write fsm------------------
assign AWREADY = (wstate == WRIDLE);
assign WREADY  = (wstate == WRDATA);
assign BRESP   = 2'b00;  // OKAY
assign BVALID  = (wstate == WRRESP);
assign wmask   = { {8{WSTRB[3]}}, {8{WSTRB[2]}}, {8{WSTRB[1]}}, {8{WSTRB[0]}} };
assign aw_hs   = AWVALID & AWREADY;
assign w_hs    = WVALID & WREADY;

// wstate
always @(posedge ACLK) begin
    if (ARESET)
        wstate <= WRRESET;
    else if (ACLK_EN)
        wstate <= wnext;
end

// wnext
always @(*) begin
    case (wstate)
        WRIDLE:
            if (AWVALID)
                wnext = WRDATA;
            else
                wnext = WRIDLE;
        WRDATA:
            if (WVALID)
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
always @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (aw_hs)
            waddr <= AWADDR[ADDR_BITS-1:0];
    end
end

//------------------------AXI read fsm-------------------
assign ARREADY = (rstate == RDIDLE);
assign RDATA   = rdata;
assign RRESP   = 2'b00;  // OKAY
assign RVALID  = (rstate == RDDATA);
assign ar_hs   = ARVALID & ARREADY;
assign raddr   = ARADDR[ADDR_BITS-1:0];

// rstate
always @(posedge ACLK) begin
    if (ARESET)
        rstate <= RDRESET;
    else if (ACLK_EN)
        rstate <= rnext;
end

// rnext
always @(*) begin
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
always @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (ar_hs) begin
            rdata <= 1'b0;
            case (raddr)
                ADDR_MAC_ADDRESS_0_DATA: begin
                    rdata <= int_mac_address0[31:0];
                end
                ADDR_MAC_ADDRESS_1_DATA: begin
                    rdata <= int_mac_address1[31:0];
                end
                ADDR_IP_ADDRESS_DATA: begin
                    rdata <= int_ip_address[31:0];
                end
                ADDR_IP_SUBNET_MASK_DATA: begin
                    rdata <= int_ip_subnet_mask[31:0];
                end
                ADDR_IP_DEFAULT_GATEWAY_DATA: begin
                    rdata <= int_ip_default_gateway[31:0];
                end
            endcase
        end
    end
end


//------------------------Register logic-----------------
assign mac_address = { int_mac_address1[15:0], int_mac_address0 };
always @(posedge ACLK) begin
    if (ARESET)
        int_mac_address0[31:0] <= 32'h35029DE5;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_MAC_ADDRESS_0_DATA)
            int_mac_address0[31:0] <= (WDATA[31:0] & wmask) | (int_mac_address0[31:0] & ~wmask);
    end
end

always @(posedge ACLK) begin
    if (ARESET)
        int_mac_address1[31:0] <= 32'h0000000A;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_MAC_ADDRESS_1_DATA) begin
            int_mac_address1[31:0] <= (WDATA[31:0] & wmask) | (int_mac_address1[31:0] & ~wmask);
            //Upper 16 bits are read only
            int_mac_address1[16+:16] <= '0;
        end
    end
end

assign ip_address = int_ip_address;
always @(posedge ACLK) begin
    if (ARESET)
        int_ip_address[31:0] <= 32'h0A0164C8;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IP_ADDRESS_DATA)
            int_ip_address[31:0] <= (WDATA[31:0] & wmask) | (int_ip_address[31:0] & ~wmask);
    end
end

assign ip_subnet_mask = int_ip_subnet_mask;
always @(posedge ACLK) begin
    if (ARESET)
        int_ip_subnet_mask[31:0] <= 32'hFFFFFF00;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IP_SUBNET_MASK_DATA)
            int_ip_subnet_mask[31:0] <= (WDATA[31:0] & wmask) | (int_ip_subnet_mask[31:0] & ~wmask);
    end
end

assign ip_default_gateway = int_ip_default_gateway;
always @(posedge ACLK) begin
    if (ARESET)
        int_ip_default_gateway[31:0] <= 32'h0A016401;
    else if (ACLK_EN) begin
        if (w_hs && waddr == ADDR_IP_DEFAULT_GATEWAY_DATA)
            int_ip_default_gateway[31:0] <= (WDATA[31:0] & wmask) | (int_ip_default_gateway[31:0] & ~wmask);
    end
end




//------------------------Memory logic-------------------

endmodule
`default_nettype wire
