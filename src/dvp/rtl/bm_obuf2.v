//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module bm_obuf2 (
	// Global Control
	rst_n, clk,
	
	// Parameter
	img_hgt,
	img_wdt,
	hwsz,
	ndisp,
	addr_a,
	addr_b,
	
	// Control
	enb,
	frm_end,
	
	// Status
	ovf, udf,
	
	// OBUF I/F
	din,
	wr,
	
	// DDR Arbiter I/F
	dwr_req,
	dwr_ack,
	dwr_vout,
	dwr_dout,
	dwr_strb
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Parameter
input	[8:0]	img_hgt;
input	[9:0]	img_wdt;
input	[3:0]	hwsz;
input	[8:0]	ndisp;
input	[11:0]	addr_a, addr_b;

// Control
input			enb;
output			frm_end;

// Status
output			ovf, udf;

// OBUF I/F
input	[15:0]	din;
input			wr;

// DDR Arbiter I/F
output			dwr_req;
input			dwr_ack;
output			dwr_vout;
output	[31:0]	dwr_dout;
output	[3:0]	dwr_strb;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Parameter
wire	[5:0]	bst_len;
wire	[4:0]	bst_len_m1;
wire	[8:0]	inv_wdt;
wire	[3:0]	inv_bst;
wire	[4:0]	inv_smp;

// FIFO Instance
wire	[7:0]	disp, frac;
wire	[15:0]	disp_ext, frac_ext;
wire	[16:0]	depth;
wire	[15:0]	depth_lim;
reg		[15:0]	depth_lim_r;
reg				wr_r;
wire			fifo_wr;
wire	[16:0]	fifo_din;
wire	[33:0]	fifo_dout;
wire			fifo_full, fifo_empty;
wire	[11:0]	fifo_wr_cnt;
wire	[10:0]	fifo_rd_cnt;
reg				ovf, udf;
wire			fifo_rd;
wire           last_bst_o;

// Buffer Controller
reg		[3:0]	state;
//wire			st_trans_1;
wire			st_trans_2, st_trans_4, st_trans_6, st_trans_7_1, st_trans_7_2;
wire			st_trans_9, st_trans_10, st_trans_11_1, st_trans_11_2, st_trans_13;
reg 	[5:0]	st_cnt;
reg		[9:0]	col;
reg		[8:0]	row;
wire			last_col, last_row;
wire			line_end, frm_end;
reg				bank, bank_r;
reg				last_bst;

// DDR Arbiter Interface
reg		[19:0]	addr_ofs;
reg				dwr_req;
wire	[31:0]	dwr_cmd, dwr_addr;
wire			dwr_vout;
wire	[31:0]	dwr_dout;


//==========================================================================
// Parameter
//==========================================================================
assign bst_len[5:0] = 32; // fixed for easy design, 32samples = 16words
assign bst_len_m1[4:0] = bst_len - 1;

assign inv_wdt[8:0] = ndisp[8:0] + hwsz[3:0] + 1;
assign inv_bst[3:0] = inv_wdt[8:5];
assign inv_smp[4:0] = inv_wdt[4:0];


//==========================================================================
// FIFO Instance
//==========================================================================
// integer part of disparity (u8.0)
assign disp[7:0] = din[15:8];

// fraction part (s-1.8)
assign frac[7:0] = din[ 7:0];

