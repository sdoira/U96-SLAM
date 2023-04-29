//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module csi_if_ch (
	// Global Control
	rst_n, clk,
	
	// Parameter
	ptn_sel,
	frm_decim,
	
	// Status
	row_r, col_r,
	frm_len_r,
	frm_cnt,
	
	// AXI Stream Video Input
	vrst_n,
	tvalid,
	tready,
	tuser,
	tlast,
	tdata,
	tdest,
	
	// Parallel Video Output
	sof_o,
	vout,
	dout
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Parameter
input	[3:0]	ptn_sel;
input	[4:0]	frm_decim;

// Status
output	[15:0]	row_r, col_r;
output	[31:0]	frm_len_r;
output	[31:0]	frm_cnt;

// AXI Stream Video Input
input			vrst_n;
input			tvalid;
output			tready;
input			tuser;
input			tlast;
input	[15:0]	tdata;
input	[3:0]	tdest;

// Parallel Video Output
output			sof_o, vout;
output	[15:0]	dout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Frame Format Measurement
reg             tready;
reg				tlast_r;
reg		[15:0]	col, col_r;
reg				tuser_r;
wire			sof;
reg		[15:0]	row, row_r;
reg		[31:0]	frm_len, frm_len_r;
reg		[31:0]	frm_cnt;

// Test Pattern Generator
wire	[7:0]	ptn1, ptn2, ptn3;
reg		[12:0]	ptn4_cnt1;
reg		[2:0]	ptn4_cnt2;
wire	[15:0]	ptn4;
wire	[7:0]	ptn5;

// Output Selector
reg		[4:0]	vcnt;
wire	[15:0]	data_sel;
reg				sof_r;
reg		[3:0]	valid_r;
reg		[15:0]	data_sel_r3, data_sel_r2, data_sel_r1, data_sel_r0;
wire			sof_o, vout;
wire	[15:0]	dout;


//==========================================================================
// Frame Format Measurement
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		tready <= 1'b0;
	end
	else begin
		tready <= tvalid & ~tready;
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		tlast_r <= 1'b0;
	end
	else begin
		if (tvalid & tready & tlast) begin
			tlast_r <= 1'b1;
		end
		else begin
			tlast_r <= 1'b0;
		end
	end
end

// column count
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		col <= 16'b0;
	end
	else begin
		if (~vrst_n) begin
			col <= 0;
		end
		else if (tlast_r) begin
			col <= 0;
		end
		else if (tvalid & tready) begin
			col <= col + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		col_r <= 16'b0;
	end
	else begin
		if (~vrst_n) begin
			col_r <= 0;
		end
		else if (tlast_r) begin
			col_r <= col;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		tuser_r <= 1'b0;
	end
	else begin
		tuser_r <= tuser;
	end
end

assign sof = tuser & ~tuser_r; // start of frame

// row count
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		row <= 16'b0;
	end
	else begin
		if (sof) begin
			row <= 0;
		end
		//else if (tlast) begin
		else if (tvalid & tready & tlast) begin
			row <= row + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		row_r <= 16'b0;
	end
	else begin
		if (~vrst_n) begin
			row_r <= 0;
		end
		else if (sof) begin
			row_r <= row;
		end
	end
end

// interval between sof
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		frm_len <= 32'b0;
	end
	else begin
		if (~vrst_n | sof) begin
			frm_len <= 0;
		end
		else begin
			frm_len <= frm_len + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		frm_len_r <= 32'b0;
	end
	else begin
		if (~vrst_n) begin
			frm_len_r <= 0;
		end
		else if (sof) begin
			frm_len_r <= frm_len;
		end
	end
end

// accumulative number of frames
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		frm_cnt <= 32'b0;
	end
	else begin
		if (~vrst_n) begin
			frm_cnt <= 0;
		end
		else if (sof) begin
			frm_cnt <= frm_cnt + 1'b1;
		end
	end
end


//==========================================================================
// Test Pattern Generator
//==========================================================================
// Pattern1: horizontal incremental
assign ptn1[7:0] = col[7:0];

// Pattern2: vertical incremental
assign ptn2[7:0] = row[7:0];

// Pattern3: frame incremental
assign ptn3[7:0] = frm_cnt[7:0];

