//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module gftt_eig (
	// Global Control
	rst_n, clk,
	
	// Parameter
	wdt_m1,
	
	// Control
	start,
	enb,
	
	// Frontend I/F
	dx,
	dy,
	vin,
	
	// Backend I/F
	dout,
	vout
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Parameter
input	[10:0]	wdt_m1;

// Control
input			start;
input			enb;

// Frontend I/F
input	[11:0]	dx, dy;
input			vin;

// Backend I/F
output	[15:0]	dout;
output			vout;


//==========================================================================
// Function
//==========================================================================
// (s11.0) -> (u11.0), 2's complementary, no largest negative value
function [10:0] abs11;
	input	[11:0]	in;
	begin
		if (in[11]) begin // negative
			abs11[10:0] = 13'h1000 - in[11:0];
		end
		else begin // positive
			abs11[10:0] = in[10:0];
		end
	end
endfunction

function [15:0] abs16;
	input	[16:0]	in;
	begin
		if (in[16]) begin // negative
			abs16[15:0] = 18'h2_0000 - in[16:0];
		end
		else begin // positive
			abs16[15:0] = in[15:0];
		end
	end
endfunction


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
integer			i;

// Box Filter
wire	[10:0]	dx_abs, dy_abs;
reg		[21:0]	dx2, dy2, dxdy;
wire	[15:0]	dx2_lim, dy2_lim, dxdy_lim;
reg				vin_r;
wire	[15:0]	a, b, c;
wire			box_vout;
reg		[2:0]	box_vout_r;

// A + C Path
reg		[16:0]	apc;
reg		[16:0]	apc_r[18:0];

// Square Root Path
wire	[16:0]	amc;
reg		[15:0]	amc_abs;
reg		[15:0]	b_r;
wire	[31:0]	amc2;
reg		[21:0]	amc2_lim;
wire	[31:0]	b2;
reg		[23:0]	b2_lim;
wire	[24:0]	amc2pb2;
reg		[21:0]	amc2pb2_lim;
wire	[16:0]	sqrt;
wire			sqrt_vout;
reg				sqrt_vout_r;

// Eigen Value Output
wire	[17:0]	eig;
reg		[15:0]	eig_lim;
wire	[15:0]	dout;
wire			vout;


//==========================================================================
// Box Filter
//==========================================================================
// s11.0 -> u11.0
assign dx_abs[10:0] = abs11(dx[11:0]);
assign dy_abs[10:0] = abs11(dy[11:0]);

// u11.0 * u11.0 -> u22.0
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dx2  <= 22'b0;
		dy2  <= 22'b0;
		dxdy <= 22'b0;
	end
	else begin
		dx2  <= dx_abs[10:0] * dx_abs[10:0];
		dy2  <= dy_abs[10:0] * dy_abs[10:0];
		dxdy <= dx_abs[10:0] * dy_abs[10:0];
	end
end

// u22.0 -> u22.-6
assign dx2_lim[15:0]  = dx2[21:6];
assign dy2_lim[15:0]  = dy2[21:6];
assign dxdy_lim[15:0] = dxdy[21:6];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vin_r <= 1'b0;
	end
	else begin
		vin_r <= vin;
	end
end

gftt_box gftt_box_dx (
	// Global Control
	.rst_n (rst_n),
	.clk (clk),
	
	// Parameter
	.wdt_m1 (wdt_m1),
	
	// Control
	.start (start),
	.enb (enb),
	
	// Frontend I/F
	.din (dx2_lim[15:0]),
	.vin (vin_r),
	
	// Backend I/F
	.dout (a[15:0]),
	.vout (box_vout)
);

gftt_box gftt_box_dy (
	// Global Control
	.rst_n (rst_n),
	.clk (clk),
	
	// Parameter
	.wdt_m1 (wdt_m1),
	
	// Control
	.start (start),
	.enb (enb),
	
	// Frontend I/F
	.din (dy2_lim[15:0]),
	.vin (vin_r),
	
	// Backend I/F
	.dout (c[15:0]),
	.vout ()
);

gftt_box gftt_box_dxdy (
	// Global Control
	.rst_n (rst_n),
	.clk (clk),
	
	// Parameter
	.wdt_m1 (wdt_m1),
	
	// Control
	.start (start),
	.enb (enb),
	
	// Frontend I/F
	.din (dxdy_lim[15:0]),
	.vin (vin_r),
	
	// Backend I/F
	.dout (b[15:0]),
	.vout ()
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		box_vout_r <= 3'b0;
	end
	else begin
		box_vout_r <= {box_vout_r[1:0], box_vout};
	end
end


//==========================================================================
// A + C Path
//==========================================================================
// u21.-5 + u21.-5 -> u22.-5
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		apc <= 17'b0;
	end
	else begin
		apc <= a[15:0] + c[15:0];
		
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 19; i = i + 1) begin
			apc_r[i] <= 17'b0;
		end
	end
	else begin
		for (i = 1; i < 19; i = i + 1) begin
			apc_r[i] <= apc_r[i-1];
		end
		apc_r[0] <= apc;
	end
end


//==========================================================================
// Square Root Path
//==========================================================================
// u21.-5 - u21.-5 -> s21.-5
assign amc[16:0] = a[15:0] - c[15:0];

// s21.-5 -> u21.-5
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		amc_abs <= 16'b0;
	end
	else begin
		amc_abs <= abs16(amc[16:0]);
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		b_r <= 16'b0;
	end
	else begin
		b_r <= b[15:0];
	end
end

// u21.-5 * u21.-5 -> u42.-10
assign amc2[31:0] = amc_abs[15:0] * amc_abs[15:0];

// u42.-10 -> u42.-20
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		amc2_lim <= 22'b0;
	end
	else begin
		amc2_lim <= amc2[31:10];
	end
end

// u22.-6 * u22.-6 -> u44.-12
assign b2[31:0] = b_r[15:0] * b_r[15:0];

// u44.-12 -> u44.-20
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		b2_lim <= 24'b0;
	end
	else begin
		b2_lim <= b2[31:8];
	end
end

// u42.-20 + u44.-20 -> u45.-20
assign amc2pb2[24:0] = amc2_lim[21:0] + b2_lim[23:0];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		amc2pb2_lim <= 22'b0;
	end
	else begin
		amc2pb2_lim <= (|amc2pb2[24:22]) ? 22'h3F_FFFF : amc2pb2[21:0];
	end
end

gftt_sqrt gftt_sqrt (
	.aclk (clk),
	.s_axis_cartesian_tdata  ({amc2pb2_lim[21:0], 10'b0}),
	.s_axis_cartesian_tvalid (box_vout_r[2]),
	.m_axis_dout_tdata       (sqrt[16:0]),
	.m_axis_dout_tvalid      (sqrt_vout)
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		sqrt_vout_r <= 1'b0;
	end
	else begin
		sqrt_vout_r <= sqrt_vout;
	end
end


//==========================================================================
// Eigen Value Output
//==========================================================================
// u22.-5 - u21.-5 -> s22.-5
assign eig[17:0] = apc_r[18][16:0] - sqrt[15:0];

// s22.-5 -> u21.-5
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		eig_lim <= 16'b0;
	end
	else begin
		if (eig[17]) begin
			eig_lim <= 16'h0000;
		end
		else if (eig[16]) begin
			eig_lim <= 16'hFFFF;
		end
		else begin
			eig_lim <= eig[15:0];
		end
	end
end

assign dout[15:0] = (vout) ? eig_lim[15:0] : 16'b0;
assign vout = sqrt_vout_r;


endmodule
