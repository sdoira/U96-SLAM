//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module csi_if (
	// Global Control
	rst_n, clk,
	
	// Internal Bus I/F
	ibus_cs,
	ibus_wr,
	ibus_addr,
	ibus_wrdata,
	ibus_rddata,
	
	// CSI Receiver Control
	vrst_n,
	
	// AXI Stream Video Input (Ch1)
	tvalid_ch1,
	tready_ch1,
	tuser_ch1,
	tlast_ch1,
	tdata_ch1,
	tdest_ch1,
	
	// AXI Stream Video Input (Ch2)
	tvalid_ch2,
	tready_ch2,
	tuser_ch2,
	tlast_ch2,
	tdata_ch2,
	tdest_ch2,
	
	// Parallel Video Output
	sof_out,
	vout,
	d1_out,
	d2_out
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Internal Bus I/F
input			ibus_cs;
input			ibus_wr;
input	[7:0]	ibus_addr;
input	[31:0]	ibus_wrdata;
output	[31:0]	ibus_rddata;

// CSI Receiver Control
output			vrst_n;

// AXI Stream Video Input (Ch1)
input			tvalid_ch1;
output			tready_ch1;
input			tuser_ch1;
input			tlast_ch1;
input	[15:0]	tdata_ch1;
input	[3:0]	tdest_ch1;

// AXI Stream Video Input (Ch2)
input			tvalid_ch2;
output			tready_ch2;
input			tuser_ch2;
input			tlast_ch2;
input	[15:0]	tdata_ch2;
input	[3:0]	tdest_ch2;

// Parallel Video Output
output			sof_out, vout;
output	[15:0]	d1_out, d2_out;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Register Interface
reg		[1:0]	ctrl;
reg		[3:0]	ptn_sel1, ptn_sel2;
reg		[4:0]	frm_decim;
wire			vrst_n, enb;
wire	[31:0]	ibus_rddata;

// CSI Interface (Ch1)
wire	[15:0]	row_ch1, col_ch1;
wire	[31:0]	frm_len_ch1;
wire	[31:0]	frm_cnt_ch1;
wire			tready_ch1;
wire			sof_ch1, vout_ch1;
wire	[15:0]	dout_ch1;

// CSI Interface (Ch2)
wire	[15:0]	row_ch2, col_ch2;
wire	[31:0]	frm_len_ch2;
wire	[31:0]	frm_cnt_ch2;
wire			tready_ch2;
wire			sof_ch2, vout_ch2;
wire	[15:0]	dout_ch2;

// Delay Matching
wire	[9:0]	fifo_cnt;
wire			fifo_ovf, fifo_udf;
wire			sof_out, vout;
wire	[15:0]	d1_out, d2_out;


//==========================================================================
// Register Interface
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ctrl     <= 2'b0;
		ptn_sel1 <= 4'b0;
		ptn_sel2 <= 4'b0;
		frm_decim <= 5'b0;
	end
	else begin
		if (ibus_cs & ibus_wr) begin
			case (ibus_addr[7:2])
				6'h00: ctrl     <= ibus_wrdata[ 1: 0];
				6'h03: ptn_sel1 <= ibus_wrdata[ 3: 0];
				6'h07: ptn_sel2 <= ibus_wrdata[ 3: 0];
				6'h0A: frm_decim <= ibus_wrdata[ 4: 0];
				default: begin
					ctrl     <= ctrl;
					ptn_sel1 <= ptn_sel1;
					ptn_sel2 <= ptn_sel2;
					frm_decim <= frm_decim;
				end
			endcase
		end
	end
end

assign vrst_n = ctrl[1];
assign enb    = ctrl[0];

assign ibus_rddata[31:0] = (
	(~ibus_cs)               ?  32'b0 :
	(ibus_addr[7:2] == 6'h0) ? {30'b0, ctrl[1:0]} :
	(ibus_addr[7:2] == 6'h1) ? {14'b0, fifo_cnt[9:0], 6'b0, fifo_udf, fifo_ovf} :
	(ibus_addr[7:2] == 6'h2) ? {row_ch1[15:0], col_ch1[15:0]} :
	(ibus_addr[7:2] == 6'h3) ? {28'b0, ptn_sel1[3:0]} :
	(ibus_addr[7:2] == 6'h4) ? {frm_len_ch1[31:0]} :
	(ibus_addr[7:2] == 6'h5) ? {frm_cnt_ch1[31:0]} :
	(ibus_addr[7:2] == 6'h6) ? {row_ch2[15:0], col_ch2[15:0]} :
	(ibus_addr[7:2] == 6'h7) ? {28'b0, ptn_sel2[3:0]} :
	(ibus_addr[7:2] == 6'h8) ? {frm_len_ch2[31:0]} :
	(ibus_addr[7:2] == 6'h9) ? {frm_cnt_ch2[31:0]} :
	(ibus_addr[7:2] == 6'hA) ? {27'b0, frm_decim[4:0]} :
	                            32'b0
);


//==========================================================================
// CSI Interface
//==========================================================================
csi_if_ch csi_if_ch1 (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Parameter
	.ptn_sel (ptn_sel1[3:0]),
	.frm_decim (frm_decim[4:0]),
	
	// Status
	.row_r     (row_ch1[15:0]),
	.col_r     (col_ch1[15:0]),
	.frm_len_r (frm_len_ch1[31:0]),
	.frm_cnt   (frm_cnt_ch1[31:0]),
	
	// AXI Stream Video Input
	.vrst_n (vrst_n),
	.tvalid (tvalid_ch1),
	.tready (tready_ch1),
	.tuser  (tuser_ch1),
	.tlast  (tlast_ch1),
	.tdata  (tdata_ch1[15:0]),
	.tdest  (tdest_ch1[3:0]),
	
	// Parallel Video Output
	.sof_o (sof_ch1),
	.vout  (vout_ch1),
	.dout  (dout_ch1[15:0])
);

csi_if_ch csi_if_ch2 (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Parameter
	.ptn_sel (ptn_sel2[3:0]),
	.frm_decim (frm_decim[4:0]),
	
	// Status
	.row_r     (row_ch2[15:0]),
	.col_r     (col_ch2[15:0]),
	.frm_len_r (frm_len_ch2[31:0]),
	.frm_cnt   (frm_cnt_ch2[31:0]),
	
	// AXI Stream Video Input
	.vrst_n (vrst_n),
	.tvalid (tvalid_ch2),
	.tready (tready_ch2),
	.tuser  (tuser_ch2),
	.tlast  (tlast_ch2),
	.tdata  (tdata_ch2[15:0]),
	.tdest  (tdest_ch2[3:0]),
	
	// Parallel Video Output
	.sof_o (sof_ch2),
	.vout  (vout_ch2),
	.dout  (dout_ch2[15:0])
);


//==========================================================================
// Delay Matching
//==========================================================================
csi_dly csi_dly (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Parameter
	.enb (enb),
	
	// Status
	.fifo_cnt (fifo_cnt[9:0]),
	.fifo_ovf (fifo_ovf),
	.fifo_udf (fifo_udf),
	
	// Parallel Video Input (Ch1)
	.sof1_in (sof_ch1),
	.v1_in   (vout_ch1),
	.d1_in   (dout_ch1[15:0]),
	
	// Parallel Video Input (Ch2)
	.sof2_in (sof_ch2),
	.v2_in   (vout_ch2),
	.d2_in   (dout_ch2[15:0]),
	
	// Parallel Video Output
	.sof_out (sof_out),
	.vout    (vout),
	.d1_out  (d1_out[15:0]),
	.d2_out  (d2_out[15:0])
);


endmodule
