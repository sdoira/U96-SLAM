//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module gftt_sbl (
	// Global Control
	rst_n, clk,
	
	// Frontend I/F
	line0,
	line1,
	line2,
	vin,
	first_smpl,
	last_smpl,
	
	// Backend I/F
	xsbl,
	ysbl,
	vout
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Frontend I/F
input	[7:0]	line0, line1, line2;
input			vin;
input			first_smpl, last_smpl;

// Backend I/F
output	[11:0]	xsbl, ysbl;
output			vout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Shift Register
wire	[7:0]	line0_r0, line1_r0, line2_r0;
reg		[7:0]	line0_r1, line0_r2;
reg		[7:0]	line1_r1, line1_r2;
reg		[7:0]	line2_r1, line2_r2;
reg		[2:0]	first_r, last_r;
reg		[3:0]	vin_r;
wire			vout;

// X-Sobel Calculation
reg		[8:0]	xdif0, xdif1, xdif2;
reg		[10:0]	xadd;
reg		[8:0]	xdif2_r;
reg		[11:0]	xsbl;

// Y-Sobel Calculation
reg		[8:0]	ydif, ydif_r;
reg		[10:0]	yadd;
reg		[11:0]	ysbl;


//==========================================================================
// Shift Register
//==========================================================================
assign line0_r0[7:0] = line0;
assign line1_r0[7:0] = line1;
assign line2_r0[7:0] = line2;

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		line0_r2 <= 8'b0;
		line0_r1 <= 8'b0;
		line1_r2 <= 8'b0;
		line1_r1 <= 8'b0;
		line2_r2 <= 8'b0;
		line2_r1 <= 8'b0;
	end
	else begin
		line0_r2 <= line0_r1;
		line0_r1 <= line0_r0;
		line1_r2 <= line1_r1;
		line1_r1 <= line1_r0;
		line2_r2 <= line2_r1;
		line2_r1 <= line2_r0;
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		first_r <= 3'b0;
		last_r  <= 3'b0;
	end
	else begin
		first_r <= {first_r[1:0], first_smpl};
		last_r  <= {last_r[1:0], last_smpl};
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vin_r <= 4'b0;
	end
	else begin
		vin_r <= {vin_r[2:0], vin};
	end
end

assign vout = vin_r[3];


//==========================================================================
// X-Sobel Calculation
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xdif0 <= 9'b0;
		xdif1 <= 9'b0;
		xdif2 <= 9'b0;
	end
	else begin
		xdif0 <= line0_r0[7:0] - line0_r2[7:0];
		xdif1 <= line1_r0[7:0] - line1_r2[7:0];
		xdif2 <= line2_r0[7:0] - line2_r2[7:0];
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xadd <= 11'b0;
	end
	else begin
		xadd <= {{2{xdif0[8]}}, xdif0[8:0]} + {xdif1[8], xdif1[8:0], 1'b0};
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xdif2_r <= 9'b0;
	end
	else begin
		xdif2_r <= xdif2;
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xsbl <= 12'b0;
	end
	else begin
		if (~vin_r[2] | first_r[2] | last_r[2]) begin
			xsbl <= 0;
		end
		else begin
			xsbl <= {xadd[10], xadd[10:0]} + {{3{xdif2_r[8]}}, xdif2_r[8:0]};
		end
	end
end


//==========================================================================
// Y-Sobel Calculation
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ydif <= 9'b0;
	end
	else begin
		ydif <= line2_r1[7:0] - line0_r1[7:0];
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ydif_r <= 9'b0;
	end
	else begin
		ydif_r <= ydif;
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		yadd <= 11'b0;
	end
	else begin
		yadd <= {{2{ydif_r[8]}}, ydif_r[8:0]} + {ydif[8], ydif[8:0], 1'b0};
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ysbl <= 12'b0;
	end
	else begin
		if (~vin_r[2] | first_r[2] | last_r[2]) begin
			ysbl <= 0;
		end
		else begin
			ysbl <= {yadd[10], yadd[10:0]} + {{3{ydif[8]}}, ydif[8:0]};
		end
	end
end


endmodule
