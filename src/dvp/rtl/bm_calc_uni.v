//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module bm_calc_uni (
	// Global Control
	rst_n, clk,
	
	// Control
	uni_on,
	
	// Input
	upd_in,
	min1_in,
	min2_in,
	disp1_in,
	disp2_in,
	frac_in,
	vin,
	
	// Output
	upd_out,
	min1_out, min2_out,
	disp1_out, disp2_out,
	frac_out,
    uni_ratio,
    vout
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Control
input			uni_on;

// Input
input			upd_in;
input	[15:0]	min1_in, min2_in;
input	[7:0]	disp1_in, disp2_in;
input	[7:0]	frac_in;
input			vin;

// Output
output			upd_out;
output	[15:0]	min1_out, min2_out;
output	[7:0]	disp1_out, disp2_out;
output	[7:0]	frac_out;
output	[9:0]	uni_ratio;
output			vout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
integer			i;

// Delay Match
reg				upd_in_r[11:0];
reg		[15:0]	min1_in_r[11:0];
reg		[15:0]	min2_in_r[11:0];
reg		[7:0]	disp1_in_r[11:0];
reg		[7:0]	disp2_in_r[11:0];
reg		[7:0]	frac_in_r[11:0];

// Uniqueness Filter
wire	[10:0]	div_out_uni;
wire			vout;
wire	[9:0]	uni_ratio;


//==========================================================================
// Delay Match
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 12; i = i + 1) begin
			upd_in_r[i]   <=  1'b0;
			min1_in_r[i]  <= 16'b0;
			min2_in_r[i]  <= 16'b0;
			disp1_in_r[i] <=  8'b0;
			disp2_in_r[i] <=  8'b0;
			frac_in_r[i]  <=  8'b0;
		end
	end
	else begin
		for (i = 1; i < 12; i = i + 1) begin
			upd_in_r[i]   <= upd_in_r[i-1];
			min1_in_r[i]  <= min1_in_r[i-1];
			min2_in_r[i]  <= min2_in_r[i-1];
			disp1_in_r[i] <= disp1_in_r[i-1];
			disp2_in_r[i] <= disp2_in_r[i-1];
			frac_in_r[i]  <= frac_in_r[i-1];
		end
		upd_in_r[0]   <= upd_in;
		min1_in_r[0]  <= min1_in;
		min2_in_r[0]  <= min2_in;
		disp1_in_r[0] <= disp1_in;
		disp2_in_r[0] <= disp2_in;
		frac_in_r[0]  <= frac_in;
	end
end

assign upd_out   = upd_in_r[11];
assign disp1_out = disp1_in_r[11];
assign disp2_out = disp2_in_r[11];
assign min1_out  = min1_in_r[11];
assign min2_out  = min2_in_r[11];
assign frac_out  = frac_in_r[11];


//==========================================================================
// Uniqueness Filter
//==========================================================================
diven #(17, 17, 11, 16) div_uni (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	.cke   (1'b1),
	
	// Divider I/F
	.vin      (vin),
	.dividend ({1'b0, min1_in[15:0]}),
	.divisor  ({1'b0, min2_in[15:0]}),
	.quotient (div_out_uni[10:0]),
	.vout     (vout)
);

assign uni_ratio[9:0] = (uni_on) ? div_out_uni[9:0] : 10'b0; // always positive, drop sign bit


endmodule
