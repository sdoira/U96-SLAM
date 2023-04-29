//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module csi_dly (
	// Global Control
	rst_n, clk,
	
	// Parameter
	enb,
	
	// Status
	fifo_cnt,
	fifo_ovf,
	fifo_udf,
	
	// Parallel Video Input (Ch1)
	sof1_in, v1_in, d1_in,
	
	// Parallel Video Input (Ch2)
	sof2_in, v2_in, d2_in,
	
	// Parallel Video Output
	sof_out, vout, d1_out, d2_out
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Parameter
input			enb;

// Status
output	[5:0]	fifo_cnt;
output			fifo_ovf, fifo_udf;

// Parallel Video Input (Ch1)
input			sof1_in, v1_in;
input	[15:0]	d1_in;

// Parallel Video Input (Ch2)
input			sof2_in, v2_in;
input	[15:0]	d2_in;

// Parallel Video Output
output			sof_out, vout;
output	[15:0]	d1_out, d2_out;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Input Registers
reg				sof1_in_r, v1_in_r;
reg		[15:0]	d1_in_r2, d1_in_r1;
reg				sof2_in_r, v2_in_r;
reg		[15:0]	d2_in_r2,d2_in_r1;

// FIFO Control
reg		[1:0]	ch;
wire			fifo_wr;
wire	[15:0]	fifo_din;
wire			fifo_full;
wire			fifo_empty;
wire	[15:0]	fifo_dout;
wire	[9:0]	fifo_cnt;
wire			fifo_rd;
reg				fifo_rd_r;
reg				fifo_ovf, fifo_udf;

// Parallel Video Output
reg				sof_out, vout;
reg		[15:0]	d1_out, d2_out;


//==========================================================================
// Input Registers
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		sof1_in_r <=  1'b0;
		v1_in_r   <=  1'b0;
		d1_in_r2  <= 16'b0;
		d1_in_r1  <= 16'b0;
		sof2_in_r <=  1'b0;
		v2_in_r   <=  1'b0;
		d2_in_r2  <= 16'b0;
		d2_in_r1  <= 16'b0;
	end
	else begin
		if (~enb) begin
			sof1_in_r <=  1'b0;
			v1_in_r   <=  1'b0;
			d1_in_r2  <= 16'b0;
			d1_in_r1  <= 16'b0;
			sof2_in_r <=  1'b0;
			v2_in_r   <=  1'b0;
			d2_in_r2  <= 16'b0;
			d2_in_r1  <= 16'b0;
		end
		else begin
			sof1_in_r <= sof1_in;
			v1_in_r   <= v1_in;
			d1_in_r2  <= d1_in_r1;
			d1_in_r1  <= d1_in;
			sof2_in_r <= sof2_in;
			v2_in_r   <= v2_in;
			d2_in_r2  <= d2_in_r1;
			d2_in_r1  <= d2_in;
		end
	end
end


//==========================================================================
// FIFO Control
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ch <= 2'b0;
	end
	else begin
		if (~enb) begin
			ch <= 0;
		end
		else begin
			if (ch == 0) begin
				if (sof1_in & sof2_in) begin // in-synch
					ch <= 2'd3;
				end
				else if (sof1_in) begin // ch1 is early
					ch <= 2'd1;
				end
				else if (sof2_in) begin // ch2 is early
					ch <= 2'd2;
				end
			end
		end
	end
end

assign fifo_wr = (
	(~enb)    ? 1'b0 :
	(ch == 1) ? v1_in_r :
	(ch == 2) ? v2_in_r :
	            1'b0
);

assign fifo_din[15:0] = (ch == 1) ? d1_in_r1[15:0] : d2_in_r1[15:0];

csi_dly_fifo csi_dly_fifo (
	// Global Control
	.clk  (clk),
	.srst (~rst_n),
	
	// Write port
	.full  (fifo_full),
	.din   (fifo_din[15:0]),
	.wr_en (fifo_wr),
	
	// Read port
	.empty (fifo_empty),
	.dout  (fifo_dout[15:0]),
	.rd_en (fifo_rd),
	
	// Status
	.data_count  (fifo_cnt[9:0]),
	.wr_rst_busy (),
	.rd_rst_busy ()
);

assign fifo_rd = (
	(~enb)    ? 1'b0 :
	(ch == 1) ? v2_in_r :
	(ch == 2) ? v1_in_r :
	            1'b0
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		fifo_rd_r <= 1'b0;
	end
	else begin
		if (~enb) begin
			fifo_rd_r <= 1'b0;
		end
		else begin
			fifo_rd_r <= fifo_rd;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		fifo_ovf <= 1'b0;
	end
	else begin
		if (~enb) begin
			fifo_ovf <= 1'b0;
		end
		else if (fifo_full & fifo_wr) begin
			fifo_ovf <= 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		fifo_udf <= 1'b0;
	end
	else begin
		if (~enb) begin
			fifo_udf <= 1'b0;
		end
		else if (fifo_empty & fifo_rd) begin
			fifo_udf <= 1'b1;
		end
	end
end


//==========================================================================
// Parallel Video Output
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		sof_out <= 1'b0;
		vout    <= 1'b0;
		d1_out  <= 16'b0;
		d2_out  <= 16'b0;
	end
	else begin
		if (~enb) begin
			sof_out <= 0;
		end
		else begin
			if (ch == 1) begin
				sof_out <= sof1_in_r;
				vout    <= fifo_rd_r;
				d1_out  <= (fifo_rd_r) ? fifo_dout[15:0] : 16'b0;
				d2_out  <= (fifo_rd_r) ? d2_in_r2[15:0] : 16'b0;
			end
			else if (ch == 2) begin
				sof_out <= sof2_in_r;
				vout    <= fifo_rd_r;
				d1_out  <= (fifo_rd_r) ? d1_in_r2[15:0] : 16'b0;
				d2_out  <= (fifo_rd_r) ? fifo_dout[15:0] : 16'b0;
			end
			else if (ch == 3) begin
				sof_out <= sof1_in_r;
				vout    <= v1_in_r;
				d1_out  <= d1_in_r1;
				d2_out  <= d2_in_r1;
			end
		end
	end
end


endmodule
