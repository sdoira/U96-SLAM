//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module bm (
	// Global Control
	rst_n, clk,
	
	// Internal Bus I/F
	ibus_cs,
	ibus_wr,
	ibus_addr_7_2,
	ibus_wrdata,
	ibus_rddata,
	
	// Control
	xsbl_done,
	bm_done,
	xsbl_fcnt,
	bm_fcnt,
	enb,
	
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
	dwr_dout,
	dwr_strb
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
input			xsbl_done, bm_done;
input	[3:0]	xsbl_fcnt;
output	[3:0]	bm_fcnt;
output			enb;

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
output	[3:0]	dwr_strb;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Register Interface
reg				ctrl;
reg		[8:0]	hgt;
reg		[9:0]	wdt;
reg		[4:0]	wsz;
reg		[8:0]	ndisp;
reg		[11:0]	lr_addr_a, lr_addr_b;
reg		[9:0]	sad_addr_a, sad_addr_b;
reg		[7:0]	bst_len_m1;
reg				uni_enb_cue, uni_mode_cue;
reg		[9:0]	uni_thr_cue;
reg		[11:0]	disp2_addr_a, disp2_addr_b;
wire			enb;
wire	[31:0]	ibus_rddata;

reg				uni_enb, uni_mode;
reg		[9:0]	uni_thr;

// Secondary Parameters
wire	[3:0]	hwsz;
wire	[9:0]	hsad_wdt;
wire	[9:0]	sad_wdt;
wire	[8:0]	sad_hgt;
wire	[8:0]	bst_len;

// Sequence Control
reg				bm_sim_start;
wire			bm_start;
reg				bm_pend, bm_on;
reg		[3:0]	bm_fcnt;

// Sub Module Instance
// bm_ibuf
wire			lr_rdy, sad_rdy;
wire	[15:0]	buf_info;
wire			drd_req;
wire			drd_vout;
wire	[31:0]	drd_dout;
wire	[15:0]	lr_dout;
wire	[63:0]	sad_dout;
wire			ibuf_udf, ibuf_ovf;
// bm_calc
wire			lr_done, sad_done;
wire	[9:0]	lr_rdaddr;
wire	[9:0]	sad_rdaddr;
wire	[65:0]	obuf_wrdata;
wire			obuf_wr;
wire	[15:0]	obuf2_wrdata;
wire			obuf2_wr;
// bm_obuf
wire			obuf_ovf, obuf_udf;
wire			dwr1_req;
wire			dwr1_vout;
wire	[31:0]	dwr1_dout;
wire	[7:0]	frm_cnt;
// bm_obuf2
wire			obuf2_ovf, obuf2_udf;
wire			frm_end_obuf2;
wire			dwr2_req;
wire			dwr2_vout;
wire	[31:0]	dwr2_dout;
wire			dwr_req;
wire			dwr_vout;
wire	[31:0]	dwr_dout;

//==========================================================================
// Register Interface
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ctrl         <=  1'b0;
		hgt          <=  9'd480;
		wdt          <= 10'd640;
		wsz          <=  5'd21;
		ndisp        <=  9'd128;
		lr_addr_a    <= 12'd0;
		lr_addr_b    <= 12'd0;
		sad_addr_a   <= 10'd0;
		sad_addr_b   <= 10'd0;
		bst_len_m1   <=  8'd63;
		uni_enb_cue  <=  1'b0;
		uni_mode_cue <=  1'b0;
		uni_thr_cue  <= 10'b0;
		disp2_addr_a <= 12'd0;
		disp2_addr_b <= 12'd0;
	end
	else begin
		if (ibus_cs & ibus_wr) begin
			case (ibus_addr_7_2)
				6'h00: ctrl       <= ibus_wrdata[0];
				6'h02: begin
					hgt           <= ibus_wrdata[24:16];
					wdt           <= ibus_wrdata[ 9: 0];
				end
				6'h03: begin
					wsz           <= ibus_wrdata[20:16];
					ndisp         <= ibus_wrdata[ 8: 0];
				end
				6'h04: lr_addr_a  <= ibus_wrdata[31:20];
				6'h05: lr_addr_b  <= ibus_wrdata[31:20];
				6'h06: sad_addr_a <= ibus_wrdata[31:22];
				6'h07: sad_addr_b <= ibus_wrdata[31:22];
				6'h08: bst_len_m1 <= ibus_wrdata[ 7: 0];
				6'h0A: begin
					uni_enb_cue   <= ibus_wrdata[31];
					uni_mode_cue  <= ibus_wrdata[16];
					uni_thr_cue   <= ibus_wrdata[ 9: 0];
				end
				6'h0B: disp2_addr_a <= ibus_wrdata[31:20];
				6'h0C: disp2_addr_b <= ibus_wrdata[31:20];
			endcase
		end
	end
