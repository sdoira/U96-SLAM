//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module bm_ibuf_sad (
	// Global Control
	rst_n, clk,
	
	// Write I/F
	wr,
	din,
	wr_rdy,
	
	// Read I/F
	sad_rdaddr,
	sad_dout,
	rd_rdy,
	
	// Control
	enb,
	start,
	line_end,
	next_line,
	
	// Parameter
	line_size,
	bst_len
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input				rst_n, clk;

// Write I/F
input				wr;
input	[31:0]		din;
output				wr_rdy;

// Read I/F
input	[9:0]		sad_rdaddr;
output	[63:0]		sad_dout;
output				rd_rdy;

// Control
input				enb, start;
input				line_end, next_line;

// Parameter
input	[9:0]		line_size; // line size in words
input	[8:0]		bst_len; // burst length


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// LR Buffer
reg		[31:0]	din_r;
reg				wr_phase;
wire			wr_64bit;
reg		[9:0]	wraddr;
reg		[10:0]	avl;
wire			wr_rdy;
wire	[9:0]	sad_addr;
wire	[63:0]	sad_dout;

// Status FIFO
reg				line_end_r;
reg		[9:0]	wraddr_r;
wire			fifo_wr;
wire	[9:0]	addr_ofs;
wire			fifo_empty;
wire			rd_rdy;


//==========================================================================
// SAD Buffer
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		din_r <= 32'b0;
	end
	else begin
		if (wr) begin
			din_r <= din;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		wr_phase <= 1'b0;
	end
	else begin
		if (start) begin
			wr_phase <= 1'b0;
		end
		else if (wr) begin
			wr_phase <= ~wr_phase;
		end
	end
end

assign wr_64bit = wr_phase & wr;

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		wraddr <= 10'b0;
	end
	else begin
		if (start) begin
			wraddr <= 0;
		end
		else if (wr_64bit) begin
			// roll over
			wraddr <= wraddr + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		avl <= 11'b0;
	end
	else begin
		if (start) begin
			avl <= 1024;
		end
		else if (wr_64bit & next_line) begin
			avl <= avl + line_size - 1'b1;
		end
		else if (wr_64bit) begin
			// consume 1 word space
			avl <= avl - 1'b1;
		end
		else if (next_line) begin
			// release one line buffer
			avl <= avl + line_size;
		end
	end
end

assign wr_rdy = (
	(bst_len[8]) ? |avl[10:8] : // 256
	(bst_len[7]) ? |avl[10:7] : // 128
	               |avl[10:6]   // <=64
);

assign sad_addr[9:0] = sad_rdaddr[9:0] + addr_ofs[9:0];

dpram_64_1024 dpram_64_1024 (
	.clka  (clk),
	.ena   (enb),
	.wea   (wr_64bit),
	.addra (wraddr[9:0]),
	.dina  ({din_r[31:0], din[31:0]}),
	.clkb  (clk),
	.enb   (enb),
	.addrb (sad_addr),
	.doutb (sad_dout[63:0])
);


//==========================================================================
// Status FIFO
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		line_end_r <= 1'b0;
	end
	else begin
		line_end_r <= line_end;
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		wraddr_r <= 10'b0;
	end
	else begin
		if (start) begin
			wraddr_r <= 0;
		end
		else if (line_end_r) begin
			// store the line start address
			wraddr_r <= wraddr;
		end
	end
end

assign fifo_wr = line_end_r;

simple_fifo #(10, 2) fifo_10_4 (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// FIFO I/F
	.wr     (fifo_wr),
	.wrdata (wraddr_r[9:0]),
	.rd     (next_line),
	.rddata (addr_ofs[9:0]),
	
	// Status
	.empty (fifo_empty),
	.full  (),
	.data_count ()
);

assign rd_rdy = ~fifo_empty;


endmodule
