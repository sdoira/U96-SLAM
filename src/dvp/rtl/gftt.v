//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module gftt (
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
	gftt_done,
	rect_fcnt,
	gftt_fcnt,
	
	// DDR Read Arbiter I/F
	drd_req,
	drd_ack,
	drd_vin,
	drd_din,
	drd_vout,
	drd_dout,
	
	// DDR Write Arbiter I/F
	dwr_req,
	dwr_ack,
	dwr_vout,
	dwr_dout
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
input			gftt_done;
input	[3:0]	rect_fcnt;
output	[3:0]	gftt_fcnt;

// DDR Read Arbiter I/F
output			drd_req;
input			drd_ack;
input			drd_vin;
input	[31:0]	drd_din;
output			drd_vout;
output	[31:0]	drd_dout;

// DDR Write Arbiter I/F
output			dwr_req;
input			dwr_ack;
output			dwr_vout;
output	[31:0]	dwr_dout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Register Interface
reg				ctrl;
reg		[11:0]	drd_base_a, drd_base_b;
reg		[11:0]	dwr_base_a, dwr_base_b;
reg		[7:0]	bst_len_m1;
reg		[9:0]	hgt;
reg		[10:0]	wdt;
wire	[9:0]	hgt_m1;
wire	[10:0]	wdt_m1;
wire			enb;
wire	[31:0]	ibus_rddata;

// Sequence Control
reg				gftt_sim_start;
wire			gftt_start;
reg				gftt_pend, gftt_on;
reg		[3:0]	gftt_fcnt;

// gftt_ibuf
wire			drd_req;
wire			drd_vout;
wire	[31:0]	drd_dout;
wire	[7:0]	line0, line1, line2;
wire			ibuf_vout;
wire			first_smpl, last_smpl;

// gftt_sbl
wire	[11:0]	xsbl, ysbl;
wire			sbl_vout;

// gftt_eig
wire	[15:0]	eig_dout;
wire			eig_vout;

// gftt_obuf
wire			obuf_ovf, obuf_udf;
wire	[15:0]	max_a, max_b;
wire			dwr_req;
wire			dwr_vout;
wire	[31:0]	dwr_dout;


//==========================================================================
// Register Interface
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ctrl       <= 1'b0;
		drd_base_a <= 12'b0;
		drd_base_b <= 12'b0;
		dwr_base_a <= 12'b0;
		dwr_base_b <= 12'b0;
		bst_len_m1 <= 5'd31;
		hgt        <= 10'd480;
		wdt        <= 11'd640;
	end
	else begin
		if (ibus_cs & ibus_wr) begin
			case (ibus_addr_7_2)
				6'h00: ctrl       <= ibus_wrdata[0];
				6'h02: drd_base_a <= ibus_wrdata[31:20];
				6'h03: drd_base_b <= ibus_wrdata[31:20];
				6'h04: dwr_base_a <= ibus_wrdata[31:20];
				6'h05: dwr_base_b <= ibus_wrdata[31:20];
				6'h06: bst_len_m1 <= ibus_wrdata[7:0];
				6'h07: begin
					hgt           <= ibus_wrdata[25:16];
					wdt           <= ibus_wrdata[10: 0];
				end
				default: begin
					ctrl       <= ctrl;
					hgt        <= hgt;
					wdt        <= wdt;
				end
			endcase
		end
	end
end

assign hgt_m1[9:0] = hgt[9:0] - 1'b1; // zero-based image size
assign wdt_m1[10:0] = wdt[10:0] - 1'b1;
assign enb = ctrl;

