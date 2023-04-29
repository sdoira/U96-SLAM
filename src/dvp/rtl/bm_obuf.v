//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module bm_obuf (
	// Global Control
	rst_n, clk,
	
	// Parameter
	sad_hgt,
	sad_wdt,
	addr_a,
	addr_b,
	bst_len_m1,
	bst_len,
	
	// Control
	enb,
	frm_end_i,
	
	// Status
	ovf, udf,
	
	// OBUF I/F
	din,
	wr,
	
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
input	[8:0]	sad_hgt;
input	[9:0]	sad_wdt;
input	[9:0]	addr_a, addr_b;
input	[7:0]	bst_len_m1;
input	[8:0]	bst_len;

// Control
input			enb;
input			frm_end_i;

// Status
output			ovf, udf;

// OBUF I/F
input	[65:0]	din;
input			wr;

// DDR Arbiter I/F
output			dwr_req;
input			dwr_ack;
output			dwr_vout;
output	[31:0]	dwr_dout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// FIFO Instance
wire			last_dphase;
wire	[31:0]	fifo_dout;
wire			fifo_full, fifo_empty;
wire	[12:0]	data_count;
wire			data_rdy;
reg				ovf, udf;

// Buffer Controller
reg		[2:0]	state;
reg		[7:0]	cnt;
wire			bst_end;
reg		[21:0]	next_addr;
reg		[19:0]	rem;
reg		[8:0]	payload;
wire			payload_end;
reg				padding;
wire			fifo_rd;
wire			line_end;
reg		[8:0]	line_cnt;
wire			last_line;
reg				last_bit;
reg				last_bit_r;
wire			frm_end;
reg				bank;

// DDR Arbiter Interface
reg				dwr_req;
wire	[31:0]	dwr_addr;
wire			dwr_vout;
wire	[31:0]	dwr_dout;


//==========================================================================
// FIFO Instance
//==========================================================================
assign last_dphase = din[65];

bm_obuf_fifo bm_obuf_fifo (
	.clk   (clk),
	.srst  (~rst_n),
	.din   (din[63:0]),
	.wr_en (wr),
	.rd_en (fifo_rd),
	.dout  (fifo_dout[31:0]),
	.full  (fifo_full),
	.empty (fifo_empty),
	//.data_count(data_count[12:0]),
	.rd_data_count(data_count[12:0]),
	.wr_rst_busy(),
	.rd_rst_busy()
);

assign data_rdy = (data_count[12:0] >= payload);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ovf <= 1'b0;
	end
	else begin
		if (fifo_full & wr) begin
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
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		state <= 3'b0;
	end
	else begin
		case (state)
			0: if (wr)       state <= 1;
			1:               state <= 2;
			2: if (data_rdy) state <= 3;
			3: if (dwr_ack)  state <= 4;
			4:               state <= 5;
			5: if (bst_end)  state <= (rem == 0) ? 0 : 1;
		endcase
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		cnt <= 8'b0;
	end
	else begin
		if (state == 5) begin
			if (bst_end) begin
				cnt <= 0;
			end
			else begin
				cnt <= cnt + 1'b1;
			end
		end
	end
end

assign bst_end = (state == 5) & (cnt == bst_len_m1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		next_addr <= 22'b0;
	end
	else begin
		if (line_end) begin
			next_addr <= 0;
		end
		else if (bst_end) begin
			next_addr <= next_addr + {bst_len[8:0], 2'b0};
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		rem     <= 20'b0;
		payload <= 9'b0;
	end
	else begin
		if ((state == 0) & wr) begin
			if (last_dphase) begin
				rem     <= sad_wdt * sad_hgt; // 32-bit
				payload <= 0;
			end
			else begin
				rem     <= {sad_wdt, 1'b0}; // 64-bit
				payload <= 0;
			end
		end
		else if (state == 1) begin
			if (rem < bst_len) begin
				rem     <= 0;
				payload <= rem;
			end
			else begin
				rem     <= rem - bst_len;
				payload <= bst_len;
			end
		end
	end
end

assign payload_end = (state == 5) & (cnt == payload - 1'b1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		padding <= 1'b0;
	end
	else begin
		if (bst_end) begin
			padding <= 1'b0;
		end
		//else if (cnt == payload - 1'b1) begin
		else if (payload_end) begin
			padding <= 1'b1;
		end
	end
end

assign fifo_rd = (state == 4) | ((state == 5) & ~payload_end & ~padding & ~bst_end);

assign line_end = bst_end & (rem == 0);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		line_cnt <= 9'b0;
	end
	else begin
		if ((state == 0) & wr & last_dphase) begin
			line_cnt <= sad_hgt - 1'b1;
		end
		else if (line_end) begin
			if (last_line) begin
				line_cnt <= 0;
			end
			else begin
				line_cnt <= line_cnt + 1'b1;
			end
		end
	end
end

assign last_line = (line_cnt == sad_hgt - 1'b1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		last_bit <= 1'b0;
	end
	else begin
		if (frm_end) begin
			last_bit <= 1'b0;
		end
		else if (wr & din[64]) begin
			last_bit <= 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		last_bit_r <= 1'b0;
	end
	else begin
		if (frm_end) begin
			last_bit_r <= 1'b0;
		end
		else if (last_bit & (rem == 0)) begin
			last_bit_r <= 1'b1;
		end
	end
end

assign frm_end = (last_bit_r & line_end) | frm_end_i;

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		bank <= 1'b0;
	end
	else begin
		if (~enb) begin
			bank <= 1'b0;
		end
		else if (frm_end) begin
			bank <= ~bank;
		end
	end
end


//==========================================================================
// DDR Arbiter Interface
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dwr_req <= 1'b0;
	end
	else begin
		if ((state == 2) & data_rdy) begin
			dwr_req <= 1'b1;
		end
		else if (bst_end) begin
			dwr_req <= 1'b0;
		end
	end
end

assign dwr_addr[31:22] = (~bank) ? addr_a[9:0] : addr_b[9:0];
assign dwr_addr[21:13] = (last_dphase) ? next_addr[21:13] : line_cnt[8:0];
assign dwr_addr[12: 0] = next_addr[12:0];

assign dwr_vout = dwr_req & dwr_ack;
assign dwr_dout[31:0] = (
	(~dwr_vout)  ? 32'b0 :
	(state == 3) ? {22'h00_0000, last_bit_r, 1'b0, bst_len_m1[7:0]} :
	(state == 4) ? dwr_addr[31:0] :
	(padding)    ? 32'b0 :
	(state == 5) ? fifo_dout[31:0] :
	//(state == 5) ? (last_dphase) ? 32'hFF000080 : fifo_dout[31:0] :
	               32'b0
);


endmodule
