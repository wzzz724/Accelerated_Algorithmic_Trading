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

module ll_udp_skidbuffer #(
    parameter   REG_OUTPUT = 1,
    parameter   DATA_WIDTH = 64
)(
    input   wire                       i_clk,
    input   wire                       i_reset,
    input   wire                       i_valid,
    output  reg                        o_ready,    
    input   wire      [DATA_WIDTH-1:0] i_data,
	input   wire  [(DATA_WIDTH/8)-1:0] i_keep,
	input   wire                       i_last,
	input   wire                       i_user,
    output  reg                        o_valid,
	input   wire                       i_ready,
    output  reg       [DATA_WIDTH-1:0] o_data,
	output  reg   [(DATA_WIDTH/8)-1:0] o_keep,
	output  reg                        o_last,
	output  reg                        o_user
);

logic                      r_valid;
logic     [DATA_WIDTH-1:0] r_data;
logic [(DATA_WIDTH/8)-1:0] r_keep;
logic                      r_last;
logic                      r_user;

always_ff @(posedge i_clk) begin
    if (i_reset) begin
        r_valid <= 0;
    end
    else if ((i_valid && o_ready) && (o_valid && !i_ready)) begin
        // We have incoming data, but the output is stalled
        r_valid <= 1;
    end
    else if (i_ready) begin
        r_valid <= 0;
    end
end

always_ff @(posedge i_clk) begin
    if (i_reset) begin
        r_data <= '0;
		r_keep <= '0;
		r_last <= 1'b0;
		r_user <= 1'b0;
	end
    else if((!REG_OUTPUT || i_valid) && o_ready) begin
        r_data <= i_data;
		r_keep <= i_keep;
		r_last <= i_last;
		r_user <= i_user;
    end
end

always_comb begin
    o_ready = !r_valid;
end

if (!REG_OUTPUT) begin : NET_OUTPUT
    // Outputs are combinatorially determined from inputs
    always_comb begin
        o_valid = !i_reset && (i_valid || r_valid);
    end

    // o_data, keep and last
    always_comb begin
        if (r_valid) begin
            o_data = r_data;
			o_keep = r_keep;
			o_last = r_last;
			o_user = r_user;
        end
        else if (i_valid) begin
            o_data = i_data;
			o_keep = i_keep;
			o_last = i_last;
			o_user = i_user;
        end
        else begin
            o_data = '0;
			o_keep = '0;
			o_last = 1'b0;
			o_user = 1'b0;
        end
    end
end
else begin : REG_OUT
    // Register our outputs
    always @(posedge i_clk) begin
        if (i_reset) begin
            o_valid <= 0;
        end
        else if (!o_valid || i_ready) begin
            o_valid <= (i_valid || r_valid);
        end
    end

    // o_data, keep and last
    always @(posedge i_clk) begin
        if (i_reset) begin
            o_data <= '0;
			o_keep <= '0;
			o_last <= 1'b0;
			o_user <= 1'b0;
        end
        else if (!o_valid || i_ready) begin
            if (r_valid) begin
                o_data <= r_data;
				o_keep <= r_keep;
				o_last <= r_last;
				o_user <= r_user;
            end
            else if (i_valid) begin
                o_data <= i_data;
				o_keep <= i_keep;
				o_last <= i_last;
				o_user <= i_user;
            end
            else begin
                o_data <= '0;
				o_keep <= '0;
				o_last <= 1'b0;
				o_user <= 1'b0;
            end
        end
    end
end

endmodule