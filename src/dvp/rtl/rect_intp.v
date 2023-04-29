//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module rect_intp (
	// Global Control
	rst_n, clk,
	
	// Line Buffer I/F
	lr_sel,
	rdaddr,
	up,
	lo,
	
	// Remap I/F
	vin,
	last_i,
	ydst_i,
	xdst_i,
	len_i,
	lr_i,
	ysrc_i,
	xsrc_i,
	skip_wait,
	
	// Obuf I/F
	vout,
	last_o,
	lr_o,
	ydst_o,
	xdst_o,
	len_o,
	intp
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Line Buffer I/F
output			lr_sel;
output	[9:0]	rdaddr;
input	[7:0]	up, lo;

// Remap I/F
input			vin;
input			last_i;
input	[8:0]	ydst_i;
input	[9:0]	xdst_i;
input	[6:0]	len_i;
input			lr_i;
input	[13:0]	ysrc_i;
input	[14:0]	xsrc_i;
output			skip_wait;

// Obuf I/F
output			vout;
output			last_o;
output			lr_o;
output	[8:0]	ydst_o;
output	[9:0]	xdst_o;
output	[6:0]	len_o;
output	[7:0]	intp;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Loop Index
integer         i;
// Delay FIFO
reg				vin_r;
wire			vpos;
wire			bst_st;
reg		[5:0]	bst_st_r;
wire			fifo_wr;
wire	[27:0]	fifo_wrdata, fifo_rddata;
wire			vout;
wire			lr_o;
wire	[8:0]	ydst_o;
wire	[9:0]	xdst_o;
wire	[6:0]	len_o;

// Bilinear Interpolation
wire	[9:0]	xi;
reg		[1:0]	xi_r;
wire	[1:0]	xi_p2;
wire			overlap, skip;
reg		[1:0]	overlap_r;
reg		[3:0]	skip_r;
reg		[7:0]	vcnt;
wire			valid;
reg		[5:0]	valid_r;
reg		[6:0]	incr_cnt;
reg		[9:0]	rdaddr;
reg				lr_sel;
reg		[7:0]	up_r, lo_r;
wire	[4:0]	xf, yf;
wire	[5:0]	xf_inv, yf_inv;
reg		[4:0]	xf_r[1:0], yf_r[1:0];
reg		[5:0]	xfi_r[1:0], yfi_r[1:0];
reg 	[10:0]	xyi, xiyi, xy, xiy;
reg		[17:0]	ur, ul, dr, dl;
reg		[18:0]	ulr, dlr;
wire	[17:0]	ulr_lim, dlr_lim;
reg		[18:0]	udlr;
wire	[8:0]	udlr_lim;
wire	[9:0]	udlr_rnd;
reg		[7:0]	intp;
wire			fifo_rd;


//==========================================================================
// Delay FIFO
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vin_r <= 1'b0;
	end
	else begin
		vin_r <= vin;
	end
end

assign vpos = vin & ~vin_r;

assign bst_st = vpos & (vcnt == 0);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		bst_st_r <= 6'b0;
	end
	else begin
		bst_st_r <= {bst_st_r[4:0], bst_st};
	end
end

assign fifo_wr = bst_st;

assign fifo_wrdata[27:0] = {
	last_i,
	lr_i,
	ydst_i[8:0],
	xdst_i[9:0],
	len_i[6:0]
};

rect_intp_fifo #(28, 2) rect_intp_fifo (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// FIFO I/F
	.fifo_wr   (fifo_wr),
	.fifo_din  (fifo_wrdata),
	.fifo_rd   (fifo_rd),
	.fifo_dout (fifo_rddata)
);
/*
// simple_fifo is implemented as first-word-fall-through and can not
// replace rect_intp_fifo.
simple_fifo # (28, 2) rect_intp_fifo (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// FIFO I/F
	.wr     (fifo_wr),
	.wrdata (fifo_wrdata),
	.rd     (fifo_rd),
	.rddata (fifo_rddata),
	
	// Status
	.empty (),
	.full  (),
	.data_count ()
);
*/

assign last_o      = fifo_rddata[27];
assign lr_o        = fifo_rddata[26];
assign ydst_o[8:0] = fifo_rddata[25:17];
assign xdst_o[9:0] = fifo_rddata[16: 7];
assign len_o[6:0]  = fifo_rddata[ 6: 0];


//==========================================================================
// Bilinear Interpolation
//==========================================================================
assign xi[9:0] = xsrc_i[14:5]; // integer part

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xi_r <= 2'b0;
	end
	else begin
		xi_r <= xi[1:0];
	end
end

//assign xi_p1[9:0] = xi[9:0] + 1'b1;
assign xi_p2[1:0] = xi_r[1:0] + 2'b10;

assign overlap = vin & vin_r & (xi[1:0] == xi_r[1:0]); // same index
//assign skip    = vin & vin_r & ((xi[1:0] + 1'b1) != xi_p1[1:0]);
assign skip    = vin & vin_r & (xi[1:0] == xi_p2[1:0]); // incr by 2

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		overlap_r <= 2'b0;
		skip_r    <= 4'b0;
	end
	else begin
		overlap_r <= {overlap_r[0], overlap};
		skip_r    <= {skip_r[2:0], skip};
	end
