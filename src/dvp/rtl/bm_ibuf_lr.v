//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module bm_ibuf_lr (
	// Global Control
	rst_n, clk,
	
	// Write I/F
	wr,
	din,
	wr_rdy,
	
	// Read I/F
	lr_rdaddr,
	lr_dout,
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
input	[9:0]		lr_rdaddr;
output	[15:0]		lr_dout;
output				rd_rdy;

// Control
input				enb, start;
input				line_end, next_line;

// Parameter
input	[8:0]		line_size; // line size in words
input	[8:0]		bst_len; // burst length


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// LR Buffer
reg		[8:0]	wraddr;
reg		[9:0]	avl;
wire			wr_rdy;
wire	[8:0]	ram_rdaddr;
reg				lr_rdaddr_r;
wire	[31:0]	lr_doutb;
wire	[15:0]	lr_dout;

// Status FIFO
reg				line_end_r;
reg		[8:0]	wraddr_r;
wire			fifo_wr;
wire	[8:0]	addr_ofs;
wire			fifo_empty;
wire			rd_rdy;


//==========================================================================
// LR Buffer
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		wraddr <= 9'b0;
	end
	else begin
		if (start) begin
			wraddr <= 0;
		end
		else if (wr) begin
			// roll over
			wraddr <= wraddr + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		avl <= 10'b0;
	end
	else begin
		if (start) begin
			avl <= 512;
		end
		else if (wr & next_line) begin
			avl <= avl + line_size - 1'b1;
		end
		else if (wr) begin
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
	(bst_len[8]) ? |avl[9:8] : // 256
	(bst_len[7]) ? |avl[9:7] : // 128
	               |avl[9:6]   // >=64
);

assign ram_rdaddr[8:0] = lr_rdaddr[9:1] + addr_ofs[8:0];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		lr_rdaddr_r <= 1'b0;
	end
	else begin
		lr_rdaddr_r <= lr_rdaddr[0];
	end
end

dpram_32_512 dpram_32_512 (
	.clka  (clk),
	.ena   (enb),
	.wea   (wr),
	.addra (wraddr[8:0]),
	.dina  (din[31:0]),
	.clkb  (clk),
	.enb   (enb),
	.addrb (ram_rdaddr[8:0]),
	.doutb (lr_doutb[31:0])
);

assign lr_dout[15:0] = (
	(~lr_rdaddr_r) ? lr_doutb[31:16] :
	                 lr_doutb[15: 0]
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
		wraddr_r <= 9'b0;
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

wire	[2:0]	data_count;
simple_fifo #(9, 2) fifo_9_4 (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// FIFO I/F
	.wr     (fifo_wr),
	.wrdata (wraddr_r[8:0]),
	.rd     (next_line),
	.rddata (addr_ofs[8:0]),
	
	// Status
	.empty (fifo_empty),
	.full  (),
	.data_count (data_count[2:0])
);

assign rd_rdy = ~fifo_empty;


endmodule
