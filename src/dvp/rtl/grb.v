//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module grb (
	// Global Control
	rst_n, clk,
	
	// DVP Interface
	pclk, vsync, href, d1, d2,
	
	// Internal Bus I/F
	ibus_cs,
	ibus_wr,
	ibus_addr_7_2,
	ibus_wrdata,
	ibus_rddata,
	
	// Arbiter I/F
	req,
	ack,
	dout,
	vout
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// DVP Interface
input			pclk;
input			vsync, href;
input	[15:0]	d1, d2;

// Internal Bus I/F
input			ibus_cs;
input			ibus_wr;
input	[5:0]	ibus_addr_7_2;
input	[31:0]	ibus_wrdata;
output	[31:0]	ibus_rddata;

// Arbiter I/F
output			req;
input			ack;
output	[31:0]	dout;
output			vout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Register Interface
reg				ctrl;
reg		[31:0]	addr_a, addr_b;
reg		[7:0]	bst_len_m1;
reg		[9:0]	hgt;
reg		[10:0]	wdt;
wire	[8:0]	bst_len;
wire	[31:0]	ibus_rddata;
wire			enb;

// FIFO Write
reg				wr_on;
reg				href_r;
wire			h_pos;
//reg				byte_cnt;
reg		[31:0]	d_r;
wire	[31:0]	fifo_din;
wire			fifo_wr;

// FIFO Instance
wire			fifo_full;
wire	[31:0]	fifo_dout;
wire			fifo_empty;
wire	[8:0]	fifo_rd_cnt;
reg				fifo_ovf, fifo_udf;

// FIFO Read
wire			fifo_rd_rdy;
reg		[2:0]	rd_state;
reg		[7:0]	bst_cnt;
wire			bst_end;
reg				req;
wire			fifo_rd;
reg				cmd_phase, addr_phase, data_phase;
reg		[10:0]	pcnt;
wire			line_end;
reg		[9:0]	hcnt;
wire			frm_end;
reg				bank;
wire			last;
reg		[31:0]	addr;
wire			vout;
wire	[31:0]	dout;


//==========================================================================
// Register Interface
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ctrl       <=  1'b0;
		addr_a     <= 32'h0140_0040; // for 640x480, header 64Bytes
		addr_b     <= 32'h0152_C080;
		bst_len_m1 <=  8'd63; // actual length minus 1
		hgt        <= 10'd480;
		wdt        <= 11'd640;
	end
	else begin
		if (ibus_cs & ibus_wr) begin
			case (ibus_addr_7_2)
				6'h00: ctrl       <= ibus_wrdata[   0];
				6'h02: addr_a     <= ibus_wrdata[31:0];
				6'h03: addr_b     <= ibus_wrdata[31:0];
				6'h04: bst_len_m1 <= ibus_wrdata[ 7:0];
				6'h05: begin
					hgt            <= ibus_wrdata[25:16];
					wdt            <= ibus_wrdata[10: 0];
				end
				default: begin
					ctrl       <= ctrl;
					addr_a     <= addr_a;
					addr_b     <= addr_b;
					bst_len_m1 <= bst_len_m1;
					hgt        <= hgt;
					wdt        <= wdt;
				end
			endcase
		end
	end
end

assign bst_len[8:0] = bst_len_m1 + 1'b1;

