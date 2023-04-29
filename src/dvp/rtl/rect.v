//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module rect (
	// Global Control
	rst_n, clk,
	
	// Internal Bus I/F
	ibus_cs,
	ibus_wr,
	ibus_addr_7_2,
	ibus_wrdata,
	ibus_rddata,
	
	// Control
	rect_done,
	rect_fcnt,
	bm_done,
	bm_enb,
	
	// Sensor Input
	pclk,
	vsync,
	href,
	d_l,
	d_r,
	
	// DDR I/F
	ddr_req,
	ddr_ack,
	ddr_dout,
	ddr_strb,
	ddr_vout
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Internal Bus I/F
input			ibus_cs;
input			ibus_wr;
input	[5:0]	ibus_addr_7_2;
input	[31:0]	ibus_wrdata;
output	[31:0]	ibus_rddata;

// Control
input			rect_done;
output	[3:0]	rect_fcnt;
input			bm_done;
input			bm_enb;

// Sensor Input
input			pclk, vsync, href;
input	[7:0]	d_l, d_r;

// DDR I/F
output			ddr_req;
input			ddr_ack;
output	[31:0]	ddr_dout;
output	[3:0]	ddr_strb;
output			ddr_vout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Register Interface
reg				enb;
reg				cmd_mode;
reg		[25:0]	l_fx, l_fy, r_fx, r_fy;
reg		[9:0]	cx;
reg		[8:0]	cy;
reg		[23:0]	fx2_inv, fy2_inv;
reg		[23:0]	cx2_fx2, cy2_fy2;
reg		[24:0]	l_r11, l_r12, l_r13;
reg		[24:0]	l_r21, l_r22, l_r23;
reg		[24:0]	l_r31, l_r32, l_r33;
reg		[24:0]	r_r11, r_r12, r_r13;
reg		[24:0]	r_r21, r_r22, r_r23;
reg		[24:0]	r_r31, r_r32, r_r33;
reg		[11:0]	base_a, base_b;
wire	[31:0]	ibus_rddata;
reg				cmd_rst;
wire	[17:0]	cmd_in;
wire			cmd_wr;

// Sequence Control (PCLK domain)
reg				vsync_r;
wire			vsync_pos_pclk;
reg				rect_start_pclk;
wire			bm_done_pclk;
reg		[1:0]	cue_cnt;

// Sequence Control (CLK domain)
wire			vsync_pos, rect_start;
reg				rect_on;
reg		[3:0]	frm_cnt, rect_fcnt;

// rect_ibuf
wire			ibuf_rdy, ibuf_upd;
wire	[7:0]	ibuf_up, ibuf_lo;

// rect_rmp
wire	[14:0]	cmd_cnt;
wire			rmp_v;
wire			rmp_last;
wire	[8:0]	rmp_ydst;
wire	[9:0]	rmp_xdst;
wire	[6:0]	rmp_len;
wire			rmp_lr;
wire	[13:0]	rmp_ysrc;
wire	[14:0]	rmp_xsrc;

// rect_intp
wire			ibuf_lr;
wire	[9:0]	ibuf_rdaddr;
wire			intp_v;
wire			intp_last;
wire			intp_lr;
wire	[8:0]	intp_ydst;
wire	[9:0]	intp_xdst;
wire	[6:0]	intp_len;
wire	[7:0]	intp_dout;
wire			skip_wait;

// rect_obuf
wire			frm_end;
wire			dff_ovf, cff_ovf;
wire			ddr_req;
wire	[31:0]	ddr_dout;
wire	[3:0]	ddr_strb;
wire			ddr_vout;


