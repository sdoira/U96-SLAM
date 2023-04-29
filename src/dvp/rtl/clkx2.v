//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1ns / 1ps

//==========================================================================
// Conveys a high-active pulse signal across different clock domains.
//--------------------------------------------------------------------------
// This module assumes that clk2 is higher than clk1.
//==========================================================================
module clkx2 (
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
reg		[2:0]	in_r;


//==========================================================================
// Clock Domain Crossing
//==========================================================================
always @(negedge rst_n or posedge clk2) begin
	if (~rst_n) begin
		in_r <= 3'b0;
	end
	else begin
		in_r <= {in_r[1:0], in};
	end
end

assign out = ~in_r[2] & in_r[1];


endmodule
