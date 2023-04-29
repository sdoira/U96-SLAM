//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module rect_rmp (
	// Global Control
	rst_n, clk,
	
	// Remap Parameters
	l_fx, l_fy,
	r_fx, r_fy,
	cx, cy,
	fx2_inv, fy2_inv,
	cx2_fx2, cy2_fy2,
	l_r11, l_r12, l_r13,
	l_r21, l_r22, l_r23,
	l_r31, l_r32, l_r33,
	r_r11, r_r12, r_r13,
	r_r21, r_r22, r_r23,
	r_r31, r_r32, r_r33,
	
	// Control
	rdy,
	upd,
	vsync,
	
	// Command I/F
	cmd_mode,
	cmd_in,
	cmd_wr,
	cmd_rst,
	cmd_cnt,
	
	// Remap I/F
	vout,
	last_o,
	ydst_o,
	xdst_o,
	len_o,
	lr_o,
	ysrc_o,
	xsrc_o,
	skip_wait
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input					rst_n, clk;

// Remap Parameters
input			[25:0]	l_fx, l_fy, r_fx, r_fy;
input			[9:0]	cx;
input			[8:0]	cy;
input			[23:0]	fx2_inv, fy2_inv;
input			[23:0]	cx2_fx2, cy2_fy2;
input signed	[24:0]	l_r11, l_r12, l_r13;
input signed	[24:0]	l_r21, l_r22, l_r23;
input signed	[24:0]	l_r31, l_r32, l_r33;
input signed	[24:0]	r_r11, r_r12, r_r13;
input signed	[24:0]	r_r21, r_r22, r_r23;
input signed	[24:0]	r_r31, r_r32, r_r33;

// Control
input					rdy, upd;
input					vsync;

// Command I/F
input					cmd_mode;
input			[17:0]	cmd_in;
input					cmd_wr;
input					cmd_rst;
output			[14:0]	cmd_cnt;

// Remap I/F
output					vout;
output					last_o;
output			[8:0]	ydst_o;
output			[9:0]	xdst_o;
output			[6:0]	len_o;
output					lr_o;
output			[13:0]	ysrc_o;
output			[14:0]	xsrc_o;
input					skip_wait;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Loop Index
integer				i;

// Command Cueue
reg					cmd_mode_r;
wire				cmd_chg;
reg			[14:0]	cmd_addr;
wire				cmd_end;
reg			[14:0]	cmd_cnt;
wire		[17:0]	cmd_out;


// Pdst Calculation
reg					cmd_rd;
wire				cmd_y, cmd_x;
wire				cmd_lr;
wire		[8:0]	cmd_ydst;
wire		[9:0]	cmd_xdst;
wire				cmd_ydif;
wire		[6:0]	cmd_len;
reg			[8:0]	ydst;
reg					lr;
reg			[9:0]	xdst;
reg			[6:0]	len;
reg			[6:0]	lcnt;
wire				pdst_valid;

// Reverse Mapping Calculation
wire signed	[24:0]	r11, r12, r13;
wire signed	[24:0]	r21, r22, r23;
wire signed	[24:0]	r31, r32, r33;
reg			[25:0]	fx, fy;
wire		[33:0]	xdst_fx2, ydst_fy2;
reg			[24:0]	xdst_fx2_lim, ydst_fy2_lim;
reg  signed	[25:0]	xd, yd;
wire signed	[49:0]	r11x, r21y, r12x, r22y, r13x, r23y;
reg  signed	[25:0]	r11x_lim, r21y_lim;
reg  signed	[25:0]	r12x_lim, r22y_lim;
reg  signed	[25:0]	r13x_lim, r23y_lim;
wire signed	[26:0]	r11x_21y, r12x_22y, r13x_23y;
reg  signed	[25:0]	r11x_21y_lim, r12x_22y_lim, r13x_23y_lim;
wire signed	[26:0]	lx, ly, lw;
reg  signed	[25:0]	lx_lim, ly_lim, lw_lim;
wire		[80:0]	dly_in, dly_out;
reg			[80:0]	dly_out_r;
wire				pdst_v_dly;
wire		[8:0]	ydst_dly;
wire		[9:0]	xdst_dly;
wire		[6:0]	len_dly;
wire				lr_dly;
wire signed	[25:0]	lx_dly, ly_dly;
wire signed	[25:0]	lw_inv;
reg  signed	[25:0]	lw_inv_r;
wire signed	[50:0]	x2, y2;
reg  signed	[25:0]	x2_lim, y2_lim;
wire signed	[51:0]	x_fx, y_fy;
reg  signed	[16:0]	x_fx_lim, y_fy_lim;
wire signed	[17:0]	x_fx_cx, y_fy_cy;
wire		[15:0]	x_fx_cx_rnd;
wire		[14:0]	y_fy_cy_rnd;
reg			[14:0]	xsrc;
reg			[13:0]	ysrc;
reg			[8:0]	pdst_v_r;
reg			[8:0]	last_r;
reg			[8:0]	ydst_r[8:0];
reg			[9:0]	xdst_r[8:0];
reg			[6:0]	len_r[8:0];
reg			[36:0]	lr_r;

