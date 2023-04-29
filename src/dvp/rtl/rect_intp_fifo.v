//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module rect_intp_fifo (
	// Global Control
	rst_n, clk,
	
	// FIFO I/F
	fifo_wr,
	fifo_din,
	fifo_rd,
	fifo_dout
);

parameter W = 28;
parameter D = 2;
parameter DD = (1 << D);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input				rst_n, clk;

// FIFO I/F
input				fifo_wr;
input	[W-1:0]		fifo_din;
input				fifo_rd;
output	[W-1:0]		fifo_dout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Loop Index
integer				i;

// Simple FIFO
reg		[D-1:0]		wr_ptr;
reg		[W-1:0]		store[DD-1:0];
reg		[D-1:0]		rd_ptr;
wire	[W-1:0]		fifo_dout;


//==========================================================================
// Simple FIFO
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		wr_ptr <= {D{1'b0}};
	end
	else begin
		if (fifo_wr) begin
			wr_ptr <= wr_ptr + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < DD; i = i + 1) begin
			store[i] <= {W{1'b0}};
		end
	end
	else begin
		if (fifo_wr) begin
			store[wr_ptr] <= fifo_din;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		rd_ptr <= {D{1'b1}};
	end
	else begin
		if (fifo_rd) begin
			rd_ptr <= rd_ptr + 1'b1;
		end
	end
end

assign fifo_dout = store[rd_ptr];


endmodule
