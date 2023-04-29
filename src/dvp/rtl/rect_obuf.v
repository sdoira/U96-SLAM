//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module rect_obuf (
	// Global Control
	rst_n, clk,
	
	// Control
	enb,
	frm_end,
	
	// Status
	dff_ovf,
	cff_ovf,
	
	// Parameter
	base_a,
	base_b,
	
	// Front I/F
	vin,
	last,
	lr,
	ydst,
	xdst,
	len,
	intp,
	
	// DDR I/F
	ddr_req,
	ddr_ack,
	ddr_dout,
	ddr_strb,
	ddr_vout
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Control
input			enb;
output			frm_end;

// Status
output			dff_ovf, cff_ovf;

// Parameter
input	[11:0]	base_a, base_b;

// Front I/F
input			vin;
input			last;
input			lr;
input	[8:0]	ydst;
input	[9:0]	xdst;
input	[6:0]	len;
input	[7:0]	intp;

// DDR I/F
output			ddr_req;
input			ddr_ack;
output	[31:0]	ddr_dout;
output	[3:0]	ddr_strb;
output			ddr_vout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Data FIFO Instance & Write Interface
reg		[6:0]	vcnt;
wire			last_byte;
wire	[1:0]	dphase;
wire			dff_wr;
reg		[5:0]	wlen;
wire	[31:0]	next_dbuf;
wire	[3:0]	next_byten;
reg		[31:0]	dbuf;
reg		[3:0]	byten;
wire	[35:0]	dff_din;
wire	[35:0]	dff_dout;
wire			data_full;
reg				dff_ovf;

// Command FIFO Instance & Write Interface
wire			cmd_wr;
wire	[24:0]	cmd_wrdata, cmd_rddata;
wire			cmd_empty;
wire			cmd_full;
reg				cff_ovf;

// FIFO Read & DDR Arbiter Interface
reg		[2:0]	state;
reg		[6:0]	st_cnt;
wire			rd_end, bst_end;
wire			cmd_rd;
reg				rd_last;
reg				rd_lr;
reg		[8:0]	rd_ydst;
reg		[7:0]	rd_xdst_9_2;
reg		[5:0]	rd_wlen;
wire	[7:0]	bst_len_m1;
wire			frm_end;
reg				frame_cnt;
reg				ddr_req;
wire	[31:0]	ddr_cmd;
wire	[11:0]	base_sel;
wire	[31:0]	ddr_addr;
wire			dff_rd;
reg				dff_rd_r;
wire	[31:0]	ddr_dout;
wire			ddr_vout;


//==========================================================================
// Data FIFO Instance & Write Interface
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vcnt <= 7'b0;
	end
	else begin
		if (vin) begin
			if (last_byte) begin
				vcnt <= 0;
			end
			else begin
				vcnt <= vcnt + 1'b1;
			end
		end
	end
end

