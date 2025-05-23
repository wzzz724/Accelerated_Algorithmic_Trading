//
`timescale 1ns/1ps
module ethernet_control_s_axi
#(parameter
    C_S_AXI_ADDR_WIDTH = 10,
    C_S_AXI_DATA_WIDTH = 32,
    C_NUM_GT = 4
)(
    input  wire                               ACLK,
    input  wire                               ARESET,
    input  wire                               ACLK_EN,
    input  wire   [C_S_AXI_ADDR_WIDTH-1:0]    AWADDR,
    input  wire                               AWVALID,
    output wire                               AWREADY,
    input  wire   [C_S_AXI_DATA_WIDTH-1:0]    WDATA,
    input  wire [C_S_AXI_DATA_WIDTH/8-1:0]    WSTRB,
    input  wire                               WVALID,
    output wire                               WREADY,
    output wire                      [1:0]    BRESP,
    output wire                               BVALID,
    input  wire                               BREADY,
    input  wire   [C_S_AXI_ADDR_WIDTH-1:0]    ARADDR,
    input  wire                               ARVALID,
    output wire                               ARREADY,
    output wire   [C_S_AXI_DATA_WIDTH-1:0]    RDATA,
    output wire                      [1:0]    RRESP,
    output wire                               RVALID,
    input  wire                               RREADY,
    input  logic            [C_NUM_GT-1:0]    rx_block_lock,
    input  logic            [C_NUM_GT-1:0]    rx_block_lock_ll,
    output logic            [C_NUM_GT-1:0]    rx_block_lock_clr = '0,
    input  logic         [C_NUM_GT*32-1:0]    cnt_rx_overflow,
    input  logic         [C_NUM_GT*32-1:0]    cnt_rx_unicastdrop,
    input  logic         [C_NUM_GT*32-1:0]    cnt_rx_multicastdrop,
    input  logic         [C_NUM_GT*32-1:0]    cnt_rx_oversizedrop,
    input  logic         [C_NUM_GT*32-1:0]    cnt_tx_underflow,
    input  logic            [C_NUM_GT-1:0]    tx_fifo_full,
    input  logic            [C_NUM_GT-1:0]    tx_fifo_full_lh,
    output logic            [C_NUM_GT-1:0]    tx_fifo_full_clr,
    input  logic            [C_NUM_GT-1:0]    rx_datafifo_overflow,
    input  logic            [C_NUM_GT-1:0]    rx_cmdfifo_overflow,
    input  logic            [C_NUM_GT-1:0]    rx_datafifo_overflow_lh,
    input  logic            [C_NUM_GT-1:0]    rx_cmdfifo_overflow_lh,    
    output logic            [C_NUM_GT-1:0]    rx_datafifo_overflow_clr = '0,
    output logic            [C_NUM_GT-1:0]    rx_cmdfifo_overflow_clr = '0,
    output logic                              reset_out = 0,
    output logic         [C_NUM_GT*48-1:0]    unicast_mac_addr = '0,
    output logic            [C_NUM_GT-1:0]    rx_fifo_reset = '0,
    output logic            [C_NUM_GT-1:0]    rx_traf_proc_reset = '0,
    output logic            [C_NUM_GT-1:0]    tx_fifo_reset = '0,
    output logic            [C_NUM_GT-1:0]    tx_traf_proc_reset = '0,    
    output logic            [C_NUM_GT-1:0]    unicast_promiscuous_en = '1,    
    output logic            [C_NUM_GT-1:0]    multicast_promiscuous_en = '1,
    output logic            [C_NUM_GT-1:0]    cut_through_en = '0,
    output logic          [C_NUM_GT*8-1:0]    tx_threshold = '0,
    output logic          [C_NUM_GT*3-1:0]    gt_rxoutclksel = {C_NUM_GT{3'd5}},
    output logic          [C_NUM_GT*3-1:0]    gt_txoutclksel = {C_NUM_GT{3'd5}},
    output logic          [C_NUM_GT*3-1:0]    gt_loopback = '0,
    output logic            [C_NUM_GT-1:0]    gt_rxpolarity = '0,
    output logic            [C_NUM_GT-1:0]    gt_txpolarity = '0,
    output logic          [C_NUM_GT*4-1:0]    gt_txprbssel = '0,
    output logic            [C_NUM_GT-1:0]    gt_txprbsforceerr = '0,
    output logic          [C_NUM_GT*4-1:0]    gt_rxprbssel = '0,
    output logic            [C_NUM_GT-1:0]    gt_rxprbscntreset = '0,    
    input  logic            [C_NUM_GT-1:0]    gt_rxprbslocked,
    input  logic            [C_NUM_GT-1:0]    gt_rxprbserr,
    input  logic            [C_NUM_GT-1:0]    gt_rxprbserr_lh,
    output logic            [C_NUM_GT-1:0]    gt_rxprbserr_clr = '0,  
    output logic            [C_NUM_GT-1:0]    gt_txpmareset = '0,
    output logic            [C_NUM_GT-1:0]    gt_txpcsreset = '0,
    output logic            [C_NUM_GT-1:0]    gt_txelecidle = '0,
    input  logic          [C_NUM_GT*2-1:0]    gt_txbufstatus,
    input  logic            [C_NUM_GT-1:0]    gt_gtpowergood,
    input  logic            [C_NUM_GT-1:0]    gt_gtpowergood_ll,
    output logic            [C_NUM_GT-1:0]    gt_gtpowergood_clr = '0,
    output logic            [C_NUM_GT-1:0]    gt_rxpmareset = '0,
    output logic            [C_NUM_GT-1:0]    gt_rxpcsreset = '0,
    output logic            [C_NUM_GT-1:0]    gt_rxbufreset = '0,
    output logic            [C_NUM_GT-1:0]    gt_eyescanreset = '0,
    output logic            [C_NUM_GT-1:0]    gt_gtwizrxreset = '0,
    output logic            [C_NUM_GT-1:0]    gt_gtwiztxreset = '0,
    input  logic          [C_NUM_GT*3-1:0]    gt_rxbufstatus,
    input  logic            [C_NUM_GT-1:0]    gt_rxbufstatus_underflow_lh,
    input  logic            [C_NUM_GT-1:0]    gt_rxbufstatus_overflow_lh,
    output logic            [C_NUM_GT-1:0]    gt_rxbufstatus_underflow_clr = '0,
    output logic            [C_NUM_GT-1:0]    gt_rxbufstatus_overflow_clr = '0,
    output logic          [C_NUM_GT*5-1:0]    gt_txdiffctrl = {C_NUM_GT{5'h18}},
    output logic          [C_NUM_GT*7-1:0]    gt_txmaincursor = {C_NUM_GT{7'h50}},
    output logic          [C_NUM_GT*5-1:0]    gt_txpostcursor = {C_NUM_GT{5'd0}},
    output logic          [C_NUM_GT*5-1:0]    gt_txprecursor = {C_NUM_GT{5'd0}},
    output logic            [C_NUM_GT-1:0]    gt_txinhibit = '0,
    output logic            [C_NUM_GT-1:0]    gt_rxlpmen = '1,
    output logic            [C_NUM_GT-1:0]    gt_rxdfelpmreset = '0,
    input  logic            [C_NUM_GT-1:0]    gt_eyescandataerror_lh,
    output logic            [C_NUM_GT-1:0]    gt_eyescantrigger = '0,
    output logic            [C_NUM_GT-1:0]    gt_eyescandataerror_clr = '0
);
//------------------------Parameter----------------------
localparam
    KERNEL_CFG0              = 'h10,
    CHAN0_MAC_L              = 'h20,
    CHAN0_MAC_U              = 'h24,
    CHAN0_RESET_CTRL         = 'h28,
    CHAN0_CFG                = 'h2C,
    CHAN0_THRES              = 'h30,
    CHAN1_MAC_L              = 'h40,
    CHAN1_MAC_U              = 'h44,
    CHAN1_RESET_CTRL         = 'h48,
    CHAN1_CFG                = 'h4C,
    CHAN1_THRES              = 'h50,
    CHAN0_STAT               = 'h80,
    CHAN0_RXTRAFPROCSTAT     = 'h84,
    CHAN0_TXTRAFPROCSTAT     = 'h88,
    CHAN0_CNTRXOVERFLOW      = 'h90,
    CHAN0_CNTRXUNICASTDROP   = 'h94,
    CHAN0_CNTRXMULTICASTDROP = 'h98,
    CHAN0_CNTRXOVERSIZEDROP  = 'h9C,
    CHAN0_CNTTXUNDERFLOW     = 'hA0,    
    CHAN1_STAT               = 'hC0, 
    CHAN1_RXTRAFPROCSTAT     = 'hC4,
    CHAN1_TXTRAFPROCSTAT     = 'hC8,
    CHAN1_CNTRXOVERFLOW      = 'hD0,    
    CHAN1_CNTRXUNICASTDROP   = 'hD4,
    CHAN1_CNTRXMULTICASTDROP = 'hD8,    
    CHAN1_CNTRXOVERSIZEDROP  = 'hDC,
    CHAN1_CNTTXUNDERFLOW     = 'hE0,  
    GT0_CLKSEL               = 'h100,
    GT0_LOOPBACK             = 'h104,
    GT0_POLARITY             = 'h108,
    GT0_TXPRBS_CTRL          = 'h10C,
    GT0_RXPRBS_CTRL          = 'h110,
    GT0_RXPRBS_STAT          = 'h114,
    GT0_TXRESET_CTRL         = 'h118,
    GT0_TXRESET_STAT         = 'h11C,
    GT0_RXRESET_CTRL         = 'h120,
    GT0_RXRESET_STAT         = 'h124,
    GT0_TXDRIVER_CTRL        = 'h128,
    GT0_RXEQ_CTRL            = 'h12C,
    GT0_RXMARGIN             = 'h130,
    GT1_CLKSEL               = 'h140,
    GT1_LOOPBACK             = 'h144,
    GT1_POLARITY             = 'h148,
    GT1_TXPRBS_CTRL          = 'h14C,
    GT1_RXPRBS_CTRL          = 'h150,
    GT1_RXPRBS_STAT          = 'h154,
    GT1_TXRESET_CTRL         = 'h158,
    GT1_TXRESET_STAT         = 'h15C,
    GT1_RXRESET_CTRL         = 'h160,
    GT1_RXRESET_STAT         = 'h164,
    GT1_TXDRIVER_CTRL        = 'h168,
    GT1_RXEQ_CTRL            = 'h16C,
    GT1_RXMARGIN             = 'h170,
    CHAN2_MAC_L              = 'h180,
    CHAN2_MAC_U              = 'h184,
    CHAN2_RESET_CTRL         = 'h188,
    CHAN2_CFG                = 'h18C,
    CHAN2_THRES              = 'h190,
    CHAN3_MAC_L              = 'h1A0,
    CHAN3_MAC_U              = 'h1A4,
    CHAN3_RESET_CTRL         = 'h1A8,
    CHAN3_CFG                = 'h1AC,
    CHAN3_THRES              = 'h1B0,
    CHAN2_STAT               = 'h1E0,
    CHAN2_RXTRAFPROCSTAT     = 'h1E4,
    CHAN2_TXTRAFPROCSTAT     = 'h1E8,
    CHAN2_CNTRXOVERFLOW      = 'h1F0,
    CHAN2_CNTRXUNICASTDROP   = 'h1F4,
    CHAN2_CNTRXMULTICASTDROP = 'h1F8,
    CHAN2_CNTRXOVERSIZEDROP  = 'h1FC,
    CHAN2_CNTTXUNDERFLOW     = 'h200,    
    CHAN3_STAT               = 'h220, 
    CHAN3_RXTRAFPROCSTAT     = 'h224,
    CHAN3_TXTRAFPROCSTAT     = 'h228,
    CHAN3_CNTRXOVERFLOW      = 'h230,    
    CHAN3_CNTRXUNICASTDROP   = 'h234,
    CHAN3_CNTRXMULTICASTDROP = 'h238,    
    CHAN3_CNTRXOVERSIZEDROP  = 'h23C,
    CHAN3_CNTTXUNDERFLOW     = 'h240,  
    GT2_CLKSEL               = 'h260,
    GT2_LOOPBACK             = 'h264,
    GT2_POLARITY             = 'h268,
    GT2_TXPRBS_CTRL          = 'h26C,
    GT2_RXPRBS_CTRL          = 'h270,
    GT2_RXPRBS_STAT          = 'h274,
    GT2_TXRESET_CTRL         = 'h278,
    GT2_TXRESET_STAT         = 'h27C,
    GT2_RXRESET_CTRL         = 'h280,
    GT2_RXRESET_STAT         = 'h284,
    GT2_TXDRIVER_CTRL        = 'h288,
    GT2_RXEQ_CTRL            = 'h28C,
    GT2_RXMARGIN             = 'h290,
    GT3_CLKSEL               = 'h2A0,
    GT3_LOOPBACK             = 'h2A4,
    GT3_POLARITY             = 'h2A8,
    GT3_TXPRBS_CTRL          = 'h2AC,
    GT3_RXPRBS_CTRL          = 'h2B0,
    GT3_RXPRBS_STAT          = 'h2B4,
    GT3_TXRESET_CTRL         = 'h2B8,
    GT3_TXRESET_STAT         = 'h2BC,
    GT3_RXRESET_CTRL         = 'h2C0,
    GT3_RXRESET_STAT         = 'h2C4,
    GT3_TXDRIVER_CTRL        = 'h2C8,
    GT3_RXEQ_CTRL            = 'h2CC,
    GT3_RXMARGIN             = 'h2D0, 
    WRIDLE                   = 2'd0,
    WRDATA                   = 2'd1,
    WRRESP                   = 2'd2,
    WRRESET                  = 2'd3,
    RDIDLE                   = 2'd0,
    RDDATA                   = 2'd1,
    RDRESET                  = 2'd2,
    ADDR_BITS                = C_S_AXI_ADDR_WIDTH;

//------------------------Local signal-------------------
    logic  [1:0]                   wstate = WRRESET;
    logic  [1:0]                   wnext;
    logic  [ADDR_BITS-1:0]         waddr = '0;
    logic [31:0]                   wmask;
    logic                          aw_hs;
    logic                          w_hs;
    logic  [1:0]                   rstate = RDRESET;
    logic  [1:0]                   rnext;
    logic [31:0]                   rdata = '0;
    logic                          ar_hs;
    logic [ADDR_BITS-1:0]          raddr;

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
always_ff @(posedge ACLK) begin
    if (ARESET)
        wstate <= WRRESET;
    else if (ACLK_EN)
        wstate <= wnext;
end

// wnext
always_comb begin
    wnext = WRIDLE;
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
assign RVALID  = (rstate == RDDATA);
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
    rnext = RDIDLE;
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
            rdata <= '0;
            case (raddr)
                KERNEL_CFG0 : begin
                    rdata[7:0] <= {7'd0, reset_out};
                    // Indicates that kernel is 4-channel
                    rdata[8] <= (C_NUM_GT == 4);
                    rdata[31:9] <= 23'd0;
                end
                CHAN0_MAC_L : begin
                    rdata <= unicast_mac_addr[0+:32];
                end
                CHAN0_MAC_U : begin
                    rdata[0+:16] <= unicast_mac_addr[32+:16];
                end                
                CHAN0_RESET_CTRL : begin
                    rdata[0] <= rx_fifo_reset[0];
                    rdata[1] <= rx_traf_proc_reset[0];
                    rdata[16] <= tx_fifo_reset[0];
                    rdata[17] <= tx_traf_proc_reset[0];
                end
                CHAN0_CFG : begin
                    rdata[0] <= unicast_promiscuous_en[0];
                    rdata[1] <= multicast_promiscuous_en[0];
                    rdata[2] <= cut_through_en[0];
                end
                CHAN0_THRES : begin
                    rdata[0+:8] <= tx_threshold[0+:8];
                end                   
                CHAN1_MAC_L : begin
                    rdata <= unicast_mac_addr[48+:32];
                end
                CHAN1_MAC_U : begin
                    rdata[0+:16] <= unicast_mac_addr[80+:16];
                end                
                CHAN1_RESET_CTRL : begin
                    rdata[0] <= rx_fifo_reset[1];
                    rdata[1] <= rx_traf_proc_reset[1];
                    rdata[16] <= tx_fifo_reset[1];
                    rdata[17] <= tx_traf_proc_reset[1];
                end
                CHAN1_CFG : begin
                    rdata[0] <= unicast_promiscuous_en[1];
                    rdata[1] <= multicast_promiscuous_en[1];
                    rdata[2] <= cut_through_en[1];
                end
                CHAN1_THRES : begin
                    rdata[0+:8] <= tx_threshold[8+:8];
                end
                CHAN2_MAC_L : begin
                    rdata <= unicast_mac_addr[96+:32];
                end
                CHAN2_MAC_U : begin
                    rdata[0+:16] <= unicast_mac_addr[128+:16];
                end                
                CHAN2_RESET_CTRL : begin
                    rdata[0] <= rx_fifo_reset[2];
                    rdata[1] <= rx_traf_proc_reset[2];
                    rdata[16] <= tx_fifo_reset[2];
                    rdata[17] <= tx_traf_proc_reset[2];
                end
                CHAN2_CFG : begin
                    rdata[0] <= unicast_promiscuous_en[2];
                    rdata[1] <= multicast_promiscuous_en[2];
                    rdata[2] <= cut_through_en[2];
                end
                CHAN2_THRES : begin
                    rdata[0+:8] <= tx_threshold[16+:8];
                end
                CHAN3_MAC_L : begin
                    rdata <= unicast_mac_addr[144+:32];
                end
                CHAN3_MAC_U : begin
                    rdata[0+:16] <= unicast_mac_addr[176+:16];
                end                
                CHAN3_RESET_CTRL : begin
                    rdata[0] <= rx_fifo_reset[3];
                    rdata[1] <= rx_traf_proc_reset[3];
                    rdata[16] <= tx_fifo_reset[3];
                    rdata[17] <= tx_traf_proc_reset[3];
                end
                CHAN3_CFG : begin
                    rdata[0] <= unicast_promiscuous_en[3];
                    rdata[1] <= multicast_promiscuous_en[3];
                    rdata[2] <= cut_through_en[3];
                end
                CHAN3_THRES : begin
                    rdata[0+:8] <= tx_threshold[24+:8];
                end                       
                CHAN0_STAT : begin
                    rdata[0] <= rx_block_lock[0];
                    rdata[16] <= rx_block_lock_ll[0];
                end      
                CHAN0_RXTRAFPROCSTAT : begin
                    rdata[0] <= rx_datafifo_overflow[0];
                    rdata[1] <= rx_cmdfifo_overflow[0];
                    rdata[16] <= rx_datafifo_overflow_lh[0];
                    rdata[17] <= rx_cmdfifo_overflow_lh[0];
                end
                CHAN0_TXTRAFPROCSTAT : begin
                    rdata[0] <= tx_fifo_full[0];
                    rdata[16] <= tx_fifo_full_lh[0];
                end                
                CHAN0_CNTRXOVERFLOW : begin
                    rdata <= cnt_rx_overflow[0+:32];
                end
                CHAN0_CNTRXUNICASTDROP : begin
                    rdata <= cnt_rx_unicastdrop[0+:32];
                end
                CHAN0_CNTRXMULTICASTDROP : begin
                    rdata <= cnt_rx_multicastdrop[0+:32];
                end
                CHAN0_CNTRXOVERSIZEDROP : begin
                    rdata <= cnt_rx_oversizedrop[0+:32];
                end
                CHAN0_CNTTXUNDERFLOW : begin
                    rdata <= cnt_tx_underflow[0+:32];
                end            
                CHAN1_STAT : begin
                    rdata[0] <= rx_block_lock[1];
                    rdata[16] <= rx_block_lock_ll[1];
                end    
                CHAN1_RXTRAFPROCSTAT : begin
                    rdata[0] <= rx_datafifo_overflow[1];
                    rdata[1] <= rx_cmdfifo_overflow[1];
                    rdata[16] <= rx_datafifo_overflow_lh[1];
                    rdata[17] <= rx_cmdfifo_overflow_lh[1];
                end                        
                CHAN1_TXTRAFPROCSTAT : begin
                    rdata[0] <= tx_fifo_full[1];
                    rdata[16] <= tx_fifo_full_lh[1];
                end                     
                CHAN1_CNTRXOVERFLOW : begin
                    rdata <= cnt_rx_overflow[32+:32];
                end
                CHAN1_CNTRXUNICASTDROP : begin
                    rdata <= cnt_rx_unicastdrop[32+:32];
                end
                CHAN1_CNTRXMULTICASTDROP : begin
                    rdata <= cnt_rx_multicastdrop[32+:32];
                end  
                CHAN1_CNTRXOVERSIZEDROP : begin
                    rdata <= cnt_rx_oversizedrop[32+:32];
                end
                CHAN1_CNTTXUNDERFLOW : begin
                    rdata <= cnt_tx_underflow[32+:32];
                end
                CHAN2_STAT : begin
                    rdata[0] <= rx_block_lock[2];
                    rdata[16] <= rx_block_lock_ll[2];
                end    
                CHAN2_RXTRAFPROCSTAT : begin
                    rdata[0] <= rx_datafifo_overflow[2];
                    rdata[1] <= rx_cmdfifo_overflow[2];
                    rdata[16] <= rx_datafifo_overflow_lh[2];
                    rdata[17] <= rx_cmdfifo_overflow_lh[2];
                end                        
                CHAN2_TXTRAFPROCSTAT : begin
                    rdata[0] <= tx_fifo_full[2];
                    rdata[16] <= tx_fifo_full_lh[2];
                end                     
                CHAN2_CNTRXOVERFLOW : begin
                    rdata <= cnt_rx_overflow[64+:32];
                end
                CHAN2_CNTRXUNICASTDROP : begin
                    rdata <= cnt_rx_unicastdrop[64+:32];
                end
                CHAN2_CNTRXMULTICASTDROP : begin
                    rdata <= cnt_rx_multicastdrop[64+:32];
                end  
                CHAN2_CNTRXOVERSIZEDROP : begin
                    rdata <= cnt_rx_oversizedrop[64+:32];
                end
                CHAN2_CNTTXUNDERFLOW : begin
                    rdata <= cnt_tx_underflow[64+:32];
                end
                CHAN3_STAT : begin
                    rdata[0] <= rx_block_lock[3];
                    rdata[16] <= rx_block_lock_ll[3];
                end    
                CHAN3_RXTRAFPROCSTAT : begin
                    rdata[0] <= rx_datafifo_overflow[3];
                    rdata[1] <= rx_cmdfifo_overflow[3];
                    rdata[16] <= rx_datafifo_overflow_lh[3];
                    rdata[17] <= rx_cmdfifo_overflow_lh[3];
                end                        
                CHAN3_TXTRAFPROCSTAT : begin
                    rdata[0] <= tx_fifo_full[3];
                    rdata[16] <= tx_fifo_full_lh[3];
                end                     
                CHAN3_CNTRXOVERFLOW : begin
                    rdata <= cnt_rx_overflow[96+:32];
                end
                CHAN3_CNTRXUNICASTDROP : begin
                    rdata <= cnt_rx_unicastdrop[96+:32];
                end
                CHAN3_CNTRXMULTICASTDROP : begin
                    rdata <= cnt_rx_multicastdrop[96+:32];
                end  
                CHAN3_CNTRXOVERSIZEDROP : begin
                    rdata <= cnt_rx_oversizedrop[96+:32];
                end
                CHAN3_CNTTXUNDERFLOW : begin
                    rdata <= cnt_tx_underflow[96+:32];
                end
                GT0_CLKSEL : begin
                    rdata[4+:3] <= gt_rxoutclksel[0+:3];
                    rdata[20+:3] <= gt_txoutclksel[0+:3];
                end
                GT0_LOOPBACK: begin
                    rdata[0+:3] <= gt_loopback[0+:3];
                end
                GT0_POLARITY: begin
                    rdata[0] <= gt_rxpolarity[0];
                    rdata[16] <= gt_txpolarity[0];
                end     
                GT0_TXPRBS_CTRL: begin
                    rdata[0+:4] <= gt_txprbssel[0+:4];
                    rdata[8] <= gt_txprbsforceerr[0];
                end       
                GT0_RXPRBS_CTRL: begin
                    rdata[0+:4] <= gt_rxprbssel[0+:4];
                    rdata[8] <= gt_rxprbscntreset[0];
                end        
                GT0_RXPRBS_STAT: begin
                    rdata[0] <= gt_rxprbslocked[0];
                    rdata[8] <= gt_rxprbserr[0];
                    rdata[24] <= gt_rxprbserr_lh[0];
                end               
                GT0_TXRESET_CTRL: begin
                    rdata[1] <= gt_txpmareset[0];
                    rdata[3] <= gt_txpcsreset[0];
                    rdata[7] <= gt_txelecidle[0];
                    rdata[28] <= gt_gtwiztxreset[0];
                end 
                GT0_TXRESET_STAT: begin
                    rdata[4+:2] <= gt_txbufstatus[0+:2];
                    rdata[6] <= gt_gtpowergood[0];
                    rdata[22] <= gt_gtpowergood_ll[0];
                end 
                GT0_RXRESET_CTRL: begin
                    rdata[1] <= gt_rxpmareset[0];
                    rdata[2] <= gt_rxbufreset[0];
                    rdata[3] <= gt_rxpcsreset[0];
                    rdata[8] <= gt_eyescanreset[0];
                    rdata[28] <= gt_gtwizrxreset[0];
                end    
                GT0_RXRESET_STAT: begin
                    rdata[4+:3] <= gt_rxbufstatus[0+:3];
                    rdata[20] <= gt_rxbufstatus_underflow_lh[0];
                    rdata[21] <= gt_rxbufstatus_overflow_lh[0];                    
                end   
                GT0_TXDRIVER_CTRL: begin
                    rdata[0+:5] <= gt_txdiffctrl[0+:5];
                    rdata[8+:7] <= gt_txmaincursor[0+:7];
                    rdata[16+:5] <= gt_txpostcursor[0+:5];
                    rdata[24+:5] <= gt_txprecursor[0+:5];                    
                    rdata[31] <= gt_txinhibit[0];
                end               
                GT0_RXEQ_CTRL : begin
                    rdata[0] <= gt_rxlpmen[0];
                    rdata[1] <= gt_rxdfelpmreset[0];
                end  
                GT0_RXMARGIN : begin
                    rdata[0] <= gt_eyescantrigger[0];
                    rdata[16] <= gt_eyescandataerror_lh[0];
                end      
                GT1_CLKSEL : begin
                    rdata[4+:3] <= gt_rxoutclksel[3+:3];
                    rdata[20+:3] <= gt_txoutclksel[3+:3];
                end
                GT1_LOOPBACK: begin
                    rdata[0+:3] <= gt_loopback[3+:3];
                end
                GT1_POLARITY: begin
                    rdata[0] <= gt_rxpolarity[1];
                    rdata[16] <= gt_txpolarity[1];
                end     
                GT1_TXPRBS_CTRL: begin
                    rdata[0+:4] <= gt_txprbssel[4+:4];
                    rdata[8] <= gt_txprbsforceerr[1];
                end       
                GT1_RXPRBS_CTRL: begin
                    rdata[0+:4] <= gt_rxprbssel[4+:4];
                    rdata[8] <= gt_rxprbscntreset[1];
                end        
                GT1_RXPRBS_STAT: begin
                    rdata[0] <= gt_rxprbslocked[1];
                    rdata[8] <= gt_rxprbserr[1];
                    rdata[24] <= gt_rxprbserr_lh[1];
                end               
                GT1_TXRESET_CTRL: begin
                    rdata[1] <= gt_txpmareset[1];
                    rdata[3] <= gt_txpcsreset[1];
                    rdata[7] <= gt_txelecidle[1];
                    rdata[28] <= gt_gtwiztxreset[1];
                end 
                GT1_TXRESET_STAT: begin
                    rdata[4+:2] <= gt_txbufstatus[2+:2];
                    rdata[6] <= gt_gtpowergood[1];
                    rdata[22] <= gt_gtpowergood_ll[1];
                end 
                GT1_RXRESET_CTRL: begin
                    rdata[1] <= gt_rxpmareset[1];
                    rdata[2] <= gt_rxbufreset[1];
                    rdata[3] <= gt_rxpcsreset[1];
                    rdata[8] <= gt_eyescanreset[1];
                    rdata[28] <= gt_gtwizrxreset[1];
                end    
                GT1_RXRESET_STAT: begin
                    rdata[4+:3] <= gt_rxbufstatus[3+:3];
                    rdata[20] <= gt_rxbufstatus_underflow_lh[1];
                    rdata[21] <= gt_rxbufstatus_overflow_lh[1];                    
                end   
                GT1_TXDRIVER_CTRL: begin
                    rdata[0+:5] <= gt_txdiffctrl[5+:5];
                    rdata[8+:7] <= gt_txmaincursor[7+:7];
                    rdata[16+:5] <= gt_txpostcursor[5+:5];
                    rdata[24+:5] <= gt_txprecursor[5+:5];                    
                    rdata[31] <= gt_txinhibit[1];
                end               
                GT1_RXEQ_CTRL : begin
                    rdata[0] <= gt_rxlpmen[1];
                    rdata[1] <= gt_rxdfelpmreset[1];
                end  
                GT1_RXMARGIN : begin
                    rdata[0] <= gt_eyescantrigger[1];
                    rdata[16] <= gt_eyescandataerror_lh[1];
                end
                GT2_CLKSEL : begin
                    rdata[4+:3] <= gt_rxoutclksel[6+:3];
                    rdata[20+:3] <= gt_txoutclksel[6+:3];
                end
                GT2_LOOPBACK: begin
                    rdata[0+:3] <= gt_loopback[6+:3];
                end
                GT2_POLARITY: begin
                    rdata[0] <= gt_rxpolarity[2];
                    rdata[16] <= gt_txpolarity[2];
                end     
                GT2_TXPRBS_CTRL: begin
                    rdata[0+:4] <= gt_txprbssel[8+:4];
                    rdata[8] <= gt_txprbsforceerr[2];
                end       
                GT2_RXPRBS_CTRL: begin
                    rdata[0+:4] <= gt_rxprbssel[8+:4];
                    rdata[8] <= gt_rxprbscntreset[2];
                end        
                GT2_RXPRBS_STAT: begin
                    rdata[0] <= gt_rxprbslocked[2];
                    rdata[8] <= gt_rxprbserr[2];
                    rdata[24] <= gt_rxprbserr_lh[2];
                end               
                GT2_TXRESET_CTRL: begin
                    rdata[1] <= gt_txpmareset[2];
                    rdata[3] <= gt_txpcsreset[2];
                    rdata[7] <= gt_txelecidle[2];
                    rdata[28] <= gt_gtwiztxreset[2];
                end 
                GT2_TXRESET_STAT: begin
                    rdata[4+:2] <= gt_txbufstatus[4+:2];
                    rdata[6] <= gt_gtpowergood[2];
                    rdata[22] <= gt_gtpowergood_ll[2];
                end 
                GT2_RXRESET_CTRL: begin
                    rdata[1] <= gt_rxpmareset[2];
                    rdata[2] <= gt_rxbufreset[2];
                    rdata[3] <= gt_rxpcsreset[2];
                    rdata[8] <= gt_eyescanreset[2];
                    rdata[28] <= gt_gtwizrxreset[2];
                end    
                GT2_RXRESET_STAT: begin
                    rdata[4+:3] <= gt_rxbufstatus[6+:3];
                    rdata[20] <= gt_rxbufstatus_underflow_lh[2];
                    rdata[21] <= gt_rxbufstatus_overflow_lh[2];                    
                end   
                GT2_TXDRIVER_CTRL: begin
                    rdata[0+:5] <= gt_txdiffctrl[10+:5];
                    rdata[8+:7] <= gt_txmaincursor[14+:7];
                    rdata[16+:5] <= gt_txpostcursor[10+:5];
                    rdata[24+:5] <= gt_txprecursor[10+:5];                    
                    rdata[31] <= gt_txinhibit[2];
                end               
                GT2_RXEQ_CTRL : begin
                    rdata[0] <= gt_rxlpmen[2];
                    rdata[1] <= gt_rxdfelpmreset[2];
                end  
                GT2_RXMARGIN : begin
                    rdata[0] <= gt_eyescantrigger[2];
                    rdata[16] <= gt_eyescandataerror_lh[2];
                end
                GT3_CLKSEL : begin
                    rdata[4+:3] <= gt_rxoutclksel[9+:3];
                    rdata[20+:3] <= gt_txoutclksel[9+:3];
                end
                GT3_LOOPBACK: begin
                    rdata[0+:3] <= gt_loopback[9+:3];
                end
                GT3_POLARITY: begin
                    rdata[0] <= gt_rxpolarity[3];
                    rdata[16] <= gt_txpolarity[3];
                end     
                GT3_TXPRBS_CTRL: begin
                    rdata[0+:4] <= gt_txprbssel[12+:4];
                    rdata[8] <= gt_txprbsforceerr[3];
                end       
                GT3_RXPRBS_CTRL: begin
                    rdata[0+:4] <= gt_rxprbssel[12+:4];
                    rdata[8] <= gt_rxprbscntreset[3];
                end        
                GT3_RXPRBS_STAT: begin
                    rdata[0] <= gt_rxprbslocked[3];
                    rdata[8] <= gt_rxprbserr[3];
                    rdata[24] <= gt_rxprbserr_lh[3];
                end               
                GT3_TXRESET_CTRL: begin
                    rdata[1] <= gt_txpmareset[3];
                    rdata[3] <= gt_txpcsreset[3];
                    rdata[7] <= gt_txelecidle[3];
                    rdata[28] <= gt_gtwiztxreset[3];
                end 
                GT3_TXRESET_STAT: begin
                    rdata[4+:2] <= gt_txbufstatus[6+:2];
                    rdata[6] <= gt_gtpowergood[3];
                    rdata[22] <= gt_gtpowergood_ll[3];
                end 
                GT3_RXRESET_CTRL: begin
                    rdata[1] <= gt_rxpmareset[3];
                    rdata[2] <= gt_rxbufreset[3];
                    rdata[3] <= gt_rxpcsreset[3];
                    rdata[8] <= gt_eyescanreset[3];
                    rdata[28] <= gt_gtwizrxreset[3];
                end    
                GT3_RXRESET_STAT: begin
                    rdata[4+:3] <= gt_rxbufstatus[9+:3];
                    rdata[20] <= gt_rxbufstatus_underflow_lh[3];
                    rdata[21] <= gt_rxbufstatus_overflow_lh[3];                    
                end   
                GT3_TXDRIVER_CTRL: begin
                    rdata[0+:5] <= gt_txdiffctrl[15+:5];
                    rdata[8+:7] <= gt_txmaincursor[21+:7];
                    rdata[16+:5] <= gt_txpostcursor[15+:5];
                    rdata[24+:5] <= gt_txprecursor[15+:5];                    
                    rdata[31] <= gt_txinhibit[3];
                end               
                GT3_RXEQ_CTRL : begin
                    rdata[0] <= gt_rxlpmen[3];
                    rdata[1] <= gt_rxdfelpmreset[3];
                end  
                GT3_RXMARGIN : begin
                    rdata[0] <= gt_eyescantrigger[3];
                    rdata[16] <= gt_eyescandataerror_lh[3];
                end
                default: begin
                    rdata <= '0;
                end                
            endcase
        end
    end
end


//------------------------Register logic-----------------

always_ff @(posedge ACLK) begin
    if (ACLK_EN) begin
        if (w_hs && waddr == KERNEL_CFG0) begin
            reset_out <= (WDATA[0] & wmask[0]) | (reset_out & ~wmask[0]);
        end
        else if (w_hs && waddr == CHAN0_MAC_L) begin
            unicast_mac_addr[0+:32] <= (WDATA & wmask) | (unicast_mac_addr[0+:32] & ~wmask[0+:32]);
        end
        else if (w_hs && waddr == CHAN0_MAC_U) begin
            unicast_mac_addr[32+:16] <= (WDATA[0+:16] & wmask[0+:16]) | (unicast_mac_addr[32+:16] & ~wmask[0+:16]);
        end        
        else if (w_hs && waddr == CHAN0_RESET_CTRL) begin
            rx_fifo_reset[0] <= (WDATA[0] & wmask[0]) | (rx_fifo_reset[0] & ~wmask[0]);
            rx_traf_proc_reset[0] <= (WDATA[1] & wmask[1]) | (rx_traf_proc_reset[0] & ~wmask[1]);
            tx_fifo_reset[0] <= (WDATA[16] & wmask[16]) | (tx_fifo_reset[0] & ~wmask[16]);
            tx_traf_proc_reset[0] <= (WDATA[17] & wmask[17]) | (tx_traf_proc_reset[0] & ~wmask[17]);
        end
        else if (w_hs && waddr == CHAN0_CFG) begin
            unicast_promiscuous_en[0] <= (WDATA[0] & wmask[0]) | (unicast_promiscuous_en[0] & ~wmask[0]);
            multicast_promiscuous_en[0] <= (WDATA[1] & wmask[1]) | (multicast_promiscuous_en[0] & ~wmask[1]);
            cut_through_en[0] <= (WDATA[2] & wmask[2]) | (cut_through_en[0] & ~wmask[2]);
        end
        else if (w_hs && waddr == CHAN0_THRES) begin
            tx_threshold[0+:8] <= (WDATA & wmask) | (tx_threshold[0+:8] & ~wmask[0+:8]);
        end        
        else if (w_hs && waddr == CHAN1_MAC_L) begin
            unicast_mac_addr[48+:32] <= (WDATA & wmask) | (unicast_mac_addr[48+:32] & ~wmask[0+:32]);
        end
        else if (w_hs && waddr == CHAN1_MAC_U) begin
            unicast_mac_addr[80+:16] <= (WDATA[0+:16] & wmask[0+:16]) | (unicast_mac_addr[80+:16] & ~wmask[0+:16]);
        end        
        else if (w_hs && waddr == CHAN1_RESET_CTRL) begin
            rx_fifo_reset[1] <= (WDATA[0] & wmask[0]) | (rx_fifo_reset[1] & ~wmask[0]);
            rx_traf_proc_reset[1] <= (WDATA[1] & wmask[1]) | (rx_traf_proc_reset[1] & ~wmask[1]);
            tx_fifo_reset[1] <= (WDATA[16] & wmask[16]) | (tx_fifo_reset[1] & ~wmask[16]);
            tx_traf_proc_reset[1] <= (WDATA[17] & wmask[17]) | (tx_traf_proc_reset[1] & ~wmask[17]);
        end
        else if (w_hs && waddr == CHAN1_CFG) begin
            unicast_promiscuous_en[1] <= (WDATA[0] & wmask[0]) | (unicast_promiscuous_en[1] & ~wmask[0]);
            multicast_promiscuous_en[1] <= (WDATA[1] & wmask[1]) | (multicast_promiscuous_en[1] & ~wmask[1]);
            cut_through_en[1] <= (WDATA[2] & wmask[2]) | (cut_through_en[1] & ~wmask[2]);
        end
        else if (w_hs && waddr == CHAN1_THRES) begin
            tx_threshold[8+:8] <= (WDATA & wmask) | (tx_threshold[8+:8] & ~wmask[0+:8]);
        end
        else if (w_hs && waddr == CHAN2_MAC_L) begin
            unicast_mac_addr[96+:32] <= (WDATA & wmask) | (unicast_mac_addr[96+:32] & ~wmask[0+:32]);
        end
        else if (w_hs && waddr == CHAN2_MAC_U) begin
            unicast_mac_addr[128+:16] <= (WDATA[0+:16] & wmask[0+:16]) | (unicast_mac_addr[128+:16] & ~wmask[0+:16]);
        end        
        else if (w_hs && waddr == CHAN2_RESET_CTRL) begin
            rx_fifo_reset[2] <= (WDATA[0] & wmask[0]) | (rx_fifo_reset[2] & ~wmask[0]);
            rx_traf_proc_reset[2] <= (WDATA[1] & wmask[1]) | (rx_traf_proc_reset[2] & ~wmask[1]);
            tx_fifo_reset[2] <= (WDATA[16] & wmask[16]) | (tx_fifo_reset[2] & ~wmask[16]);
            tx_traf_proc_reset[2] <= (WDATA[17] & wmask[17]) | (tx_traf_proc_reset[2] & ~wmask[17]);
        end
        else if (w_hs && waddr == CHAN2_CFG) begin
            unicast_promiscuous_en[2] <= (WDATA[0] & wmask[0]) | (unicast_promiscuous_en[2] & ~wmask[0]);
            multicast_promiscuous_en[2] <= (WDATA[1] & wmask[1]) | (multicast_promiscuous_en[2] & ~wmask[1]);
            cut_through_en[2] <= (WDATA[2] & wmask[2]) | (cut_through_en[2] & ~wmask[2]);
        end
        else if (w_hs && waddr == CHAN2_THRES) begin
            tx_threshold[16+:8] <= (WDATA & wmask) | (tx_threshold[16+:8] & ~wmask[0+:8]);
        end
        else if (w_hs && waddr == CHAN3_MAC_L) begin
            unicast_mac_addr[144+:32] <= (WDATA & wmask) | (unicast_mac_addr[144+:32] & ~wmask[0+:32]);
        end
        else if (w_hs && waddr == CHAN3_MAC_U) begin
            unicast_mac_addr[176+:16] <= (WDATA[0+:16] & wmask[0+:16]) | (unicast_mac_addr[176+:16] & ~wmask[0+:16]);
        end        
        else if (w_hs && waddr == CHAN3_RESET_CTRL) begin
            rx_fifo_reset[3] <= (WDATA[0] & wmask[0]) | (rx_fifo_reset[3] & ~wmask[0]);
            rx_traf_proc_reset[3] <= (WDATA[1] & wmask[1]) | (rx_traf_proc_reset[3] & ~wmask[1]);
            tx_fifo_reset[3] <= (WDATA[16] & wmask[16]) | (tx_fifo_reset[3] & ~wmask[16]);
            tx_traf_proc_reset[3] <= (WDATA[17] & wmask[17]) | (tx_traf_proc_reset[3] & ~wmask[17]);
        end
        else if (w_hs && waddr == CHAN3_CFG) begin
            unicast_promiscuous_en[3] <= (WDATA[0] & wmask[0]) | (unicast_promiscuous_en[3] & ~wmask[0]);
            multicast_promiscuous_en[3] <= (WDATA[1] & wmask[1]) | (multicast_promiscuous_en[3] & ~wmask[1]);
            cut_through_en[3] <= (WDATA[2] & wmask[2]) | (cut_through_en[3] & ~wmask[2]);
        end
        else if (w_hs && waddr == CHAN3_THRES) begin
            tx_threshold[24+:8] <= (WDATA & wmask) | (tx_threshold[24+:8] & ~wmask[0+:8]);
        end
        else if (w_hs && waddr == GT0_CLKSEL) begin
            gt_rxoutclksel[0+:3] <= (WDATA[4+:3] & wmask[4+:3]) | (gt_rxoutclksel[0+:3] & ~wmask[4+:3]);
            gt_txoutclksel[0+:3] <= (WDATA[20+:3] & wmask[20+:3]) | (gt_txoutclksel[0+:3] & ~wmask[20+:3]);
        end
        else if (w_hs && waddr == GT0_LOOPBACK) begin
            gt_loopback[0+:3] <= (WDATA[0+:3] & wmask[0+:3]) | (gt_loopback[0+:3] & ~wmask[0+:3]);
        end
        else if (w_hs && waddr == GT0_POLARITY) begin
            gt_rxpolarity[0] <= (WDATA[0] & wmask[0]) | (gt_rxpolarity[0] & ~wmask[0]);
            gt_txpolarity[0] <= (WDATA[16] & wmask[16]) | (gt_rxpolarity[0] & ~wmask[16]);
        end        
        else if (w_hs && waddr == GT0_TXPRBS_CTRL) begin
            gt_txprbssel[0+:4] <= (WDATA[0+:4] & wmask[0+:4]) | (gt_txprbssel[0+:4] & ~wmask[0+:4]);
            gt_txprbsforceerr[0] <= (WDATA[8] & wmask[8]) | (gt_txprbsforceerr[0] & ~wmask[8]);
        end  
        else if (w_hs && waddr == GT0_RXPRBS_CTRL) begin
            gt_rxprbssel[0+:4] <= (WDATA[0+:4] & wmask[0+:4]) | (gt_rxprbssel[0+:4] & ~wmask[0+:4]);
            gt_rxprbscntreset[0] <= (WDATA[8] & wmask[8]) | (gt_rxprbscntreset[0] & ~wmask[8]);
        end                
        else if (w_hs && waddr == GT0_TXRESET_CTRL) begin
            gt_txpmareset[0] <= (WDATA[1] & wmask[1]) | (gt_txpmareset[0] & ~wmask[1]);
            gt_txpcsreset[0] <= (WDATA[3] & wmask[3]) | (gt_txpcsreset[0] & ~wmask[3]);
            gt_txelecidle[0] <= (WDATA[7] & wmask[7]) | (gt_txelecidle[0] & ~wmask[7]);
            gt_gtwiztxreset[0] <= (WDATA[28] & wmask[28]) | (gt_gtwiztxreset[0] & ~wmask[28]);
        end   
        else if (w_hs && waddr == GT0_RXRESET_CTRL) begin
            gt_rxpmareset[0] <= (WDATA[1] & wmask[1]) | (gt_rxpmareset[0] & ~wmask[1]);
            gt_rxbufreset[0] <= (WDATA[2] & wmask[2]) | (gt_rxbufreset[0] & ~wmask[3]);
            gt_rxpcsreset[0] <= (WDATA[3] & wmask[3]) | (gt_rxpcsreset[0] & ~wmask[3]);
            gt_eyescanreset[0] <= (WDATA[8] & wmask[8]) | (gt_eyescanreset[0] & ~wmask[8]);
            gt_gtwizrxreset[0] <= (WDATA[28] & wmask[28]) | (gt_gtwizrxreset[0] & ~wmask[28]);
        end     
        else if (w_hs && waddr == GT0_TXDRIVER_CTRL) begin
            gt_txdiffctrl[0+:5] <= (WDATA[0+:5] & wmask[0+:5]) | (gt_txdiffctrl[0+:5] & ~wmask[0+:5]);
            gt_txmaincursor[0+:7] <= (WDATA[8+:7] & wmask[8+:7]) | (gt_txmaincursor[0+:7] & ~wmask[8+:7]);
            gt_txpostcursor[0+:5] <= (WDATA[16+:5] & wmask[8+:5]) | (gt_txpostcursor[0+:5] & ~wmask[16+:5]);
            gt_txprecursor[0+:5] <= (WDATA[24+:5] & wmask[24+:5]) | (gt_txprecursor[0+:5] & ~wmask[24+:5]);
            gt_txinhibit[0] <= (WDATA[31] & wmask[31]) | (gt_txinhibit[0] & ~wmask[31]);
        end       
        else if (w_hs && waddr == GT0_RXEQ_CTRL) begin
            gt_rxlpmen[0] <= (WDATA[0] & wmask[0]) | (gt_rxlpmen[0] & ~wmask[0]);
            gt_rxdfelpmreset[0] <= (WDATA[1] & wmask[1]) | (gt_rxdfelpmreset[0] & ~wmask[1]);
        end               
        else if (w_hs && waddr == GT0_RXMARGIN) begin
            gt_eyescantrigger[0] <= (WDATA[0] & wmask[0]) | (gt_eyescantrigger[0] & ~wmask[0]);
        end  
        else if (w_hs && waddr == GT1_CLKSEL) begin
            gt_rxoutclksel[3+:3] <= (WDATA[4+:3] & wmask[4+:3]) | (gt_rxoutclksel[3+:3] & ~wmask[4+:3]);
            gt_txoutclksel[3+:3] <= (WDATA[20+:3] & wmask[20+:3]) | (gt_txoutclksel[3+:3] & ~wmask[20+:3]);
        end
        else if (w_hs && waddr == GT1_LOOPBACK) begin
            gt_loopback[3+:3] <= (WDATA[0+:3] & wmask[0+:3]) | (gt_loopback[3+:3] & ~wmask[0+:3]);
        end
        else if (w_hs && waddr == GT1_POLARITY) begin
            gt_rxpolarity[1] <= (WDATA[0] & wmask[0]) | (gt_rxpolarity[1] & ~wmask[0]);
            gt_txpolarity[1] <= (WDATA[16] & wmask[16]) | (gt_rxpolarity[1] & ~wmask[16]);
        end        
        else if (w_hs && waddr == GT1_TXPRBS_CTRL) begin
            gt_txprbssel[4+:4] <= (WDATA[0+:4] & wmask[0+:4]) | (gt_txprbssel[4+:4] & ~wmask[0+:4]);
            gt_txprbsforceerr[1] <= (WDATA[8] & wmask[8]) | (gt_txprbsforceerr[1] & ~wmask[8]);
        end  
        else if (w_hs && waddr == GT1_RXPRBS_CTRL) begin
            gt_rxprbssel[4+:4] <= (WDATA[0+:4] & wmask[0+:4]) | (gt_rxprbssel[4+:4] & ~wmask[0+:4]);
            gt_rxprbscntreset[1] <= (WDATA[8] & wmask[8]) | (gt_rxprbscntreset[1] & ~wmask[8]);
        end                
        else if (w_hs && waddr == GT1_TXRESET_CTRL) begin
            gt_txpmareset[1] <= (WDATA[1] & wmask[1]) | (gt_txpmareset[1] & ~wmask[1]);
            gt_txpcsreset[1] <= (WDATA[3] & wmask[3]) | (gt_txpcsreset[1] & ~wmask[3]);
            gt_txelecidle[1] <= (WDATA[7] & wmask[7]) | (gt_txelecidle[1] & ~wmask[7]);
            gt_gtwiztxreset[1] <= (WDATA[28] & wmask[28]) | (gt_gtwiztxreset[1] & ~wmask[28]);
        end   
        else if (w_hs && waddr == GT1_RXRESET_CTRL) begin
            gt_rxpmareset[1] <= (WDATA[1] & wmask[1]) | (gt_rxpmareset[1] & ~wmask[1]);
            gt_rxbufreset[1] <= (WDATA[2] & wmask[2]) | (gt_rxbufreset[1] & ~wmask[3]);
            gt_rxpcsreset[1] <= (WDATA[3] & wmask[3]) | (gt_rxpcsreset[1] & ~wmask[3]);
            gt_eyescanreset[1] <= (WDATA[8] & wmask[8]) | (gt_eyescanreset[1] & ~wmask[8]);
            gt_gtwizrxreset[1] <= (WDATA[28] & wmask[28]) | (gt_gtwizrxreset[1] & ~wmask[28]);
        end     
        else if (w_hs && waddr == GT1_TXDRIVER_CTRL) begin
            gt_txdiffctrl[5+:5] <= (WDATA[0+:5] & wmask[0+:5]) | (gt_txdiffctrl[5+:5] & ~wmask[0+:5]);
            gt_txmaincursor[7+:7] <= (WDATA[8+:7] & wmask[8+:7]) | (gt_txmaincursor[7+:7] & ~wmask[8+:7]);
            gt_txpostcursor[5+:5] <= (WDATA[16+:5] & wmask[8+:5]) | (gt_txpostcursor[5+:5] & ~wmask[16+:5]);
            gt_txprecursor[5+:5] <= (WDATA[24+:5] & wmask[24+:5]) | (gt_txprecursor[5+:5] & ~wmask[24+:5]);
            gt_txinhibit[1] <= (WDATA[31] & wmask[31]) | (gt_txinhibit[1] & ~wmask[31]);
        end       
        else if (w_hs && waddr == GT1_RXEQ_CTRL) begin
            gt_rxlpmen[1] <= (WDATA[0] & wmask[0]) | (gt_rxlpmen[1] & ~wmask[0]);
            gt_rxdfelpmreset[1] <= (WDATA[1] & wmask[1]) | (gt_rxdfelpmreset[1] & ~wmask[1]);
        end               
        else if (w_hs && waddr == GT1_RXMARGIN) begin
            gt_eyescantrigger[1] <= (WDATA[0] & wmask[0]) | (gt_eyescantrigger[1] & ~wmask[0]);
        end
        else if (w_hs && waddr == GT2_CLKSEL) begin
            gt_rxoutclksel[6+:3] <= (WDATA[4+:3] & wmask[4+:3]) | (gt_rxoutclksel[6+:3] & ~wmask[4+:3]);
            gt_txoutclksel[6+:3] <= (WDATA[20+:3] & wmask[20+:3]) | (gt_txoutclksel[6+:3] & ~wmask[20+:3]);
        end
        else if (w_hs && waddr == GT2_LOOPBACK) begin
            gt_loopback[6+:3] <= (WDATA[0+:3] & wmask[0+:3]) | (gt_loopback[6+:3] & ~wmask[0+:3]);
        end
        else if (w_hs && waddr == GT2_POLARITY) begin
            gt_rxpolarity[2] <= (WDATA[0] & wmask[0]) | (gt_rxpolarity[2] & ~wmask[0]);
            gt_txpolarity[2] <= (WDATA[16] & wmask[16]) | (gt_rxpolarity[2] & ~wmask[16]);
        end        
        else if (w_hs && waddr == GT2_TXPRBS_CTRL) begin
            gt_txprbssel[8+:4] <= (WDATA[0+:4] & wmask[0+:4]) | (gt_txprbssel[8+:4] & ~wmask[0+:4]);
            gt_txprbsforceerr[2] <= (WDATA[8] & wmask[8]) | (gt_txprbsforceerr[2] & ~wmask[8]);
        end  
        else if (w_hs && waddr == GT2_RXPRBS_CTRL) begin
            gt_rxprbssel[8+:4] <= (WDATA[0+:4] & wmask[0+:4]) | (gt_rxprbssel[8+:4] & ~wmask[0+:4]);
            gt_rxprbscntreset[2] <= (WDATA[8] & wmask[8]) | (gt_rxprbscntreset[2] & ~wmask[8]);
        end                
        else if (w_hs && waddr == GT2_TXRESET_CTRL) begin
            gt_txpmareset[2] <= (WDATA[1] & wmask[1]) | (gt_txpmareset[2] & ~wmask[1]);
            gt_txpcsreset[2] <= (WDATA[3] & wmask[3]) | (gt_txpcsreset[2] & ~wmask[3]);
            gt_txelecidle[2] <= (WDATA[7] & wmask[7]) | (gt_txelecidle[2] & ~wmask[7]);
            gt_gtwiztxreset[2] <= (WDATA[28] & wmask[28]) | (gt_gtwiztxreset[2] & ~wmask[28]);
        end   
        else if (w_hs && waddr == GT2_RXRESET_CTRL) begin
            gt_rxpmareset[2] <= (WDATA[1] & wmask[1]) | (gt_rxpmareset[2] & ~wmask[1]);
            gt_rxbufreset[2] <= (WDATA[2] & wmask[2]) | (gt_rxbufreset[2] & ~wmask[3]);
            gt_rxpcsreset[2] <= (WDATA[3] & wmask[3]) | (gt_rxpcsreset[2] & ~wmask[3]);
            gt_eyescanreset[2] <= (WDATA[8] & wmask[8]) | (gt_eyescanreset[2] & ~wmask[8]);
            gt_gtwizrxreset[2] <= (WDATA[28] & wmask[28]) | (gt_gtwizrxreset[2] & ~wmask[28]);
        end     
        else if (w_hs && waddr == GT2_TXDRIVER_CTRL) begin
            gt_txdiffctrl[10+:5] <= (WDATA[0+:5] & wmask[0+:5]) | (gt_txdiffctrl[10+:5] & ~wmask[0+:5]);
            gt_txmaincursor[14+:7] <= (WDATA[8+:7] & wmask[8+:7]) | (gt_txmaincursor[14+:7] & ~wmask[8+:7]);
            gt_txpostcursor[10+:5] <= (WDATA[16+:5] & wmask[8+:5]) | (gt_txpostcursor[10+:5] & ~wmask[16+:5]);
            gt_txprecursor[10+:5] <= (WDATA[24+:5] & wmask[24+:5]) | (gt_txprecursor[10+:5] & ~wmask[24+:5]);
            gt_txinhibit[2] <= (WDATA[31] & wmask[31]) | (gt_txinhibit[2] & ~wmask[31]);
        end       
        else if (w_hs && waddr == GT2_RXEQ_CTRL) begin
            gt_rxlpmen[2] <= (WDATA[0] & wmask[0]) | (gt_rxlpmen[2] & ~wmask[0]);
            gt_rxdfelpmreset[2] <= (WDATA[1] & wmask[1]) | (gt_rxdfelpmreset[2] & ~wmask[1]);
        end               
        else if (w_hs && waddr == GT2_RXMARGIN) begin
            gt_eyescantrigger[2] <= (WDATA[0] & wmask[0]) | (gt_eyescantrigger[2] & ~wmask[0]);
        end
        else if (w_hs && waddr == GT3_CLKSEL) begin
            gt_rxoutclksel[9+:3] <= (WDATA[4+:3] & wmask[4+:3]) | (gt_rxoutclksel[9+:3] & ~wmask[4+:3]);
            gt_txoutclksel[9+:3] <= (WDATA[20+:3] & wmask[20+:3]) | (gt_txoutclksel[9+:3] & ~wmask[20+:3]);
        end
        else if (w_hs && waddr == GT3_LOOPBACK) begin
            gt_loopback[9+:3] <= (WDATA[0+:3] & wmask[0+:3]) | (gt_loopback[9+:3] & ~wmask[0+:3]);
        end
        else if (w_hs && waddr == GT3_POLARITY) begin
            gt_rxpolarity[3] <= (WDATA[0] & wmask[0]) | (gt_rxpolarity[3] & ~wmask[0]);
            gt_txpolarity[3] <= (WDATA[16] & wmask[16]) | (gt_rxpolarity[3] & ~wmask[16]);
        end        
        else if (w_hs && waddr == GT3_TXPRBS_CTRL) begin
            gt_txprbssel[12+:4] <= (WDATA[0+:4] & wmask[0+:4]) | (gt_txprbssel[12+:4] & ~wmask[0+:4]);
            gt_txprbsforceerr[3] <= (WDATA[8] & wmask[8]) | (gt_txprbsforceerr[3] & ~wmask[8]);
        end  
        else if (w_hs && waddr == GT3_RXPRBS_CTRL) begin
            gt_rxprbssel[12+:4] <= (WDATA[0+:4] & wmask[0+:4]) | (gt_rxprbssel[12+:4] & ~wmask[0+:4]);
            gt_rxprbscntreset[3] <= (WDATA[8] & wmask[8]) | (gt_rxprbscntreset[3] & ~wmask[8]);
        end                
        else if (w_hs && waddr == GT3_TXRESET_CTRL) begin
            gt_txpmareset[3] <= (WDATA[1] & wmask[1]) | (gt_txpmareset[3] & ~wmask[1]);
            gt_txpcsreset[3] <= (WDATA[3] & wmask[3]) | (gt_txpcsreset[3] & ~wmask[3]);
            gt_txelecidle[3] <= (WDATA[7] & wmask[7]) | (gt_txelecidle[3] & ~wmask[7]);
            gt_gtwiztxreset[3] <= (WDATA[28] & wmask[28]) | (gt_gtwiztxreset[3] & ~wmask[28]);
        end   
        else if (w_hs && waddr == GT3_RXRESET_CTRL) begin
            gt_rxpmareset[3] <= (WDATA[1] & wmask[1]) | (gt_rxpmareset[3] & ~wmask[1]);
            gt_rxbufreset[3] <= (WDATA[2] & wmask[2]) | (gt_rxbufreset[3] & ~wmask[3]);
            gt_rxpcsreset[3] <= (WDATA[3] & wmask[3]) | (gt_rxpcsreset[3] & ~wmask[3]);
            gt_eyescanreset[3] <= (WDATA[8] & wmask[8]) | (gt_eyescanreset[3] & ~wmask[8]);
            gt_gtwizrxreset[3] <= (WDATA[28] & wmask[28]) | (gt_gtwizrxreset[3] & ~wmask[28]);
        end     
        else if (w_hs && waddr == GT3_TXDRIVER_CTRL) begin
            gt_txdiffctrl[15+:5] <= (WDATA[0+:5] & wmask[0+:5]) | (gt_txdiffctrl[15+:5] & ~wmask[0+:5]);
            gt_txmaincursor[21+:7] <= (WDATA[8+:7] & wmask[8+:7]) | (gt_txmaincursor[21+:7] & ~wmask[8+:7]);
            gt_txpostcursor[15+:5] <= (WDATA[16+:5] & wmask[8+:5]) | (gt_txpostcursor[15+:5] & ~wmask[16+:5]);
            gt_txprecursor[15+:5] <= (WDATA[24+:5] & wmask[24+:5]) | (gt_txprecursor[15+:5] & ~wmask[24+:5]);
            gt_txinhibit[3] <= (WDATA[31] & wmask[31]) | (gt_txinhibit[3] & ~wmask[31]);
        end       
        else if (w_hs && waddr == GT3_RXEQ_CTRL) begin
            gt_rxlpmen[3] <= (WDATA[0] & wmask[0]) | (gt_rxlpmen[3] & ~wmask[0]);
            gt_rxdfelpmreset[3] <= (WDATA[1] & wmask[1]) | (gt_rxdfelpmreset[3] & ~wmask[1]);
        end               
        else if (w_hs && waddr == GT3_RXMARGIN) begin
            gt_eyescantrigger[3] <= (WDATA[0] & wmask[0]) | (gt_eyescantrigger[3] & ~wmask[0]);
        end
    end
end

// Write to clear 
always_ff @(posedge ACLK) begin
    gt_rxprbserr_clr <= '0;
    gt_gtpowergood_clr <= '0;
    gt_rxbufstatus_underflow_clr <= '0;
    gt_rxbufstatus_overflow_clr <= '0;        
    gt_eyescandataerror_clr <= '0;   
    rx_block_lock_clr <= '0;
    rx_datafifo_overflow_clr <= '0;
    rx_cmdfifo_overflow_clr <= '0;
    tx_fifo_full_clr <= '0;
    if (w_hs && waddr == CHAN0_STAT)  begin
        rx_block_lock_clr[0] <= ~WDATA[16] & wmask[16];
    end
    if (w_hs && waddr == CHAN1_STAT)  begin
        rx_block_lock_clr[1] <= ~WDATA[16] & wmask[16];
    end
    if (w_hs && waddr == CHAN2_STAT)  begin
        rx_block_lock_clr[2] <= ~WDATA[16] & wmask[16];
    end
    if (w_hs && waddr == CHAN3_STAT)  begin
        rx_block_lock_clr[3] <= ~WDATA[16] & wmask[16];
    end        
    if (w_hs && waddr == GT0_RXPRBS_STAT) begin
        gt_rxprbserr_clr[0] <= (WDATA[24] & wmask[24]);
    end
    else if (w_hs && waddr == GT0_TXRESET_STAT) begin
        gt_gtpowergood_clr[0] <= ~WDATA[22] & wmask[22];
    end                     
    else if (w_hs && waddr == GT0_RXRESET_STAT) begin
        gt_rxbufstatus_underflow_clr[0] <= WDATA[20] & wmask[20];
        gt_rxbufstatus_overflow_clr[0] <= WDATA[21] & wmask[21];
    end 
    else if (w_hs && waddr == GT0_RXMARGIN) begin    
        gt_eyescandataerror_clr[0] <= WDATA[16] & wmask[16];
    end                
    if (w_hs && waddr == GT1_RXPRBS_STAT) begin
        gt_rxprbserr_clr[1] <= (WDATA[24] & wmask[24]);
    end
    else if (w_hs && waddr == GT1_TXRESET_STAT) begin
        gt_gtpowergood_clr[1] <= ~WDATA[22] & wmask[22];
    end                     
    else if (w_hs && waddr == GT1_RXRESET_STAT) begin
        gt_rxbufstatus_underflow_clr[1] <= WDATA[20] & wmask[20];
        gt_rxbufstatus_overflow_clr[1] <= WDATA[21] & wmask[21];
    end 
    else if (w_hs && waddr == GT1_RXMARGIN) begin    
        gt_eyescandataerror_clr[1] <= WDATA[16] & wmask[16];
    end
    if (w_hs && waddr == GT2_RXPRBS_STAT) begin
        gt_rxprbserr_clr[2] <= (WDATA[24] & wmask[24]);
    end
    else if (w_hs && waddr == GT2_TXRESET_STAT) begin
        gt_gtpowergood_clr[2] <= ~WDATA[22] & wmask[22];
    end                     
    else if (w_hs && waddr == GT2_RXRESET_STAT) begin
        gt_rxbufstatus_underflow_clr[2] <= WDATA[20] & wmask[20];
        gt_rxbufstatus_overflow_clr[2] <= WDATA[21] & wmask[21];
    end 
    else if (w_hs && waddr == GT2_RXMARGIN) begin    
        gt_eyescandataerror_clr[2] <= WDATA[16] & wmask[16];
    end
    if (w_hs && waddr == GT3_RXPRBS_STAT) begin
        gt_rxprbserr_clr[3] <= (WDATA[24] & wmask[24]);
    end
    else if (w_hs && waddr == GT3_TXRESET_STAT) begin
        gt_gtpowergood_clr[3] <= ~WDATA[22] & wmask[22];
    end                     
    else if (w_hs && waddr == GT3_RXRESET_STAT) begin
        gt_rxbufstatus_underflow_clr[3] <= WDATA[20] & wmask[20];
        gt_rxbufstatus_overflow_clr[3] <= WDATA[21] & wmask[21];
    end 
    else if (w_hs && waddr == GT3_RXMARGIN) begin    
        gt_eyescandataerror_clr[3] <= WDATA[16] & wmask[16];
    end
    else if (w_hs && waddr == CHAN0_RXTRAFPROCSTAT) begin
        rx_datafifo_overflow_clr[0] <= WDATA[16] & wmask[16];
        rx_cmdfifo_overflow_clr[0] <= WDATA[17] & wmask[17];
    end                       
    else if (w_hs && waddr == CHAN1_RXTRAFPROCSTAT) begin
        rx_datafifo_overflow_clr[1] <= WDATA[16] & wmask[16];
        rx_cmdfifo_overflow_clr[1] <= WDATA[17] & wmask[17];
    end
    else if (w_hs && waddr == CHAN2_RXTRAFPROCSTAT) begin
        rx_datafifo_overflow_clr[2] <= WDATA[16] & wmask[16];
        rx_cmdfifo_overflow_clr[2] <= WDATA[17] & wmask[17];
    end
    else if (w_hs && waddr == CHAN3_RXTRAFPROCSTAT) begin
        rx_datafifo_overflow_clr[3] <= WDATA[16] & wmask[16];
        rx_cmdfifo_overflow_clr[3] <= WDATA[17] & wmask[17];
    end 
    else if (w_hs && waddr == CHAN0_TXTRAFPROCSTAT) begin
        tx_fifo_full_clr[0] <= WDATA[16] & wmask[16];
    end                       
    else if (w_hs && waddr == CHAN1_TXTRAFPROCSTAT) begin
        tx_fifo_full_clr[1] <= WDATA[16] & wmask[16];
    end
    else if (w_hs && waddr == CHAN2_TXTRAFPROCSTAT) begin
        tx_fifo_full_clr[2] <= WDATA[16] & wmask[16];
    end
    else if (w_hs && waddr == CHAN3_TXTRAFPROCSTAT) begin
        tx_fifo_full_clr[3] <= WDATA[16] & wmask[16];
    end                           
end

//------------------------Memory logic-------------------

endmodule
