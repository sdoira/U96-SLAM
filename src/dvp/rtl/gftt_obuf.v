//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module gftt_obuf (
	// Global Control
	rst_n, clk,
	
	// Parameter
	hgt,
	wdt,
	wdt_m1,
	addr_a,
	addr_b,
	bst_len_m1,
	
	// Control
	enb,
	start,
	
	// Status
	ovf, udf,
	max_a,
	max_b,
	
	// Frontend I/F
	din,
	vin,
	
	// DDR Arbiter I/F
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

// Parameter
input	[8:0]	hgt;
input	[9:0]	wdt, wdt_m1;
input	[11:0]	addr_a, addr_b;
input	[7:0]	bst_len_m1;

// Control
input			enb, start;

// Status
output			ovf, udf;
output	[15:0]	max_a, max_b;

// OBUF I/F
input	[15:0]	din;
input			vin;

// DDR Arbiter I/F
output			dwr_req;
input			dwr_ack;
output			dwr_vout;
output	[31:0]	dwr_dout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Max Value Detection
reg		[15:0]	max, max_a, max_b;

// FIFO Instance
wire			fifo_wr;
wire	[15:0]	fifo_din;
wire	[31:0]	fifo_dout;
wire			fifo_full, fifo_empty;
wire	[10:0]	fifo_wr_cnt;
wire	[9:0]	fifo_rd_cnt;
reg				ovf, udf;

// Buffer Controller
wire			data_rdy;
reg		[2:0]	state;
wire			fifo_rd;
reg		[7:0]	st_cnt;
wire			bst_end;
reg		[9:0]	col;
wire			col_end;
reg		[8:0]	row;
wire			row_end;
reg				bank;
wire			last_bst;
reg				last_bst_r;

// DDR Write Request
reg				dwr_req;
reg		[17:0]	addr_ofs;
wire	[31:0]	dwr_cmd, dwr_addr;
wire			dwr_vout;
wire	[31:0]	dwr_dout;


//==========================================================================
// Max Value Detection
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		max <= 16'b0;
	end
	else begin
		if (~enb | start | row_end) begin
			max <= 0;
		end
		else if (vin & (din[15:0] > max[15:0])) begin
			max <= din[15:0];
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		max_a <= 16'b0;
		max_b <= 16'b0;
	end
	else begin
		if (~enb) begin
			max_a <= 0;
			max_b <= 0;
		end
		else if (row_end) begin
			if (~bank) begin
				max_a <= max;
			end
			else begin
				max_b <= max;
			end
		end
	end
end


//==========================================================================
// FIFO Instance
//==========================================================================
assign fifo_wr = vin;
assign fifo_din[15:0] = din[15:0];

gftt_obuf_fifo gftt_obuf_fifo (
	.clk   (clk),
	.srst  (~rst_n),
	.din   (fifo_din[15:0]),
	.wr_en (fifo_wr),
	.rd_en (fifo_rd),
	.dout  (fifo_dout[31:0]),
	.full  (fifo_full),
	.empty (fifo_empty),
	.wr_data_count(fifo_wr_cnt[10:0]),
	.rd_data_count(fifo_rd_cnt[9:0]),
	.wr_rst_busy(),
	.rd_rst_busy()
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ovf <= 1'b0;
	end
	else begin
		if (fifo_full & fifo_wr) begin
			ovf <= 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		udf <= 1'b0;
	end
	else begin
		if (fifo_empty & fifo_rd) begin
			udf <= 1'b1;
		end
	end
end


//==========================================================================
// Buffer Controller
//==========================================================================
assign data_rdy = (fifo_rd_cnt > bst_len_m1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		state <= 3'b0;
	end
	else begin
		if (~enb | start) begin
			state <= 0;
		end
		else begin
			case (state)
				 0: state <= (data_rdy) ? 1 : 0;
				 1: state <=  2;
				 2: state <= (dwr_ack)  ? 3 : 2;
				 3: state <=  4;
				 4: state <= (bst_end)  ? 5 : 4;
				 5: state <=  6;
				 6: state <=  0;
			endcase
		end
	end
end

assign fifo_rd = (state == 3) | (state == 4);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		st_cnt <= 8'b0;
	end
	else begin
		if (fifo_rd) begin
			if (bst_end) begin
				st_cnt <= 0;
			end
			else begin
				st_cnt <= st_cnt + 1'b1;
			end
		end
	end
end

assign bst_end = (state == 4) & (st_cnt == bst_len_m1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		col <= 10'b0;
	end
	else begin
		if (~enb | start) begin
			col <= 0;
		end
		else if (fifo_rd) begin
			if (col_end) begin
				col <= 0;
			end
			else begin
				col <= col + 1'b1;
			end
		end
	end
end

assign col_end = (col == (wdt[9:1] - 1'b1));

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		row <= 9'b0;
	end
	else begin
		if (~enb | start) begin
			row <= 0;
		end
		else if (col_end) begin
			if (row_end) begin
				row <= 0;
			end
			else begin
				row <= row + 1'b1;
			end
		end
	end
end

assign last_row = (row == hgt - 3'd5); // exclude 4 border lines
assign row_end = col_end & last_row;

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		bank <= 1'b0;
	end
	else begin
		if (~enb) begin
			bank <= 0;
		end
		else if (row_end) begin
			bank <= ~bank;
		end
	end
end

assign last_bst = last_row & ((col + bst_len_m1) == (wdt[9:1] - 1));

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		last_bst_r <= 1'b0;
	end
	else begin
		if (~enb | start | row_end) begin
			last_bst_r <= 1'b0;
		end
		else if (last_bst) begin
			last_bst_r <= 1'b1;
		end
	end
end


//==========================================================================
// DDR Write Request
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dwr_req <= 1'b0;
	end
	else begin
		if (state == 1) begin
			dwr_req <= 1'b1;
		end
		else if (state == 5) begin
			dwr_req <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		addr_ofs <= 18'b0;
	end
	else begin
		if (~enb | start | row_end) begin
			addr_ofs <= {8'b0, wdt[9:1], 1'b0}; // 2 lines offset
		end
		else if (fifo_rd) begin
			addr_ofs <= addr_ofs + 1'b1;
		end
	end
end

assign dwr_cmd[31:0] = {22'h00_0000, last_bst_r, 1'b0, bst_len_m1[7:0]};
assign dwr_addr[31:20] = (~bank) ? addr_a[11:0] : addr_b[11:0];
assign dwr_addr[19: 0] = {addr_ofs[17:0], 2'b0};
assign dwr_vout = dwr_req & dwr_ack;

assign dwr_dout[31:0] = (
	(~dwr_vout)                   ? 32'b0 :
	(state == 2)                  ? dwr_cmd[31:0] :
	(state == 3)                  ? dwr_addr[31:0] :
	((state == 4) | (state == 5)) ? {fifo_dout[15:0], fifo_dout[31:16]} :
	                                32'b0
);


endmodule
