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
module ethernet_krnl_axis_x4 #(
  parameter integer C_S_AXI_CONTROL_ADDR_WIDTH = 10 ,
  parameter integer C_S_AXI_CONTROL_DATA_WIDTH = 32 ,
  parameter integer C_M_AXI_ADDR_WIDTH       = 64 ,
  parameter integer C_M_AXI_DATA_WIDTH       = 64,
  parameter integer C_M_AXI_ID_WIDTH         = 1,
  parameter integer C_XFER_SIZE_WIDTH        = 32
)
(
  // System Signals
  input  wire                                    ap_clk               ,
  input  wire                                    ap_rst_n             ,
  //  Note: A minimum subset of AXI4 memory mapped signals are declared.  AXI
  // signals omitted from these interfaces are automatically inferred with the
  // optimal values for Xilinx SDx systems.  This allows Xilinx AXI4 Interconnects
  // within the system to be optimized by removing logic for AXI4 protocol
  // features that are not necessary. When adapting AXI4 masters within the RTL
  // kernel that have signals not declared below, it is suitable to add the
  // signals to the declarations below to connect them to the AXI4 Master.
  //
  // List of ommited signals - effect
  // -------------------------------
  // ID - Transaction ID are used for multithreading and out of order
  // transactions.  This increases complexity. This saves logic and increases Fmax
  // in the system when ommited.
  // SIZE - Default value is log2(data width in bytes). Needed for subsize bursts.
  // This saves logic and increases Fmax in the system when ommited.
  // BURST - Default value (0b01) is incremental.  Wrap and fixed bursts are not
  // recommended. This saves logic and increases Fmax in the system when ommited.
  // LOCK - Not supported in AXI4
  // CACHE - Default value (0b0011) allows modifiable transactions. No benefit to
  // changing this.
  // PROT - Has no effect in SDx systems.
  // QOS - Has no effect in SDx systems.
  // REGION - Has no effect in SDx systems.
  // USER - Has no effect in SDx systems.
  // RESP - Not useful in most SDx systems.
  //
  // AXI4-Lite slave interface
  input  wire                                    s_axi_control_awvalid,
  output wire                                    s_axi_control_awready,
  input  wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   s_axi_control_awaddr ,
  input  wire                                    s_axi_control_wvalid ,
  output wire                                    s_axi_control_wready ,
  input  wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   s_axi_control_wdata  ,
  input  wire [C_S_AXI_CONTROL_DATA_WIDTH/8-1:0] s_axi_control_wstrb  ,
  input  wire                                    s_axi_control_arvalid,
  output wire                                    s_axi_control_arready,
  input  wire [C_S_AXI_CONTROL_ADDR_WIDTH-1:0]   s_axi_control_araddr ,
  output wire                                    s_axi_control_rvalid ,
  input  wire                                    s_axi_control_rready ,
  output wire [C_S_AXI_CONTROL_DATA_WIDTH-1:0]   s_axi_control_rdata  ,
  output wire [2-1:0]                            s_axi_control_rresp  ,
  output wire                                    s_axi_control_bvalid ,
  input  wire                                    s_axi_control_bready ,
  output wire [2-1:0]                            s_axi_control_bresp  ,

  //
  output wire        rx0_axis_tvalid,
  output wire [63:0] rx0_axis_tdata,
  output wire        rx0_axis_tlast,
  output wire [7:0]  rx0_axis_tkeep,
  input  wire        rx0_axis_tready,
  output wire        rx0_axis_tuser,

  output wire        rx1_axis_tvalid,
  output wire [63:0] rx1_axis_tdata,
  output wire        rx1_axis_tlast,
  output wire [7:0]  rx1_axis_tkeep,
  input  wire        rx1_axis_tready,
  output wire        rx1_axis_tuser,

  output wire        rx2_axis_tvalid,
  output wire [63:0] rx2_axis_tdata,
  output wire        rx2_axis_tlast,
  output wire [7:0]  rx2_axis_tkeep,
  input  wire        rx2_axis_tready,
  output wire        rx2_axis_tuser,

  output wire        rx3_axis_tvalid,
  output wire [63:0] rx3_axis_tdata,
  output wire        rx3_axis_tlast,
  output wire [7:0]  rx3_axis_tkeep,
  input  wire        rx3_axis_tready,
  output wire        rx3_axis_tuser,  

  output wire        tx0_axis_tready,
  input  wire        tx0_axis_tvalid,
  input  wire [63:0] tx0_axis_tdata,
  input  wire        tx0_axis_tlast,
  input  wire [7:0]  tx0_axis_tkeep,
  input  wire        tx0_axis_tuser,

  output wire        tx1_axis_tready,
  input  wire        tx1_axis_tvalid,
  input  wire [63:0] tx1_axis_tdata,
  input  wire        tx1_axis_tlast,
  input  wire [7:0]  tx1_axis_tkeep,
  input  wire        tx1_axis_tuser,

  output wire        tx2_axis_tready,
  input  wire        tx2_axis_tvalid,
  input  wire [63:0] tx2_axis_tdata,
  input  wire        tx2_axis_tlast,
  input  wire [7:0]  tx2_axis_tkeep,
  input  wire        tx2_axis_tuser,

  output wire        tx3_axis_tready,
  input  wire        tx3_axis_tvalid,
  input  wire [63:0] tx3_axis_tdata,
  input  wire        tx3_axis_tlast,
  input  wire [7:0]  tx3_axis_tkeep,
  input  wire        tx3_axis_tuser,  

  //
  input  wire clk_gt_freerun,
  input  wire [3:0] gt_rxp_in,
  input  wire [3:0] gt_rxn_in,
  output wire [3:0] gt_txp_out,
  output wire [3:0] gt_txn_out,
  input  wire gt_refclk_p,
  input  wire gt_refclk_n
);
localparam C_NUM_LANES = 4;
///////////////////////////////////////////////////////////////////////////////
// Wires and Variables
///////////////////////////////////////////////////////////////////////////////
(* DONT_TOUCH = "yes" *)
reg areset = 1'b0;

wire [C_NUM_LANES-1:0]  rx_axis_tvalid_i;
wire            [63:0]  rx_axis_tdata_i [C_NUM_LANES-1:0];
wire [C_NUM_LANES-1:0]  rx_axis_tlast_i;
wire             [7:0]  rx_axis_tkeep_i [C_NUM_LANES-1:0];
wire [C_NUM_LANES-1:0]  rx_axis_tready_i;
wire [C_NUM_LANES-1:0]  rx_axis_tuser_i;

wire [C_NUM_LANES-1:0]  tx_axis_tvalid_i;
wire            [63:0]  tx_axis_tdata_i [C_NUM_LANES-1:0];
wire [C_NUM_LANES-1:0]  tx_axis_tlast_i;
wire             [7:0]  tx_axis_tkeep_i [C_NUM_LANES-1:0];
wire [C_NUM_LANES-1:0]  tx_axis_tready_i;
wire [C_NUM_LANES-1:0]  tx_axis_tuser_i;

wire [C_NUM_LANES-1:0]  axi_str_txd_tvalid;
wire [C_NUM_LANES-1:0]  axi_str_txd_tready;
wire [C_NUM_LANES-1:0]  axi_str_txd_tlast;
wire             [3:0]  axi_str_txd_tkeep [C_NUM_LANES-1:0];
wire             [3:0]  axi_str_txd_tuser [C_NUM_LANES-1:0];
wire            [31:0]  axi_str_txd_tdata [C_NUM_LANES-1:0];
wire [C_NUM_LANES-1:0]  axi_str_rxd_tvalid;
wire [C_NUM_LANES-1:0]  axi_str_rxd_tready;
wire [C_NUM_LANES-1:0]  axi_str_rxd_tlast;
wire             [3:0]  axi_str_rxd_tkeep [C_NUM_LANES-1:0];
wire            [31:0]  axi_str_rxd_tdata [C_NUM_LANES-1:0];
wire [C_NUM_LANES-1:0]  axi_str_rxd_tuser;

wire                    ap_sys_reset;
wire [C_NUM_LANES-1:0]  stat_rx_block_lock;

wire [C_NUM_LANES-1:0]  ap_rx_block_lock;
wire [C_NUM_LANES-1:0]  ap_rx_block_lock_ll;
wire [C_NUM_LANES-1:0]  ap_rx_block_lock_clr;
wire [C_NUM_LANES-1:0]  ap_rx_fifo_reset;
wire [C_NUM_LANES-1:0]  ap_tx_fifo_reset;
wire [C_NUM_LANES*48-1:0] ap_unicast_mac_addr;
wire [C_NUM_LANES*8-1:0] ap_tx_threshold;

wire [C_NUM_LANES-1:0]  ap_multicast_promiscuous_en;
wire [C_NUM_LANES-1:0]  ap_unicast_promiscuous_en;
wire [C_NUM_LANES-1:0]  ap_cut_through_en;
wire [C_NUM_LANES-1:0]  ap_rx_traf_proc_reset;
wire [C_NUM_LANES-1:0]  rx_traf_proc_reset;

wire [C_NUM_LANES*32-1:0] cnt_rx_overflow;
wire [C_NUM_LANES*32-1:0] cnt_rx_unicastdrop;
wire [C_NUM_LANES*32-1:0] cnt_rx_multicastdrop;
wire [C_NUM_LANES*32-1:0] cnt_rx_oversizedrop;
wire [C_NUM_LANES*32-1:0] cnt_tx_underflow;
wire [C_NUM_LANES-1:0] rx_datafifo_overflow;
wire [C_NUM_LANES-1:0] rx_cmdfifo_overflow;
wire [C_NUM_LANES-1:0] rx_datafifo_overflow_clr;
wire [C_NUM_LANES-1:0] rx_cmdfifo_overflow_clr;
wire [C_NUM_LANES-1:0] tx_fifo_full;
wire [C_NUM_LANES-1:0] tx_fifo_full_clr;


wire [C_NUM_LANES-1:0] ap_tx_traf_proc_reset;
wire [C_NUM_LANES-1:0] tx_traf_proc_reset;

wire [C_NUM_LANES-1:0]  tx_reset;
wire [C_NUM_LANES-1:0]  rx_reset;

wire [C_NUM_LANES-1:0]  rx_core_clk;
wire [C_NUM_LANES-1:0]  tx_clk_out;
wire [C_NUM_LANES-1:0]  rx_clk_out;

// GT Control signals
wire [C_NUM_LANES*3-1:0]     ap_gt_rxoutclksel;
wire [C_NUM_LANES*3-1:0]     ap_gt_txoutclksel;
wire [C_NUM_LANES*3-1:0]     ap_gt_loopback;
wire [C_NUM_LANES-1:0]       ap_gt_rxpolarity;
wire [C_NUM_LANES-1:0]       ap_gt_txpolarity;
wire [C_NUM_LANES-1:0]       tx_gt_txpolarity;
wire [C_NUM_LANES-1:0]       rx_gt_rxpolarity;
wire [C_NUM_LANES*4-1:0]     ap_gt_txprbssel;
wire [C_NUM_LANES*4-1:0]     tx_gt_txprbssel;
wire [C_NUM_LANES-1:0]       ap_gt_txprbsforceerr;
wire [C_NUM_LANES-1:0]       tx_gt_txprbsforceerr;
wire [C_NUM_LANES*4-1:0]     ap_gt_rxprbssel;
wire [C_NUM_LANES*4-1:0]     rx_gt_rxprbssel;
wire [C_NUM_LANES-1:0]       ap_gt_rxprbscntreset;
wire [C_NUM_LANES-1:0]       rx_gt_rxprbscntreset;
wire [C_NUM_LANES-1:0]       ap_gt_rxprbserr;
wire [C_NUM_LANES-1:0]       rx_gt_rxprbserr;
wire [C_NUM_LANES-1:0]       ap_gt_rxprbserr_lh;
wire [C_NUM_LANES-1:0]       ap_gt_rxprbserr_clr;
wire [C_NUM_LANES-1:0]       rx_gt_rxprbserr_clr;
wire [C_NUM_LANES-1:0]       ap_gt_rxprbslocked;
wire [C_NUM_LANES-1:0]       rx_gt_rxprbslocked = 0;
wire [C_NUM_LANES-1:0]       ap_gt_txpmareset;
wire [C_NUM_LANES-1:0]       ap_gt_txpcsreset;
wire [C_NUM_LANES-1:0]       ap_gt_txelecidle;
wire [C_NUM_LANES-1:0]       tx_gt_txelecidle;
wire [C_NUM_LANES*2-1:0]     ap_gt_txbufstatus;
wire [C_NUM_LANES*2-1:0]     tx_gt_txbufstatus;
wire [C_NUM_LANES-1:0]       gtpowergood;
wire [C_NUM_LANES-1:0]       ap_gtpowergood;
wire [C_NUM_LANES-1:0]       ap_gtpowergood_clr;
wire [C_NUM_LANES-1:0]       ap_gt_rxpmareset;
wire [C_NUM_LANES-1:0]       ap_gt_rxpcsreset;
wire [C_NUM_LANES-1:0]       ap_gt_rxbufreset;
wire [C_NUM_LANES-1:0]       ap_gt_eyescanreset;
wire [C_NUM_LANES-1:0]       ap_gt_gtwizrxreset;
wire [C_NUM_LANES-1:0]       ap_gt_gtwiztxreset;
wire [C_NUM_LANES*3-1:0]     ap_gt_rxbufstatus;
wire [C_NUM_LANES*3-1:0]     rx_gt_rxbufstatus;
wire [C_NUM_LANES-1:0]       ap_gt_rxbufstatus_underflow_lh;
wire [C_NUM_LANES-1:0]       ap_gt_rxbufstatus_overflow_lh;
wire [C_NUM_LANES-1:0]       rx_gt_rxbufstatus_underflow_clr;
wire [C_NUM_LANES-1:0]       ap_gt_rxbufstatus_underflow_clr;
wire [C_NUM_LANES-1:0]       rx_gt_rxbufstatus_overflow_clr;
wire [C_NUM_LANES-1:0]       ap_gt_rxbufstatus_overflow_clr;
wire [C_NUM_LANES*5-1:0]     ap_gt_txdiffctrl;
wire [C_NUM_LANES*7-1:0]     ap_gt_txmaincursor;
wire [C_NUM_LANES*5-1:0]     ap_gt_txprecursor;
wire [C_NUM_LANES*5-1:0]     ap_gt_txpostcursor;
wire [C_NUM_LANES-1:0]       ap_gt_txinhibit;
wire [C_NUM_LANES-1:0]       tx_gt_txinhibit;
wire [C_NUM_LANES-1:0]       ap_gt_rxlpmen;
wire [C_NUM_LANES-1:0]       ap_gt_rxdfelpmreset;
wire [C_NUM_LANES-1:0]       ap_gt_eyescantrigger;
wire [C_NUM_LANES-1:0]       ap_gt_eyescandataerror_clr;
wire [C_NUM_LANES-1:0]       gt_eyescandataerror;
wire [C_NUM_LANES-1:0]       ap_gt_eyescandataerror_sync;
wire [C_NUM_LANES-1:0]       stat_rx_block_lock_clr;

