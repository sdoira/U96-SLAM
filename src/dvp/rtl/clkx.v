//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1ns / 1ps

//==========================================================================
// Conveys an active-high pulse signal across different clock domains.
//--------------------------------------------------------------------------
// clk1 can be higher than clk2.
//==========================================================================
module clkx (
	rst_n,
	clk1, in,
	clk2, out
);


//==========================================================================
// Port Declaration
//==========================================================================
// Reset/Clock
input			rst_n;

// Clock Domain 1
input			clk1, in;

// Clock Domain 2
input			clk2;
output			out;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
reg				in_r1, in_r2, in_r3;


//==========================================================================
// Clock Domain Crossing
//==========================================================================
always @(negedge rst_n or posedge clk1) begin
	if (~rst_n) begin
		in_r1 <= 1'b0;
	end
	else begin
		if (in_r3) begin
			in_r1 <= 1'b0;
		end
		else if (in) begin
			in_r1 <= 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk2) begin
	if (~rst_n) begin
		in_r2 <= 1'b0;
	end
	else begin
		if (in_r3) begin
			in_r2 <= 1'b0;
		end
		else if (in_r1) begin
			in_r2 <= 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk2) begin
	if (~rst_n) begin
		in_r3 <= 1'b0;
	end
	else begin
		if (in_r3) begin
			in_r3 <= 1'b0;
		end
		else if (in_r2) begin
			in_r3 <= 1'b1;
		end
	end
end

assign out = in_r3;


endmodule
