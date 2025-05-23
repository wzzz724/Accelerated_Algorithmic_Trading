/*
 * Copyright (c) 2019, Xilinx, Inc.
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




`timescale 1ns / 1ps

module axi_lite_crossbar_tcp_udp #(
    parameter integer C_S_AXI_CONTROL_ADDR_WIDTH = 16,
    parameter integer C_S_AXI_CONTROL_DATA_WIDTH = 32
)(
    input wire ap_clk,
    input wire ap_rst_n,

    // Slave interface ports
    input  wire                                    s_axi_awvalid,
    output wire                                    s_axi_awready,
    input  wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   s_axi_awaddr ,
    input  wire                                    s_axi_wvalid ,
    output wire                                    s_axi_wready ,
    input  wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   s_axi_wdata  ,
    input  wire [C_S_AXI_CONTROL_DATA_WIDTH/8-1:0] s_axi_wstrb  ,
    input  wire                                    s_axi_arvalid,
    output wire                                    s_axi_arready,
    input  wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   s_axi_araddr ,
    output wire                                    s_axi_rvalid ,
    input  wire                                    s_axi_rready ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   s_axi_rdata  ,
    output wire [2-1:0]                            s_axi_rresp  ,
    output wire                                    s_axi_bvalid ,
    input  wire                                    s_axi_bready ,
    output wire [2-1:0]                            s_axi_bresp  ,

    // Master interface ports
    output wire                                    m00_axi_awvalid,
    input  wire                                    m00_axi_awready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m00_axi_awaddr ,
    output wire                                    m00_axi_wvalid ,
    input  wire                                    m00_axi_wready ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m00_axi_wdata  ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH/8-1:0] m00_axi_wstrb  ,
    output wire                                    m00_axi_arvalid,
    input  wire                                    m00_axi_arready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m00_axi_araddr ,
    input  wire                                    m00_axi_rvalid ,
    output wire                                    m00_axi_rready ,
    input  wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m00_axi_rdata  ,
    input  wire [2-1:0]                            m00_axi_rresp  ,
    input  wire                                    m00_axi_bvalid ,
    output wire                                    m00_axi_bready ,
    input  wire [2-1:0]                            m00_axi_bresp,
    output wire                                    m01_axi_awvalid,
    input  wire                                    m01_axi_awready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m01_axi_awaddr ,
    output wire                                    m01_axi_wvalid ,
    input  wire                                    m01_axi_wready ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m01_axi_wdata  ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH/8-1:0] m01_axi_wstrb  ,
    output wire                                    m01_axi_arvalid,
    input  wire                                    m01_axi_arready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m01_axi_araddr ,
    input  wire                                    m01_axi_rvalid ,
    output wire                                    m01_axi_rready ,
    input  wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m01_axi_rdata  ,
    input  wire [2-1:0]                            m01_axi_rresp  ,
    input  wire                                    m01_axi_bvalid ,
    output wire                                    m01_axi_bready ,
    input  wire [2-1:0]                            m01_axi_bresp,
    output wire                                    m02_axi_awvalid,
    input  wire                                    m02_axi_awready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m02_axi_awaddr ,
    output wire                                    m02_axi_wvalid ,
    input  wire                                    m02_axi_wready ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m02_axi_wdata  ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH/8-1:0] m02_axi_wstrb  ,
    output wire                                    m02_axi_arvalid,
    input  wire                                    m02_axi_arready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m02_axi_araddr ,
    input  wire                                    m02_axi_rvalid ,
    output wire                                    m02_axi_rready ,
    input  wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m02_axi_rdata  ,
    input  wire [2-1:0]                            m02_axi_rresp  ,
    input  wire                                    m02_axi_bvalid ,
    output wire                                    m02_axi_bready ,
    input  wire [2-1:0]                            m02_axi_bresp,
    output wire                                    m03_axi_awvalid,
    input  wire                                    m03_axi_awready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m03_axi_awaddr ,
    output wire                                    m03_axi_wvalid ,
    input  wire                                    m03_axi_wready ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m03_axi_wdata  ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH/8-1:0] m03_axi_wstrb  ,
    output wire                                    m03_axi_arvalid,
    input  wire                                    m03_axi_arready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m03_axi_araddr ,
    input  wire                                    m03_axi_rvalid ,
    output wire                                    m03_axi_rready ,
    input  wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m03_axi_rdata  ,
    input  wire [2-1:0]                            m03_axi_rresp  ,
    input  wire                                    m03_axi_bvalid ,
    output wire                                    m03_axi_bready ,
    input  wire [2-1:0]                            m03_axi_bresp,
    output wire                                    m04_axi_awvalid,
    input  wire                                    m04_axi_awready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m04_axi_awaddr ,
    output wire                                    m04_axi_wvalid ,
    input  wire                                    m04_axi_wready ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m04_axi_wdata  ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH/8-1:0] m04_axi_wstrb  ,
    output wire                                    m04_axi_arvalid,
    input  wire                                    m04_axi_arready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m04_axi_araddr ,
    input  wire                                    m04_axi_rvalid ,
    output wire                                    m04_axi_rready ,
    input  wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m04_axi_rdata  ,
    input  wire [2-1:0]                            m04_axi_rresp  ,
    input  wire                                    m04_axi_bvalid ,
    output wire                                    m04_axi_bready ,
    input  wire [2-1:0]                            m04_axi_bresp,
    output wire                                    m05_axi_awvalid,
    input  wire                                    m05_axi_awready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m05_axi_awaddr ,
    output wire                                    m05_axi_wvalid ,
    input  wire                                    m05_axi_wready ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m05_axi_wdata  ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH/8-1:0] m05_axi_wstrb  ,
    output wire                                    m05_axi_arvalid,
    input  wire                                    m05_axi_arready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m05_axi_araddr ,
    input  wire                                    m05_axi_rvalid ,
    output wire                                    m05_axi_rready ,
    input  wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m05_axi_rdata  ,
    input  wire [2-1:0]                            m05_axi_rresp  ,
    input  wire                                    m05_axi_bvalid ,
    output wire                                    m05_axi_bready ,
    input  wire [2-1:0]                            m05_axi_bresp,
    output wire                                    m06_axi_awvalid,
    input  wire                                    m06_axi_awready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m06_axi_awaddr ,
    output wire                                    m06_axi_wvalid ,
    input  wire                                    m06_axi_wready ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m06_axi_wdata  ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH/8-1:0] m06_axi_wstrb  ,
    output wire                                    m06_axi_arvalid,
    input  wire                                    m06_axi_arready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m06_axi_araddr ,
    input  wire                                    m06_axi_rvalid ,
    output wire                                    m06_axi_rready ,
    input  wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m06_axi_rdata  ,
    input  wire [2-1:0]                            m06_axi_rresp  ,
    input  wire                                    m06_axi_bvalid ,
    output wire                                    m06_axi_bready ,
    input  wire [2-1:0]                            m06_axi_bresp,
    output wire                                    m07_axi_awvalid,
    input  wire                                    m07_axi_awready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m07_axi_awaddr ,
    output wire                                    m07_axi_wvalid ,
    input  wire                                    m07_axi_wready ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m07_axi_wdata  ,
    output wire [C_S_AXI_CONTROL_DATA_WIDTH/8-1:0] m07_axi_wstrb  ,
    output wire                                    m07_axi_arvalid,
    input  wire                                    m07_axi_arready,
    output wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   m07_axi_araddr ,
    input  wire                                    m07_axi_rvalid ,
    output wire                                    m07_axi_rready ,
    input  wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   m07_axi_rdata  ,
    input  wire [2-1:0]                            m07_axi_rresp  ,
    input  wire                                    m07_axi_bvalid ,
    output wire                                    m07_axi_bready ,
    input  wire [2-1:0]                            m07_axi_bresp
);
    // NUM_MI is the number of master ports in the crossbar instantiation
    localparam NUM_MI = 10;
    // MASTER_ADDR_WIDTH is the combined width of the concatenated address
    // port for the master side of the crossbar
    localparam MASTER_ADDR_WIDTH =  C_S_AXI_CONTROL_ADDR_WIDTH * NUM_MI;

    // Create intermediate signals as full
    wire [MASTER_ADDR_WIDTH-1 : 0] m_axi_awaddr;
    wire [NUM_MI-1 : 0] m_axi_awvalid;
    wire [NUM_MI-1 : 0] m_axi_awready;
    wire [NUM_MI*32-1 : 0] m_axi_wdata;
    wire [NUM_MI*4-1 : 0] m_axi_wstrb;
    wire [NUM_MI-1 : 0] m_axi_wvalid;
    wire [NUM_MI-1 : 0] m_axi_wready;
    wire [NUM_MI*2-1 : 0] m_axi_bresp;
    wire [NUM_MI-1 : 0] m_axi_bvalid;
    wire [NUM_MI-1 : 0] m_axi_bready;
    wire [MASTER_ADDR_WIDTH-1 : 0] m_axi_araddr;
    wire [NUM_MI-1 : 0] m_axi_arvalid;
    wire [NUM_MI-1 : 0] m_axi_arready;
    wire [NUM_MI*32-1 : 0] m_axi_rdata;
    wire [NUM_MI*2-1 : 0] m_axi_rresp;
    wire [NUM_MI-1   : 0] m_axi_rvalid;
    wire [NUM_MI-1   : 0] m_axi_rready;

    // See PG059 for more information on the crossbar IP.
    axi_crossbar_10 crossbar_i (
        .aclk(ap_clk),                    // input wire aclk
        .aresetn(ap_rst_n),              // input wire aresetn
        .s_axi_awaddr,    // input wire [SLAVE_ADDR_WIDTH-1 : 0] s_axi_awaddr
        .s_axi_awprot(3'b000),    // input wire [2 : 0] s_axi_awprot
        .s_axi_awvalid,  // input wire [0 : 0] s_axi_awvalid
        .s_axi_awready,  // output wire [0 : 0] s_axi_awready
        .s_axi_wdata,      // input wire [31 : 0] s_axi_wdata
        .s_axi_wstrb,      // input wire [3 : 0] s_axi_wstrb
        .s_axi_wvalid,    // input wire [0 : 0] s_axi_wvalid
        .s_axi_wready,    // output wire [0 : 0] s_axi_wready
        .s_axi_bresp,      // output wire [1 : 0] s_axi_bresp
        .s_axi_bvalid,    // output wire [0 : 0] s_axi_bvalid
        .s_axi_bready,    // input wire [0 : 0] s_axi_bready
        .s_axi_araddr,    // input wire [12 : 0] s_axi_araddr
        .s_axi_arprot(3'b000),    // input wire [2 : 0] s_axi_arprot
        .s_axi_arvalid,  // input wire [0 : 0] s_axi_arvalid
        .s_axi_arready,  // output wire [0 : 0] s_axi_arready
        .s_axi_rdata,      // output wire [31 : 0] s_axi_rdata
        .s_axi_rresp,      // output wire [1 : 0] s_axi_rresp
        .s_axi_rvalid,    // output wire [0 : 0] s_axi_rvalid
        .s_axi_rready,    // input wire [0 : 0] s_axi_rready
        .m_axi_awaddr,    // output wire [25 : 0] m_axi_awaddr
        .m_axi_awprot(),    // output wire [5 : 0] m_axi_awprot
        .m_axi_awvalid,  // output wire [1 : 0] m_axi_awvalid
        .m_axi_awready,  // input wire [1 : 0] m_axi_awready
        .m_axi_wdata,      // output wire [63 : 0] m_axi_wdata
        .m_axi_wstrb,      // output wire [7 : 0] m_axi_wstrb
        .m_axi_wvalid,    // output wire [1 : 0] m_axi_wvalid
        .m_axi_wready,    // input wire [1 : 0] m_axi_wready
        .m_axi_bresp,      // input wire [3 : 0] m_axi_bresp
        .m_axi_bvalid,    // input wire [1 : 0] m_axi_bvalid
        .m_axi_bready,    // output wire [1 : 0] m_axi_bready
        .m_axi_araddr,    // output wire [25 : 0] m_axi_araddr
        .m_axi_arprot(),    // output wire [5 : 0] m_axi_arprot
        .m_axi_arvalid,  // output wire [1 : 0] m_axi_arvalid
        .m_axi_arready,  // input wire [1 : 0] m_axi_arready
        .m_axi_rdata,      // input wire [63 : 0] m_axi_rdata
        .m_axi_rresp,      // input wire [3 : 0] m_axi_rresp
        .m_axi_rvalid,    // input wire [1 : 0] m_axi_rvalid
        .m_axi_rready    // output wire [1 : 0] m_axi_rready
    );
     
    // Assign the intermediate signals to the individual module ports
    assign m00_axi_awaddr = m_axi_awaddr[0 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m00_axi_awvalid = m_axi_awvalid[0];
    assign m00_axi_wdata = m_axi_wdata[0 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH];
    assign m00_axi_wstrb = m_axi_wstrb[0 * C_S_AXI_CONTROL_DATA_WIDTH / 8 +: C_S_AXI_CONTROL_DATA_WIDTH / 8];
    assign m00_axi_wvalid = m_axi_wvalid[0];
    assign m00_axi_bready = m_axi_bready[0];
    assign m00_axi_araddr = m_axi_araddr[0 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m00_axi_arvalid = m_axi_arvalid[0];
    assign m00_axi_rready = m_axi_rready[0];

    assign m_axi_awready[0] = m00_axi_awready;
    assign m_axi_wready[0] = m00_axi_wready;
    assign m_axi_bresp[0 +: 2] = m00_axi_bresp;
    assign m_axi_bvalid[0] = m00_axi_bvalid;
    assign m_axi_arready[0] = m00_axi_arready;
    assign m_axi_rdata[0 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH ] = m00_axi_rdata;
    assign m_axi_rresp[0 +: 2] = m00_axi_rresp;
    assign m_axi_rvalid[0] = m00_axi_rvalid;

    assign m01_axi_awaddr = m_axi_awaddr[1 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m01_axi_awvalid = m_axi_awvalid[1];
    assign m01_axi_wdata = m_axi_wdata[1 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH];
    assign m01_axi_wstrb = m_axi_wstrb[1 * C_S_AXI_CONTROL_DATA_WIDTH / 8 +: C_S_AXI_CONTROL_DATA_WIDTH / 8];
    assign m01_axi_wvalid = m_axi_wvalid[1];
    assign m01_axi_bready = m_axi_bready[1];
    assign m01_axi_araddr = m_axi_araddr[1 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m01_axi_arvalid = m_axi_arvalid[1];
    assign m01_axi_rready = m_axi_rready[1];

    assign m_axi_awready[1] = m01_axi_awready;
    assign m_axi_wready[1] = m01_axi_wready;
    assign m_axi_bresp[2 +: 2] = m01_axi_bresp;
    assign m_axi_bvalid[1] = m01_axi_bvalid;
    assign m_axi_arready[1] = m01_axi_arready;
    assign m_axi_rdata[1 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH ] = m01_axi_rdata;
    assign m_axi_rresp[2 +: 2] = m01_axi_rresp;
    assign m_axi_rvalid[1] = m01_axi_rvalid;

    assign m02_axi_awaddr = m_axi_awaddr[2 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m02_axi_awvalid = m_axi_awvalid[2];
    assign m02_axi_wdata = m_axi_wdata[2 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH];
    assign m02_axi_wstrb = m_axi_wstrb[2 * C_S_AXI_CONTROL_DATA_WIDTH / 8 +: C_S_AXI_CONTROL_DATA_WIDTH / 8];
    assign m02_axi_wvalid = m_axi_wvalid[2];
    assign m02_axi_bready = m_axi_bready[2];
    assign m02_axi_araddr = m_axi_araddr[2 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m02_axi_arvalid = m_axi_arvalid[2];
    assign m02_axi_rready = m_axi_rready[2];

    assign m_axi_awready[2] = m02_axi_awready;
    assign m_axi_wready[2] = m02_axi_wready;
    assign m_axi_bresp[4 +: 2] = m02_axi_bresp;
    assign m_axi_bvalid[2] = m02_axi_bvalid;
    assign m_axi_arready[2] = m02_axi_arready;
    assign m_axi_rdata[2 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH ] = m02_axi_rdata;
    assign m_axi_rresp[4 +: 2] = m02_axi_rresp;
    assign m_axi_rvalid[2] = m02_axi_rvalid;

    assign m03_axi_awaddr = m_axi_awaddr[3 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m03_axi_awvalid = m_axi_awvalid[3];
    assign m03_axi_wdata = m_axi_wdata[3 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH];
    assign m03_axi_wstrb = m_axi_wstrb[3 * C_S_AXI_CONTROL_DATA_WIDTH / 8 +: C_S_AXI_CONTROL_DATA_WIDTH / 8];
    assign m03_axi_wvalid = m_axi_wvalid[3];
    assign m03_axi_bready = m_axi_bready[3];
    assign m03_axi_araddr = m_axi_araddr[3 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m03_axi_arvalid = m_axi_arvalid[3];
    assign m03_axi_rready = m_axi_rready[3];

    assign m_axi_awready[3] = m03_axi_awready;
    assign m_axi_wready[3] = m03_axi_wready;
    assign m_axi_bresp[6 +: 2] = m03_axi_bresp;
    assign m_axi_bvalid[3] = m03_axi_bvalid;
    assign m_axi_arready[3] = m03_axi_arready;
    assign m_axi_rdata[3 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH ] = m03_axi_rdata;
    assign m_axi_rresp[6 +: 2] = m03_axi_rresp;
    assign m_axi_rvalid[3] = m03_axi_rvalid;

    assign m04_axi_awaddr = m_axi_awaddr[4 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m04_axi_awvalid = m_axi_awvalid[4];
    assign m04_axi_wdata = m_axi_wdata[4 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH];
    assign m04_axi_wstrb = m_axi_wstrb[4 * C_S_AXI_CONTROL_DATA_WIDTH / 8 +: C_S_AXI_CONTROL_DATA_WIDTH / 8];
    assign m04_axi_wvalid = m_axi_wvalid[4];
    assign m04_axi_bready = m_axi_bready[4];
    assign m04_axi_araddr = m_axi_araddr[4 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m04_axi_arvalid = m_axi_arvalid[4];
    assign m04_axi_rready = m_axi_rready[4];

    assign m_axi_awready[4] = m04_axi_awready;
    assign m_axi_wready[4] = m04_axi_wready;
    assign m_axi_bresp[8 +: 2] = m04_axi_bresp;
    assign m_axi_bvalid[4] = m04_axi_bvalid;
    assign m_axi_arready[4] = m04_axi_arready;
    assign m_axi_rdata[4 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH ] = m04_axi_rdata;
    assign m_axi_rresp[8 +: 2] = m04_axi_rresp;
    assign m_axi_rvalid[4] = m04_axi_rvalid;

    assign m05_axi_awaddr = m_axi_awaddr[5 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m05_axi_awvalid = m_axi_awvalid[5];
    assign m05_axi_wdata = m_axi_wdata[5 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH];
    assign m05_axi_wstrb = m_axi_wstrb[5 * C_S_AXI_CONTROL_DATA_WIDTH / 8 +: C_S_AXI_CONTROL_DATA_WIDTH / 8];
    assign m05_axi_wvalid = m_axi_wvalid[5];
    assign m05_axi_bready = m_axi_bready[5];
    assign m05_axi_araddr = m_axi_araddr[5 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m05_axi_arvalid = m_axi_arvalid[5];
    assign m05_axi_rready = m_axi_rready[5];

    assign m_axi_awready[5] = m05_axi_awready;
    assign m_axi_wready[5] = m05_axi_wready;
    assign m_axi_bresp[10 +: 2] = m05_axi_bresp;
    assign m_axi_bvalid[5] = m05_axi_bvalid;
    assign m_axi_arready[5] = m05_axi_arready;
    assign m_axi_rdata[5 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH ] = m05_axi_rdata;
    assign m_axi_rresp[10 +: 2] = m05_axi_rresp;
    assign m_axi_rvalid[5] = m05_axi_rvalid;

    assign m06_axi_awaddr = m_axi_awaddr[6 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m06_axi_awvalid = m_axi_awvalid[6];
    assign m06_axi_wdata = m_axi_wdata[6 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH];
    assign m06_axi_wstrb = m_axi_wstrb[6 * C_S_AXI_CONTROL_DATA_WIDTH / 8 +: C_S_AXI_CONTROL_DATA_WIDTH / 8];
    assign m06_axi_wvalid = m_axi_wvalid[6];
    assign m06_axi_bready = m_axi_bready[6];
    assign m06_axi_araddr = m_axi_araddr[6 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m06_axi_arvalid = m_axi_arvalid[6];
    assign m06_axi_rready = m_axi_rready[6];

    assign m_axi_awready[6] = m06_axi_awready;
    assign m_axi_wready[6] = m06_axi_wready;
    assign m_axi_bresp[12 +: 2] = m06_axi_bresp;
    assign m_axi_bvalid[6] = m06_axi_bvalid;
    assign m_axi_arready[6] = m06_axi_arready;
    assign m_axi_rdata[6 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH ] = m06_axi_rdata;
    assign m_axi_rresp[12 +: 2] = m06_axi_rresp;
    assign m_axi_rvalid[6] = m06_axi_rvalid;

    assign m07_axi_awaddr = m_axi_awaddr[7 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m07_axi_awvalid = m_axi_awvalid[7];
    assign m07_axi_wdata = m_axi_wdata[7 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH];
    assign m07_axi_wstrb = m_axi_wstrb[7 * C_S_AXI_CONTROL_DATA_WIDTH / 8 +: C_S_AXI_CONTROL_DATA_WIDTH / 8];
    assign m07_axi_wvalid = m_axi_wvalid[7];
    assign m07_axi_bready = m_axi_bready[7];
    assign m07_axi_araddr = m_axi_araddr[7 * C_S_AXI_CONTROL_ADDR_WIDTH +: C_S_AXI_CONTROL_ADDR_WIDTH];
    assign m07_axi_arvalid = m_axi_arvalid[7];
    assign m07_axi_rready = m_axi_rready[7];

    assign m_axi_awready[7] = m07_axi_awready;
    assign m_axi_wready[7] = m07_axi_wready;
    assign m_axi_bresp[14 +: 2] = m07_axi_bresp;
    assign m_axi_bvalid[7] = m07_axi_bvalid;
    assign m_axi_arready[7] = m07_axi_arready;
    assign m_axi_rdata[7 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH ] = m07_axi_rdata;
    assign m_axi_rresp[14 +: 2] = m07_axi_rresp;
    assign m_axi_rvalid[7] = m07_axi_rvalid;

    assign m_axi_awready[8] = 'b0;
    assign m_axi_wready[8] =  'b0;
    assign m_axi_bresp[16 +: 2] = 'b0;
    assign m_axi_bvalid[8] = 'b0;
    assign m_axi_arready[8] = 'b0;
    assign m_axi_rdata[8 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH ] = 'b0;
    assign m_axi_rresp[16 +: 2] = 'b0;
    assign m_axi_rvalid[8] = 'b0;
    
    assign m_axi_awready[9] = 'b0;
    assign m_axi_wready[9] =  'b0;
    assign m_axi_bresp[18 +: 2] = 'b0;
    assign m_axi_bvalid[9] = 'b0;
    assign m_axi_arready[9] = 'b0;
    assign m_axi_rdata[9 * C_S_AXI_CONTROL_DATA_WIDTH +: C_S_AXI_CONTROL_DATA_WIDTH ] = 'b0;
    assign m_axi_rresp[18 +: 2] = 'b0;
    assign m_axi_rvalid[9] = 'b0;
    
endmodule : axi_lite_crossbar_tcp_udp