assign ibus_rddata[31:0] = (
	(~ibus_cs)              ?  32'b0 :
	(ibus_addr_7_2 == 6'h0) ? {31'b0, ctrl} :
	(ibus_addr_7_2 == 6'h1) ? {30'b0, fifo_udf, fifo_ovf} :
	(ibus_addr_7_2 == 6'h2) ?  addr_a[31:0] :
	(ibus_addr_7_2 == 6'h3) ?  addr_b[31:0] :
	(ibus_addr_7_2 == 6'h4) ? {24'b0, bst_len_m1[7:0]} :
	(ibus_addr_7_2 == 6'h5) ? {6'b0, hgt[9:0], 5'b0, wdt[10:0]} :
	                           32'b0
);

assign enb = ctrl;


//==========================================================================
// FIFO Write
//==========================================================================
always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		wr_on <= 1'b0;
	end
	else begin
		if (~enb) begin
			wr_on <= 1'b0;
		end
		else if (vsync) begin
			wr_on <= 1'b1;
		end
	end
end

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		href_r  <= 1'b0;
	end
	else begin
		href_r  <= href;
	end
end

assign h_pos = (href  & ~href_r);
/*
always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		byte_cnt <= 1'b0;
	end
	else begin
		if (wr_on) begin
			if (h_pos) begin
				byte_cnt <= 1'b0;
			end
			else if (href) begin
				byte_cnt <= ~byte_cnt;
			end
		end
		else begin
			byte_cnt <= 0;
		end
	end
end
*/
always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		d_r <= 32'b0;
	end
	else begin
		if (wr_on) begin
			d_r <= {d2, d1};
			//d_r <= {24'b0, d1[7:0]};
		end
		else begin
			d_r <= 0;
		end
	end
end

//assign fifo_din[31:0] = {d_r[15:0], d2[7:0], d1[7:0]};
//assign fifo_wr = (wr_on & href_r & ~byte_cnt);
assign fifo_din[31:0] = d_r[31:0];
assign fifo_wr = (wr_on & href_r);


//==========================================================================
// FIFO Instance
//==========================================================================
grb_fifo grb_fifo (
	// Common
	.rst    (~enb),
	
	// Write Port
	.wr_clk (pclk),
	.din    (fifo_din[31:0]),
	.wr_en  (fifo_wr),
	.full   (fifo_full),
	.wr_data_count (),
	.wr_rst_busy   (),
	
	// Read Port
	.rd_clk (clk),
	.rd_en  (fifo_rd),
	.dout   (fifo_dout[31:0]),
	.empty  (fifo_empty),
	.rd_data_count (fifo_rd_cnt[8:0]),
	.rd_rst_busy   ()
);

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
// FIFO Read
//==========================================================================
assign fifo_rd_rdy = (fifo_rd_cnt[8:0] >= bst_len[8:0]);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		rd_state <= 3'b0;
	end
	else begin
		if (~enb) begin
			rd_state <= 0;
		end
		else begin
			case (rd_state)
				0: if (fifo_rd_rdy) rd_state <= 1;
				1: if (ack)         rd_state <= 2;
				2:                  rd_state <= 3;
				3: if (bst_end)     rd_state <= 4;
				4:                  rd_state <= 0;
			endcase
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		bst_cnt <= 8'b0;
	end
	else begin
		if (rd_state == 3) begin
			if (bst_end) begin
				bst_cnt <= 0;
			end
			else begin
				bst_cnt <= bst_cnt + 1'b1;
			end
		end
		else begin
			bst_cnt <= 0;
		end
	end
end

assign bst_end = (bst_cnt == bst_len_m1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		req <= 1'b0;
	end
	else begin
		if (~enb) begin
			req <= 1'b0;
		end
		else if ((rd_state == 0) & fifo_rd_rdy) begin
			req <= 1'b1;
		end
		else if (rd_state == 4) begin
			req <= 1'b0;
		end
	end
end

assign fifo_rd = (rd_state == 3);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		cmd_phase <= 1'b0;
	end
	else begin
		if ((rd_state == 1) & ack) begin
			cmd_phase <= 1'b1;
		end
		else begin
			cmd_phase <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		addr_phase <= 1'b0;
	end
	else begin
		if (rd_state == 2) begin
			addr_phase <= 1'b1;
		end
		else begin
			addr_phase <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		data_phase <= 1'b0;
	end
	else begin
		if (rd_state == 3) begin
			data_phase <= 1'b1;
		end
		else begin
			data_phase <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		pcnt <= 11'b0;
	end
	else begin
		if (~enb) begin
			pcnt <= 0;
		end
		else if (fifo_rd) begin
			if (line_end) begin
				pcnt <= 0;
			end
			else begin
				pcnt <= pcnt + 1'b1;
			end
		end
	end
end

assign line_end = (pcnt == wdt - 1'b1);
//assign line_end = (pcnt == wdt[10:1] - 1'b1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		hcnt <= 10'b0;
	end
	else begin
		if (~enb) begin
			hcnt <= 0;
		end
		else if (line_end) begin
			if (frm_end) begin
				hcnt <= 0;
			end
			else begin
				hcnt <= hcnt + 1'b1;
			end
		end
	end
end

assign frm_end = line_end & (hcnt == hgt - 1'b1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		bank <= 1'b0;
	end
	else begin
		if (~enb) begin
			bank <= 0;
		end
		else if (frm_end) begin
			bank <= ~bank;
		end
	end
end

assign last = (hcnt == hgt - 1'b1) & (pcnt == wdt - bst_len);
//assign last = (hcnt == hgt - 1'b1) & (pcnt == wdt[10:1] - bst_len);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		addr <= 32'b0;
	end
	else begin
		if (~enb) begin
			addr <= addr_a[31:0];
		end
		else if (frm_end) begin
			addr <= (~bank) ? addr_b[31:0] : addr_a[31:0];
		end
		else if (bst_end) begin
			addr <= addr + {bst_len[8:0], 2'b00};
		end
	end
end

assign vout = cmd_phase | addr_phase | data_phase;
assign dout[31:0] = (
	(cmd_phase)  ? {22'b0, last, 1'b0, bst_len_m1[7:0]} :
	(addr_phase) ? addr[31:0] :
	(data_phase) ? fifo_dout[31:0] :
	               32'b0
);


endmodule