reg  [C_NUM_LANES-1:0]       rx_gt_rxprbserr_lh = {C_NUM_LANES {1'b0}};
reg  [C_NUM_LANES-1:0]       ap_gtpowergood_ll = {C_NUM_LANES {1'b1}};
reg [C_NUM_LANES-1:0]        rx_gt_rxbufstatus_underflow = {C_NUM_LANES {1'b0}};
reg [C_NUM_LANES-1:0]        rx_gt_rxbufstatus_overflow = {C_NUM_LANES {1'b0}};
reg [C_NUM_LANES-1:0]        rx_gt_rxbufstatus_underflow_lh = {C_NUM_LANES {1'b0}};
reg [C_NUM_LANES-1:0]        rx_gt_rxbufstatus_overflow_lh = {C_NUM_LANES {1'b0}};
reg [C_NUM_LANES-1:0]        ap_gt_eyescandataerror_lh = {C_NUM_LANES {1'b0}};
reg [C_NUM_LANES-1:0]        stat_rx_block_lock_ll = {C_NUM_LANES {1'b1}};
reg [C_NUM_LANES-1:0]        rx_datafifo_overflow_lh = {C_NUM_LANES{1'b0}};
reg [C_NUM_LANES-1:0]        rx_cmdfifo_overflow_lh = {C_NUM_LANES{1'b0}};
reg [C_NUM_LANES-1:0]        tx_fifo_full_lh = {C_NUM_LANES{1'b0}};

wire           [75:0]  debug_port_low_latency;


// AXI4-Lite slave interface
ethernet_control_s_axi #(
  .C_S_AXI_ADDR_WIDTH ( C_S_AXI_CONTROL_ADDR_WIDTH ),
  .C_S_AXI_DATA_WIDTH ( C_S_AXI_CONTROL_DATA_WIDTH ),
  .C_NUM_GT (4)
)
inst_control_s_axi (
  .ACLK                  ( ap_clk                ),
  .ARESET                ( 1'b0                  ),
  .ACLK_EN               ( 1'b1                  ),
  .AWVALID               ( s_axi_control_awvalid ),
  .AWREADY               ( s_axi_control_awready ),
  .AWADDR                ( s_axi_control_awaddr  ),
  .WVALID                ( s_axi_control_wvalid  ),
  .WREADY                ( s_axi_control_wready  ),
  .WDATA                 ( s_axi_control_wdata   ),
  .WSTRB                 ( s_axi_control_wstrb   ),
  .ARVALID               ( s_axi_control_arvalid ),
  .ARREADY               ( s_axi_control_arready ),
  .ARADDR                ( s_axi_control_araddr  ),
  .RVALID                ( s_axi_control_rvalid  ),
  .RREADY                ( s_axi_control_rready  ),
  .RDATA                 ( s_axi_control_rdata   ),
  .RRESP                 ( s_axi_control_rresp   ),
  .BVALID                ( s_axi_control_bvalid  ),
  .BREADY                ( s_axi_control_bready  ),
  .BRESP                 ( s_axi_control_bresp   ),
  .rx_block_lock         ( ap_rx_block_lock),
  .rx_block_lock_ll      ( ap_rx_block_lock_ll),
  .rx_block_lock_clr     ( ap_rx_block_lock_clr),
  .cnt_rx_overflow       ( cnt_rx_overflow ),
  .cnt_rx_unicastdrop    ( cnt_rx_unicastdrop ),
  .cnt_rx_multicastdrop  ( cnt_rx_multicastdrop ),
  .cnt_rx_oversizedrop   ( cnt_rx_oversizedrop ),
  .cnt_tx_underflow      ( cnt_tx_underflow ),
  .rx_datafifo_overflow  (rx_datafifo_overflow),
  .rx_datafifo_overflow_lh  (rx_datafifo_overflow_lh),
  .rx_datafifo_overflow_clr (rx_datafifo_overflow_clr),
  .rx_cmdfifo_overflow  (rx_cmdfifo_overflow),
  .rx_cmdfifo_overflow_lh  (rx_cmdfifo_overflow_lh),
  .rx_cmdfifo_overflow_clr (rx_cmdfifo_overflow_clr),
  .tx_fifo_full            (tx_fifo_full),
  .tx_fifo_full_lh         (tx_fifo_full_lh),
  .tx_fifo_full_clr        (tx_fifo_full_clr),
  .reset_out             ( ap_sys_reset),
  .unicast_mac_addr      ( ap_unicast_mac_addr),
  .rx_fifo_reset         ( ap_rx_fifo_reset),
  .rx_traf_proc_reset    ( ap_rx_traf_proc_reset),
  .tx_fifo_reset         ( ap_tx_fifo_reset),
  .tx_traf_proc_reset    ( ap_tx_traf_proc_reset),
  .unicast_promiscuous_en (ap_unicast_promiscuous_en),    
  .multicast_promiscuous_en (ap_multicast_promiscuous_en),
  .cut_through_en        (ap_cut_through_en),
  .tx_threshold          (ap_tx_threshold),
  .gt_rxoutclksel        (ap_gt_rxoutclksel),
  .gt_txoutclksel        (ap_gt_txoutclksel),
  .gt_loopback           (ap_gt_loopback),
  .gt_rxpolarity         (ap_gt_rxpolarity),
  .gt_txpolarity         (ap_gt_txpolarity),
  .gt_txprbssel          (ap_gt_txprbssel),
  .gt_txprbsforceerr     (ap_gt_txprbsforceerr),
  .gt_rxprbssel          (ap_gt_rxprbssel),
  .gt_rxprbscntreset     (ap_gt_rxprbscntreset),
  .gt_rxprbslocked       (ap_gt_rxprbslocked),
  .gt_rxprbserr          (ap_gt_rxprbserr),
  .gt_rxprbserr_lh       (ap_gt_rxprbserr_lh),
  .gt_rxprbserr_clr      (ap_gt_rxprbserr_clr),
  .gt_txpmareset         (ap_gt_txpmareset),
  .gt_txpcsreset         (ap_gt_txpcsreset),
  .gt_txelecidle         (ap_gt_txelecidle),
  .gt_txbufstatus        (ap_gt_txbufstatus),
  .gt_gtpowergood        (ap_gtpowergood),
  .gt_gtpowergood_ll     (ap_gtpowergood_ll),
  .gt_gtpowergood_clr    (ap_gtpowergood_clr),
  .gt_rxpmareset         (ap_gt_rxpmareset),
  .gt_rxpcsreset         (ap_gt_rxpcsreset),
  .gt_rxbufreset         (ap_gt_rxbufreset),
  .gt_eyescanreset       (ap_gt_eyescanreset),
  .gt_gtwizrxreset       (ap_gt_gtwizrxreset),
  .gt_gtwiztxreset       (ap_gt_gtwiztxreset),
  .gt_rxbufstatus        (ap_gt_rxbufstatus),
  .gt_rxbufstatus_underflow_lh (ap_gt_rxbufstatus_underflow_lh),
  .gt_rxbufstatus_overflow_lh (ap_gt_rxbufstatus_overflow_lh),
  .gt_rxbufstatus_underflow_clr (ap_gt_rxbufstatus_underflow_clr),
  .gt_rxbufstatus_overflow_clr (ap_gt_rxbufstatus_overflow_clr),
  .gt_txdiffctrl          (ap_gt_txdiffctrl),
  .gt_txmaincursor        (ap_gt_txmaincursor),
  .gt_txpostcursor        (ap_gt_txpostcursor),
  .gt_txprecursor         (ap_gt_txprecursor),
  .gt_txinhibit           (ap_gt_txinhibit),
  .gt_rxlpmen             (ap_gt_rxlpmen),
  .gt_rxdfelpmreset       (ap_gt_rxdfelpmreset),
  .gt_eyescandataerror_lh (ap_gt_eyescandataerror_lh),
  .gt_eyescantrigger      (ap_gt_eyescantrigger),
  .gt_eyescandataerror_clr (ap_gt_eyescandataerror_clr)
);

genvar ii;
generate    
    for (ii = 0; ii < C_NUM_LANES; ii = ii + 1) begin
        // Synchronize GT signals into TXUSRCLK domain        
        xpm_cdc_array_single #(
            .DEST_SYNC_FF(4),   
            .INIT_SYNC_FF(0),
            .SIM_ASSERT_CHK(0),
            .SRC_INPUT_REG(0),
            .WIDTH(8)
        ) tx_gt_sync_i (
            .dest_out({tx_gt_txpolarity[ii], tx_gt_txprbssel[ii*4+:4], tx_gt_txprbsforceerr[ii], tx_gt_txelecidle[ii], tx_gt_txinhibit[ii]}),
            .dest_clk(tx_clk_out[ii]),
            .src_clk(1'b0),
            .src_in({ap_gt_txpolarity[ii], ap_gt_txprbssel[ii*4+:4], ap_gt_txprbsforceerr[ii], ap_gt_txelecidle[ii], ap_gt_txinhibit[ii]})
        );

        // Synchronize GT signals into RXUSRCLK domain
        xpm_cdc_array_single #(
            .DEST_SYNC_FF(4),
            .INIT_SYNC_FF(0),
            .SIM_ASSERT_CHK(0),
            .SRC_INPUT_REG(0),   
            .WIDTH(6)
        ) rx_gt_sync_i (
            .dest_out({rx_gt_rxpolarity[ii],rx_gt_rxprbssel[ii*4+:4],rx_gt_rxprbscntreset[ii]}),
            .dest_clk(rx_clk_out[ii]),
            .src_clk(1'b0),
            .src_in({ap_gt_rxpolarity[ii],ap_gt_rxprbssel[ii*4+:4],ap_gt_rxprbscntreset[ii]})
        );

        // Synchronize GT signals into ap_clk domain
        xpm_cdc_array_single #(
            .DEST_SYNC_FF(4),
            .INIT_SYNC_FF(0),
            .SIM_ASSERT_CHK(0),
            .SRC_INPUT_REG(0),
            .WIDTH(13)
        ) ap_gt_sync_i (
            .dest_out({ap_rx_block_lock[ii],ap_rx_block_lock_ll[ii], 
                       ap_gt_rxprbserr[ii],ap_gt_rxprbslocked[ii],
                       ap_gt_rxprbserr_lh[ii],ap_gt_txbufstatus[ii*2+:2], 
                       ap_gt_rxbufstatus[ii*3+:3],
                       ap_gt_rxbufstatus_underflow_lh[ii],
                       ap_gt_rxbufstatus_overflow_lh[ii],
                       ap_gtpowergood[ii]
                       }),
            .dest_clk(ap_clk),
            .src_clk(1'b0),
            .src_in({stat_rx_block_lock[ii], stat_rx_block_lock_ll[ii], 
                     rx_gt_rxprbserr[ii], rx_gt_rxprbslocked[ii], 
                     rx_gt_rxprbserr_lh[ii],tx_gt_txbufstatus[ii*2+:2], 
                     rx_gt_rxbufstatus[ii*3+:3], 
                     rx_gt_rxbufstatus_underflow_lh[ii],
                     rx_gt_rxbufstatus_overflow_lh[ii],
                     gtpowergood[ii]})
        );

        // Stretch the single cycle ap_clk clr into destination clock
        xpm_cdc_pulse #(
            .DEST_SYNC_FF(4),
            .INIT_SYNC_FF(0),
            .REG_OUTPUT(0),
            .RST_USED(0),
            .SIM_ASSERT_CHK(0)
        ) gt_rxprbserr_clr_pulse_i (
            .dest_pulse(rx_gt_rxprbserr_clr[ii]),
            .dest_rst(1'b0),
            .dest_clk(rx_clk_out[ii]),
            .src_rst(1'b0),
            .src_clk(ap_clk),
            .src_pulse(ap_gt_rxprbserr_clr[ii])
        );

        xpm_cdc_pulse #(
            .DEST_SYNC_FF(4),
            .INIT_SYNC_FF(0),
            .REG_OUTPUT(0),
            .RST_USED(0),
            .SIM_ASSERT_CHK(0)
        ) gt_rxbufstatus_underflow_clr_pulse_i (
            .dest_pulse(rx_gt_rxbufstatus_underflow_clr[ii]),
            .dest_rst(1'b0),
            .dest_clk(rx_clk_out[ii]),
            .src_rst(1'b0),
            .src_clk(ap_clk),
            .src_pulse(ap_gt_rxbufstatus_underflow_clr[ii])
        );        

        xpm_cdc_pulse #(
            .DEST_SYNC_FF(4),
            .INIT_SYNC_FF(0),
            .REG_OUTPUT(0),
            .RST_USED(0),
            .SIM_ASSERT_CHK(0)
        ) gt_rxbufstatus_overflow_clr_pulse_i (
            .dest_pulse(rx_gt_rxbufstatus_overflow_clr[ii]),
            .dest_rst(1'b0),
            .dest_clk(rx_clk_out[ii]),
            .src_rst(1'b0),
            .src_clk(ap_clk),
            .src_pulse(ap_gt_rxbufstatus_overflow_clr[ii])
        );

        xpm_cdc_pulse #(
            .DEST_SYNC_FF(4),
            .INIT_SYNC_FF(0),
            .REG_OUTPUT(0),
            .RST_USED(0),
            .SIM_ASSERT_CHK(0)
        ) gt_rx_block_lock_clr_pulse_i (
            .dest_pulse(stat_rx_block_lock_clr[ii]),
            .dest_rst(1'b0),
            .dest_clk(rx_clk_out[ii]),
            .src_rst(1'b0),
            .src_clk(ap_clk),
            .src_pulse(ap_rx_block_lock_clr[ii])
        );        

        // The following style of code is to prevent X propagation
        always @(posedge rx_clk_out[ii]) begin
            if (rx_gt_rxprbserr_lh[ii] != 1) begin
                if (rx_gt_rxprbserr[ii] == 1)
                    rx_gt_rxprbserr_lh[ii] <= 1;
                else
                    rx_gt_rxprbserr_lh[ii] <= 0;
            end
            else if (rx_gt_rxprbserr_clr[ii]) begin
                rx_gt_rxprbserr_lh[ii] <= 1'b0;
            end
        end

        always @(posedge rx_clk_out[ii]) begin
            rx_gt_rxbufstatus_underflow[ii] <= 0;
            rx_gt_rxbufstatus_overflow[ii] <= 0;
            if (rx_gt_rxbufstatus[ii*3+:3] == 3'b101) begin
                rx_gt_rxbufstatus_underflow[ii] <= 1'b1;
            end
            else if (rx_gt_rxbufstatus[ii*3+:3] == 3'b110) begin
                rx_gt_rxbufstatus_overflow[ii] <= 1'b1;
            end
        end

        always @(posedge rx_clk_out[ii]) begin
            if (rx_gt_rxbufstatus_underflow_lh[ii] != 1) begin
                if (rx_gt_rxbufstatus_underflow[ii] == 1)
                    rx_gt_rxbufstatus_underflow_lh[ii] <= 1;
                else
                    rx_gt_rxbufstatus_underflow_lh[ii] <= 0;
            end
            else if (rx_gt_rxbufstatus_underflow_clr[ii]) begin
                rx_gt_rxbufstatus_underflow_lh[ii] <= 1'b0;
            end
        end        

        always @(posedge rx_clk_out[ii]) begin
            if (rx_gt_rxbufstatus_overflow_lh[ii] != 1) begin
                if (rx_gt_rxbufstatus_overflow[ii] == 1) 
                    rx_gt_rxbufstatus_overflow_lh[ii] <= 1;
                else 
                    rx_gt_rxbufstatus_overflow_lh[ii] <= 0;
            end
            else if (rx_gt_rxbufstatus_overflow_clr[ii]) begin
                rx_gt_rxbufstatus_overflow_lh[ii] <= 1'b0;
            end
        end       

        always @(posedge rx_clk_out[ii]) begin
            if (stat_rx_block_lock_ll[ii] != 0) begin
                if (stat_rx_block_lock[ii] == 1)
                  stat_rx_block_lock_ll[ii] <= 1;
                else
                  stat_rx_block_lock_ll[ii] <= 0;
            end
            else if (stat_rx_block_lock_clr[ii]) begin
                stat_rx_block_lock_ll[ii] <= 1'b1;
            end
        end

        always @(posedge ap_clk) begin
            if (ap_gtpowergood_ll[ii] != 0) begin
                if (ap_gtpowergood_ll[ii] == 1)
                    ap_gtpowergood_ll[ii] <= 1;
                else
                    ap_gtpowergood_ll[ii] <= 0;
            end
            else if (ap_gtpowergood_clr[ii]) begin
                ap_gtpowergood_ll[ii] <= 1'b1;
            end
        end

        // Eyescan data error is defined as asynchronous. Use this sync_rst xpm
        // to synchronize edge into ap_clk
        xpm_cdc_sync_rst #(
            .DEST_SYNC_FF(4),
            .INIT(0),
            .INIT_SYNC_FF(0),
            .SIM_ASSERT_CHK(0)
        ) xpm_cdc_eyescandataerror_sync (
            .dest_rst(ap_gt_eyescandataerror_sync[ii]),
            .dest_clk(ap_clk),
            .src_rst(gt_eyescandataerror[ii])
        );
        
        always @(posedge ap_clk) begin
            if (ap_gt_eyescandataerror_sync[ii]) begin
                ap_gt_eyescandataerror_lh[ii] <= 1;
            end
            else if (ap_gt_eyescandataerror_clr[ii]) begin
                ap_gt_eyescandataerror_lh[ii] <= 0;
            end
        end

        xpm_cdc_sync_rst #(
            .DEST_SYNC_FF(4),
            .INIT(0),
            .INIT_SYNC_FF(0),
            .SIM_ASSERT_CHK(0)
        ) xpm_cdc_rx_traf_proc_rst_sync (
            .dest_rst(rx_traf_proc_reset[ii]),
            .dest_clk(rx_clk_out[ii]),
            .src_rst(ap_rx_traf_proc_reset[ii])
        );      

        xpm_cdc_sync_rst #(
            .DEST_SYNC_FF(4),
            .INIT(0),
            .INIT_SYNC_FF(0),
            .SIM_ASSERT_CHK(0)
        ) xpm_cdc_tx_traf_proc_rst_sync (
            .dest_rst(tx_traf_proc_reset[ii]),
            .dest_clk(tx_clk_out[ii]),
            .src_rst(ap_tx_traf_proc_reset[ii])
        );          


        // Signals from the rx traffic processor 
        always @(posedge ap_clk) begin
            if (rx_datafifo_overflow[ii] == 1) begin
                rx_datafifo_overflow_lh[ii] <= 1;
            end
            else if (rx_datafifo_overflow_clr[ii] == 1) begin
                rx_datafifo_overflow_lh[ii] <= 0;
            end
        end

        always @(posedge ap_clk) begin
            if (rx_cmdfifo_overflow[ii] == 1) begin
                rx_cmdfifo_overflow_lh[ii] <= 1;
            end
            else if (rx_cmdfifo_overflow_clr[ii] == 1) begin
                rx_cmdfifo_overflow_lh[ii] <= 0;
            end
        end      

        always @(posedge ap_clk) begin
            if (tx_fifo_full[ii] == 1) begin
                tx_fifo_full_lh[ii] <= 1;
            end
            else if (tx_fifo_full_clr[ii] == 1) begin
                tx_fifo_full_lh[ii] <= 0;
            end
        end

        rx_ethernet_krnl_axis_traf_proc rx_traf_proc(
          // System Signals
          .ap_clk(ap_clk),
        
          .fifo_resetn(~ap_rx_fifo_reset[ii]),
          .rx_resetn(~rx_traf_proc_reset[ii]),
          .areset(areset),
          .rx_reset(rx_traf_proc_reset[ii]),
        
          .rx_clk_in(rx_clk_out[ii]),
        
          .unicast_mac_addr(ap_unicast_mac_addr[(ii*48)+:48]),
          .unicast_promiscuous_en(ap_unicast_promiscuous_en[ii]),
          .multicast_promiscuous_en(ap_multicast_promiscuous_en[ii]),
          .reset_drop_frame_count(1'b0),
          .cutthrough_en(ap_cut_through_en[ii]),
          .rx_overflow({rx_cmdfifo_overflow[ii],rx_datafifo_overflow[ii]}),
          .drop_unicast_count(cnt_rx_unicastdrop[(ii*32)+:32]),
          .drop_multicast_count(cnt_rx_multicastdrop[(ii*32)+:32]),
          .drop_fifo_count(cnt_rx_overflow[(ii*32)+:32]),
          .oversize_frame_count(cnt_rx_oversizedrop[(ii*32)+:32]),
        
          .rx0_axis_tvalid_from_fsm(rx_axis_tvalid_i[ii]),
          .rx0_axis_tdata(rx_axis_tdata_i[ii]),
          .rx0_axis_tlast(rx_axis_tlast_i[ii]),
          .rx0_axis_tkeep(rx_axis_tkeep_i[ii]),
          .rx0_axis_tready(rx_axis_tready_i[ii]),
          .rx0_axis_tuser(rx_axis_tuser_i[ii]),
        
          .axi_str_rxd_tvalid(axi_str_rxd_tvalid[ii]),
          .axi_str_rxd_tready(axi_str_rxd_tready[ii]),
          .axi_str_rxd_tlast(axi_str_rxd_tlast[ii]),
          .axi_str_rxd_tkeep(axi_str_rxd_tkeep[ii]),
          .axi_str_rxd_tdata(axi_str_rxd_tdata[ii]),
          .axi_str_rxd_tuser(axi_str_rxd_tuser[ii])
        );
        
        tx_ethernet_krnl_axis_traf_proc tx_traf_proc(
          // System Signals
          .ap_clk(ap_clk),
          .fifo_resetn(~ap_tx_fifo_reset[ii]),
          .tx_resetn(~tx_traf_proc_reset[ii]),
          .areset(areset),
        
          .tx_clk_in(tx_clk_out[ii]),
          .tx_reset(tx_traf_proc_reset[ii]),
        
          .tx_threshold(ap_tx_threshold[(ii*8)+:8]),
          .tx_fifo_full(tx_fifo_full[ii]),
          .underflow_count(cnt_tx_underflow[(ii*32)+:32]),
        
          .tx0_axis_tready(tx_axis_tready_i[ii]),
          .tx0_axis_tvalid(tx_axis_tvalid_i[ii]),
          .tx0_axis_tdata(tx_axis_tdata_i[ii]),
          .tx0_axis_tlast(tx_axis_tlast_i[ii]),
          .tx0_axis_tkeep(tx_axis_tkeep_i[ii]),
        
          .axi_str_txd_tvalid(axi_str_txd_tvalid[ii]),
          .axi_str_txd_tready(axi_str_txd_tready[ii]),
          .axi_str_txd_tlast(axi_str_txd_tlast[ii]),
          .axi_str_txd_tkeep(axi_str_txd_tkeep[ii]),
          .axi_str_txd_tuser(axi_str_txd_tuser[ii]),
          .axi_str_txd_tdata(axi_str_txd_tdata[ii])
        
        );        
end
endgenerate

always @(posedge ap_clk) begin
    areset <= ~ap_rst_n || ap_sys_reset;    
end

assign rx_core_clk[0] = rx_clk_out[0];
assign rx_core_clk[1] = rx_clk_out[1];
assign rx_core_clk[2] = rx_clk_out[2];
assign rx_core_clk[3] = rx_clk_out[3];

//Ethernet Subsystem instance
xxv_ethernet_x4_0 i_ethernet_0 (
   .gt_txp_out(gt_txp_out),                                   // output wire [3 : 0] gt_txp_out
   .gt_txn_out(gt_txn_out),                                   // output wire [3 : 0] gt_txn_out
   .gt_rxp_in(gt_rxp_in),                                     // input wire [3 : 0] gt_rxp_in
   .gt_rxn_in(gt_rxn_in),                                     // input wire [3 : 0] gt_rxn_in
  .rx_core_clk_0(rx_core_clk[0]),                            // input wire rx_core_clk_0
  .rx_core_clk_1(rx_core_clk[1]),                            // input wire rx_core_clk_1
  .rx_core_clk_2(rx_core_clk[2]),                            // input wire rx_core_clk_2
  .rx_core_clk_3(rx_core_clk[3]),                            // input wire rx_core_clk_3  
  .txoutclksel_in_0(ap_gt_txoutclksel[0+:3]),                // input wire [2 : 0] txoutclksel_in_0
  .rxoutclksel_in_0(ap_gt_rxoutclksel[0+:3]),                // input wire [2 : 0] rxoutclksel_in_0
  .txoutclksel_in_1(ap_gt_txoutclksel[3+:3]),                // input wire [2 : 0] txoutclksel_in_1
  .rxoutclksel_in_1(ap_gt_rxoutclksel[3+:3]),                // input wire [2 : 0] rxoutclksel_in_1
  .txoutclksel_in_2(ap_gt_txoutclksel[6+:3]),                // input wire [2 : 0] txoutclksel_in_2
  .rxoutclksel_in_2(ap_gt_rxoutclksel[6+:3]),                // input wire [2 : 0] rxoutclksel_in_2
  .txoutclksel_in_3(ap_gt_txoutclksel[9+:3]),                // input wire [2 : 0] txoutclksel_in_3
  .rxoutclksel_in_3(ap_gt_rxoutclksel[9+:3]),                // input wire [2 : 0] rxoutclksel_in_3  

  .gt_dmonitorout_0(),                                       // output wire [16 : 0] gt_dmonitorout_0
  .gt_dmonitorout_1(),                                       // output wire [16 : 0] gt_dmonitorout_1
  .gt_dmonitorout_2(),                                       // output wire [16 : 0] gt_dmonitorout_2
  .gt_dmonitorout_3(),                                       // output wire [16 : 0] gt_dmonitorout_3  
  .gt_eyescandataerror_0(gt_eyescandataerror[0]),            // output wire gt_eyescandataerror_0
  .gt_eyescandataerror_1(gt_eyescandataerror[1]),            // output wire gt_eyescandataerror_1
  .gt_eyescandataerror_2(gt_eyescandataerror[2]),            // output wire gt_eyescandataerror_2
  .gt_eyescandataerror_3(gt_eyescandataerror[3]),            // output wire gt_eyescandataerror_3  
  .gt_eyescanreset_0(ap_gt_eyescanreset[0]),                 // input wire gt_eyescanreset_0
  .gt_eyescanreset_1(ap_gt_eyescanreset[1]),                 // input wire gt_eyescanreset_1
  .gt_eyescanreset_2(ap_gt_eyescanreset[2]),                 // input wire gt_eyescanreset_2
  .gt_eyescanreset_3(ap_gt_eyescanreset[3]),                 // input wire gt_eyescanreset_3  
  .gt_eyescantrigger_0(ap_gt_eyescantrigger[0]),             // input wire gt_eyescantrigger_0
  .gt_eyescantrigger_1(ap_gt_eyescantrigger[1]),             // input wire gt_eyescantrigger_1
  .gt_eyescantrigger_2(ap_gt_eyescantrigger[2]),             // input wire gt_eyescantrigger_2
  .gt_eyescantrigger_3(ap_gt_eyescantrigger[3]),             // input wire gt_eyescantrigger_3  
  .gt_pcsrsvdin_0(16'd0),                                    // input wire [15 : 0] gt_pcsrsvdin_0
  .gt_pcsrsvdin_1(16'd0),                                    // input wire [15 : 0] gt_pcsrsvdin_1
  .gt_pcsrsvdin_2(16'd0),                                    // input wire [15 : 0] gt_pcsrsvdin_2
  .gt_pcsrsvdin_3(16'd0),                                    // input wire [15 : 0] gt_pcsrsvdin_3  
  .gt_rxbufreset_0(ap_gt_rxbufreset[0]),                     // input wire gt_rxbufreset_0
  .gt_rxbufreset_1(ap_gt_rxbufreset[1]),                     // input wire gt_rxbufreset_1
  .gt_rxbufreset_2(ap_gt_rxbufreset[2]),                     // input wire gt_rxbufreset_2
  .gt_rxbufreset_3(ap_gt_rxbufreset[3]),                     // input wire gt_rxbufreset_3  
  .gt_rxbufstatus_0(rx_gt_rxbufstatus[0+:3]),                // output wire [2 : 0] gt_rxbufstatus_0
  .gt_rxbufstatus_1(rx_gt_rxbufstatus[3+:3]),                // output wire [2 : 0] gt_rxbufstatus_1
  .gt_rxbufstatus_2(rx_gt_rxbufstatus[6+:3]),                // output wire [2 : 0] gt_rxbufstatus_2
  .gt_rxbufstatus_3(rx_gt_rxbufstatus[9+:3]),                // output wire [2 : 0] gt_rxbufstatus_3  
  .gt_rxcdrhold_0(1'b0),                                     // input wire gt_rxcdrhold_0
  .gt_rxcdrhold_1(1'b0),                                     // input wire gt_rxcdrhold_1
  .gt_rxcdrhold_2(1'b0),                                     // input wire gt_rxcdrhold_2
  .gt_rxcdrhold_3(1'b0),                                     // input wire gt_rxcdrhold_3  
  .gt_rxcommadeten_0(1'b0),                                  // input wire gt_rxcommadeten_0
  .gt_rxcommadeten_1(1'b0),                                  // input wire gt_rxcommadeten_1
  .gt_rxcommadeten_2(1'b0),                                  // input wire gt_rxcommadeten_2
  .gt_rxcommadeten_3(1'b0),                                  // input wire gt_rxcommadeten_3  
  .gt_rxdfeagchold_0(1'b0),                                  // input wire gt_rxdfeagchold_0
  .gt_rxdfeagchold_1(1'b0),                                  // input wire gt_rxdfeagchold_1
  .gt_rxdfeagchold_2(1'b0),                                  // input wire gt_rxdfeagchold_2
  .gt_rxdfeagchold_3(1'b0),                                  // input wire gt_rxdfeagchold_3  
  .gt_rxdfelpmreset_0(ap_gt_rxdfelpmreset[0]),               // input wire gt_rxdfelpmreset_0
  .gt_rxdfelpmreset_1(ap_gt_rxdfelpmreset[1]),               // input wire gt_rxdfelpmreset_1
  .gt_rxdfelpmreset_2(ap_gt_rxdfelpmreset[2]),               // input wire gt_rxdfelpmreset_2
  .gt_rxdfelpmreset_3(ap_gt_rxdfelpmreset[3]),               // input wire gt_rxdfelpmreset_3  
  .gt_rxlatclk_0(1'b0),                                      // input wire gt_rxlatclk_0
  .gt_rxlatclk_1(1'b0),                                      // input wire gt_rxlatclk_1
  .gt_rxlatclk_2(1'b0),                                      // input wire gt_rxlatclk_2
  .gt_rxlatclk_3(1'b0),                                      // input wire gt_rxlatclk_3  
  .gt_rxlpmen_0(ap_gt_rxlpmen[0]),                           // input wire gt_rxlpmen_0
  .gt_rxlpmen_1(ap_gt_rxlpmen[1]),                           // input wire gt_rxlpmen_1
  .gt_rxlpmen_2(ap_gt_rxlpmen[2]),                           // input wire gt_rxlpmen_2
  .gt_rxlpmen_3(ap_gt_rxlpmen[3]),                           // input wire gt_rxlpmen_3  
  .gt_rxpcsreset_0(ap_gt_rxpcsreset[0]),                     // input wire gt_rxpcsreset_0
  .gt_rxpcsreset_1(ap_gt_rxpcsreset[1]),                     // input wire gt_rxpcsreset_1
  .gt_rxpcsreset_2(ap_gt_rxpcsreset[2]),                     // input wire gt_rxpcsreset_2
  .gt_rxpcsreset_3(ap_gt_rxpcsreset[3]),                     // input wire gt_rxpcsreset_3  
  .gt_rxpmareset_0(ap_gt_rxpmareset[0]),                     // input wire gt_rxpmareset_0
  .gt_rxpmareset_1(ap_gt_rxpmareset[1]),                     // input wire gt_rxpmareset_1
  .gt_rxpmareset_2(ap_gt_rxpmareset[2]),                     // input wire gt_rxpmareset_2
  .gt_rxpmareset_3(ap_gt_rxpmareset[3]),                     // input wire gt_rxpmareset_3  
  .gt_rxpolarity_0(rx_gt_rxpolarity[0]),                     // input wire gt_rxpolarity_0
  .gt_rxpolarity_1(rx_gt_rxpolarity[1]),                     // input wire gt_rxpolarity_1
  .gt_rxpolarity_2(rx_gt_rxpolarity[2]),                     // input wire gt_rxpolarity_2
  .gt_rxpolarity_3(rx_gt_rxpolarity[3]),                     // input wire gt_rxpolarity_3  
  .gt_rxprbscntreset_0(rx_gt_rxprbscntreset[0]),             // input wire gt_rxprbscntreset_0
  .gt_rxprbscntreset_1(rx_gt_rxprbscntreset[1]),             // input wire gt_rxprbscntreset_1
  .gt_rxprbscntreset_2(rx_gt_rxprbscntreset[2]),             // input wire gt_rxprbscntreset_2
  .gt_rxprbscntreset_3(rx_gt_rxprbscntreset[3]),             // input wire gt_rxprbscntreset_3  
  .gt_rxprbserr_0(rx_gt_rxprbserr[0]),                       // output wire gt_rxprbserr_0
  .gt_rxprbserr_1(rx_gt_rxprbserr[1]),                       // output wire gt_rxprbserr_1
  .gt_rxprbserr_2(rx_gt_rxprbserr[2]),                       // output wire gt_rxprbserr_2
  .gt_rxprbserr_3(rx_gt_rxprbserr[3]),                       // output wire gt_rxprbserr_3  
  .gt_rxprbssel_0(rx_gt_rxprbssel[0+:4]),                    // input wire [3 : 0] gt_rxprbssel_0
  .gt_rxprbssel_1(rx_gt_rxprbssel[4+:4]),                    // input wire [3 : 0] gt_rxprbssel_1
  .gt_rxprbssel_2(rx_gt_rxprbssel[8+:4]),                    // input wire [3 : 0] gt_rxprbssel_2
  .gt_rxprbssel_3(rx_gt_rxprbssel[12+:4]),                   // input wire [3 : 0] gt_rxprbssel_3  
  .gt_rxrate_0(3'd0),                                        // input wire [2 : 0] gt_rxrate_0
  .gt_rxrate_1(3'd0),                                        // input wire [2 : 0] gt_rxrate_1
  .gt_rxrate_2(3'd0),                                        // input wire [2 : 0] gt_rxrate_2
  .gt_rxrate_3(3'd0),                                        // input wire [2 : 0] gt_rxrate_3  
  .gt_rxslide_in_0(1'b0),                                    // input wire gt_rxslide_in_0
  .gt_rxslide_in_1(1'b0),                                    // input wire gt_rxslide_in_1
  .gt_rxslide_in_2(1'b0),                                    // input wire gt_rxslide_in_2
  .gt_rxslide_in_3(1'b0),                                    // input wire gt_rxslide_in_3  
  .gt_rxstartofseq_0(),                                      // output wire [1 : 0] gt_rxstartofseq_0
  .gt_rxstartofseq_1(),                                      // output wire [1 : 0] gt_rxstartofseq_1
  .gt_rxstartofseq_2(),                                      // output wire [1 : 0] gt_rxstartofseq_0
  .gt_rxstartofseq_3(),                                      // output wire [1 : 0] gt_rxstartofseq_1  
  .gt_txbufstatus_0(tx_gt_txbufstatus[0+:2]),                // output wire [1 : 0] gt_txbufstatus_0
  .gt_txbufstatus_1(tx_gt_txbufstatus[2+:2]),                // output wire [1 : 0] gt_txbufstatus_1
  .gt_txbufstatus_2(tx_gt_txbufstatus[4+:2]),                // output wire [1 : 0] gt_txbufstatus_2
  .gt_txbufstatus_3(tx_gt_txbufstatus[6+:2]),                // output wire [1 : 0] gt_txbufstatus_3  
  .gt_txdiffctrl_0(ap_gt_txdiffctrl[0+:5]),                  // input wire [4 : 0] gt_txdiffctrl_0
  .gt_txdiffctrl_1(ap_gt_txdiffctrl[5+:5]),                  // input wire [4 : 0] gt_txdiffctrl_1
  .gt_txdiffctrl_2(ap_gt_txdiffctrl[10+:5]),                 // input wire [4 : 0] gt_txdiffctrl_2
  .gt_txdiffctrl_3(ap_gt_txdiffctrl[15+:5]),                 // input wire [4 : 0] gt_txdiffctrl_3  
  .gt_txinhibit_0(tx_gt_txinhibit[0]),                       // input wire gt_txinhibit_0
  .gt_txinhibit_1(tx_gt_txinhibit[1]),                       // input wire gt_txinhibit_1
  .gt_txinhibit_2(tx_gt_txinhibit[2]),                       // input wire gt_txinhibit_2
  .gt_txinhibit_3(tx_gt_txinhibit[3]),                       // input wire gt_txinhibit_3  
  .gt_txlatclk_0(1'b0),                                      // input wire gt_txlatclk_0
  .gt_txlatclk_1(1'b0),                                      // input wire gt_txlatclk_1
  .gt_txlatclk_2(1'b0),                                      // input wire gt_txlatclk_2
  .gt_txlatclk_3(1'b0),                                      // input wire gt_txlatclk_3  
  .gt_txmaincursor_0(ap_gt_txmaincursor[0+:7]),              // input wire [6 : 0] gt_txmaincursor_0
  .gt_txmaincursor_1(ap_gt_txmaincursor[7+:7]),              // input wire [6 : 0] gt_txmaincursor_1
  .gt_txmaincursor_2(ap_gt_txmaincursor[14+:7]),             // input wire [6 : 0] gt_txmaincursor_2
  .gt_txmaincursor_3(ap_gt_txmaincursor[21+:7]),             // input wire [6 : 0] gt_txmaincursor_3  
  .gt_txpcsreset_0(ap_gt_txpcsreset[0]),                     // input wire gt_txpcsreset_0
  .gt_txpcsreset_1(ap_gt_txpcsreset[1]),                     // input wire gt_txpcsreset_1
  .gt_txpcsreset_2(ap_gt_txpcsreset[2]),                     // input wire gt_txpcsreset_2
  .gt_txpcsreset_3(ap_gt_txpcsreset[3]),                     // input wire gt_txpcsreset_3  
  .gt_txpmareset_0(ap_gt_txpmareset[0]),                     // input wire gt_txpmareset_0
  .gt_txpmareset_1(ap_gt_txpmareset[1]),                     // input wire gt_txpmareset_1
  .gt_txpmareset_2(ap_gt_txpmareset[2]),                     // input wire gt_txpmareset_2
  .gt_txpmareset_3(ap_gt_txpmareset[3]),                     // input wire gt_txpmareset_3  
  .gt_txpolarity_0(tx_gt_txpolarity[0]),                     // input wire gt_txpolarity_0
  .gt_txpolarity_1(tx_gt_txpolarity[1]),                     // input wire gt_txpolarity_1
  .gt_txpolarity_2(tx_gt_txpolarity[2]),                     // input wire gt_txpolarity_2
  .gt_txpolarity_3(tx_gt_txpolarity[3]),                     // input wire gt_txpolarity_3  
  .gt_txpostcursor_0(ap_gt_txpostcursor[0+:5]),              // input wire [4 : 0] gt_txpostcursor_0
  .gt_txpostcursor_1(ap_gt_txpostcursor[5+:5]),              // input wire [4 : 0] gt_txpostcursor_1
  .gt_txpostcursor_2(ap_gt_txpostcursor[10+:5]),             // input wire [4 : 0] gt_txpostcursor_2
  .gt_txpostcursor_3(ap_gt_txpostcursor[15+:5]),             // input wire [4 : 0] gt_txpostcursor_3  
  .gt_txprbsforceerr_0(tx_gt_txprbsforceerr[0]),             // input wire gt_txprbsforceerr_0
  .gt_txprbsforceerr_1(tx_gt_txprbsforceerr[1]),             // input wire gt_txprbsforceerr_1
  .gt_txprbsforceerr_2(tx_gt_txprbsforceerr[2]),             // input wire gt_txprbsforceerr_2
  .gt_txprbsforceerr_3(tx_gt_txprbsforceerr[3]),             // input wire gt_txprbsforceerr_3  
  .gt_txelecidle_0(tx_gt_txelecidle[0]),                     // input wire gt_txelecidle_0
  .gt_txelecidle_1(tx_gt_txelecidle[1]),                     // input wire gt_txelecidle_1
  .gt_txelecidle_2(tx_gt_txelecidle[2]),                     // input wire gt_txelecidle_2
  .gt_txelecidle_3(tx_gt_txelecidle[3]),                     // input wire gt_txelecidle_3  
  .gt_txprbssel_0(tx_gt_txprbssel[0+:4]),                    // input wire [3 : 0] gt_txprbssel_0
  .gt_txprbssel_1(tx_gt_txprbssel[4+:4]),                    // input wire [3 : 0] gt_txprbssel_1
  .gt_txprbssel_2(tx_gt_txprbssel[8+:4]),                    // input wire [3 : 0] gt_txprbssel_2
  .gt_txprbssel_3(tx_gt_txprbssel[12+:4]),                   // input wire [3 : 0] gt_txprbssel_3  
  .gt_txprecursor_0(ap_gt_txprecursor[0+:5]),                // input wire [4 : 0] gt_txprecursor_0
  .gt_txprecursor_1(ap_gt_txprecursor[5+:5]),                // input wire [4 : 0] gt_txprecursor_1
  .gt_txprecursor_2(ap_gt_txprecursor[10+:5]),               // input wire [4 : 0] gt_txprecursor_2
  .gt_txprecursor_3(ap_gt_txprecursor[15+:5]),               // input wire [4 : 0] gt_txprecursor_3  

  .gtwiz_reset_tx_datapath_0(ap_gt_gtwiztxreset[0]),         // input wire gtwiz_reset_tx_datapath_0
  .gtwiz_reset_tx_datapath_1(ap_gt_gtwiztxreset[1]),         // input wire gtwiz_reset_tx_datapath_1
  .gtwiz_reset_tx_datapath_2(ap_gt_gtwiztxreset[2]),         // input wire gtwiz_reset_tx_datapath_2
  .gtwiz_reset_tx_datapath_3(ap_gt_gtwiztxreset[3]),         // input wire gtwiz_reset_tx_datapath_3  
  .gtwiz_reset_rx_datapath_0(ap_gt_gtwizrxreset[0]),         // input wire gtwiz_reset_rx_datapath_0
  .gtwiz_reset_rx_datapath_1(ap_gt_gtwizrxreset[1]),         // input wire gtwiz_reset_rx_datapath_1
  .gtwiz_reset_rx_datapath_2(ap_gt_gtwizrxreset[2]),         // input wire gtwiz_reset_rx_datapath_2
  .gtwiz_reset_rx_datapath_3(ap_gt_gtwizrxreset[3]),         // input wire gtwiz_reset_rx_datapath_3  
  .rxrecclkout_0(),                                          // output wire rxrecclkout_0
  .rxrecclkout_1(),                                          // output wire rxrecclkout_1
  .rxrecclkout_2(),                                          // output wire rxrecclkout_2
  .rxrecclkout_3(),                                          // output wire rxrecclkout_3  

  .gt_drpclk_0(1'b0),                                        // input wire gt_drpclk_0
  .gt_drpclk_1(1'b0),                                        // input wire gt_drpclk_1
  .gt_drpclk_2(1'b0),                                        // input wire gt_drpclk_2
  .gt_drpclk_3(1'b0),                                        // input wire gt_drpclk_3  
  .gt_drprst_0(1'b0),                                        // input wire gt_drprst_0
  .gt_drprst_1(1'b0),                                        // input wire gt_drprst_1
  .gt_drprst_2(1'b0),                                        // input wire gt_drprst_2
  .gt_drprst_3(1'b0),                                        // input wire gt_drprst_3  
  .gt_drpdo_0(),                                             // output wire [15 : 0] gt_drpdo_0
  .gt_drpdo_1(),                                             // output wire [15 : 0] gt_drpdo_1
  .gt_drpdo_2(),                                             // output wire [15 : 0] gt_drpdo_2
  .gt_drpdo_3(),                                             // output wire [15 : 0] gt_drpdo_3  
  .gt_drprdy_0(),                                            // output wire gt_drprdy_0
  .gt_drprdy_1(),                                            // output wire gt_drprdy_1
  .gt_drprdy_2(),                                            // output wire gt_drprdy_2
  .gt_drprdy_3(),                                            // output wire gt_drprdy_3  
  .gt_drpen_0(1'b0),                                         // input wire gt_drpen_0
  .gt_drpen_1(1'b0),                                         // input wire gt_drpen_1
  .gt_drpen_2(1'b0),                                         // input wire gt_drpen_2
  .gt_drpen_3(1'b0),                                         // input wire gt_drpen_3  
  .gt_drpwe_0(1'b0),                                         // input wire gt_drpwe_0
  .gt_drpwe_1(1'b0),                                         // input wire gt_drpwe_1
  .gt_drpwe_2(1'b0),                                         // input wire gt_drpwe_2
  .gt_drpwe_3(1'b0),                                         // input wire gt_drpwe_3  
  .gt_drpaddr_0(10'd0),                                      // input wire [9 : 0] gt_drpaddr_0
  .gt_drpaddr_1(10'd0),                                      // input wire [9 : 0] gt_drpaddr_1
  .gt_drpaddr_2(10'd0),                                      // input wire [9 : 0] gt_drpaddr_2
  .gt_drpaddr_3(10'd0),                                      // input wire [9 : 0] gt_drpaddr_3  
  .gt_drpdi_0(16'd0),                                        // input wire [15 : 0] gt_drpdi_0
  .gt_drpdi_1(16'd0),                                        // input wire [15 : 0] gt_drpdi_1
  .gt_drpdi_2(16'd0),                                        // input wire [15 : 0] gt_drpdi_2
  .gt_drpdi_3(16'd0),                                        // input wire [15 : 0] gt_drpdi_3  

  .sys_reset(areset),                                        // input wire sys_reset
  .dclk(clk_gt_freerun),                                     // input wire dclk
  .tx_clk_out_0(tx_clk_out[0]),                              // output wire tx_clk_out_0
  .tx_clk_out_1(tx_clk_out[1]),                              // output wire tx_clk_out_1
  .tx_clk_out_2(tx_clk_out[2]),                              // output wire tx_clk_out_2
  .tx_clk_out_3(tx_clk_out[3]),                              // output wire tx_clk_out_3  
  .rx_clk_out_0(rx_clk_out[0]),                              // output wire rx_clk_out_0
  .rx_clk_out_1(rx_clk_out[1]),                              // output wire rx_clk_out_1
  .rx_clk_out_2(rx_clk_out[2]),                              // output wire rx_clk_out_2
  .rx_clk_out_3(rx_clk_out[3]),                              // output wire rx_clk_out_3  
  .gt_refclk_p(gt_refclk_p),                                 // input wire gt_refclk_p
  .gt_refclk_n(gt_refclk_n),                                 // input wire gt_refclk_n
  .gt_refclk_out(),                                          // output wire gt_refclk_out
  .gtpowergood_out_0(gtpowergood[0]),                        // output wire gtpowergood_out_0
  .gtpowergood_out_1(gtpowergood[1]),                        // output wire gtpowergood_out_1
  .gtpowergood_out_2(gtpowergood[2]),                        // output wire gtpowergood_out_2
  .gtpowergood_out_3(gtpowergood[3]),                        // output wire gtpowergood_out_3  
  .rx_reset_0(1'b0),                                         // input wire rx_reset_0
  .rx_reset_1(1'b0),                                         // input wire rx_reset_1
  .rx_reset_2(1'b0),                                         // input wire rx_reset_2
  .rx_reset_3(1'b0),                                         // input wire rx_reset_3  
  .user_rx_reset_0(rx_reset[0]),                             // output wire user_rx_reset_0
  .user_rx_reset_1(rx_reset[1]),                             // output wire user_rx_reset_1
  .user_rx_reset_2(rx_reset[2]),                             // output wire user_rx_reset_2
  .user_rx_reset_3(rx_reset[3]),                             // output wire user_rx_reset_3  
  .rx_axis_tvalid_0(axi_str_rxd_tvalid[0]),                  // output wire rx_axis_tvalid_0
  .rx_axis_tvalid_1(axi_str_rxd_tvalid[1]),                  // output wire rx_axis_tvalid_1
  .rx_axis_tvalid_2(axi_str_rxd_tvalid[2]),                  // output wire rx_axis_tvalid_2
  .rx_axis_tvalid_3(axi_str_rxd_tvalid[3]),                  // output wire rx_axis_tvalid_3  
  .rx_axis_tdata_0(axi_str_rxd_tdata[0]),                    // output wire [63 : 0] rx_axis_tdata_0
  .rx_axis_tdata_1(axi_str_rxd_tdata[1]),                    // output wire [63 : 0] rx_axis_tdata_1
  .rx_axis_tdata_2(axi_str_rxd_tdata[2]),                    // output wire [63 : 0] rx_axis_tdata_2
  .rx_axis_tdata_3(axi_str_rxd_tdata[3]),                    // output wire [63 : 0] rx_axis_tdata_3  
  .rx_axis_tlast_0(axi_str_rxd_tlast[0]),                    // output wire rx_axis_tlast_0
  .rx_axis_tlast_1(axi_str_rxd_tlast[1]),                    // output wire rx_axis_tlast_1
  .rx_axis_tlast_2(axi_str_rxd_tlast[2]),                    // output wire rx_axis_tlast_2
  .rx_axis_tlast_3(axi_str_rxd_tlast[3]),                    // output wire rx_axis_tlast_3  
  .rx_axis_tkeep_0(axi_str_rxd_tkeep[0]),                    // output wire [7 : 0] rx_axis_tkeep_0
  .rx_axis_tkeep_1(axi_str_rxd_tkeep[1]),                    // output wire [7 : 0] rx_axis_tkeep_1
  .rx_axis_tkeep_2(axi_str_rxd_tkeep[2]),                    // output wire [7 : 0] rx_axis_tkeep_2
  .rx_axis_tkeep_3(axi_str_rxd_tkeep[3]),                    // output wire [7 : 0] rx_axis_tkeep_3  
  .rx_axis_tuser_0(axi_str_rxd_tuser[0]),                    // output wire rx_axis_tuser_0
  .rx_axis_tuser_1(axi_str_rxd_tuser[1]),                    // output wire rx_axis_tuser_1
  .rx_axis_tuser_2(axi_str_rxd_tuser[2]),                    // output wire rx_axis_tuser_2
  .rx_axis_tuser_3(axi_str_rxd_tuser[3]),                    // output wire rx_axis_tuser_3  
  .ctl_rx_enable_0(1'b1),                                    // input wire ctl_rx_enable_0
  .ctl_rx_enable_1(1'b1),                                    // input wire ctl_rx_enable_1
  .ctl_rx_enable_2(1'b1),                                    // input wire ctl_rx_enable_2
  .ctl_rx_enable_3(1'b1),                                    // input wire ctl_rx_enable_3  
  .ctl_rx_check_preamble_0(1'b1),                            // input wire ctl_rx_check_preamble_0
  .ctl_rx_check_preamble_1(1'b1),                            // input wire ctl_rx_check_preamble_1
  .ctl_rx_check_preamble_2(1'b1),                            // input wire ctl_rx_check_preamble_2
  .ctl_rx_check_preamble_3(1'b1),                            // input wire ctl_rx_check_preamble_3  
  .ctl_rx_check_sfd_0(1'b1),                                 // input wire ctl_rx_check_sfd_0
  .ctl_rx_check_sfd_1(1'b1),                                 // input wire ctl_rx_check_sfd_1
  .ctl_rx_check_sfd_2(1'b1),                                 // input wire ctl_rx_check_sfd_2
  .ctl_rx_check_sfd_3(1'b1),                                 // input wire ctl_rx_check_sfd_3
  .ctl_rx_force_resync_0(1'b0),                              // input wire ctl_rx_force_resync_0
  .ctl_rx_force_resync_1(1'b0),                              // input wire ctl_rx_force_resync_1
  .ctl_rx_force_resync_2(1'b0),                              // input wire ctl_rx_force_resync_2
  .ctl_rx_force_resync_3(1'b0),                              // input wire ctl_rx_force_resync_3  
  .ctl_rx_delete_fcs_0(1'b1),                                // input wire ctl_rx_delete_fcs_0
  .ctl_rx_delete_fcs_1(1'b1),                                // input wire ctl_rx_delete_fcs_1
  .ctl_rx_delete_fcs_2(1'b1),                                // input wire ctl_rx_delete_fcs_2
  .ctl_rx_delete_fcs_3(1'b1),                                // input wire ctl_rx_delete_fcs_3  
  .ctl_rx_ignore_fcs_0(1'b0),                                // input wire ctl_rx_ignore_fcs_0
  .ctl_rx_ignore_fcs_1(1'b0),                                // input wire ctl_rx_ignore_fcs_1
  .ctl_rx_ignore_fcs_2(1'b0),                                // input wire ctl_rx_ignore_fcs_2
  .ctl_rx_ignore_fcs_3(1'b0),                                // input wire ctl_rx_ignore_fcs_3  
  .ctl_rx_max_packet_len_0(15'd9600),                        // input wire [14 : 0] ctl_rx_max_packet_len_0
  .ctl_rx_max_packet_len_1(15'd9600),                        // input wire [14 : 0] ctl_rx_max_packet_len_1
  .ctl_rx_max_packet_len_2(15'd9600),                        // input wire [14 : 0] ctl_rx_max_packet_len_2
  .ctl_rx_max_packet_len_3(15'd9600),                        // input wire [14 : 0] ctl_rx_max_packet_len_3  
  .ctl_rx_min_packet_len_0(8'd64),                           // input wire [7 : 0] ctl_rx_min_packet_len_0
  .ctl_rx_min_packet_len_1(8'd64),                           // input wire [7 : 0] ctl_rx_min_packet_len_1
  .ctl_rx_min_packet_len_2(8'd64),                           // input wire [7 : 0] ctl_rx_min_packet_len_2
  .ctl_rx_min_packet_len_3(8'd64),                           // input wire [7 : 0] ctl_rx_min_packet_len_3  
  .ctl_rx_process_lfi_0(1'b0),                               // input wire ctl_rx_process_lfi_0
  .ctl_rx_process_lfi_1(1'b0),                               // input wire ctl_rx_process_lfi_1
  .ctl_rx_process_lfi_2(1'b0),                               // input wire ctl_rx_process_lfi_2
  .ctl_rx_process_lfi_3(1'b0),                               // input wire ctl_rx_process_lfi_3  
  .ctl_rx_test_pattern_0(1'b0),                              // input wire ctl_rx_test_pattern_0
  .ctl_rx_test_pattern_1(1'b0),                              // input wire ctl_rx_test_pattern_1
  .ctl_rx_test_pattern_2(1'b0),                              // input wire ctl_rx_test_pattern_2
  .ctl_rx_test_pattern_3(1'b0),                              // input wire ctl_rx_test_pattern_3  
  .ctl_rx_data_pattern_select_0(1'b0),                       // input wire ctl_rx_data_pattern_select_0
  .ctl_rx_data_pattern_select_1(1'b0),                       // input wire ctl_rx_data_pattern_select_1
  .ctl_rx_data_pattern_select_2(1'b0),                       // input wire ctl_rx_data_pattern_select_2
  .ctl_rx_data_pattern_select_3(1'b0),                       // input wire ctl_rx_data_pattern_select_3  
  .ctl_rx_test_pattern_enable_0(1'b0),                       // input wire ctl_rx_test_pattern_enable_0
  .ctl_rx_test_pattern_enable_1(1'b0),                       // input wire ctl_rx_test_pattern_enable_1
  .ctl_rx_test_pattern_enable_2(1'b0),                       // input wire ctl_rx_test_pattern_enable_2
  .ctl_rx_test_pattern_enable_3(1'b0),                       // input wire ctl_rx_test_pattern_enable_3  
  .stat_rx_framing_err_0(),                                  // output wire stat_rx_framing_err_0
  .stat_rx_framing_err_1(),                                  // output wire stat_rx_framing_err_1
  .stat_rx_framing_err_2(),                                  // output wire stat_rx_framing_err_2
  .stat_rx_framing_err_3(),                                  // output wire stat_rx_framing_err_3  
  .stat_rx_framing_err_valid_0(),                            // output wire stat_rx_framing_err_valid_0
  .stat_rx_framing_err_valid_1(),                            // output wire stat_rx_framing_err_valid_1
  .stat_rx_framing_err_valid_2(),                            // output wire stat_rx_framing_err_valid_2
  .stat_rx_framing_err_valid_3(),                            // output wire stat_rx_framing_err_valid_3  
  .stat_rx_local_fault_0(),                                  // output wire stat_rx_local_fault_0
  .stat_rx_local_fault_1(),                                  // output wire stat_rx_local_fault_1
  .stat_rx_local_fault_2(),                                  // output wire stat_rx_local_fault_2
  .stat_rx_local_fault_3(),                                  // output wire stat_rx_local_fault_3  
  .stat_rx_block_lock_0(stat_rx_block_lock[0]),              // output wire stat_rx_block_lock_0
  .stat_rx_block_lock_1(stat_rx_block_lock[1]),              // output wire stat_rx_block_lock_1
  .stat_rx_block_lock_2(stat_rx_block_lock[2]),              // output wire stat_rx_block_lock_2
  .stat_rx_block_lock_3(stat_rx_block_lock[3]),              // output wire stat_rx_block_lock_3  
  .stat_rx_valid_ctrl_code_0(),                              // output wire stat_rx_valid_ctrl_code_0
  .stat_rx_valid_ctrl_code_1(),                              // output wire stat_rx_valid_ctrl_code_1
  .stat_rx_valid_ctrl_code_2(),                              // output wire stat_rx_valid_ctrl_code_2
  .stat_rx_valid_ctrl_code_3(),                              // output wire stat_rx_valid_ctrl_code_3  
  .stat_rx_status_0(),                                       // output wire stat_rx_status_0
  .stat_rx_status_1(),                                       // output wire stat_rx_status_1
  .stat_rx_status_2(),                                       // output wire stat_rx_status_2
  .stat_rx_status_3(),                                       // output wire stat_rx_status_3  
  .stat_rx_remote_fault_0(),                                 // output wire stat_rx_remote_fault_0
  .stat_rx_remote_fault_1(),                                 // output wire stat_rx_remote_fault_1
  .stat_rx_remote_fault_2(),                                 // output wire stat_rx_remote_fault_2
  .stat_rx_remote_fault_3(),                                 // output wire stat_rx_remote_fault_3  
  .stat_rx_bad_fcs_0(),                                      // output wire [1 : 0] stat_rx_bad_fcs_0
  .stat_rx_bad_fcs_1(),                                      // output wire [1 : 0] stat_rx_bad_fcs_1
  .stat_rx_bad_fcs_2(),                                      // output wire [1 : 0] stat_rx_bad_fcs_2
  .stat_rx_bad_fcs_3(),                                      // output wire [1 : 0] stat_rx_bad_fcs_3  
  .stat_rx_stomped_fcs_0(),                                  // output wire [1 : 0] stat_rx_stomped_fcs_0
  .stat_rx_stomped_fcs_1(),                                  // output wire [1 : 0] stat_rx_stomped_fcs_1
  .stat_rx_stomped_fcs_2(),                                  // output wire [1 : 0] stat_rx_stomped_fcs_2
  .stat_rx_stomped_fcs_3(),                                  // output wire [1 : 0] stat_rx_stomped_fcs_3  
  .stat_rx_truncated_0(),                                    // output wire stat_rx_truncated_0
  .stat_rx_truncated_1(),                                    // output wire stat_rx_truncated_1
  .stat_rx_truncated_2(),                                    // output wire stat_rx_truncated_2
  .stat_rx_truncated_3(),                                    // output wire stat_rx_truncated_3  
  .stat_rx_internal_local_fault_0(),                         // output wire stat_rx_internal_local_fault_0
  .stat_rx_internal_local_fault_1(),                         // output wire stat_rx_internal_local_fault_1
  .stat_rx_internal_local_fault_2(),                         // output wire stat_rx_internal_local_fault_2
  .stat_rx_internal_local_fault_3(),                         // output wire stat_rx_internal_local_fault_3
  .stat_rx_received_local_fault_0(),                         // output wire stat_rx_received_local_fault_0
  .stat_rx_received_local_fault_1(),                         // output wire stat_rx_received_local_fault_1
  .stat_rx_received_local_fault_2(),                         // output wire stat_rx_received_local_fault_2
  .stat_rx_received_local_fault_3(),                         // output wire stat_rx_received_local_fault_3  
  .stat_rx_hi_ber_0(),                                       // output wire stat_rx_hi_ber_0
  .stat_rx_hi_ber_1(),                                       // output wire stat_rx_hi_ber_1
  .stat_rx_hi_ber_2(),                                       // output wire stat_rx_hi_ber_2
  .stat_rx_hi_ber_3(),                                       // output wire stat_rx_hi_ber_3  
  .stat_rx_got_signal_os_0(),                                // output wire stat_rx_got_signal_os_0
  .stat_rx_got_signal_os_1(),                                // output wire stat_rx_got_signal_os_1
  .stat_rx_got_signal_os_2(),                                // output wire stat_rx_got_signal_os_2
  .stat_rx_got_signal_os_3(),                                // output wire stat_rx_got_signal_os_3  
  .stat_rx_test_pattern_mismatch_0(),                        // output wire stat_rx_test_pattern_mismatch_0
  .stat_rx_test_pattern_mismatch_1(),                        // output wire stat_rx_test_pattern_mismatch_1
  .stat_rx_test_pattern_mismatch_2(),                        // output wire stat_rx_test_pattern_mismatch_2
  .stat_rx_test_pattern_mismatch_3(),                        // output wire stat_rx_test_pattern_mismatch_3  
  .stat_rx_total_bytes_0(),                                  // output wire [3 : 0] stat_rx_total_bytes_0
  .stat_rx_total_bytes_1(),                                  // output wire [3 : 0] stat_rx_total_bytes_1
  .stat_rx_total_bytes_2(),                                  // output wire [3 : 0] stat_rx_total_bytes_2
  .stat_rx_total_bytes_3(),                                  // output wire [3 : 0] stat_rx_total_bytes_3  
  .stat_rx_total_packets_0(),                                // output wire [1 : 0] stat_rx_total_packets_0
  .stat_rx_total_packets_1(),                                // output wire [1 : 0] stat_rx_total_packets_1
  .stat_rx_total_packets_2(),                                // output wire [1 : 0] stat_rx_total_packets_2
  .stat_rx_total_packets_3(),                                // output wire [1 : 0] stat_rx_total_packets_3  
  .stat_rx_total_good_bytes_0(),                             // output wire [13 : 0] stat_rx_total_good_bytes_0
  .stat_rx_total_good_bytes_1(),                             // output wire [13 : 0] stat_rx_total_good_bytes_1
  .stat_rx_total_good_bytes_2(),                             // output wire [13 : 0] stat_rx_total_good_bytes_2
  .stat_rx_total_good_bytes_3(),                             // output wire [13 : 0] stat_rx_total_good_bytes_3  
  .stat_rx_total_good_packets_0(),                           // output wire stat_rx_total_good_packets_0
  .stat_rx_total_good_packets_1(),                           // output wire stat_rx_total_good_packets_1
  .stat_rx_total_good_packets_2(),                           // output wire stat_rx_total_good_packets_2
  .stat_rx_total_good_packets_3(),                           // output wire stat_rx_total_good_packets_3  
  .stat_rx_packet_bad_fcs_0(),                               // output wire stat_rx_packet_bad_fcs_0
  .stat_rx_packet_bad_fcs_1(),                               // output wire stat_rx_packet_bad_fcs_1
  .stat_rx_packet_bad_fcs_2(),                               // output wire stat_rx_packet_bad_fcs_2
  .stat_rx_packet_bad_fcs_3(),                               // output wire stat_rx_packet_bad_fcs_3  
  .stat_rx_packet_64_bytes_0(),                              // output wire stat_rx_packet_64_bytes_0
  .stat_rx_packet_64_bytes_1(),                              // output wire stat_rx_packet_64_bytes_1
  .stat_rx_packet_64_bytes_2(),                              // output wire stat_rx_packet_64_bytes_2
  .stat_rx_packet_64_bytes_3(),                              // output wire stat_rx_packet_64_bytes_3  
  .stat_rx_packet_65_127_bytes_0(),                          // output wire stat_rx_packet_65_127_bytes_0
  .stat_rx_packet_65_127_bytes_1(),                          // output wire stat_rx_packet_65_127_bytes_1
  .stat_rx_packet_65_127_bytes_2(),                          // output wire stat_rx_packet_65_127_bytes_2
  .stat_rx_packet_65_127_bytes_3(),                          // output wire stat_rx_packet_65_127_bytes_3  
  .stat_rx_packet_128_255_bytes_0(),                         // output wire stat_rx_packet_128_255_bytes_0
  .stat_rx_packet_128_255_bytes_1(),                         // output wire stat_rx_packet_128_255_bytes_1
  .stat_rx_packet_128_255_bytes_2(),                         // output wire stat_rx_packet_128_255_bytes_2
  .stat_rx_packet_128_255_bytes_3(),                         // output wire stat_rx_packet_128_255_bytes_3  
  .stat_rx_packet_256_511_bytes_0(),                         // output wire stat_rx_packet_256_511_bytes_0
  .stat_rx_packet_256_511_bytes_1(),                         // output wire stat_rx_packet_256_511_bytes_1
  .stat_rx_packet_256_511_bytes_2(),                         // output wire stat_rx_packet_256_511_bytes_2
  .stat_rx_packet_256_511_bytes_3(),                         // output wire stat_rx_packet_256_511_bytes_3  
  .stat_rx_packet_512_1023_bytes_0(),                        // output wire stat_rx_packet_512_1023_bytes_0
  .stat_rx_packet_512_1023_bytes_1(),                        // output wire stat_rx_packet_512_1023_bytes_1
  .stat_rx_packet_512_1023_bytes_2(),                        // output wire stat_rx_packet_512_1023_bytes_2
  .stat_rx_packet_512_1023_bytes_3(),                        // output wire stat_rx_packet_512_1023_bytes_3  
  .stat_rx_packet_1024_1518_bytes_0(),                       // output wire stat_rx_packet_1024_1518_bytes_0
  .stat_rx_packet_1024_1518_bytes_1(),                       // output wire stat_rx_packet_1024_1518_bytes_1
  .stat_rx_packet_1024_1518_bytes_2(),                       // output wire stat_rx_packet_1024_1518_bytes_2
  .stat_rx_packet_1024_1518_bytes_3(),                       // output wire stat_rx_packet_1024_1518_bytes_3  
  .stat_rx_packet_1519_1522_bytes_0(),                       // output wire stat_rx_packet_1519_1522_bytes_0
  .stat_rx_packet_1519_1522_bytes_1(),                       // output wire stat_rx_packet_1519_1522_bytes_1
  .stat_rx_packet_1519_1522_bytes_2(),                       // output wire stat_rx_packet_1519_1522_bytes_2
  .stat_rx_packet_1519_1522_bytes_3(),                       // output wire stat_rx_packet_1519_1522_bytes_3  
  .stat_rx_packet_1523_1548_bytes_0(),                       // output wire stat_rx_packet_1523_1548_bytes_0
  .stat_rx_packet_1523_1548_bytes_1(),                       // output wire stat_rx_packet_1523_1548_bytes_1
  .stat_rx_packet_1523_1548_bytes_2(),                       // output wire stat_rx_packet_1523_1548_bytes_2
  .stat_rx_packet_1523_1548_bytes_3(),                       // output wire stat_rx_packet_1523_1548_bytes_3  
  .stat_rx_packet_1549_2047_bytes_0(),                       // output wire stat_rx_packet_1549_2047_bytes_0
  .stat_rx_packet_1549_2047_bytes_1(),                       // output wire stat_rx_packet_1549_2047_bytes_1
  .stat_rx_packet_1549_2047_bytes_2(),                       // output wire stat_rx_packet_1549_2047_bytes_2
  .stat_rx_packet_1549_2047_bytes_3(),                       // output wire stat_rx_packet_1549_2047_bytes_3  
  .stat_rx_packet_2048_4095_bytes_0(),                       // output wire stat_rx_packet_2048_4095_bytes_0
  .stat_rx_packet_2048_4095_bytes_1(),                       // output wire stat_rx_packet_2048_4095_bytes_1
  .stat_rx_packet_2048_4095_bytes_2(),                       // output wire stat_rx_packet_2048_4095_bytes_2
  .stat_rx_packet_2048_4095_bytes_3(),                       // output wire stat_rx_packet_2048_4095_bytes_3  
  .stat_rx_packet_4096_8191_bytes_0(),                       // output wire stat_rx_packet_4096_8191_bytes_0
  .stat_rx_packet_4096_8191_bytes_1(),                       // output wire stat_rx_packet_4096_8191_bytes_1
  .stat_rx_packet_4096_8191_bytes_2(),                       // output wire stat_rx_packet_4096_8191_bytes_2
  .stat_rx_packet_4096_8191_bytes_3(),                       // output wire stat_rx_packet_4096_8191_bytes_3  
  .stat_rx_packet_8192_9215_bytes_0(),                       // output wire stat_rx_packet_8192_9215_bytes_0
  .stat_rx_packet_8192_9215_bytes_1(),                       // output wire stat_rx_packet_8192_9215_bytes_1
  .stat_rx_packet_8192_9215_bytes_2(),                       // output wire stat_rx_packet_8192_9215_bytes_2
  .stat_rx_packet_8192_9215_bytes_3(),                       // output wire stat_rx_packet_8192_9215_bytes_3  
  .stat_rx_packet_small_0(),                                 // output wire stat_rx_packet_small_0
  .stat_rx_packet_small_1(),                                 // output wire stat_rx_packet_small_1
  .stat_rx_packet_small_2(),                                 // output wire stat_rx_packet_small_2
  .stat_rx_packet_small_3(),                                 // output wire stat_rx_packet_small_3  
  .stat_rx_packet_large_0(),                                 // output wire stat_rx_packet_large_0
  .stat_rx_packet_large_1(),                                 // output wire stat_rx_packet_large_1
  .stat_rx_packet_large_2(),                                 // output wire stat_rx_packet_large_2
  .stat_rx_packet_large_3(),                                 // output wire stat_rx_packet_large_3  
  .stat_rx_unicast_0(),                                      // output wire stat_rx_unicast_0
  .stat_rx_unicast_1(),                                      // output wire stat_rx_unicast_1
  .stat_rx_unicast_2(),                                      // output wire stat_rx_unicast_2
  .stat_rx_unicast_3(),                                      // output wire stat_rx_unicast_3  
  .stat_rx_multicast_0(),                                    // output wire stat_rx_multicast_0
  .stat_rx_multicast_1(),                                    // output wire stat_rx_multicast_1
  .stat_rx_multicast_2(),                                    // output wire stat_rx_multicast_2
  .stat_rx_multicast_3(),                                    // output wire stat_rx_multicast_3  
  .stat_rx_broadcast_0(),                                    // output wire stat_rx_broadcast_0
  .stat_rx_broadcast_1(),                                    // output wire stat_rx_broadcast_1
  .stat_rx_broadcast_2(),                                    // output wire stat_rx_broadcast_2
  .stat_rx_broadcast_3(),                                    // output wire stat_rx_broadcast_3  
  .stat_rx_oversize_0(),                                     // output wire stat_rx_oversize_0
  .stat_rx_oversize_1(),                                     // output wire stat_rx_oversize_1
  .stat_rx_oversize_2(),                                     // output wire stat_rx_oversize_2
  .stat_rx_oversize_3(),                                     // output wire stat_rx_oversize_3  
  .stat_rx_toolong_0(),                                      // output wire stat_rx_toolong_0
  .stat_rx_toolong_1(),                                      // output wire stat_rx_toolong_1
  .stat_rx_toolong_2(),                                      // output wire stat_rx_toolong_2
  .stat_rx_toolong_3(),                                      // output wire stat_rx_toolong_3  
  .stat_rx_undersize_0(),                                    // output wire stat_rx_undersize_0
  .stat_rx_undersize_1(),                                    // output wire stat_rx_undersize_1
  .stat_rx_undersize_2(),                                    // output wire stat_rx_undersize_2
  .stat_rx_undersize_3(),                                    // output wire stat_rx_undersize_3  
  .stat_rx_fragment_0(),                                     // output wire stat_rx_fragment_0
  .stat_rx_fragment_1(),                                     // output wire stat_rx_fragment_1
  .stat_rx_fragment_2(),                                     // output wire stat_rx_fragment_2
  .stat_rx_fragment_3(),                                     // output wire stat_rx_fragment_3  
  .stat_rx_vlan_0(),                                         // output wire stat_rx_vlan_0
  .stat_rx_vlan_1(),                                         // output wire stat_rx_vlan_1
  .stat_rx_vlan_2(),                                         // output wire stat_rx_vlan_2
  .stat_rx_vlan_3(),                                         // output wire stat_rx_vlan_3  
  //.stat_rx_inrangeerr_0(),
  //.stat_rx_inrangeerr_1(),
  .stat_rx_jabber_0(),                                       // output wire stat_rx_jabber_0
  .stat_rx_jabber_1(),                                       // output wire stat_rx_jabber_1
  .stat_rx_jabber_2(),                                       // output wire stat_rx_jabber_2
  .stat_rx_jabber_3(),                                       // output wire stat_rx_jabber_3  
  .stat_rx_bad_code_0(),                                     // output wire stat_rx_bad_code_0
  .stat_rx_bad_code_1(),                                     // output wire stat_rx_bad_code_1
  .stat_rx_bad_code_2(),                                     // output wire stat_rx_bad_code_2
  .stat_rx_bad_code_3(),                                     // output wire stat_rx_bad_code_3  
  .stat_rx_bad_sfd_0(),                                      // output wire stat_rx_bad_sfd_0
  .stat_rx_bad_sfd_1(),                                      // output wire stat_rx_bad_sfd_1
  .stat_rx_bad_sfd_2(),                                      // output wire stat_rx_bad_sfd_2
  .stat_rx_bad_sfd_3(),                                      // output wire stat_rx_bad_sfd_3  
  .stat_rx_bad_preamble_0(),                                 // output wire stat_rx_bad_preamble_0
  .stat_rx_bad_preamble_1(),                                 // output wire stat_rx_bad_preamble_1
  .stat_rx_bad_preamble_2(),                                 // output wire stat_rx_bad_preamble_2
  .stat_rx_bad_preamble_3(),                                 // output wire stat_rx_bad_preamble_3  
  .tx_reset_0(1'b0),                                         // input wire tx_reset_0
  .tx_reset_1(1'b0),                                         // input wire tx_reset_1
  .tx_reset_2(1'b0),                                         // input wire tx_reset_2
  .tx_reset_3(1'b0),                                         // input wire tx_reset_3  
  .user_tx_reset_0(tx_reset[0]),                             // output wire user_tx_reset_0
  .user_tx_reset_1(tx_reset[1]),                             // output wire user_tx_reset_1
  .user_tx_reset_2(tx_reset[2]),                             // output wire user_tx_reset_2
  .user_tx_reset_3(tx_reset[3]),                             // output wire user_tx_reset_3  
  .tx_axis_tready_0(axi_str_txd_tready[0]),                  // output wire tx_axis_tready_0
  .tx_axis_tready_1(axi_str_txd_tready[1]),                  // output wire tx_axis_tready_1
  .tx_axis_tready_2(axi_str_txd_tready[2]),                  // output wire tx_axis_tready_2
  .tx_axis_tready_3(axi_str_txd_tready[3]),                  // output wire tx_axis_tready_3  
  .tx_axis_tvalid_0(axi_str_txd_tvalid[0]),                  // input wire tx_axis_tvalid_0
  .tx_axis_tvalid_1(axi_str_txd_tvalid[1]),                  // input wire tx_axis_tvalid_1
  .tx_axis_tvalid_2(axi_str_txd_tvalid[2]),                  // input wire tx_axis_tvalid_2
  .tx_axis_tvalid_3(axi_str_txd_tvalid[3]),                  // input wire tx_axis_tvalid_3  
  .tx_axis_tdata_0(axi_str_txd_tdata[0]),                    // input wire [63 : 0] tx_axis_tdata_0
  .tx_axis_tdata_1(axi_str_txd_tdata[1]),                    // input wire [63 : 0] tx_axis_tdata_1
  .tx_axis_tdata_2(axi_str_txd_tdata[2]),                    // input wire [63 : 0] tx_axis_tdata_2
  .tx_axis_tdata_3(axi_str_txd_tdata[3]),                    // input wire [63 : 0] tx_axis_tdata_3  
  .tx_axis_tlast_0(axi_str_txd_tlast[0]),                    // input wire tx_axis_tlast_0
  .tx_axis_tlast_1(axi_str_txd_tlast[1]),                    // input wire tx_axis_tlast_1
  .tx_axis_tlast_2(axi_str_txd_tlast[2]),                    // input wire tx_axis_tlast_2
  .tx_axis_tlast_3(axi_str_txd_tlast[3]),                    // input wire tx_axis_tlast_3  
  .tx_axis_tkeep_0(axi_str_txd_tkeep[0]),                    // input wire [7 : 0] tx_axis_tkeep_0
  .tx_axis_tkeep_1(axi_str_txd_tkeep[1]),                    // input wire [7 : 0] tx_axis_tkeep_1
  .tx_axis_tkeep_2(axi_str_txd_tkeep[2]),                    // input wire [7 : 0] tx_axis_tkeep_2
  .tx_axis_tkeep_3(axi_str_txd_tkeep[3]),                    // input wire [7 : 0] tx_axis_tkeep_3  
  .tx_axis_tuser_0(axi_str_txd_tuser[0][0]),                 // input wire tx_axis_tuser_0
  .tx_axis_tuser_1(axi_str_txd_tuser[1][0]),                 // input wire tx_axis_tuser_1
  .tx_axis_tuser_2(axi_str_txd_tuser[2][0]),                 // input wire tx_axis_tuser_2
  .tx_axis_tuser_3(axi_str_txd_tuser[3][0]),                 // input wire tx_axis_tuser_3  
  .tx_unfout_0(),                                            // output wire tx_unfout_0
  .tx_unfout_1(),                                            // output wire tx_unfout_1
  .tx_unfout_2(),                                            // output wire tx_unfout_2
  .tx_unfout_3(),                                            // output wire tx_unfout_3  
  //.tx_preamblein_0(56'd0),                                 // input wire [55 : 0] tx_preamblein_0
  //.rx_preambleout_0(),                                     // output wire [55 : 0] rx_preambleout_0
  //.tx_preamblein_1(56'd0),                                 // input wire [55 : 0] tx_preamblein_1
  //.rx_preambleout_1(),                                     // output wire [55 : 0] rx_preambleout_1
  //.tx_preamblein_2(56'd0),                                 // input wire [55 : 0] tx_preamblein_0
  //.rx_preambleout_2(),                                     // output wire [55 : 0] rx_preambleout_0
  //.tx_preamblein_3(56'd0),                                 // input wire [55 : 0] tx_preamblein_1
  //.rx_preambleout_3(),                                     // output wire [55 : 0] rx_preambleout_1  
  .stat_tx_local_fault_0(),                                  // output wire stat_tx_local_fault_0
  .stat_tx_local_fault_1(),                                  // output wire stat_tx_local_fault_1
  .stat_tx_local_fault_2(),                                  // output wire stat_tx_local_fault_2
  .stat_tx_local_fault_3(),                                  // output wire stat_tx_local_fault_3  
  .stat_tx_total_bytes_0(),                                  // output wire [3 : 0] stat_tx_total_bytes_0
  .stat_tx_total_bytes_1(),                                  // output wire [3 : 0] stat_tx_total_bytes_1
  .stat_tx_total_bytes_2(),                                  // output wire [3 : 0] stat_tx_total_bytes_2
  .stat_tx_total_bytes_3(),                                  // output wire [3 : 0] stat_tx_total_bytes_3  
  .stat_tx_total_packets_0(),                                // output wire stat_tx_total_packets_0
  .stat_tx_total_packets_1(),                                // output wire stat_tx_total_packets_1
  .stat_tx_total_packets_2(),                                // output wire stat_tx_total_packets_2
  .stat_tx_total_packets_3(),                                // output wire stat_tx_total_packets_3  
  .stat_tx_total_good_bytes_0(),                             // output wire [13 : 0] stat_tx_total_good_bytes_0
  .stat_tx_total_good_bytes_1(),                             // output wire [13 : 0] stat_tx_total_good_bytes_1
  .stat_tx_total_good_bytes_2(),                             // output wire [13 : 0] stat_tx_total_good_bytes_2
  .stat_tx_total_good_bytes_3(),                             // output wire [13 : 0] stat_tx_total_good_bytes_3  
  .stat_tx_total_good_packets_0(),                           // output wire stat_tx_total_good_packets_0
  .stat_tx_total_good_packets_1(),                           // output wire stat_tx_total_good_packets_1
  .stat_tx_total_good_packets_2(),                           // output wire stat_tx_total_good_packets_2
  .stat_tx_total_good_packets_3(),                           // output wire stat_tx_total_good_packets_3  
  .stat_tx_bad_fcs_0(),                                      // output wire stat_tx_bad_fcs_0
  .stat_tx_bad_fcs_1(),                                      // output wire stat_tx_bad_fcs_1
  .stat_tx_bad_fcs_2(),                                      // output wire stat_tx_bad_fcs_2
  .stat_tx_bad_fcs_3(),                                      // output wire stat_tx_bad_fcs_3  
  .stat_tx_packet_64_bytes_0(),                              // output wire stat_tx_packet_64_bytes_0
  .stat_tx_packet_64_bytes_1(),                              // output wire stat_tx_packet_64_bytes_1
  .stat_tx_packet_64_bytes_2(),                              // output wire stat_tx_packet_64_bytes_2
  .stat_tx_packet_64_bytes_3(),                              // output wire stat_tx_packet_64_bytes_3  
  .stat_tx_packet_65_127_bytes_0(),                          // output wire stat_tx_packet_65_127_bytes_0
  .stat_tx_packet_65_127_bytes_1(),                          // output wire stat_tx_packet_65_127_bytes_1
  .stat_tx_packet_65_127_bytes_2(),                          // output wire stat_tx_packet_65_127_bytes_2
  .stat_tx_packet_65_127_bytes_3(),                          // output wire stat_tx_packet_65_127_bytes_3  
  .stat_tx_packet_128_255_bytes_0(),                         // output wire stat_tx_packet_128_255_bytes_0
  .stat_tx_packet_128_255_bytes_1(),                         // output wire stat_tx_packet_128_255_bytes_1
  .stat_tx_packet_128_255_bytes_2(),                         // output wire stat_tx_packet_128_255_bytes_2
  .stat_tx_packet_128_255_bytes_3(),                         // output wire stat_tx_packet_128_255_bytes_3  
  .stat_tx_packet_256_511_bytes_0(),                         // output wire stat_tx_packet_256_511_bytes_0
  .stat_tx_packet_256_511_bytes_1(),                         // output wire stat_tx_packet_256_511_bytes_1
  .stat_tx_packet_256_511_bytes_2(),                         // output wire stat_tx_packet_256_511_bytes_2
  .stat_tx_packet_256_511_bytes_3(),                         // output wire stat_tx_packet_256_511_bytes_3  
  .stat_tx_packet_512_1023_bytes_0(),                        // output wire stat_tx_packet_512_1023_bytes_0
  .stat_tx_packet_512_1023_bytes_1(),                        // output wire stat_tx_packet_512_1023_bytes_1
  .stat_tx_packet_512_1023_bytes_2(),                        // output wire stat_tx_packet_512_1023_bytes_2
  .stat_tx_packet_512_1023_bytes_3(),                        // output wire stat_tx_packet_512_1023_bytes_3  
  .stat_tx_packet_1024_1518_bytes_0(),                       // output wire stat_tx_packet_1024_1518_bytes_0
  .stat_tx_packet_1024_1518_bytes_1(),                       // output wire stat_tx_packet_1024_1518_bytes_1
  .stat_tx_packet_1024_1518_bytes_2(),                       // output wire stat_tx_packet_1024_1518_bytes_2
  .stat_tx_packet_1024_1518_bytes_3(),                       // output wire stat_tx_packet_1024_1518_bytes_3  
  .stat_tx_packet_1519_1522_bytes_0(),                       // output wire stat_tx_packet_1519_1522_bytes_0
  .stat_tx_packet_1519_1522_bytes_1(),                       // output wire stat_tx_packet_1519_1522_bytes_1
  .stat_tx_packet_1519_1522_bytes_2(),                       // output wire stat_tx_packet_1519_1522_bytes_2
  .stat_tx_packet_1519_1522_bytes_3(),                       // output wire stat_tx_packet_1519_1522_bytes_3  
  .stat_tx_packet_1523_1548_bytes_0(),                       // output wire stat_tx_packet_1523_1548_bytes_0
  .stat_tx_packet_1523_1548_bytes_1(),                       // output wire stat_tx_packet_1523_1548_bytes_1
  .stat_tx_packet_1523_1548_bytes_2(),                       // output wire stat_tx_packet_1523_1548_bytes_2
  .stat_tx_packet_1523_1548_bytes_3(),                       // output wire stat_tx_packet_1523_1548_bytes_3  
  .stat_tx_packet_1549_2047_bytes_0(),                       // output wire stat_tx_packet_1549_2047_bytes_0
  .stat_tx_packet_1549_2047_bytes_1(),                       // output wire stat_tx_packet_1549_2047_bytes_1
  .stat_tx_packet_1549_2047_bytes_2(),                       // output wire stat_tx_packet_1549_2047_bytes_2
  .stat_tx_packet_1549_2047_bytes_3(),                       // output wire stat_tx_packet_1549_2047_bytes_3  
  .stat_tx_packet_2048_4095_bytes_0(),                       // output wire stat_tx_packet_2048_4095_bytes_0
  .stat_tx_packet_2048_4095_bytes_1(),                       // output wire stat_tx_packet_2048_4095_bytes_1
  .stat_tx_packet_2048_4095_bytes_2(),                       // output wire stat_tx_packet_2048_4095_bytes_2
  .stat_tx_packet_2048_4095_bytes_3(),                       // output wire stat_tx_packet_2048_4095_bytes_3  
  .stat_tx_packet_4096_8191_bytes_0(),                       // output wire stat_tx_packet_4096_8191_bytes_0
  .stat_tx_packet_4096_8191_bytes_1(),                       // output wire stat_tx_packet_4096_8191_bytes_1
  .stat_tx_packet_4096_8191_bytes_2(),                       // output wire stat_tx_packet_4096_8191_bytes_2
  .stat_tx_packet_4096_8191_bytes_3(),                       // output wire stat_tx_packet_4096_8191_bytes_3  
  .stat_tx_packet_8192_9215_bytes_0(),                       // output wire stat_tx_packet_8192_9215_bytes_0
  .stat_tx_packet_8192_9215_bytes_1(),                       // output wire stat_tx_packet_8192_9215_bytes_1
  .stat_tx_packet_8192_9215_bytes_2(),                       // output wire stat_tx_packet_8192_9215_bytes_2
  .stat_tx_packet_8192_9215_bytes_3(),                       // output wire stat_tx_packet_8192_9215_bytes_3  
  .stat_tx_packet_small_0(),                                 // output wire stat_tx_packet_small_0
  .stat_tx_packet_small_1(),                                 // output wire stat_tx_packet_small_1
  .stat_tx_packet_small_2(),                                 // output wire stat_tx_packet_small_2
  .stat_tx_packet_small_3(),                                 // output wire stat_tx_packet_small_3  
  .stat_tx_packet_large_0(),                                 // output wire stat_tx_packet_large_0
  .stat_tx_packet_large_1(),                                 // output wire stat_tx_packet_large_1
  .stat_tx_packet_large_2(),                                 // output wire stat_tx_packet_large_2
  .stat_tx_packet_large_3(),                                 // output wire stat_tx_packet_large_3  
  .stat_tx_unicast_0(),                                      // output wire stat_tx_unicast_0
  .stat_tx_unicast_1(),                                      // output wire stat_tx_unicast_1
  .stat_tx_unicast_2(),                                      // output wire stat_tx_unicast_2
  .stat_tx_unicast_3(),                                      // output wire stat_tx_unicast_3  
  .stat_tx_multicast_0(),                                    // output wire stat_tx_multicast_0
  .stat_tx_multicast_1(),                                    // output wire stat_tx_multicast_1
  .stat_tx_multicast_2(),                                    // output wire stat_tx_multicast_2
  .stat_tx_multicast_3(),                                    // output wire stat_tx_multicast_3  
  .stat_tx_broadcast_0(),                                    // output wire stat_tx_broadcast_0
  .stat_tx_broadcast_1(),                                    // output wire stat_tx_broadcast_1
  .stat_tx_broadcast_2(),                                    // output wire stat_tx_broadcast_2
  .stat_tx_broadcast_3(),                                    // output wire stat_tx_broadcast_3  
  .stat_tx_vlan_0(),                                         // output wire stat_tx_vlan_0
  .stat_tx_vlan_1(),                                         // output wire stat_tx_vlan_1
  .stat_tx_vlan_2(),                                         // output wire stat_tx_vlan_2
  .stat_tx_vlan_3(),                                         // output wire stat_tx_vlan_3  
  .stat_tx_frame_error_0(),                                  // output wire stat_tx_frame_error_0
  .stat_tx_frame_error_1(),                                  // output wire stat_tx_frame_error_1
  .stat_tx_frame_error_2(),                                  // output wire stat_tx_frame_error_2
  .stat_tx_frame_error_3(),                                  // output wire stat_tx_frame_error_3  
  .ctl_tx_enable_0(1'b1),                                    // input wire ctl_tx_enable_0
  .ctl_tx_enable_1(1'b1),                                    // input wire ctl_tx_enable_1
  .ctl_tx_enable_2(1'b1),                                    // input wire ctl_tx_enable_2
  .ctl_tx_enable_3(1'b1),                                    // input wire ctl_tx_enable_3  
  .ctl_tx_send_rfi_0(1'b0),                                  // input wire ctl_tx_send_rfi_0
  .ctl_tx_send_rfi_1(1'b0),                                  // input wire ctl_tx_send_rfi_1
  .ctl_tx_send_rfi_2(1'b0),                                  // input wire ctl_tx_send_rfi_2
  .ctl_tx_send_rfi_3(1'b0),                                  // input wire ctl_tx_send_rfi_1  
  .ctl_tx_send_lfi_0(1'b0),                                  // input wire ctl_tx_send_lfi_0
  .ctl_tx_send_lfi_1(1'b0),                                  // input wire ctl_tx_send_lfi_1
  .ctl_tx_send_lfi_2(1'b0),                                  // input wire ctl_tx_send_lfi_2
  .ctl_tx_send_lfi_3(1'b0),                                  // input wire ctl_tx_send_lfi_3  
  .ctl_tx_send_idle_0(1'b0),                                 // input wire ctl_tx_send_idle_0
  .ctl_tx_send_idle_1(1'b0),                                 // input wire ctl_tx_send_idle_1
  .ctl_tx_send_idle_2(1'b0),                                 // input wire ctl_tx_send_idle_2
  .ctl_tx_send_idle_3(1'b0),                                 // input wire ctl_tx_send_idle_1  
  .ctl_tx_fcs_ins_enable_0(1'b1),                            // input wire ctl_tx_fcs_ins_enable_0
  .ctl_tx_fcs_ins_enable_1(1'b1),                            // input wire ctl_tx_fcs_ins_enable_1
  .ctl_tx_fcs_ins_enable_2(1'b1),                            // input wire ctl_tx_fcs_ins_enable_2
  .ctl_tx_fcs_ins_enable_3(1'b1),                            // input wire ctl_tx_fcs_ins_enable_3  
  .ctl_tx_ignore_fcs_0(1'b0),                                // input wire ctl_tx_ignore_fcs_0
  .ctl_tx_ignore_fcs_1(1'b0),                                // input wire ctl_tx_ignore_fcs_1
  .ctl_tx_ignore_fcs_2(1'b0),                                // input wire ctl_tx_ignore_fcs_2
  .ctl_tx_ignore_fcs_3(1'b0),                                // input wire ctl_tx_ignore_fcs_3  
  .ctl_tx_test_pattern_0(1'b0),                              // input wire ctl_tx_test_pattern_0
  .ctl_tx_test_pattern_1(1'b0),                              // input wire ctl_tx_test_pattern_1
  .ctl_tx_test_pattern_2(1'b0),                              // input wire ctl_tx_test_pattern_2
  .ctl_tx_test_pattern_3(1'b0),                              // input wire ctl_tx_test_pattern_3  
  .ctl_tx_test_pattern_enable_0(1'b0),                       // input wire ctl_tx_test_pattern_enable_0
  .ctl_tx_test_pattern_enable_1(1'b0),                       // input wire ctl_tx_test_pattern_enable_1
  .ctl_tx_test_pattern_enable_2(1'b0),                       // input wire ctl_tx_test_pattern_enable_2
  .ctl_tx_test_pattern_enable_3(1'b0),                       // input wire ctl_tx_test_pattern_enable_3  
  .ctl_tx_test_pattern_select_0(1'b0),                       // input wire ctl_tx_test_pattern_select_0
  .ctl_tx_test_pattern_select_1(1'b0),                       // input wire ctl_tx_test_pattern_select_1
  .ctl_tx_test_pattern_select_2(1'b0),                       // input wire ctl_tx_test_pattern_select_2
  .ctl_tx_test_pattern_select_3(1'b0),                       // input wire ctl_tx_test_pattern_select_3  
  .ctl_tx_data_pattern_select_0(1'b0),                       // input wire ctl_tx_data_pattern_select_0
  .ctl_tx_data_pattern_select_1(1'b0),                       // input wire ctl_tx_data_pattern_select_1
  .ctl_tx_data_pattern_select_2(1'b0),                       // input wire ctl_tx_data_pattern_select_2
  .ctl_tx_data_pattern_select_3(1'b0),                       // input wire ctl_tx_data_pattern_select_3  
  .ctl_tx_test_pattern_seed_a_0(58'd0),                      // input wire [57 : 0] ctl_tx_test_pattern_seed_a_0
  .ctl_tx_test_pattern_seed_a_1(58'd0),                      // input wire [57 : 0] ctl_tx_test_pattern_seed_a_1
  .ctl_tx_test_pattern_seed_a_2(58'd0),                      // input wire [57 : 0] ctl_tx_test_pattern_seed_a_2
  .ctl_tx_test_pattern_seed_a_3(58'd0),                      // input wire [57 : 0] ctl_tx_test_pattern_seed_a_3  
  .ctl_tx_test_pattern_seed_b_0(58'd0),                      // input wire [57 : 0] ctl_tx_test_pattern_seed_b_0
  .ctl_tx_test_pattern_seed_b_1(58'd0),                      // input wire [57 : 0] ctl_tx_test_pattern_seed_b_1
  .ctl_tx_test_pattern_seed_b_2(58'd0),                      // input wire [57 : 0] ctl_tx_test_pattern_seed_b_2
  .ctl_tx_test_pattern_seed_b_3(58'd0),                      // input wire [57 : 0] ctl_tx_test_pattern_seed_b_3  
  //.ctl_tx_ipg_value_0(4'd12),                              // input wire [3 : 0] ctl_tx_ipg_value_0
  //.ctl_tx_ipg_value_1(4'd12),                              // input wire [3 : 0] ctl_tx_ipg_value_1
  //.ctl_tx_custom_preamble_enable_0(1'b0),                  // input wire ctl_tx_custom_preamble_enable_0
  //.ctl_tx_custom_preamble_enable_1(1'b0),                  // input wire ctl_tx_custom_preamble_enable_1
  .gt_loopback_in_0(ap_gt_loopback[0+:3]),                   // input wire [2 : 0] gt_loopback_in_0
  .gt_loopback_in_1(ap_gt_loopback[3+:3]),                   // input wire [2 : 0] gt_loopback_in_1
  .gt_loopback_in_2(ap_gt_loopback[6+:3]),                   // input wire [2 : 0] gt_loopback_in_2
  .gt_loopback_in_3(ap_gt_loopback[9+:3]),                   // input wire [2 : 0] gt_loopback_in_3  
  .qpllreset_in_0(1'b0)
);

assign  rx0_axis_tvalid     = rx_axis_tvalid_i[0];
assign  rx0_axis_tdata      = rx_axis_tdata_i[0];
assign  rx0_axis_tlast      = rx_axis_tlast_i[0];
assign  rx0_axis_tkeep      = rx_axis_tkeep_i[0];
assign  rx_axis_tready_i[0] = rx0_axis_tready;
assign  rx0_axis_tuser      = rx_axis_tuser_i[0];

assign  rx1_axis_tvalid     = rx_axis_tvalid_i[1];
assign  rx1_axis_tdata      = rx_axis_tdata_i[1];
assign  rx1_axis_tlast      = rx_axis_tlast_i[1];
assign  rx1_axis_tkeep      = rx_axis_tkeep_i[1];
assign  rx_axis_tready_i[1] = rx1_axis_tready;
assign  rx1_axis_tuser      = rx_axis_tuser_i[1];

assign  rx2_axis_tvalid     = rx_axis_tvalid_i[2];
assign  rx2_axis_tdata      = rx_axis_tdata_i[2];
assign  rx2_axis_tlast      = rx_axis_tlast_i[2];
assign  rx2_axis_tkeep      = rx_axis_tkeep_i[2];
assign  rx_axis_tready_i[2] = rx2_axis_tready;
assign  rx2_axis_tuser      = rx_axis_tuser_i[2];  

assign  rx3_axis_tvalid     = rx_axis_tvalid_i[3];
assign  rx3_axis_tdata      = rx_axis_tdata_i[3];
assign  rx3_axis_tlast      = rx_axis_tlast_i[3];
assign  rx3_axis_tkeep      = rx_axis_tkeep_i[3];
assign  rx_axis_tready_i[3] = rx3_axis_tready;
assign  rx3_axis_tuser      = rx_axis_tuser_i[3];    

assign  tx0_axis_tready     = tx_axis_tready_i[0];
assign  tx_axis_tvalid_i[0] = tx0_axis_tvalid;
assign  tx_axis_tdata_i[0]  = tx0_axis_tdata;
assign  tx_axis_tlast_i[0]  = tx0_axis_tlast;
assign  tx_axis_tkeep_i[0]  = tx0_axis_tkeep;
assign  tx_axis_tuser_i[0]  = tx0_axis_tuser;

assign  tx1_axis_tready     = tx_axis_tready_i[1];
assign  tx_axis_tvalid_i[1] = tx1_axis_tvalid;
assign  tx_axis_tdata_i[1]  = tx1_axis_tdata;
assign  tx_axis_tlast_i[1]  = tx1_axis_tlast;
assign  tx_axis_tkeep_i[1]  = tx1_axis_tkeep;
assign  tx_axis_tuser_i[1]  = tx1_axis_tuser;  

assign  tx2_axis_tready     = tx_axis_tready_i[2];
assign  tx_axis_tvalid_i[2] = tx2_axis_tvalid;
assign  tx_axis_tdata_i[2]  = tx2_axis_tdata;
assign  tx_axis_tlast_i[2]  = tx2_axis_tlast;
assign  tx_axis_tkeep_i[2]  = tx2_axis_tkeep;
assign  tx_axis_tuser_i[2]  = tx2_axis_tuser;

assign  tx3_axis_tready     = tx_axis_tready_i[3];
assign  tx_axis_tvalid_i[3] = tx3_axis_tvalid;
assign  tx_axis_tdata_i[3]  = tx3_axis_tdata;
assign  tx_axis_tlast_i[3]  = tx3_axis_tlast;
assign  tx_axis_tkeep_i[3]  = tx3_axis_tkeep;
assign  tx_axis_tuser_i[3]  = tx3_axis_tuser;   



assign debug_port_low_latency [0]      = tx0_axis_tvalid;
assign debug_port_low_latency [1]      = tx0_axis_tready;
assign debug_port_low_latency [2]      = tx0_axis_tlast;
assign debug_port_low_latency [10:3]   = tx0_axis_tkeep;
assign debug_port_low_latency [11]     = tx0_axis_tuser;
assign debug_port_low_latency [75:12]  = tx0_axis_tdata;


endmodule
`default_nettype wire