// (u8.0) -> (8.8)
assign disp_ext[15:0] = {disp[7:0], 8'b0};

// (s-1.8) -> (s8.8)
assign frac_ext[15:0] = {{8{frac[7]}}, frac[7:0]};

// (u8.8) + (s7.8) -> (s8.8)
assign depth[16:0] = {1'b0, disp_ext[15:0]} + {frac_ext[15], frac_ext[15:0]};

// (s8.8) -> (s11.4)
assign depth_lim[15:0] = (
	(depth[16])                 ? 16'hFFFF : // negative
	//(|depth[15:11])             ? 16'h07FF : // upper limit
	(depth[16:0] == 17'h0_0000) ? 16'hFFFF : // zero
	                              {{4{depth[15]}}, depth[15:4]}
);
//assign depth_lim[15:0] = {4'b0, disp[7:0], 4'b0};

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		depth_lim_r <= 16'b0;
		wr_r <= 1'b0;
	end
	else begin
		depth_lim_r <= depth_lim;
		wr_r <= wr;
	end
end

reg		[7:0]	test_ptn;
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		test_ptn <= 8'b0;
	end
	else begin
		if (wr_r) begin
			test_ptn <= test_ptn + 1'b1;
		end
		else if (last_col) begin
			test_ptn <= 0;
		end
	end
end

assign fifo_wr = wr_r | (state == 6) | (state == 13);
assign fifo_din[16:0] = wr_r ? {last_bst, depth_lim_r[15:0]} : 17'h0FFFF;
//assign fifo_din[16:0] = wr_r ? {last_bst, 2'b0, bank, row[8:0], 4'b0} : 17'h0FFFF;
//assign fifo_din[16:0] = wr_r ? {last_bst, 3'b0, bank, test_ptn[7:0], 4'b0} : 17'h0FFFF;
/*
assign fifo_din[16:0] = (
	(wr_r)        ? {last_bst, (~bank & (row == 10)) ? 16'h0800 : 16'h0000} :
	(state ==  6) ? 17'h0FFFF :
	(state == 13) ? 17'h0C000 :
	                17'h0FFFF
);
*/
/*
assign fifo_din[16:0] = (
	(wr_r)        ? {last_bst, ~bank ? 16'h0800 : 16'h0400} :
	(state ==  6) ? 17'h0FFFF :
	(state == 13) ? 17'h0C000 :
	                17'h0FFFF
);
*/

bm_obuf2_fifo bm_obuf2_fifo (
	.clk   (clk),
	.srst  (~rst_n),
	.din   (fifo_din[16:0]),
	.wr_en (fifo_wr),
	.rd_en (fifo_rd),
	.dout  (fifo_dout[33:0]),
	.full  (fifo_full),
	.empty (fifo_empty),
	.wr_data_count(fifo_wr_cnt[11:0]),
	.rd_data_count(fifo_rd_cnt[10:0]),
	.wr_rst_busy(),
	.rd_rst_busy()
);

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

assign fifo_rd = (state == 10);
assign last_bst_o = fifo_dout[33];


//==========================================================================
// Buffer Controller
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		state <= 4'b0;
	end
	else begin
		if (~enb) begin
			state <= 0;
		end
		else begin
			case (state)
				 0: state <=  1;
				 1: state <=  2;
				// 1: state <= (st_trans_1)  ?  2 :  1;
				 2: state <= (st_trans_2)  ?  3 :  2;
				 3: state <=  4;
				 4: state <= (st_trans_4)  ?  5 :  4;
				 5: state <=  6;
				 6: state <= (st_trans_6)  ?  7 :  6;
				 7: state <= (
				 	(st_trans_7_1) ? 12 :
				 	(st_trans_7_2) ?  8 :
				 	                  7
				 );
				 8: state <=  9;
				 9: state <= (st_trans_9)  ? 10 :  9;
				10: state <= (st_trans_10) ? 11 : 10;
				11: state <= (
					(st_trans_11_1) ? 14 : // next line
					(st_trans_11_2) ?  3 : // next frame
					                   7
				);
				12: state <= 13;
				13: state <= (st_trans_13) ?  8 : 13;
				14: state <=  0;
			endcase
		end
	end
end

//assign st_trans_1    = (state ==  1) & (fifo_rd_cnt == 0);
assign st_trans_2    = (state ==  2) & (st_cnt == hwsz - 1);
assign st_trans_4    = (state ==  4) & (st_cnt == inv_bst - 1);
assign st_trans_6    = (state ==  6) & (st_cnt == inv_smp - 1);
assign st_trans_7_1  = (state ==  7) & (col == img_wdt - hwsz);
//assign st_trans_7_2  = (state ==  7) & (fifo_wr_cnt == bst_len);
assign st_trans_7_2  = (state ==  7) & (fifo_wr_cnt >= bst_len);
assign st_trans_9    = (state ==  9) & (dwr_ack);
assign st_trans_10   = (state == 10) & (st_cnt == bst_len[5:1] - 1);
//assign st_trans_11_1 = (state == 11) & (row == img_hgt - hwsz);
//assign st_trans_11_2 = (state == 11) & (col == 0);
assign st_trans_11_1 = (state == 11) & (row == img_hgt - hwsz) & (fifo_rd_cnt == 0);
assign st_trans_11_2 = (state == 11) & (col == 0) & (fifo_rd_cnt == 0);
//assign st_trans_11_2 = (state == 11) & (frm_end);
assign st_trans_13   = (state == 13) & (st_cnt == hwsz - 1);

// state machine timing counter
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		st_cnt <= 6'b0;
	end
	else begin
		if (~enb) begin
			st_cnt <= 0;
		end
		else begin
			if (
				//st_trans_2 | st_trans_4 | st_trans_6 | st_trans_7_1 | st_trans_7_2 |
				st_trans_2 | st_trans_4 | st_trans_6 |
				st_trans_9 | st_trans_10 | st_trans_13
			) begin
				st_cnt <= 0;
			end
			else if (
				//(state == 2) | (state == 4) | (state == 6) | (state == 7) |
				(state == 2) | (state == 4) | (state == 6) |
				(state == 9) | (state == 10) | (state == 13)
			) begin
				st_cnt <= st_cnt + 1'b1;
			end
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		col <= 10'b0;
		row <=  9'b0;
	end
	else begin
		if (state == 0) begin
			col <= 0;
			row <= 0;
		end
		else if (state == 2) begin
			row <= row + 1'b1;
		end
		else if (state == 4) begin
			col <= col + bst_len;
		end
		else if ((state == 6) | (state == 13) | fifo_wr) begin
			if (last_col) begin
				col <= 0;
				row <= row + 1'b1;
			end
			else begin
				col <= col + 1'b1;
			end
		end
		/*
		else if ((state == 6) | (state == 13)) begin
			if (last_col) begin
				col <= 0;
				row <= row + 1'b1;
			end
			else begin
				col <= col + 1'b1;
			end
		end
		else if (fifo_rd) begin
			col <= col + 2'd2;
		end
		*/
	end
end

assign last_col = (col == img_wdt - 1);
assign last_row = (row == img_hgt - hwsz - 1); // invalid lines at the end doesn't count

assign line_end = last_col;
assign frm_end = (last_col & last_row);

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

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		bank_r <= 1'b0;
	end
	else begin
		if (~enb) begin
			bank_r <= 1'b0;
		end
		else if (state == 0) begin
			bank_r <= bank;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		last_bst <= 1'b0;
	end
	else begin
		if (state == 0) begin
			last_bst <= 1'b0;
		end
		else if ((row == img_hgt - hwsz - 1) & (col == img_wdt - {bst_len, 1'b0})) begin // 2nd from last burst
			last_bst <= 1'b1;
		end
	end
end


//==========================================================================
// DDR Arbiter Interface
//==========================================================================
// 1word = 32bits = 4bytes = 2samples
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		addr_ofs <= 20'b0;
	end
	else begin
		if (state == 0) begin
			addr_ofs <= 0;
		end
		else if (state == 2) begin
			// invalid lines
			addr_ofs <= addr_ofs + {img_wdt, 1'b0};
		end
		else if (state == 4) begin
			// invalid burst
			addr_ofs <= addr_ofs + {bst_len[5:0], 1'b0};
		end
		else if ((state == 10) & (st_cnt == 0)) begin
			// next burst
			addr_ofs <= addr_ofs + {bst_len[5:0], 1'b0};
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dwr_req <= 1'b0;
	end
	else begin
		if (state == 8) begin
			dwr_req <= 1'b1;
		end
		else if (state == 11) begin
			dwr_req <= 1'b0;
		end
	end
end

assign dwr_cmd[31:0] = {22'h00_0000, last_bst_o, 5'b0, bst_len_m1[4:1]};

assign dwr_addr[31:20] = (~bank_r) ? addr_a[11:0] : addr_b[11:0];
assign dwr_addr[19: 0] = addr_ofs[19:0];

assign dwr_vout = dwr_req & dwr_ack;
assign dwr_dout[31:0] = (
	(~dwr_vout)                     ? 32'b0 :
	(state == 9)                    ? dwr_cmd[31:0] :
	((state == 10) & (st_cnt == 0)) ? dwr_addr[31:0] :
	((state == 10) | (state == 11)) ? {fifo_dout[15:0], fifo_dout[32:17]} : // little
	                                  32'b0
);
//assign dwr_strb[3:0] = (dwr_vout & bank) ? 4'h0 : 4'hF;
assign dwr_strb[3:0] = 4'hF;


endmodule