assign last_byte = vin & ((vcnt + 1'b1) == len);

assign dphase[1:0] = xdst[1:0] + vcnt[1:0];

assign dff_wr = (vin & (dphase == 2'b11)) | last_byte;

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		wlen <= 6'b0;
	end
	else begin
		if (dff_wr) begin
			if (last_byte) begin
				wlen <= 0;
			end
			else begin
				wlen <= wlen + 1'b1;
			end
		end
	end
end

// big endian -> little
assign next_dbuf[31:0] = (
	(dphase == 0) ? {24'b0,       intp[7:0]            } :
	(dphase == 1) ? {dbuf[31:16], intp[7:0], dbuf[ 7:0]} :
	(dphase == 2) ? {dbuf[31:24], intp[7:0], dbuf[15:0]} :
	                {             intp[7:0], dbuf[23:0]}
);

assign next_byten[3:0] = (
	(dphase == 0) ? 4'b0001                         :
	(dphase == 1) ? {byten[3:2], 1'b1, byten[  0] } :
	(dphase == 2) ? {byten[3  ], 1'b1, byten[1:0] } :
	                {            1'b1, byten[2:0] }
);
/*
assign next_dbuf[31:0] = (
	(dphase == 0) ? {             intp[7:0],      24'b0} :
	(dphase == 1) ? {dbuf[31:24], intp[7:0], dbuf[15:0]} :
	(dphase == 2) ? {dbuf[31:16], intp[7:0], dbuf[ 7:0]} :
	                {dbuf[31: 8], intp[7:0]            }
);

assign next_byten[3:0] = (
	(dphase == 0) ? 4'b1000                         :
	(dphase == 1) ? {byten[3  ], 1'b1, byten[1:0] } :
	(dphase == 2) ? {byten[3:2], 1'b1, byten[  0] } :
	                {byten[3:1], 1'b1             }
);
*/

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dbuf  <= 32'b0;
		byten <=  4'b0;
	end
	else begin
		if (last_byte) begin
			dbuf  <= 0;
			byten <= 0;
		end
		else if (vin) begin
			dbuf <= next_dbuf;
			byten <= next_byten;
		end
	end
end

assign dff_din[35:0] = {
	next_byten[3:0],
	next_dbuf[31:0]
};

wire	[9:0]	data_count;
//fifo_36_1024 data_fifo (
rect_obuf_dff data_fifo (
	.clk   (clk),
	.srst  (~rst_n),
	.din   (dff_din),
	.wr_en (dff_wr),
	.rd_en (dff_rd),
	.dout  (dff_dout),
	.full  (data_full),
	.empty (),
	.data_count (data_count)
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dff_ovf <= 1'b0;
	end
	else begin
		if (~enb) begin
			dff_ovf <= 1'b0;
		end
		else if (data_full & dff_wr) begin
			dff_ovf <= 1'b1;
		end
	end
end


//==========================================================================
// Command FIFO Instance & Write Interface
//==========================================================================
assign cmd_wr = last_byte;
assign cmd_wrdata[24:0] = {last, lr, ydst[8:0], xdst[9:2], wlen[5:0]};

wire	[6:0]	cmd_count;
//simple_fifo #(25, 4) cmd_fifo (
simple_fifo #(25, 6) cmd_fifo (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// FIFO I/F
	.wr     (cmd_wr),
	.wrdata (cmd_wrdata),
	.rd     (cmd_rd),
	.rddata (cmd_rddata),
	
	// Status
	.empty (cmd_empty),
	.full  (cmd_full),
	//.data_count (cmd_count[4:0])
	.data_count (cmd_count[6:0])
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		cff_ovf <= 1'b0;
	end
	else begin
		if (~enb) begin
			cff_ovf <= 1'b0;
		end
		else if (cmd_full & cmd_wr) begin
			cff_ovf <= 1'b1;
		end
	end
end


//==========================================================================
// FIFO Read & DDR Arbiter Interface
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		state <= 3'b0;
	end
	else begin
		if (enb) begin
			case (state)
				0: if (~cmd_empty)   state <= 1;
				1:                   state <= 2;
				2: if (ddr_ack)      state <= 3;
				3: begin
					if (rd_end)      state <= 5; // only 1 valid data
					else             state <= 4; // normal
				end
				4: begin
					if (bst_end)     state <= 6; // no mask data
					else if (rd_end) state <= 5; // normal
				end
				5: if (bst_end)      state <= 6; // masked data phase
				6:                   state <= 0;
			endcase
		end
		else begin
			state <= 0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		st_cnt <= 7'b0;
	end
	else begin
		if (dff_rd | (state == 5)) begin
			st_cnt <= st_cnt + 1'b1;
		end
		else begin
			st_cnt <= 0;
		end
	end
end

assign rd_end  = (st_cnt == rd_wlen);
assign bst_end = (st_cnt == bst_len_m1);

assign cmd_rd = (state == 1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		rd_last     <= 1'b0;
		rd_lr       <= 1'b0;
		rd_ydst     <= 9'b0;
		rd_xdst_9_2 <= 8'b0;
		rd_wlen     <= 6'b0;
	end
	else begin
		if (cmd_rd) begin
			rd_last     <= cmd_rddata[24];
			rd_lr       <= cmd_rddata[23];
			rd_ydst     <= cmd_rddata[22:14];
			rd_xdst_9_2 <= cmd_rddata[13: 6];
			rd_wlen     <= cmd_rddata[ 5: 0];
		end
	end
end

// maximum burst length is 127 bytes.
// truncate to 4-byte will yeild 31.75 -> 32 words.
// fraction of xdst will yield an additional word (33 words).
// maximum powers of 2 which is larger than data payload is 64.
// thus max of bst_len_m1 is 63.
assign bst_len_m1[7:0] = (
	(rd_wlen[5]) ? 8'd63 :
	(rd_wlen[4]) ? 8'd31 :
	(rd_wlen[3]) ? 8'd15 :
	(rd_wlen[2]) ? 8'd7  :
	               8'd3
);

assign frm_end = ((state == 6) & rd_last);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		frame_cnt <= 1'b0;
	end
	else begin
		if (~enb) begin
			frame_cnt <= 1'b0;
		end
		else if (frm_end) begin
			frame_cnt <= ~frame_cnt;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ddr_req <= 1'b0;
	end
	else begin
		if (state == 1) begin
			ddr_req <= 1'b1;
		end
		else if (state == 6) begin
			ddr_req <= 1'b0;
		end
	end
end

assign ddr_cmd[31:0] = {
	22'b0, // reserved
	rd_last,
	1'b0,  // rd_wrn
	bst_len_m1[7:0]
};

assign base_sel[11:0] = (~frame_cnt) ? base_a[11:0] : base_b[11:0];

reg		[18:0]	ddr_addr_x;
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ddr_addr_x <= 19'b0;
	end
	else begin
		ddr_addr_x <= rd_ydst[8:0] * 640 + {rd_xdst_9_2, 2'b0};
	end
end

assign ddr_addr[31:0] = {
	base_sel[11:0],
	rd_lr,
	ddr_addr_x[18:0]
};

assign dff_rd = ((state == 3) | (state == 4));

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dff_rd_r <= 1'b0;
	end
	else begin
		dff_rd_r <= dff_rd;
	end
end

assign ddr_dout[31:0] = (
	((state == 2) & ddr_ack) ? ddr_cmd[31:0]  :
	 (state == 3)            ? ddr_addr[31:0] :
	 (dff_rd_r)              ? dff_dout[31:0] :
	                           32'b0
);

assign ddr_strb[3:0] = (
	(dff_rd_r) ? dff_dout[35:32] :
	(ddr_ack)  ? 4'b0 :
	             4'hF // default enable
);

assign ddr_vout = ddr_req & ddr_ack;


endmodule