// Pattern4: color bar
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ptn4_cnt1 <= 13'b0; // bar width (1/8 total width)
		ptn4_cnt2 <=  3'b0; // color
	end
	else begin
		if (sof | (tvalid & tready & tlast)) begin
			ptn4_cnt1 <= 0;
			ptn4_cnt2 <= 0;
		end
		else if (tvalid & tready) begin
			if (ptn4_cnt1 == col_r[15:3] - 1'b1) begin
				ptn4_cnt1 <= 0;
				ptn4_cnt2 <= ptn4_cnt2 + 1'b1;
			end
			else begin
				ptn4_cnt1 <= ptn4_cnt1 + 1'b1;
			end
		end
	end
end

//   Wh  Ye  Cy  Gr  Mg  Rd  Bl  Bk
// Y 255 255 215 199  79  63  15   0
// U 128   0 223   0 255  32 255 128
// V 128 143   0   0 255 255 112 128

// Y
assign ptn4[15:8] = (
	(ptn4_cnt2[2:0] == 0) ? 255 :
	(ptn4_cnt2[2:0] == 1) ? 255 :
	(ptn4_cnt2[2:0] == 2) ? 215 :
	(ptn4_cnt2[2:0] == 3) ? 199 :
	(ptn4_cnt2[2:0] == 4) ?  79 :
	(ptn4_cnt2[2:0] == 5) ?  63 :
	(ptn4_cnt2[2:0] == 6) ?  15 :
	                          0
);

// U/V
assign ptn4[7:0] = (
	(ptn4_cnt2[2:0] == 0) ? ((~col[0]) ? 128 : 128) :
	(ptn4_cnt2[2:0] == 1) ? ((~col[0]) ?   0 : 143) :
	(ptn4_cnt2[2:0] == 2) ? ((~col[0]) ? 223 :   0) :
	(ptn4_cnt2[2:0] == 3) ? ((~col[0]) ?   0 :   0) :
	(ptn4_cnt2[2:0] == 4) ? ((~col[0]) ? 255 : 255) :
	(ptn4_cnt2[2:0] == 5) ? ((~col[0]) ?  32 : 255) :
	(ptn4_cnt2[2:0] == 6) ? ((~col[0]) ? 255 : 112) :
	                        ((~col[0]) ? 128 : 128)
);


// Pattern5: grid (64-pix)
assign ptn5[7:0] = (col[6] ^ row[6]) ? 8'd0 : 8'd255;


//==========================================================================
// Output Selector
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vcnt <= 5'b0;
	end
	else begin
		if (~vrst_n) begin
			//vcnt <= 0;
			vcnt <= frm_decim; // for RTL simulation
		end
		else if (sof) begin
			//if (vcnt == 2) begin
			if (vcnt == frm_decim) begin
				vcnt <= 0;
			end
			else begin
				vcnt <= vcnt + 1'b1;
			end
		end
	end
end

assign data_sel[15:0] = (
	(~tvalid)           ? 16'h0080 :
	(ptn_sel[3:0] == 0) ? tdata[15:0] :
	(ptn_sel[3:0] == 1) ? {ptn1[7:0], 8'h80} :
	(ptn_sel[3:0] == 2) ? {ptn2[7:0], 8'h80} :
	(ptn_sel[3:0] == 3) ? {ptn3[7:0], 8'h80} :
	(ptn_sel[3:0] == 4) ? ptn4[15:0] :
	(ptn_sel[3:0] == 5) ? {ptn5[7:0], 8'h80} :
	                      tdata[15:0]
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		sof_r       <=  1'b0;
		valid_r     <=  4'b0;
		data_sel_r3 <= 16'b0;
		data_sel_r2 <= 16'b0;
		data_sel_r1 <= 16'b0;
		data_sel_r0 <= 16'b0;
	end
	else begin
		sof_r       <= sof;
		valid_r     <= {valid_r[2:0], tvalid & tready};
		data_sel_r3 <= data_sel_r2;
		data_sel_r2 <= data_sel_r1;
		data_sel_r1 <= data_sel_r0;
		data_sel_r0 <= data_sel;
	end
end

assign sof_o = (vcnt == 0) ? sof_r : 1'b0;
assign vout  = (vcnt == 0) ? valid_r[3] : 1'b0;
assign dout  = (vcnt == 0) ? data_sel_r3 : 16'b0;
//assign sof_o = sof_r;
//assign vout  = valid_r[3];
//assign dout  = data_sel_r3;


endmodule