// Control
reg			[14:0]	xsrc_r;
reg			[13:0]	ysrc_r;
reg					init_cmd;
reg			[8:0]	int_ysrc;
wire				cke;
wire		[13:0]	ysrc_o;
wire		[14:0]	xsrc_o;
wire				vout;


//==========================================================================
// Command Cueue
//--------------------------------------------------------------------------
	// Y Command
	//  cmd[   17] : LR (L:0、R:1）
	//     [16: 8] : Y coord
	//     [    7] : not used
	//     [ 6: 0] : Length（Fixed 0）
	// X Command
	//  cmd[17: 8] : X coord
	//     [    7] : ydif
	//     [ 6: 0] : Length（1-127）
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		cmd_mode_r <= 1'b0;
	end
	else begin
		cmd_mode_r <= cmd_mode;
	end
end

assign cmd_chg = (cmd_mode != cmd_mode_r);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		cmd_addr <= 15'b0;
	end
	else begin
		if (cmd_chg) begin
			cmd_addr <= 0; // mode change to clear
		end
		else if (~cmd_mode) begin
			// write mode
			if (cmd_rst) begin
				cmd_addr <= 0;
			end
			else if (cmd_wr) begin
				cmd_addr <= cmd_addr + 1'b1;
			end
		end
		else begin
			if (cmd_rd & cke) begin
				if (cmd_end) begin
					cmd_addr <= 0;
				end
				else begin
					cmd_addr <= cmd_addr + 1'b1;
				end
			end
		end
	end
end

assign cmd_end = (cmd_addr == cmd_cnt);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		cmd_cnt <= 15'b0;
	end
	else begin
		if (cmd_rst) begin
			cmd_cnt <= 0;
		end
		else if (cmd_wr) begin
			cmd_cnt <= cmd_cnt + 1'b1;
		end
	end
end