assign ibus_rddata[31:0] = (
	(~ibus_cs)               ?  32'b0 :
	(ibus_addr_7_2 == 6'h00) ? {31'b0, ctrl} :
	(ibus_addr_7_2 == 6'h01) ? {30'b0, obuf_udf, obuf_ovf} :
	(ibus_addr_7_2 == 6'h02) ? {drd_base_a[11:0], 20'b0} :
	(ibus_addr_7_2 == 6'h03) ? {drd_base_b[11:0], 20'b0} :
	(ibus_addr_7_2 == 6'h04) ? {dwr_base_a[11:0], 20'b0} :
	(ibus_addr_7_2 == 6'h05) ? {dwr_base_b[11:0], 20'b0} :
	(ibus_addr_7_2 == 6'h06) ? {24'b0, bst_len_m1[7:0]} :
	(ibus_addr_7_2 == 6'h07) ? {6'b0, hgt[9:0], 5'b0, wdt[10:0]} :
	(ibus_addr_7_2 == 6'h08) ? {max_b[15:0], max_a[15:0]} :
	                           32'b0
);

wire		sw_start;
assign sw_start = (ibus_cs & ibus_wr & (ibus_addr_7_2[5:0] == 6'h00) & ibus_wrdata[1]);


//==========================================================================
// Sequence Control
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		// force start GFTT operation in RTL simulation
		gftt_sim_start <= 1'b0;
	end
end

assign gftt_start = (enb & (rect_done | gftt_pend | gftt_sim_start | sw_start) & ~gftt_on);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		gftt_pend <= 1'b0;
	end
	else begin
		if (enb) begin
			if (rect_done & gftt_on) begin
				gftt_pend <= 1'b1;
			end
			else if (gftt_start) begin
				gftt_pend <= 1'b0;
			end
		end
		else begin
			gftt_pend <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		gftt_on <= 1'b0;
	end
	else begin
		if (enb) begin
			if (gftt_start) begin
				gftt_on <= 1'b1;
			end
			else if (gftt_done) begin
				gftt_on <= 1'b0;
			end
		end
		else begin
			gftt_on <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		gftt_fcnt <= 4'b0;
	end
	else begin
		if (enb) begin
			if (gftt_start) begin
				gftt_fcnt <= rect_fcnt;
			end
		end
		else begin
			gftt_fcnt <= 0;
		end
	end
end


//==========================================================================
// Module Instance
//==========================================================================
gftt_ibuf gftt_ibuf (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Parameter
	.enb    (enb),
	.addr_a (drd_base_a[11:0]),
	.addr_b (drd_base_b[11:0]),
	.bst_len_m1 (bst_len_m1[7:0]),
	.hgt    (hgt[8:0]),
	.hgt_m1 (hgt_m1[8:0]),
	.wdt    (wdt[9:0]),
	.wdt_m1 (wdt_m1[9:0]),
	
	// Control
	.start (gftt_start),
	
	// DDR Arbiter I/F
	.drd_req  (drd_req),
	.drd_ack  (drd_ack),
	.drd_vin  (drd_vin),
	.drd_din  (drd_din[31:0]),
	.drd_vout (drd_vout),
	.drd_dout (drd_dout[31:0]),
	
	// IBUF I/F
	.line0 (line0[7:0]),
	.line1 (line1[7:0]),
	.line2 (line2[7:0]),
	.vout  (ibuf_vout),
	.first_smpl (first_smpl),
	.last_smpl  (last_smpl)
);

gftt_sbl gftt_sbl (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Frontend I/F
	.line0 (line0[7:0]),
	.line1 (line1[7:0]),
	.line2 (line2[7:0]),
	.vin   (ibuf_vout),
	.first_smpl (first_smpl),
	.last_smpl (last_smpl),
	
	// Backend I/F
	.xsbl (xsbl[11:0]),
	.ysbl (ysbl[11:0]),
	.vout (sbl_vout)
);

gftt_eig gftt_eig (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Parameter
	.wdt_m1 (wdt_m1[10:0]),
	
	// Control
	.start (gftt_start),
	.enb   (enb),
	
	// Frontend I/F
	.dx  (xsbl[11:0]),
	.dy  (ysbl[11:0]),
	.vin (sbl_vout),
	
	// Backend I/F
	.dout (eig_dout[15:0]),
	.vout (eig_vout)
);

gftt_obuf gftt_obuf (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Parameter
	.hgt (hgt[8:0]),
	.wdt (wdt[9:0]),
	.wdt_m1 (wdt_m1[9:0]),
	.addr_a (dwr_base_a[11:0]),
	.addr_b (dwr_base_b[11:0]),
	.bst_len_m1 (bst_len_m1[7:0]),
	
	// Control
	.enb   (enb),
	.start (gftt_start),
	
	// Status
	.ovf (obuf_ovf),
	.udf (obuf_udf),
	.max_a (max_a),
	.max_b (max_b),
	
	// Frontend I/F
	.din (eig_dout[15:0]),
	.vin (eig_vout),
	
	// DDR Arbiter I/F
	.dwr_req  (dwr_req),
	.dwr_ack  (dwr_ack),
	.dwr_vout (dwr_vout),
	.dwr_dout (dwr_dout[31:0])
);


endmodule