//==========================================================================
// Register Interface
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		enb      <=  1'b0;
		cmd_mode <=  1'b0;
		l_fx     <= 26'h283_025D;
		l_fy     <= 26'h288_27C8;
		r_fx     <= 26'h27C_293E;
		r_fy     <= 26'h27E_F988;
		cx       <= 10'd320;
		cy       <=  9'd240;
		fx2_inv  <= 24'h5E_2DE9;
		fy2_inv  <= 24'h5E_2DE9;
		cx2_fx2  <= 24'h57_3F31;
		cy2_fy2  <= 24'h58_6143;
		l_r11    <= 25'h0FE_6652;
		l_r12    <= 25'hFFF_BB30;
		l_r13    <= 25'h01C_940B;
		l_r21    <= 25'h000_1002;
		l_r22    <= 25'h0FF_FE48;
		l_r23    <= 25'h001_D9DF;
		l_r31    <= 25'hFE3_6BA7;
		l_r32    <= 25'hFFE_2AE1;
		l_r33    <= 25'h0FE_64A2;
		r_r11    <= 25'h0FE_D846;
		r_r12    <= 25'h003_28F6;
		r_r13    <= 25'h018_15FF;
		r_r21    <= 25'hFFD_038B;
		r_r22    <= 25'h0FF_F990;
		r_r23    <= 25'hFFE_0330;
		r_r31    <= 25'hFE7_E455;
		r_r32    <= 25'h001_B297;
		r_r33    <= 25'h0FE_DB4E;
		base_a   <= 12'h300;
		base_b   <= 12'h310;
	end
	else begin
		if (ibus_cs & ibus_wr) begin
			case (ibus_addr_7_2)
				6'h00: enb      <= ibus_wrdata[    0];
				6'h02: cmd_mode <= ibus_wrdata[    1];
				6'h05: l_fx     <= ibus_wrdata[25: 0];
				6'h06: l_fy     <= ibus_wrdata[25: 0];
				6'h07: r_fx     <= ibus_wrdata[25: 0];
				6'h08: r_fy     <= ibus_wrdata[25: 0];
				6'h09: begin
					cx <= ibus_wrdata[25:16];
					cy <= ibus_wrdata[8:0];
				end
				6'h0A: fx2_inv  <= ibus_wrdata[23: 0];
				6'h0B: fy2_inv  <= ibus_wrdata[23: 0];
				6'h0C: cx2_fx2  <= ibus_wrdata[23: 0];
				6'h0D: cy2_fy2  <= ibus_wrdata[23: 0];
				6'h0E: l_r11    <= ibus_wrdata[24: 0];
				6'h0F: l_r12    <= ibus_wrdata[24: 0];
				6'h10: l_r13    <= ibus_wrdata[24: 0];
				6'h11: l_r21    <= ibus_wrdata[24: 0];
				6'h12: l_r22    <= ibus_wrdata[24: 0];
				6'h13: l_r23    <= ibus_wrdata[24: 0];
				6'h14: l_r31    <= ibus_wrdata[24: 0];
				6'h15: l_r32    <= ibus_wrdata[24: 0];
				6'h16: l_r33    <= ibus_wrdata[24: 0];
				6'h17: r_r11    <= ibus_wrdata[24: 0];
				6'h18: r_r12    <= ibus_wrdata[24: 0];
				6'h19: r_r13    <= ibus_wrdata[24: 0];
				6'h1A: r_r21    <= ibus_wrdata[24: 0];
				6'h1B: r_r22    <= ibus_wrdata[24: 0];
				6'h1C: r_r23    <= ibus_wrdata[24: 0];
				6'h1D: r_r31    <= ibus_wrdata[24: 0];
				6'h1E: r_r32    <= ibus_wrdata[24: 0];
				6'h1F: r_r33    <= ibus_wrdata[24: 0];
				6'h20: base_a   <= ibus_wrdata[31:20];
				6'h21: base_b   <= ibus_wrdata[31:20];
				default: begin
					enb      <= enb;
					cmd_mode <= cmd_mode;
					l_fx     <= l_fx;
					l_fy     <= l_fy;
					r_fx     <= r_fx;
					r_fy     <= r_fy;
					cx       <= cx;
					cy       <= cy;
					fx2_inv  <= fx2_inv;
					fy2_inv  <= fy2_inv;
					cx2_fx2  <= cx2_fx2;
					cy2_fy2  <= cy2_fy2;
					l_r11    <= l_r11;
					l_r12    <= l_r12;
					l_r13    <= l_r13;
					l_r21    <= l_r21;
					l_r22    <= l_r22;
					l_r23    <= l_r23;
					l_r31    <= l_r31;
					l_r32    <= l_r32;
					l_r33    <= l_r33;
					r_r11    <= r_r11;
					r_r12    <= r_r12;
					r_r13    <= r_r13;
					r_r21    <= r_r21;
					r_r22    <= r_r22;
					r_r23    <= r_r23;
					r_r31    <= r_r31;
					r_r32    <= r_r32;
					r_r33    <= r_r33;
					base_a   <= base_a;
					base_b   <= base_b;
				end
			endcase
		end
	end
end

