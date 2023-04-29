//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module diven (
	// Global Control
	rst_n, clk, cke,
	
	// Divider I/F
	vin,
	dividend,
	divisor,
	quotient,
	vout
);


//==========================================================================
// Parameter Settings
//==========================================================================
//----------------------------------------------------------------------
// Primary Parameters
//----------------------------------------------------------------------
parameter DW = 26; // dividend bit width
parameter VW = 26; // divisor bit width
parameter QW = 26; // quotient bit width
parameter MSB_INV = 24; // number of most significant quotient bits that
                        // can be removed.

//----------------------------------------------------------------------
// Secondary Parameters
//----------------------------------------------------------------------
// how many bits remainder and divisor must be extended.
localparam EXT_REM = VW - MSB_INV;
localparam EXT_DIV = DW - MSB_INV - 1;

// bit width of remainder and extended divisor.
// polarity of EXT_DIV change the direction of extension.
localparam RW  = (EXT_DIV < 0) ? DW - EXT_DIV : DW + EXT_REM;
localparam EVW = (EXT_DIV < 0) ? VW : VW + EXT_DIV;


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input				rst_n, clk, cke;

// Divider I/F
input				vin;
input 	[DW-1:0]	dividend;
input	[VW-1:0]	divisor;
output	[QW-1:0]	quotient;
output				vout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Loop Index
integer				i;
genvar				j;

// Divider Operation
reg		[QW:0]		vin_r;
wire	[EVW-1:0]	ext_divisor;
wire	[RW-1:0]	ext_dividend;
wire	[QW:0]		op;
reg		[EVW-1:0]	div_r[QW:0];
reg		[RW-1:0]	rem_r[QW:0];
reg		[QW-1:0]	quot_r[QW:1];
wire	[QW-1:0]	quotient;
wire				vout;


//==========================================================================
// Function
//==========================================================================
function [RW-1:0] update;
	input	[EVW-1:0]	div;
	input	[RW-1:0]	rem;
	input				op;
	begin
		update = {rem[RW-2:0], ~op} + ({EVW+1{~op}} ^ {div[EVW-1:0], 1'b0});
	end
endfunction


//==========================================================================
// Divider Operation
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vin_r <= {QW+1{1'b0}};
	end
	else begin
		if (cke) begin
			vin_r <= {vin_r, vin};
		end
	end
end

// extend divisor and dividend beforehand so that the circiutry can be
// optimized.
assign ext_divisor[EVW-1:0] = (
	(EXT_DIV  < 0) ? divisor[VW-1:0] :
	(EXT_DIV == 0) ? divisor[VW-1:0] : // to avoid false bit insertion
	                 {divisor[VW-1:0], {EXT_DIV{1'b0}}}
);

assign ext_dividend[RW-1:0] = (
	(EXT_DIV < 0) ? {dividend[DW-1:0], {-EXT_DIV{1'b0}}} :
	                {{EXT_REM{dividend[DW-1]}}, dividend[DW-1:0]}
);

// operation of each stage is decided based on the polarity
// of remainder.
//   1 : add, 0 : sub
// should be inverted when divisor is negative.
for (j = 0; j < QW+1; j = j + 1) begin
	if (j == 0) begin
		assign op[0] = ext_divisor[EVW-1] ^ ext_dividend[RW-1];
	end
	else begin
		assign op[j] = div_r[j-1][EVW-1] ^ rem_r[j-1][RW-1];
	end
end

// first stage
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		div_r[0] <= {EVW{1'b0}};
		rem_r[0] <= {RW{1'b0}};
	end
	else begin
		if (cke) begin
			if (vin) begin
				div_r[0] <= ext_divisor;
				rem_r[0] <= update(ext_divisor, ext_dividend, op[0]);
			end
		end
	end
end

// ith stage
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 1; i < QW+1; i = i + 1) begin
			div_r[i]  <= {EVW{1'b0}};
			rem_r[i]  <= {RW{1'b0}};
			quot_r[i] <= 0;
		end
	end
	else begin
		if (cke) begin
			for (i = 1; i < QW+1; i = i + 1) begin
				if (vin_r[i-1]) begin
					div_r[i]  <= div_r[i-1];
					rem_r[i]  <= update(div_r[i-1], rem_r[i-1], op[i]);
					if (i == 1) begin
						quot_r[1] <= ~op[1];
					end
					else begin
						quot_r[i] <= {quot_r[i-1], ~op[i]};
					end
				end
			end
		end
	end
end

// quotient output
// add 1 in case of negative divisor.
//assign quotient[QW-1:0] = quot_r[QW][QW-1:0] + div_r[QW][VW-1];
assign quotient[QW-1:0] = quot_r[QW][QW-1:0] + div_r[QW][EVW-1];
assign vout = vin_r[QW];


endmodule