end

assign enb = ctrl;

assign ibus_rddata[31:0] = (
	(~ibus_cs)              ?  32'b0 :
	(ibus_addr_7_2 == 6'h0) ? {31'b0, ctrl} :
	//(ibus_addr_7_2 == 6'h1) ? {20'b0, obuf_udf, obuf_ovf, ibuf_udf, ibuf_ovf, frm_cnt[7:0]} :
	(ibus_addr_7_2 == 6'h1) ? {18'b0, obuf2_udf, obuf2_ovf, obuf_udf, obuf_ovf, ibuf_udf, ibuf_ovf, frm_cnt[7:0]} :
	(ibus_addr_7_2 == 6'h2) ? {7'b0, hgt[8:0], 6'b0, wdt[9:0]} :
	(ibus_addr_7_2 == 6'h3) ? {11'b0, wsz[4:0], 7'b0, ndisp[8:0]} :
	(ibus_addr_7_2 == 6'h4) ? {lr_addr_a[11:0], 20'b0} :
	(ibus_addr_7_2 == 6'h5) ? {lr_addr_b[11:0], 20'b0} :
	(ibus_addr_7_2 == 6'h6) ? {sad_addr_a[9:0], 22'b0} :
	(ibus_addr_7_2 == 6'h7) ? {sad_addr_b[9:0], 22'b0} :
	(ibus_addr_7_2 == 6'h8) ? {24'b0, bst_len_m1[7:0]} :
	(ibus_addr_7_2 == 6'h9) ? {7'b0, sad_hgt[8:0], 6'b0, sad_wdt[9:0]} :
	(ibus_addr_7_2 == 6'hA) ? {uni_enb_cue, 14'b0, uni_mode_cue, 6'b0, uni_thr_cue[9:0]} :
	(ibus_addr_7_2 == 6'hB) ? {disp2_addr_a[11:0], 20'b0} :
	(ibus_addr_7_2 == 6'hC) ? {disp2_addr_b[11:0], 20'b0} :
	                           32'b0
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		uni_enb  <=  1'b0;
		uni_mode <=  1'b0;
		uni_thr  <= 10'b0;
	end
	else begin
		if (bm_done) begin
			uni_enb  <= uni_enb_cue;
			uni_mode <= uni_mode_cue;
			uni_thr  <= uni_thr_cue;
		end
	end
end


//==========================================================================
// Secondary Parameters
//--------------------------------------------------------------------------
// [values with standard parameters]
// hgt[8:0]       = 480
// wdt[9:0]       = 640
// wsz[4:0]       =  21
// ndisp[8:0]     = 128
// hwsz[3:0]      =  10
// hsad_wdt[9:0]  = 511
// sad_wdt[9:0]   = 491
// sad_hgt[8:0]   = 460
// bst_len[8:0]   =  64
//==========================================================================
// half window size
assign hwsz[3:0] = wsz[4:1];

// HSAD width
assign hsad_wdt[9:0] = wdt[9:0] - ndisp[8:0] - 1'b1;

// SAD width
assign sad_wdt[9:0]  = hsad_wdt[9:0] - {hwsz[3:0], 1'b0};

// SAD height
assign sad_hgt[8:0] = hgt[8:0] - {hwsz[3:0], 1'b0};

// Actual burst length
assign bst_len[8:0] = bst_len_m1[7:0] + 1'b1;


//==========================================================================
// Sequence Control
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		bm_sim_start <= 1'b0;
	end
end

assign bm_start = (enb & (xsbl_done | bm_pend | bm_sim_start) & ~bm_on);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		bm_pend <= 1'b0;
	end
	else begin
		if (enb) begin
			if (xsbl_done & bm_on) begin
				bm_pend <= 1'b1;
			end
			else if (bm_start) begin
				bm_pend <= 1'b0;
			end
		end
		else begin
			bm_pend <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		bm_on <= 1'b0;
	end
	else begin
		if (enb) begin
			if (bm_start) begin
				bm_on <= 1'b1;
			end
			else if (bm_done) begin
				bm_on <= 1'b0;
			end
		end
		else begin
			bm_on <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		bm_fcnt <= 4'b0;
	end
	else begin
		if (enb) begin
			if (bm_start) begin
				bm_fcnt <= xsbl_fcnt;
			end
		end
		else begin
			bm_fcnt <= 0;
		end
	end
end


//==========================================================================
// Sub Module Instance
//==========================================================================
bm_ibuf bm_ibuf (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Parameter
	.hgt        (hgt),
	.wdt        (wdt),
	.ndisp      (ndisp),
	.wsz        (wsz),
	.lr_addr_a  (lr_addr_a),
	.lr_addr_b  (lr_addr_b),
	.sad_addr_a (sad_addr_a),
	.sad_addr_b (sad_addr_b),
	.bst_len_m1 (bst_len_m1),
	.bst_len    (bst_len),
	.sad_hgt    (sad_hgt),
	.sad_wdt    (sad_wdt),
	
	// Control
	.enb      (enb),
	.start    (bm_start),
	.lr_rdy   (lr_rdy),
	.sad_rdy  (sad_rdy),
	.lr_done  (lr_done),
	.sad_done (sad_done),
	.buf_info (buf_info),
	.bank     (bank),
	
	// DDR Arbiter I/F
	.drd_req  (drd_req),
	.drd_ack  (drd_ack),
	.drd_vin  (drd_vin),
	.drd_din  (drd_din),
	.drd_vout (drd_vout),
	.drd_dout (drd_dout),
	
	// IBUF I/F
	.lr_rdaddr  (lr_rdaddr),
	.lr_dout    (lr_dout),
	.sad_rdaddr (sad_rdaddr),
	.sad_dout   (sad_dout)
);

bm_calc bm_calc (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Parameter
	.wdt      (wdt),
	.ndisp    (ndisp),
	.hwsz     (hwsz),
	.hsad_wdt (hsad_wdt),
	.uni_enb  (uni_enb),
	.uni_mode (uni_mode),
	.uni_thr  (uni_thr),
	
	// Control
	.enb      (enb),
	.start    (bm_start),
	.lr_rdy   (lr_rdy),
	.sad_rdy  (sad_rdy),
	.lr_done  (lr_done),
	.sad_done (sad_done),
	.buf_info (buf_info),
	
	// IBUF I/F
	.lr_rdaddr  (lr_rdaddr),
	.lr_din     (lr_dout),
	.sad_rdaddr (sad_rdaddr),
	.sad_din    (sad_dout),
	
	// OBUF I/F
	.obuf_wrdata  (obuf_wrdata),
	.obuf_wr      (obuf_wr),
	.obuf2_wrdata (obuf2_wrdata),
	.obuf2_wr     (obuf2_wr)
);
assign ibuf_udf = 1'b0;
assign ibuf_ovf = 1'b0;

bm_obuf bm_obuf (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Parameter
	.sad_hgt    (sad_hgt),
	.sad_wdt    (sad_wdt),
	.addr_a     (sad_addr_a),
	.addr_b     (sad_addr_b),
	.bst_len_m1 (bst_len_m1),
	.bst_len    (bst_len),
	
	// Control
	.enb (enb),
	.frm_end_i (frm_end_obuf2),
	
	// Status
	.ovf (obuf_ovf),
	.udf (obuf_udf),
	
	// OBUF I/F
	.din (obuf_wrdata),
	.wr  (obuf_wr),
	
	// DDR Arbiter I/F
	.dwr_req  (dwr1_req),
	.dwr_ack  (dwr_ack),
	.dwr_vout (dwr1_vout),
	.dwr_dout (dwr1_dout)
);
assign frm_cnt  = 8'b0;

bm_obuf2 bm_obuf2 (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Parameter
	.img_hgt (hgt[8:0]),
	.img_wdt (wdt[9:0]),
	.hwsz    (hwsz[3:0]),
	.ndisp   (ndisp[8:0]),
	.addr_a  (disp2_addr_a[11:0]),
	.addr_b  (disp2_addr_b[11:0]),
	
	// Control
	.enb (enb),
	.frm_end (frm_end_obuf2),
	
	// Status
	.ovf (obuf2_ovf),
	.udf (obuf2_udf),
	
	// OBUF I/F
	.din (obuf2_wrdata),
	.wr  (obuf2_wr),
	
	// DDR Arbiter I/F
	.dwr_req  (dwr2_req),
	.dwr_ack  (dwr_ack),
	.dwr_vout (dwr2_vout),
	.dwr_dout (dwr2_dout),
	.dwr_strb (dwr_strb)
);
/*
assign obuf2_ovf = 1'b0;
assign obuf2_udf = 1'b0;
assign dwr2_req = 1'b0;
assign dwr2_vout = 1'b0;
assign dwr2_dout = 32'b0;
assign dwr_strb = 4'hF;
*/
assign dwr_req  = dwr1_req  | dwr2_req;
assign dwr_vout = dwr1_vout | dwr2_vout;
assign dwr_dout = (dwr1_vout) ? dwr1_dout : (dwr2_vout) ? dwr2_dout : 32'h0000_0000;


endmodule
