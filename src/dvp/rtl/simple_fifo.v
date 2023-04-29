//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module simple_fifo # (
	parameter integer W = 10,
	parameter integer D = 2
) (
	// Global Control
	rst_n, clk,
	
	// FIFO I/F
	wr,
	wrdata,
	rd,
	rddata,
	
	// Status
	empty,
	full,
	data_count
);


//==========================================================================
// Parameters
//==========================================================================
parameter integer MAX_COUNT = (1'b1 << D);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input				rst_n, clk;

// FIFO I/F
input				wr;
input	[W-1:0]		wrdata;
input				rd;
output	[W-1:0]		rddata;

// Status
output				empty, full;
output	[D:0]		data_count;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
integer				i;
reg		[W-1:0]		sftr[MAX_COUNT-1:0];
reg		[D-1:0]		rdaddr;
wire	[W-1:0]		rddata;
reg		[D:0]		data_count;
wire				empty, full;


//==========================================================================
// FIFO Implementation
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < MAX_COUNT; i = i + 1) begin
			sftr[i] <= {W-1{1'b0}};
		end
	end
	else begin
		if (wr) begin
			// shift register operation
			for (i = 1; i < MAX_COUNT; i = i + 1) begin
				sftr[i] <= sftr[i-1];
			end
			sftr[0] = wrdata;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		rdaddr <= {D{1'b1}};  // points the last
	end
	else begin
		if (wr & ~rd) begin
			rdaddr <= rdaddr + 1'b1;
		end
		else if (rd & ~wr) begin
			rdaddr <= rdaddr - 1'b1;
		end
	end
end

assign rddata[W-1:0] = sftr[rdaddr];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		data_count <= {D+1{1'b0}};
	end
	else begin
		if (wr & ~rd) begin
			data_count <= data_count + 1'b1;
		end
		else if (rd & ~wr) begin
			data_count <= data_count - 1'b1;
		end
	end
end

assign empty = (data_count == 0);
assign full  = (data_count == MAX_COUNT);


endmodule