cmd_cue cmd_cue (
	.clka  (clk),
	.ena   (1'b1),
	.wea   (cmd_wr),
	.addra (cmd_addr),
	.dina  (cmd_in),
	.douta (cmd_out)
);


//==========================================================================
// Receive Command
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		cmd_rd <= 1'b0;
	end
	else begin
		if (rdy) begin
			if (cke) begin
				cmd_rd <= ~cmd_rd & (lcnt == 0);
			end
		end
		else begin
			cmd_rd <= 1'b0;
		end
	end
end

// command type
assign cmd_y = (cmd_rd & (cmd_out[6:0] == 7'b0));
assign cmd_x = (cmd_rd & (cmd_out[6:0] != 7'b0));

// command encode
assign cmd_lr        = cmd_out[17  ];
assign cmd_ydst[8:0] = cmd_out[16:8];
assign cmd_xdst[9:0] = cmd_out[17:8];
assign cmd_ydif      = cmd_out[ 7  ];
assign cmd_len[6:0]  = cmd_out[ 6:0];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ydst <= 9'b0;
		lr   <= 1'b0;
	end
	else if (cke) begin
		if (cmd_y) begin
			ydst <= cmd_ydst[8:0];
			lr   <= cmd_lr;
		end
		else if (cmd_x) begin
			ydst <= ydst + cmd_ydif;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xdst <= 10'b0;
		len  <=  7'b0;
	end
	else if (cke) begin
		if (cmd_x) begin
			xdst <= cmd_xdst[9:0];
			len  <= cmd_len[6:0];
		end
		else if (pdst_valid) begin
			xdst <= xdst + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		lcnt <= 7'b0;
	end
	else if (cke) begin
		if (cmd_x) begin
			lcnt <= cmd_len[6:0];
		end
		else if (pdst_valid) begin
			lcnt <= lcnt - 1'b1;
		end
	end
end

assign pdst_valid = (lcnt != 0);


//==========================================================================
// Reverse Mapping Calculation
//==========================================================================
//----------------------------------------------------------------------
// Parameter Select
//----------------------------------------------------------------------
assign r11 = (~lr_r[1]) ? l_r11 : r_r11;
assign r21 = (~lr_r[1]) ? l_r21 : r_r21;
assign r12 = (~lr_r[1]) ? l_r12 : r_r12;
assign r22 = (~lr_r[1]) ? l_r22 : r_r22;
assign r13 = (~lr_r[1]) ? l_r13 : r_r13;
assign r23 = (~lr_r[1]) ? l_r23 : r_r23;
assign r31 = (~lr_r[3]) ? l_r31 : r_r31;
assign r32 = (~lr_r[3]) ? l_r32 : r_r32;
assign r33 = (~lr_r[3]) ? l_r33 : r_r33;
//assign fx  = (~lr_r[33]) ? l_fx  : r_fx;
//assign fy  = (~lr_r[33]) ? l_fy  : r_fy;

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		fx <= 26'b0;
		fy <= 26'b0;
	end
	else if (cke) begin
		fx <= (~lr_r[32]) ? l_fx  : r_fx;
		fy <= (~lr_r[32]) ? l_fy  : r_fy;
	end
end


//----------------------------------------------------------------------
// 3D Projection
//----------------------------------------------------------------------
// u10.0 * u-8.32 = u2.32 -> u1.24
assign xdst_fx2 = xdst * fx2_inv;
assign ydst_fy2 = ydst * fy2_inv;

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xdst_fx2_lim <= 25'b0;
		ydst_fy2_lim <= 25'b0;
	end
	else if (cke & pdst_valid) begin
		xdst_fx2_lim <= xdst_fx2[32:8];
		ydst_fy2_lim <= ydst_fy2[32:8];
	end
end

// u1.24 - u0.24 -> s1.24
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xd <= 26'b0;
		yd <= 26'b0;
	end
	else if (cke) begin
		xd <= xdst_fx2_lim - cx2_fx2;
		yd <= ydst_fy2_lim - cy2_fy2;
	end
end


//----------------------------------------------------------------------
// 3D Rotation
//----------------------------------------------------------------------
// s0.24 * s1.24 = s1.48
assign r11x = r11 * xd;
assign r21y = r21 * yd;
assign r12x = r12 * xd;
assign r22y = r22 * yd;
assign r13x = r13 * xd;
assign r23y = r23 * yd;

// s1.48 -> s1.24
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		r11x_lim <= 26'b0;
		r21y_lim <= 26'b0;
		r12x_lim <= 26'b0;
		r22y_lim <= 26'b0;
		r13x_lim <= 26'b0;
		r23y_lim <= 26'b0;
	end
	else if (cke) begin
		r11x_lim <= r11x[49:24];
		r21y_lim <= r21y[49:24];
		r12x_lim <= r12x[49:24];
		r22y_lim <= r22y[49:24];
		r13x_lim <= r13x[49:24];
		r23y_lim <= r23y[49:24];
	end
end

// s1.24 + s1.24 = s2.24
assign r11x_21y = r11x_lim + r21y_lim;
assign r12x_22y = r12x_lim + r22y_lim;
assign r13x_23y = r13x_lim + r23y_lim;

// s2.24 -> s1.24
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		r11x_21y_lim <= 26'b0;
		r12x_22y_lim <= 26'b0;
		r13x_23y_lim <= 26'b0;
	end
	else if (cke) begin
		r11x_21y_lim <= {r11x_21y[26], r11x_21y[24:0]};
		r12x_22y_lim <= {r12x_22y[26], r12x_22y[24:0]};
		r13x_23y_lim <= {r13x_23y[26], r13x_23y[24:0]};
	end
end

// s1.24 + s0.24 = s2.24
assign lx = r11x_21y_lim + r31;
assign ly = r12x_22y_lim + r32;
assign lw = r13x_23y_lim + r33;

// s2.24 -> s1.24
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		lx_lim <= 26'b0;
		ly_lim <= 26'b0;
		lw_lim <= 26'b0;
	end
	else if (cke) begin
		lx_lim <= {lx[26], lx[24:0]};
		ly_lim <= {ly[26], ly[24:0]};
		lw_lim <= {lw[26], lw[24:0]};
	end
end


//----------------------------------------------------------------------
// 2D Projection
//----------------------------------------------------------------------
// delay matching with divider
assign dly_in[80:0] = {pdst_v_r[4], last_r[4], ydst_r[4][8:0], xdst_r[4][9:0], len_r[4][6:0], lr_r[4], lx_lim[25:0], ly_lim[25:0]};

// I had to rename the module because there was an issue where number of
// pipeline stages can not be changed in the shift register.
rect_dly2 rect_dly (
	.D    (dly_in[80:0]),
	.CLK  (clk),
	.CE   (cke),
	//.SCLR (~rst_n),
	.Q    (dly_out[80:0])
);

// replaced the last stage with flipflops to improve timing.
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dly_out_r <= 81'b0;
	end
	else if (cke) begin
		dly_out_r <= dly_out;
	end
end

assign pdst_v_dly    = dly_out_r[80];
assign last_dly      = dly_out_r[79];
assign ydst_dly[8:0] = dly_out_r[78:70];
assign xdst_dly[9:0] = dly_out_r[69:60];
assign len_dly[6:0]  = dly_out_r[59:53];
assign lr_dly        = dly_out_r[52];
assign lx_dly[25:0]  = dly_out_r[51:26];
assign ly_dly[25:0]  = dly_out_r[25: 0];
/*
assign pdst_v_dly    = dly_out[80];
assign last_dly      = dly_out[79];
assign ydst_dly[8:0] = dly_out[78:70];
assign xdst_dly[9:0] = dly_out[69:60];
assign len_dly[6:0]  = dly_out[59:53];
assign lr_dly        = dly_out[52];
assign lx_dly[25:0]  = dly_out[51:26];
assign ly_dly[25:0]  = dly_out[25: 0];
*/
// s1.24 / s1.24 = s25.24 -> s1.24
diven #(26, 26, 26, 24) diven (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	.cke   (cke),
	
	// Divider I/F
	.vin      (1'b1),
	.dividend (26'h100_0000),
	.divisor  (lw_lim[25:0]),
	.quotient (lw_inv[25:0]),
	.vout     ()
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		lw_inv_r <= 26'b0;
	end
	else if (cke) begin
		lw_inv_r <= lw_inv;
	end
end

// s1.24 * s1.24 = s2.48
assign x2 = lx_dly * lw_inv_r;
assign y2 = ly_dly * lw_inv_r;

// s2.48 -> s1.24
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		x2_lim <= 26'b0;
		y2_lim <= 26'b0;
	end
	else if (cke) begin
		x2_lim <= {x2[50], x2[48:24]};
		y2_lim <= {y2[50], y2[48:24]};
	end
end

// s1.24 * u10.16 = s11.40
assign x_fx = x2_lim * $signed({1'b0, fx[25:0]});
assign y_fy = y2_lim * $signed({1'b0, fy[25:0]});

// s11.40 -> s10.6
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		x_fx_lim <= 17'b0;
		y_fy_lim <= 17'b0;
	end
	else if (cke) begin
		x_fx_lim <= {x_fx[51], x_fx[49:34]};
		y_fy_lim <= {y_fy[51], y_fy[49:34]};
	end
end

// s10.6 + u10.0 = s11.6 (X)
// s10.6 +  u9.0 = s11.6 (Y)
assign x_fx_cx = x_fx_lim + $signed({1'b0, cx[9:0], 6'b0});
assign y_fy_cy = y_fy_lim + $signed({1'b0, cy[8:0], 6'b0});

// s11.6 -> u10.6 (x, round)
// s11.6 ->  u9.6 (y, round)
assign x_fx_cx_rnd = x_fx_cx[15:0] + 1'b1;
assign y_fy_cy_rnd = y_fy_cy[14:0] + 1'b1;

// u10.6 -> u10.5 (x)
//  u9.6 ->  u9.5 (y)
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xsrc <= 15'b0;
		ysrc <= 14'b0;
	end
	else if (cke) begin
		xsrc <= x_fx_cx_rnd[15:1];
		ysrc <= y_fy_cy_rnd[14:1];
	end
end


//----------------------------------------------------------------------
// LR/LEN Delay
//----------------------------------------------------------------------
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 9; i = i + 1) begin
			pdst_v_r[i] <=  1'b0;
			last_r[i]   <=  1'b0;
			ydst_r[i]   <=  9'b0;
			xdst_r[i]   <= 10'b0;
			len_r[i]    <=  7'b0;
		end
	end
	else if (cke) begin
		for (i = 0; i < 9; i = i + 1) begin
			if (i == 0) begin
				pdst_v_r[0] <= pdst_valid;
				if (pdst_valid) begin
					last_r[0]   <= cmd_end;
					ydst_r[0]   <= ydst;
					xdst_r[0]   <= xdst;
					len_r[0]    <= len;
				end
			end
			else if (i == 5) begin
				pdst_v_r[5] <= pdst_v_dly;
				last_r[5]   <= last_dly;
				ydst_r[5]   <= ydst_dly;
				xdst_r[5]   <= xdst_dly;
				len_r[5]    <= len_dly;
			end
			else begin
				pdst_v_r[i] <= pdst_v_r[i-1];
				last_r[i]   <= last_r[i-1];
				ydst_r[i]   <= ydst_r[i-1];
				xdst_r[i]   <= xdst_r[i-1];
				len_r[i]    <= len_r[i-1];
			end
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 37; i = i + 1) begin
			lr_r[i] <=  1'b0;
		end
	end
	else if (cke) begin
		for (i = 0; i < 37; i = i + 1) begin
			if (i == 0) begin
				if (pdst_valid) begin
					lr_r[0] <= lr;
				end
			end
			else begin
				lr_r[i] <= lr_r[i-1];
			end
		end
	end
end

assign last_o = last_r[8];
assign ydst_o = ydst_r[8];
assign xdst_o = xdst_r[8];
assign len_o  = len_r[8];
assign lr_o   = lr_r[36];


//==========================================================================
// Control
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xsrc_r <= 15'b0;
		ysrc_r <= 14'b0;
	end
	else if (cke) begin
		xsrc_r <= xsrc;
		ysrc_r <= ysrc;
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		int_ysrc <= 9'b0;
	end
	else begin
		if (vsync) begin
			int_ysrc <= 0;
		end
		else if (upd) begin
			int_ysrc <= int_ysrc + 1'b1;
		end
	end
end

// high until first command is executed
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		init_cmd <= 1'b0;
	end
	else begin
		if (~cmd_mode) begin
			init_cmd <= 1'b1;
		end
		else if (pdst_v_r[7]) begin
			init_cmd <= 1'b0;
		end
	end
end

//assign cke = (init_cmd) ? 1'b1 : ((ysrc[13:5] == int_ysrc) & ~skip_wait);
assign cke = (init_cmd) ? 1'b1 : ((ysrc[13:5] == int_ysrc) & ~skip_wait & rdy);

assign ysrc_o[13:0] = ysrc_r[13:0];
assign xsrc_o[14:0] = xsrc_r[14:0];
assign vout = pdst_v_r[8] & cke;


endmodule