assign ibus_rddata[31:0] = (
	(~ibus_cs)               ?  32'b0 :
	(ibus_addr_7_2 == 6'h00) ? {31'b0, enb} :
	(ibus_addr_7_2 == 6'h01) ? {30'b0, dff_ovf, cff_ovf} :
	(ibus_addr_7_2 == 6'h02) ? {30'b0, cmd_mode, 1'b0} :
	(ibus_addr_7_2 == 6'h03) ? {17'b0, cmd_cnt[14:0]} :
	(ibus_addr_7_2 == 6'h05) ? {6'b0, l_fx[25:0]} :
	(ibus_addr_7_2 == 6'h06) ? {6'b0, l_fy[25:0]} :
	(ibus_addr_7_2 == 6'h07) ? {6'b0, r_fx[25:0]} :
	(ibus_addr_7_2 == 6'h08) ? {6'b0, r_fy[25:0]} :
	(ibus_addr_7_2 == 6'h09) ? {6'b0, cx[9:0], 7'b0, cy[8:0]} :
	(ibus_addr_7_2 == 6'h0A) ? {8'b0, fx2_inv[23:0]} :
	(ibus_addr_7_2 == 6'h0B) ? {8'b0, fy2_inv[23:0]} :
	(ibus_addr_7_2 == 6'h0C) ? {8'b0, cx2_fx2[23:0]} :
	(ibus_addr_7_2 == 6'h0D) ? {8'b0, cy2_fy2[23:0]} :
	(ibus_addr_7_2 == 6'h0E) ? {7'b0, l_r11[24:0]} :
	(ibus_addr_7_2 == 6'h0F) ? {7'b0, l_r12[24:0]} :
	(ibus_addr_7_2 == 6'h10) ? {7'b0, l_r13[24:0]} :
	(ibus_addr_7_2 == 6'h11) ? {7'b0, l_r21[24:0]} :
	(ibus_addr_7_2 == 6'h12) ? {7'b0, l_r22[24:0]} :
	(ibus_addr_7_2 == 6'h13) ? {7'b0, l_r23[24:0]} :
	(ibus_addr_7_2 == 6'h14) ? {7'b0, l_r31[24:0]} :
	(ibus_addr_7_2 == 6'h15) ? {7'b0, l_r32[24:0]} :
	(ibus_addr_7_2 == 6'h16) ? {7'b0, l_r33[24:0]} :
	(ibus_addr_7_2 == 6'h17) ? {7'b0, r_r11[24:0]} :
	(ibus_addr_7_2 == 6'h18) ? {7'b0, r_r12[24:0]} :
	(ibus_addr_7_2 == 6'h19) ? {7'b0, r_r13[24:0]} :
	(ibus_addr_7_2 == 6'h1A) ? {7'b0, r_r21[24:0]} :
	(ibus_addr_7_2 == 6'h1B) ? {7'b0, r_r22[24:0]} :
	(ibus_addr_7_2 == 6'h1C) ? {7'b0, r_r23[24:0]} :
	(ibus_addr_7_2 == 6'h1D) ? {7'b0, r_r31[24:0]} :
	(ibus_addr_7_2 == 6'h1E) ? {7'b0, r_r32[24:0]} :
	(ibus_addr_7_2 == 6'h1F) ? {7'b0, r_r33[24:0]} :
	(ibus_addr_7_2 == 6'h20) ? {base_a[11:0], 20'b0} :
	(ibus_addr_7_2 == 6'h21) ? {base_b[11:0], 20'b0} :
	                            32'b0
);

// command interface
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		cmd_rst <= 1'b0;
	end
	else begin
		cmd_rst <= ibus_cs & ibus_wr & (ibus_addr_7_2 == 6'h02) & ibus_wrdata[0];
	end
end

assign cmd_in[17:0] = ibus_wrdata[17:0];
assign cmd_wr = ibus_cs & ibus_wr & (ibus_addr_7_2 == 6'h04);


//==========================================================================
// Sequence Control (PCLK domain)
//==========================================================================
// PCLK domain
always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		vsync_r <= 1'b0;
	end
	else begin
		vsync_r <= vsync;
	end
end

assign vsync_pos_pclk = vsync & ~vsync_r;

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		rect_start_pclk <= 1'b0;
	end
	else begin
		if (enb) begin
			if (vsync_pos_pclk & ~rect_on & (~bm_enb | (cue_cnt != 2))) begin
				rect_start_pclk <= 1'b1;
			end
			else begin
				rect_start_pclk <= 1'b0;
			end
		end
		else begin
			rect_start_pclk <= 1'b0;
		end
	end
end

clkx clkx_bm_done (
	.rst_n (rst_n),
	.clk1  (clk),  .in  (bm_done),
	.clk2  (pclk), .out (bm_done_pclk)
);

// double buffering allows cueing of up to 2 frames.
always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		cue_cnt <= 2'b0;
	end
	else begin
		if (rect_start_pclk & ~bm_done_pclk) begin
			cue_cnt <= cue_cnt + 1'b1;
		end
		else if (bm_done_pclk) begin
			cue_cnt <= cue_cnt - 1'b1;
		end
	end
end


//==========================================================================
// Sequence Control (CLK domain)
//==========================================================================
clkx2 clkx_vsync_pos (
	.rst_n (rst_n),
	.clk1  (pclk), .in  (vsync_pos_pclk),
	.clk2  (clk),  .out (vsync_pos)
);

clkx2 clkx_rect_start (
	.rst_n (rst_n),
	.clk1  (pclk), .in  (rect_start_pclk),
	.clk2  (clk),  .out (rect_start)
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		rect_on <= 1'b0;
	end
	else begin
		if (rect_start) begin
			rect_on <= 1'b1;
		end
		else if (rect_done) begin
			rect_on <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		frm_cnt <= 4'b0;
	end
	else begin
		if (enb) begin
			if (vsync_pos) begin
				frm_cnt <= frm_cnt + 1'b1;
			end
		end
		else begin
			frm_cnt <= 0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		rect_fcnt <= 4'b0;
	end
	else begin
		if (enb) begin
			if (rect_start) begin
				rect_fcnt <= frm_cnt;
			end
		end
		else begin
			rect_fcnt <= 0;
		end
	end
end


//==========================================================================
// Module Instances
//==========================================================================
rect_ibuf ibuf (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Control
	.enb        (enb),
	.start_pclk (rect_start_pclk),
	.frm_end    (frm_end),
	
	// Sensor Input
	.pclk  (pclk),
	.href  (href),
	.d_l   (d_l),
	.d_r   (d_r),
	
	// Internal I/F
	.rdy       (ibuf_rdy),
	.upd       (ibuf_upd),
	.lr_sel    (ibuf_lr),
	.rdaddr    (ibuf_rdaddr),
	.rddata_up (ibuf_up),
	.rddata_lo (ibuf_lo)
);

rect_rmp rmp (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Remap Parameters
	.l_fx    (l_fx),
	.l_fy    (l_fy),
	.r_fx    (r_fx),
	.r_fy    (r_fy),
	.cx      (cx),
	.cy      (cy),
	.fx2_inv (fx2_inv),
	.fy2_inv (fy2_inv),
	.cx2_fx2 (cx2_fx2),
	.cy2_fy2 (cy2_fy2),
	.l_r11   (l_r11),
	.l_r12   (l_r12),
	.l_r13   (l_r13),
	.l_r21   (l_r21),
	.l_r22   (l_r22),
	.l_r23   (l_r23),
	.l_r31   (l_r31),
	.l_r32   (l_r32),
	.l_r33   (l_r33),
	.r_r11   (r_r11),
	.r_r12   (r_r12),
	.r_r13   (r_r13),
	.r_r21   (r_r21),
	.r_r22   (r_r22),
	.r_r23   (r_r23),
	.r_r31   (r_r31),
	.r_r32   (r_r32),
	.r_r33   (r_r33),
	
	// Control
	.rdy   (ibuf_rdy),
	.upd   (ibuf_upd),
	.vsync (vsync),
	
	// Command I/F
	.cmd_mode (cmd_mode),
	.cmd_in   (cmd_in),
	.cmd_wr   (cmd_wr),
	.cmd_rst  (cmd_rst),
	.cmd_cnt  (cmd_cnt),
	
	// Remap I/F
	.vout   (rmp_v),
	.last_o (rmp_last),
	.ydst_o (rmp_ydst),
	.xdst_o (rmp_xdst),
	.len_o  (rmp_len),
	.lr_o   (rmp_lr),
	.ysrc_o (rmp_ysrc),
	.xsrc_o (rmp_xsrc),
	.skip_wait (skip_wait)
);

rect_intp intp (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Line Buffer I/F
	.lr_sel (ibuf_lr),
	.rdaddr (ibuf_rdaddr),
	.up     (ibuf_up),
	.lo     (ibuf_lo),
	
	// Remap I/F
	.vin    (rmp_v),
	.last_i (rmp_last),
	.ydst_i (rmp_ydst),
	.xdst_i (rmp_xdst),
	.len_i  (rmp_len),
	.lr_i   (rmp_lr),
	.ysrc_i (rmp_ysrc),
	.xsrc_i (rmp_xsrc),
	.skip_wait (skip_wait),
	
	// Obuf I/F
	.vout   (intp_v),
	.last_o (intp_last),
	.lr_o   (intp_lr),
	.ydst_o (intp_ydst),
	.xdst_o (intp_xdst),
	.len_o  (intp_len),
	.intp   (intp_dout)
);

rect_obuf obuf (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Control
	.enb     (enb),
	.frm_end (frm_end),
	
	// Status
	.dff_ovf (dff_ovf),
	.cff_ovf (cff_ovf),
	
	// Parameter
	.base_a (base_a),
	.base_b (base_b),
	
	// Front I/F
	.vin  (intp_v),
	.last (intp_last),
	.lr   (intp_lr),
	.ydst (intp_ydst),
	.xdst (intp_xdst),
	.len  (intp_len),
	.intp (intp_dout),
	
	// DDR I/F
	.ddr_req  (ddr_req),
	.ddr_ack  (ddr_ack),
	.ddr_dout (ddr_dout),
	.ddr_strb (ddr_strb),
	.ddr_vout (ddr_vout)
);


endmodule
