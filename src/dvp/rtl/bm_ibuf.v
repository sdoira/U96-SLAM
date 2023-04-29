//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module bm_ibuf (
	// Global Control
	rst_n, clk,
	
	// Parameter
	hgt,
	wdt,
	ndisp,
	wsz,
	lr_addr_a,
	lr_addr_b,
	sad_addr_a,
	sad_addr_b,
	bst_len_m1,
	bst_len,
	sad_hgt,
	sad_wdt,
	
	// Control
	enb,
	start,
	lr_rdy,
	sad_rdy,
	lr_done,
	sad_done,
	buf_info,
	bank,
	
	// DDR Arbiter I/F
	drd_req,
	drd_ack,
	drd_vin,
	drd_din,
	drd_vout,
	drd_dout,
	
	// IBUF I/F
	lr_rdaddr,
	lr_dout,
	sad_rdaddr,
	sad_dout
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Parameter
input	[8:0]	hgt;
input	[9:0]	wdt;
input	[8:0]	ndisp;
input	[4:0]	wsz;
input	[11:0]	lr_addr_a, lr_addr_b;
input	[9:0]	sad_addr_a, sad_addr_b;
input	[7:0]	bst_len_m1;
input	[8:0]	bst_len;
input	[8:0]	sad_hgt;
input	[9:0]	sad_wdt;

// Control
input			enb, start;
output			lr_rdy, sad_rdy;
input			lr_done, sad_done;
output	[15:0]	buf_info;
output			bank;

// DDR Arbiter I/F
output			drd_req;
input			drd_ack;
input			drd_vin;
input	[31:0]	drd_din;
output			drd_vout;
output	[31:0]	drd_dout;

// IBUF I/F
input	[9:0]	lr_rdaddr;
output	[15:0]	lr_dout;
input	[9:0]	sad_rdaddr;
output	[63:0]	sad_dout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Disparity Search Phase
reg		[8:0]	disp_rem;
reg		[5:0]	pdisp;
wire			last_dphase;
reg		[3:0]	dphase;

// Line Counter
reg		[2:0]	op_phase;
reg		[1:0]	op_type;
reg		[8:0]	add_line, sub_line, sad_line;
wire			first_line;

// DDR Arbiter Interface
reg		[2:0]	state;
reg		[7:0]	cnt;
wire			bst_end, line_end;
reg		[1:0]	line_end_r;
wire			add_last_line, sad_last_line;
wire			frm_end;
reg				bank;
reg		[31:0]	drd_addr;
reg		[10:0]	rem;
reg		[7:0]	payload_len;
wire			dmask;
reg				drd_req;
wire			last_bit;
wire			drd_vout;
wire	[31:0]	drd_dout;

// FIFO Write Interface
wire			lr_wr, sad_wr;

// Buffer Instance
wire			lr_line_end, sad_line_end;
wire			lr_wr_rdy, sad_wr_rdy;
wire	[15:0]	lr_dout;
wire			lr_rdy;
wire	[63:0]	sad_dout;
wire			sad_rdy;
wire			buf_rdy;

// Buffer Information FIFO
wire			fifo_wr;
wire	[15:0]	fifo_wrdata;
wire			fifo_rd;
wire	[15:0]	buf_info;


//==========================================================================
// Disparity Search Phase
//--------------------------------------------------------------------------
// Search 32 differenct disparities at one time. If ndisp is greater than 
// 32 separate the search range into several phases.
//==========================================================================
// remaining disparity range (disp_rem) and actual number of disparities
//  that will be calculated this time (pdisp).
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		disp_rem <= 9'b0;
		pdisp    <= 6'b0;
	end
	else begin
		if (start) begin
			disp_rem <= ndisp - 32;
			pdisp    <= 32;
		end
		else if (dphase_end) begin
			if (disp_rem < 32) begin
				disp_rem <= 0;
				pdisp    <= disp_rem;
			end
			else begin
				disp_rem <= disp_rem - 32;
				pdisp    <= 32;
			end
		end
	end
end

// last of the disparity search phases
assign last_dphase = (disp_rem == 0);

// disparity search phase counter
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dphase <= 4'b0;
	end
	else begin
		if (start) begin
			dphase <= 0;
		end
		else if (dphase_end) begin
			dphase <= dphase + 1'b1;
		end
	end
end