end

assign skip_wait = skip_r[0];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vcnt <= 8'b0;
	end
	else begin
		if (bst_st) begin
			vcnt <= len_i;
		end
		else if (skip_r[0]) begin
			vcnt <= vcnt;
		end
		else if (vcnt != 0) begin
			vcnt <= vcnt - 1'b1;
		end
	end
end

assign valid = (vcnt != 0) & ~skip_r[0];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		valid_r <= 6'b0;
	end
	else begin
		valid_r <= {valid_r[4:0], valid};
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		incr_cnt <= 7'b0;
		rdaddr   <= 10'b0;
		lr_sel   <= 1'b0;
	end
	else begin
		if (bst_st) begin
			incr_cnt <= len_i;
			rdaddr   <= xi;
			lr_sel   <= lr_i;
		end
		else if (~overlap_r[0]) begin
			if (incr_cnt != 0) begin
				incr_cnt <= incr_cnt - 1'b1;
				rdaddr   <= rdaddr + 1'b1;
			end
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		up_r <= 8'b0;
		lo_r <= 8'b0;
	end
	else begin
		if (~overlap_r[1]) begin
			up_r <= up;
			lo_r <= lo;
		end
	end
end

// u10.5 -> u0.5
assign xf[4:0] = xsrc_i[4:0];
assign yf[4:0] = ysrc_i[4:0];

// u1.5 - u0.5 = u1.5 (max 1.0)
assign xf_inv[5:0] = 6'h20 - xf[4:0];
assign yf_inv[5:0] = 6'h20 - yf[4:0];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 2; i = i + 1) begin
			xf_r[i]  <= 5'b0;
			xfi_r[i] <= 6'b0;
			yf_r[i]  <= 5'b0;
			yfi_r[i] <= 6'b0;
		end
	end
	else begin
		for (i = 0; i < 2; i = i + 1) begin
			if (i == 0) begin
				xf_r[0]  <= (~skip_r[0]) ? xf     : xf_r[0];
				xfi_r[0] <= (~skip_r[0]) ? xf_inv : xfi_r[0];
				yf_r[0]  <= (~skip_r[0]) ? yf     : yf_r[0];
				yfi_r[0] <= (~skip_r[0]) ? yf_inv : yfi_r[0];
			end
			else begin
				xf_r[i]  <= xf_r[i-1];
				xfi_r[i] <= xfi_r[i-1];
				yf_r[i]  <= yf_r[i-1];
				yfi_r[i] <= yfi_r[i-1];
			end
		end
		/*
		xf_r[0]  <= xf;
		xfi_r[0] <= xf_inv;
		yf_r[0]  <= yf;
		yfi_r[0] <= yf_inv;
		for (i = 1; i < 2; i = i + 1) begin
			xf_r[i]  <= xf_r[i-1];
			xfi_r[i] <= xfi_r[i-1];
			yf_r[i]  <= yf_r[i-1];
			yfi_r[i] <= yfi_r[i-1];
		end
		*/
	end
end

// u1.5 * u1.5 = u1.10 (max 1.0)
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xyi  <= 11'b0;
		xiyi <= 11'b0;
		xy   <= 11'b0;
		xiy  <= 11'b0;
	end
	else begin
		xyi  <= xf_r[1]  * yfi_r[1];
		xiyi <= xfi_r[1] * yfi_r[1];
		xy   <= xf_r[1]  * yf_r[1];
		xiy  <= xfi_r[1] * yf_r[1];
	end
end

// u8.0 * u1.10 = u8.10
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ur <= 18'b0;
		ul <= 18'b0;
		dr <= 18'b0;
		dl <= 18'b0;
	end
	else begin
		ur <= up   * xyi;
		ul <= up_r * xiyi;
		dr <= lo   * xy;
		dl <= lo_r * xiy;
	end
end

// u8.10 + u8.10 = u9.10
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ulr <= 19'b0;
		dlr <= 19'b0;
	end
	else begin
		ulr <= ul + ur;
		dlr <= dl + dr;
	end
end

// u9.10 -> u8.10
assign ulr_lim[17:0] = ulr[17:0];
assign dlr_lim[17:0] = dlr[17:0];

// u8.10 + u8.10 = u9.10
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		udlr <= 19'b0;
	end
	else begin
		udlr <= ulr_lim + dlr_lim;
	end
end

// u9.10 -> u8.1
assign udlr_lim[8:0] = udlr[17:9];

// u8.1 + u8.1 = u9.1
assign udlr_rnd[9:0] = udlr_lim + 1'b1;

// u9.1 -> u8.0
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		intp <= 8'b0;
	end
	else begin
		if (udlr_rnd[9]) begin
			intp <= 8'hFF;
		end
		else begin
			intp <= udlr_rnd[8:1];
		end
	end
end

assign vout = valid_r[5];

assign fifo_rd = bst_st_r[5];


endmodule