//==========================================================================
// Line Counter
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		op_phase <= 3'b0;
	end
	else begin
		if (start) begin
			op_phase <= 0;
		end
		else if (line_end) begin
			case (op_phase)
				0: if (add_line == (wsz - 2'd2)) op_phase <= (dphase == 0) ? 1 : 3;
				1:                               op_phase <= 2;
				2: if (add_last_line)            op_phase <= 0;
				3:                               op_phase <= 4;
				4:                               op_phase <= 5;
				5: if (sad_last_line)            op_phase <= 0;
			endcase
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		op_type <= 2'b0;
	end
	else begin
		if (start) begin
			op_type <= 0;
		end
		else if (line_end_r[0]) begin
			case (op_phase)
				0: begin
					op_type <= 0;
				end
				1: op_type <= 2;
				2: begin
					case (op_type)
						1: op_type <= 2;
						2: op_type <= 1;
					endcase
				end
				3: op_type <= 0;
				4: op_type <= 3;
				5: begin
					case (op_type)
						1: op_type <= 0;
						0: op_type <= 3;
						3: op_type <= 1;
					endcase
				end
			endcase
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		add_line <= 9'b0;
		sub_line <= 9'b0;
		sad_line <= 9'b0;
	end
	else begin
		if (start | dphase_end) begin
			add_line <= 0;
			sub_line <= 0;
			sad_line <= 0;
		end
		else if (line_end) begin
			case (op_phase)
 				0: begin
 					add_line <= add_line + 1'b1;
 				end
 				1: begin
 					add_line <= add_line + 1'b1;
 				end
				2: if (op_type == 2) begin
					add_line <= add_line + 1'b1;
					sub_line <= sub_line + 1'b1;
				end
				4: begin
					add_line <= add_line + 1'b1;
					sad_line <= sad_line + 1'b1;
				end
				5: if (op_type == 3) begin
					add_line <= add_line + 1'b1;
					sub_line <= sub_line + 1'b1;
					sad_line <= sad_line + 1'b1;
				end
			endcase
		end
	end
end

assign first_line = (add_line == 0);


//==========================================================================
// DDR Arbiter Interface
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		state <= 3'b0;
	end
	else begin
		if (~enb) begin
			state <= 0;
		end
		else begin
			case (state)
				0: if (start)   state <= 1;
				1: if (buf_rdy) state <= 2;
				2: if (drd_ack) state <= 3;
				3:              state <= 4;
				4: begin
					//if (frm_end)      state <= 7;
					if (frm_end)      state <= 0;
					else if (bst_end) state <= 5;
				end
				5:              state <= 6;
				6:              state <= 1;
			endcase
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		cnt <= 8'b0;
	end
	else begin
		if (bst_end) begin
			cnt <= 0;
		end
		else if ((state == 4) & drd_vin) begin
			cnt <= cnt + 1'b1;
		end
	end
end

assign bst_end = (state == 4) & (cnt == bst_len_m1);
assign line_end = bst_end & (rem == 0);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		line_end_r <= 2'b0;
	end
	else begin
		line_end_r <= {line_end_r[0], line_end};
	end
end

assign add_last_line = line_end & ((op_type == 0) | (op_type == 2)) & (add_line[8:0] == hgt[8:0] - 1'b1);
assign sad_last_line = line_end & (op_type == 3) & (sad_line[8:0] == sad_hgt[8:0] - 1'b1);
assign dphase_end = (dphase[2:0] == 0) ? add_last_line : sad_last_line;
assign frm_end = dphase_end & last_dphase;

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
		drd_addr <= 32'b0;
	end
	else begin
		if (start) begin
			if (~bank) begin
				drd_addr <= {lr_addr_a[11:0], add_line[8:0], 11'b0};
			end
			else begin
				drd_addr <= {lr_addr_b[11:0], add_line[8:0], 11'b0};
			end
		end
		else if (line_end_r[1]) begin
			case (op_type)
				0: begin
					if (~bank) begin
						drd_addr <= {lr_addr_a[11:0], add_line[8:0], 11'b0};
					end
					else begin
						drd_addr <= {lr_addr_b[11:0], add_line[8:0], 11'b0};
					end
				end
				1: begin
					if (~bank) begin
						drd_addr <= {lr_addr_a[11:0], sub_line[8:0], 11'b0};
					end
					else begin
						drd_addr <= {lr_addr_b[11:0], sub_line[8:0], 11'b0};
					end
				end
				2: begin // same as 0
					if (~bank) begin
						drd_addr <= {lr_addr_a[11:0], add_line[8:0], 11'b0};
					end
					else begin
						drd_addr <= {lr_addr_b[11:0], add_line[8:0], 11'b0};
					end
				end
				3: begin
					if (~bank) begin
						drd_addr <= {sad_addr_a[9:0], sad_line[8:0], 13'b0};
					end
					else begin
						drd_addr <= {sad_addr_b[9:0], sad_line[8:0], 13'b0};
					end
				end
			endcase
		end
		else if (bst_end) begin
			if (op_type != 3) begin
				drd_addr <= {drd_addr[31:11], drd_addr[10:0] + {bst_len[8:0], 2'b0}};
			end
			else begin
				drd_addr <= {drd_addr[31:13], drd_addr[12:0] + {bst_len[8:0], 2'b0}};
			end
		end
	end
end

// number of data remained of the current line 
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		rem <= 11'b0;
	end
	else begin
		if (start | line_end_r[1]) begin
			if (op_type != 3) begin
				rem <= wdt[9:1] - bst_len; // LR data line
			end
			else begin
				rem <= {sad_wdt[9:0], 1'b0} - bst_len; // SAD data line
			end
		end
		else if (bst_end) begin
			if (rem < bst_len) begin
				rem <= 0;
			end
			else begin
				rem <= rem - bst_len;
			end
		end
	end
end

// actual number of data conveyed by this burst cycle.
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		payload_len <= 8'b0;
	end
	else begin
		if (start | line_end) begin
			payload_len <= bst_len;
		end
		else if (bst_end) begin
			if (rem < bst_len) begin
				payload_len <= rem;
			end
			else begin
				payload_len <= bst_len;
			end
		end
	end
end

assign dmask = (state == 4) & (cnt >= payload_len);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		drd_req <= 1'b0;
	end
	else begin
		if (bst_end) begin
			drd_req <= 1'b0;
		end
		else if ((state == 1) & (buf_rdy)) begin
			drd_req <= 1'b1;
		end
	end
end

assign last_bit = (last_dphase & frm_end);

assign drd_vout = ((state == 2) & (drd_ack)) | (state == 3);
assign drd_dout[31:0] = (
	(drd_vout & (state == 2)) ? {22'h00_0000, last_bit, 1'b1, bst_len_m1} :
	(drd_vout & (state == 3)) ? drd_addr[31:0] :
	                            32'b0
);


//==========================================================================
// FIFO Write Interface
//==========================================================================
assign lr_wr  = (op_type != 3) & drd_vin & ~dmask;
assign sad_wr = (op_type == 3) & drd_vin & ~dmask;


//==========================================================================
// Buffer Instance
//==========================================================================
assign lr_line_end  = (op_type != 3) ? line_end : 1'b0;
assign sad_line_end = (op_type == 3) ? line_end : 1'b0;

bm_ibuf_lr bm_ibuf_lr (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Write I/F
	.wr     (lr_wr),
	.din    (drd_din[31:0]),
	.wr_rdy (lr_wr_rdy),
	
	// Read I/F
	.lr_rdaddr (lr_rdaddr[9:0]),
	.lr_dout   (lr_dout[15:0]),
	.rd_rdy    (lr_rdy),
	
	// Control
	.enb       (enb),
	.start     (start),
	.line_end  (lr_line_end),
	.next_line (lr_done),
	
	// Parameter
	.line_size (wdt[9:1]),
	.bst_len   (bst_len[8:0])
);

bm_ibuf_sad bm_ibuf_sad (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Write I/F
	.wr     (sad_wr),
	.din    (drd_din[31:0]),
	.wr_rdy (sad_wr_rdy),
	
	// Read I/F
	.sad_rdaddr (sad_rdaddr[9:0]),
	.sad_dout   (sad_dout[63:0]),
	.rd_rdy     (sad_rdy),
	
	// Control
	.enb       (enb),
	.start     (start),
	.line_end  (sad_line_end),
	.next_line (sad_done),
	
	// Parameter
	.line_size (sad_wdt[9:0]),
	.bst_len   (bst_len[8:0])
);

assign buf_rdy = (op_type == 3) ? sad_wr_rdy : lr_wr_rdy;


//==========================================================================
// Buffer Information FIFO
//--------------------------------------------------------------------------
// Information associated with the current output line data.
//==========================================================================
assign fifo_wr = line_end;
assign fifo_wrdata[15:0] = {bank, last_dphase, first_line, last_bit, dphase[3:0], pdisp[5:0], op_type[1:0]};
assign fifo_rd = lr_done | sad_done;

simple_fifo #(16, 2) fifo_16_4 (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// FIFO I/F
	.wr     (fifo_wr),
	.wrdata (fifo_wrdata[15:0]),
	.rd     (fifo_rd),
	.rddata (buf_info[15:0]),
	
	// Status
	.empty (),
	.full  (),
	.data_count ()
);


endmodule
