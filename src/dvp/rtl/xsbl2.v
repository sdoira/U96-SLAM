//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module xsbl2 (
	// Global Control
	rst_n, clk,
	
	// Internal Bus I/F
	ibus_cs,
	ibus_wr,
	ibus_addr_7_2,
	ibus_wrdata,
	ibus_rddata,
	
	// Control
	rect_done,
	xsbl_done,
	rect_fcnt,
	xsbl_fcnt,
	
	// DDR Read Arbiter I/F
	drd_req,
	drd_ack,
	drd_vin,
	drd_din,
	drd_vout,
	drd_dout,
	
	// DDR Write Arbiter I/F
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

// Internal Bus I/F
input			ibus_cs;
input			ibus_wr;
input	[5:0]	ibus_addr_7_2;
input	[31:0]	ibus_wrdata;
output	[31:0]	ibus_rddata;

// Control
input			rect_done;
input			xsbl_done;
input	[3:0]	rect_fcnt;
output	[3:0]	xsbl_fcnt;

// DDR Read Arbiter I/F
output			drd_req;
input			drd_ack;
input			drd_vin;
input	[31:0]	drd_din;
output			drd_vout;
output	[31:0]	drd_dout;

// DDR Write Arbiter I/F
output			dwr_req;
input			dwr_ack;
output			dwr_vout;
output	[31:0]	dwr_dout;
output	[3:0]	dwr_strb;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Register Interface
reg				ctrl;
reg		[11:0]	drd_base_a, drd_base_b;
reg		[11:0]	dwr_base_a, dwr_base_b;
reg		[7:0]	bst_len_m1;
reg		[9:0]	hgt;
reg		[10:0]	wdt;
wire	[9:0]	hgt_m1;
wire			enb;
wire	[31:0]	ibus_rddata;

// Sequence Control
reg				xsbl_sim_start;
wire			xsbl_start;
reg				xsbl_pend, xsbl_on;
reg		[3:0]	xsbl_fcnt;

// DDR Read Control
reg		[4:0]	state;
reg		[8:0]	rem;
wire			mask;
reg				mask_r;
reg		[7:0]	vcnt;
wire			bst_end;
reg		[1:0]	bst_end_r;
reg		[8:0]	wcnt, wcnt_r;
wire			line_end;
reg				lr;
reg		[8:0]	hcnt;
reg		[8:0]	hcnt_r;
reg				henb;
reg		[3:0]	henb_r;
wire			frm_end;
reg				last_bst;
reg				bank;

// DDR Read Request
reg				drd_req;
reg				drd_ack_r;
wire			drd_ack_pos;
reg				drd_ack_pos_r, drd_ack_pos_r2;
wire	[31:0]	drd_cmd;
wire	[11:0]	drd_base_sel;
wire	[31:0]	drd_addr, drd_dout;
//wire			drd_vout;
reg		[31:0]	drd_cmd_r;
reg		[18:0]	drd_addr_x;
wire			drd_vout_x;
reg				drd_vout;

// Horizontal Difference Calculation
reg		[31:0]	drd_din_r1;
reg		[7:0]	drd_din_r2;
reg		[8:0]	dif[3:0];
reg		[8:0]	dif_r[3:0];

// Line Buffer Instance and Control
wire			v_mask;
wire			first, last;
reg		[3:0]	first_r, last_r;
wire			lbuf_enb;
reg		[3:0]	lbuf_enb_r;
wire			lbuf_rden, lbuf_wren;
reg		[7:0]	lbuf_rdaddr, lbuf_wraddr;
wire			line_phase;
wire	[1:0]	lbuf_wr;
wire	[35:0]	lbuf_din;
wire	[35:0]	lbuf_dout[1:0];

// X-Sobel Calculation
wire	[8:0]	line_1st[3:0];
wire	[8:0]	line_2nd[3:0];
reg		[10:0]	add_a[3:0];
reg		[11:0]	add_b[3:0];
reg		[5:0]	xsbl[3:0];

// Output Buffer Instace
wire	[31:0]	obuf_din;
wire			obuf_wr;
reg		[7:0]	obuf_wraddr;
reg				obuf_rd_on;
wire			obuf_rd;
reg		[1:0]	obuf_rd_r;
reg		[8:0]	obuf_rd_cnt;
reg		[1:0]	obuf_dphase;
wire	[8:0]	obuf_rdaddr;
wire	[31:0]	obuf_dout;
reg		[31:0]	obuf_dout_r1, obuf_dout_r2;

// DDR Write Request
reg				dwr_req;
reg				dwr_ack_r;
wire			dwr_ack_pos;
reg				dwr_ack_pos_r;
wire	[31:0]	dwr_cmd;
wire	[11:0]	dwr_base_sel;
wire	[31:0]	dwr_addr, obuf_dout_sel, dwr_dout;
wire	[3:0]	dwr_strb;
wire			dwr_vout;


//==========================================================================
// Functions
//==========================================================================
// limit (s11.0) -> (s5.0)
// offset binary
function [5:0] limit;
	input	[11:0]	in;
	begin
		if (in[11] & |in[10:5]) begin
			limit = 6'h3F; // upper limit
		end
		else if (~in[11] & ~&in[10:5]) begin
			limit = 6'h00; // lower limit;
		end
		else begin
			limit = {in[11], in[4:0]};
		end
	end
endfunction


//==========================================================================
// Register Interface
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ctrl       <= 1'b0;
		drd_base_a <= 12'b0;
		drd_base_b <= 12'b0;
		dwr_base_a <= 12'b0;
		dwr_base_b <= 12'b0;
		bst_len_m1 <= 8'd63;
		hgt        <= 10'd480;
		wdt        <= 11'd640;
	end
	else begin
		if (ibus_cs & ibus_wr) begin
			case (ibus_addr_7_2)
				6'h00: ctrl       <= ibus_wrdata[0];
				6'h02: drd_base_a <= ibus_wrdata[31:20];
				6'h03: drd_base_b <= ibus_wrdata[31:20];
				6'h04: dwr_base_a <= ibus_wrdata[31:20];
				6'h05: dwr_base_b <= ibus_wrdata[31:20];
				6'h06: bst_len_m1 <= ibus_wrdata[7:0];
				6'h07: begin
					hgt           <= ibus_wrdata[25:16];
					wdt           <= ibus_wrdata[10: 0];
				end
				default: begin
					ctrl       <= ctrl;
					hgt        <= hgt;
					wdt        <= wdt;
				end
			endcase
		end
	end
end

assign hgt_m1[9:0] = hgt[9:0] - 1'b1;
assign enb = ctrl;

assign ibus_rddata[31:0] = (
	(~ibus_cs)               ?  32'b0 :
	(ibus_addr_7_2 == 6'h00) ? {31'b0, ctrl} :
	(ibus_addr_7_2 == 6'h01) ? {32'b0} :
	(ibus_addr_7_2 == 6'h02) ? {drd_base_a[11:0], 20'b0} :
	(ibus_addr_7_2 == 6'h03) ? {drd_base_b[11:0], 20'b0} :
	(ibus_addr_7_2 == 6'h04) ? {dwr_base_a[11:0], 20'b0} :
	(ibus_addr_7_2 == 6'h05) ? {dwr_base_b[11:0], 20'b0} :
	(ibus_addr_7_2 == 6'h06) ? {24'b0, bst_len_m1[7:0]} :
	(ibus_addr_7_2 == 6'h07) ? {6'b0, hgt[9:0], 5'b0, wdt[10:0]} :
	                           32'b0
);

wire		sw_start;
assign sw_start = (ibus_cs & ibus_wr & (ibus_addr_7_2[5:0] == 6'h00) & ibus_wrdata[1]);


//==========================================================================
// Sequence Control
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xsbl_sim_start <= 1'b0;
	end
end

//assign xsbl_start = (enb & (rect_done | xsbl_pend | xsbl_sim_start) & ~xsbl_on);
assign xsbl_start = (enb & (rect_done | xsbl_pend | xsbl_sim_start | sw_start) & ~xsbl_on);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xsbl_pend <= 1'b0;
	end
	else begin
		if (enb) begin
			if (rect_done & xsbl_on) begin
				xsbl_pend <= 1'b1;
			end
			else if (xsbl_start) begin
				xsbl_pend <= 1'b0;
			end
		end
		else begin
			xsbl_pend <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xsbl_on <= 1'b0;
	end
	else begin
		if (enb) begin
			if (xsbl_start) begin
				xsbl_on <= 1'b1;
			end
			else if (xsbl_done) begin
				xsbl_on <= 1'b0;
			end
		end
		else begin
			xsbl_on <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xsbl_fcnt <= 4'b0;
	end
	else begin
		if (enb) begin
			if (xsbl_start) begin
				xsbl_fcnt <= rect_fcnt;
			end
		end
		else begin
			xsbl_fcnt <= 0;
		end
	end
end


//==========================================================================
// DDR Read Control
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		state <= 5'b0;
	end
	else begin
		if (enb) begin
			case (state)
				 0: if (xsbl_start)    state <=  1; // idle
				 1:                    state <=  2; // line init
				 2:                    state <=  3; // assert rdr_req
				 3: if (drd_ack)       state <=  4; // wait for rdr_ack
				 4:                    state <=  5; // ADDR phase
				 5: begin                           // DATA phase
				 	if (line_end) begin
				 		if (henb & lr) state <=  7; // data output
				 		else           state <=  1; // r line
				 	end
				 	else if (bst_end)  state <=  6; // next burst
				 end
				 6:                    state <=  2;
				 7:                    state <=  8; // dwr line initial
				 8:                    state <=  9; // assert dwr_req
				 9: if (dwr_ack)       state <= 10; // wait for dwr_ack
				10:                    state <= 11; // ADDR phase
				11: begin                           // DATA phase
					if      (frm_end)  state <= 18;
					else if (line_end) state <= 15; // next data input
					else if (bst_end)  state <= 12; // next burst
				end
				12:                    state <= 13;
				13:                    state <= 14;
				14:                    state <=  8; // next burst
				15:                    state <= 16;
				16:                    state <= 17;
				17:                    state <=  1; // next data input
				18:                    state <= 19;
				19:                    state <= 20;
				20:                    state <=  0; // end process
			endcase
		end
		else begin
			state <= 0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		rem <= 9'b0;
	end
	else begin
		if (state == 1) begin
			// LR separate when read
			rem <= {1'b0, wdt[9:2]};
		end
		else if (state == 7) begin
			// LR combined when write
			rem <= wdt[9:1];
		end
		else if (drd_vin | obuf_rd) begin
			if (rem == 0) begin
				rem <= rem;
			end
			else begin
				rem <= rem - 1'b1;
			end
		end
	end
end

assign mask = ((state == 5) | (state == 11)) & (rem == 0);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		mask_r <= 1'b0;
	end
	else begin
		mask_r <= mask;
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vcnt <= 8'b0;
	end
	else begin
		if (xsbl_start) begin
			vcnt <= 0;
		end
		else if (bst_end) begin
			vcnt <= 0;
		end
		else if (drd_vin | obuf_rd | mask) begin
			vcnt <= vcnt + 1'b1;
		end
	end
end

assign bst_end  = (vcnt == bst_len_m1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		bst_end_r <= 2'b0;
	end
	else begin
		bst_end_r <= {bst_end_r[0], bst_end};
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		wcnt <= 9'b0;
	end
	else begin
		if (xsbl_start | line_end) begin
			wcnt <= 0;
		end
		else if (drd_vin | obuf_rd) begin
			wcnt <= wcnt + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		wcnt_r <= 9'b0;
	end
	else begin
		wcnt_r <= wcnt;
	end
end

assign line_end = bst_end & ((rem == 0) | (rem == 1));

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		lr <= 1'b0;
	end
	else begin
		if (xsbl_start) begin
			lr <= 0;
		end
		else if ((state == 5) & line_end) begin
			lr <= ~lr;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		hcnt   <= 9'b0;
		hcnt_r <= 9'b0; // current line index - 1
	end
	else begin
		if (xsbl_start | frm_end) begin
			hcnt   <= 0;
			hcnt_r <= 0;
		end
		else if (
			(line_end & ~henb & lr & (state ==  5)) | 
			(line_end &  henb & (state == 11))
		) begin
			hcnt   <= hcnt + 1'b1;
			hcnt_r <= hcnt;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		henb <= 1'b0;
	end
	else begin
		if (xsbl_start | frm_end) begin
			henb <= 1'b0;
		end
		else if (line_end & lr & (hcnt == 1)) begin
			henb <= 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		henb_r <= 4'b0;
	end
	else begin
		henb_r <= {henb_r[2:0], henb};
	end
end

assign frm_end = (hcnt == hgt_m1) & line_end & (state == 11);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		last_bst <= 1'b0;
	end
	else begin
		if (~enb | bst_end) begin
			last_bst <= 1'b0;
		end
		else if ((hcnt == hgt_m1) & (state == 8) & (rem <= (bst_len_m1 + 1'b1))) begin
			last_bst <= 1'b1;
		end
	end
end

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
// DDR Read Request
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		drd_req <= 1'b0;
	end
	else begin
		if (state == 2) begin
			drd_req <= 1'b1;
		end
		else if (bst_end) begin
			drd_req <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		drd_ack_r <= 1'b0;
	end
	else begin
		drd_ack_r <= drd_ack;
	end
end

assign drd_ack_pos = (drd_ack & ~drd_ack_r);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		drd_ack_pos_r2 <= 1'b0;
		drd_ack_pos_r <= 1'b0;
	end
	else begin
		drd_ack_pos_r2 <= drd_ack_pos_r;
		drd_ack_pos_r <= drd_ack_pos;
	end
end

assign drd_cmd[31:0] = {
	22'h00_0000,
	1'b0, // last
	1'b1, // rd_wrn
	bst_len_m1[7:0]
};

assign drd_base_sel[11:0] = (~bank) ? drd_base_a[11:0] : drd_base_b[11:0];
/*
assign drd_addr[31:0] = {
	drd_base_sel[11:0],
	lr,
	hcnt[8:0],
	wcnt[7:0],
	2'b0
};

assign drd_dout[31:0] = (
	(drd_ack_pos)   ? drd_cmd[31:0]  :
	(drd_ack_pos_r) ? drd_addr[31:0] :
	                  32'b0
);

assign drd_vout = (drd_ack_pos | drd_ack_pos_r);
*/

////////////////////////////////////////////////
// changed for continuous memory

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		drd_cmd_r <= 32'b0;
	end
	else begin
		drd_cmd_r <= drd_cmd;
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		drd_addr_x <= 19'b0;
	end
	else begin
		drd_addr_x <= hcnt[8:0] * 640 + {wcnt[7:0], 2'b0};
	end
end

assign drd_addr[31:0] = {
	drd_base_sel[11:0],
	lr,
	drd_addr_x[18:0]
};

assign drd_dout[31:0] = (
	(drd_ack_pos_r)  ? drd_cmd_r[31:0]  :
	(drd_ack_pos_r2) ? drd_addr[31:0] :
	                   32'b0
);

assign drd_vout_x = (drd_ack_pos | drd_ack_pos_r);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		drd_vout <= 1'b0;
	end
	else begin
		drd_vout <= drd_vout_x;
	end
end


//==========================================================================
// Horizontal Difference Calculation
//==========================================================================
// big endian -> little
wire		[31:0]		drd_din_rvs;
assign drd_din_rvs[31:0] = {drd_din[7:0], drd_din[15:8], drd_din[23:16], drd_din[31:24]};
//assign drd_din_rvs[31:0] = drd_din[31:0];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		drd_din_r1 <= 32'b0;
		drd_din_r2 <=  8'b0;
	end
	else begin
		if (drd_vin) begin
			//drd_din_r1 <= drd_din;
			drd_din_r1 <= drd_din_rvs;
			drd_din_r2 <= drd_din_r1[7:0]; // we need upper byte only
		end
	end
end

// (u8.0) - (u8.0) = (s8.0)
// offset binary
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dif[3] <= 9'b0;
		dif[2] <= 9'b0;
		dif[1] <= 9'b0;
		dif[0] <= 9'b0;
	end
	else begin
		dif[3] <= {1'b1, drd_din_r1[23:16]} - drd_din_r2[ 7: 0];
		dif[2] <= {1'b1, drd_din_r1[15: 8]} - drd_din_r1[31:24];
		dif[1] <= {1'b1, drd_din_r1[ 7: 0]} - drd_din_r1[23:16];
		//dif[0] <= {1'b1,    drd_din[31:24]} - drd_din_r1[15: 8];
		dif[0] <= {1'b1, drd_din_rvs[31:24]} - drd_din_r1[15: 8];
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dif_r[3] <= 9'b0;
		dif_r[2] <= 9'b0;
		dif_r[1] <= 9'b0;
		dif_r[0] <= 9'b0;
	end
	else begin
		dif_r[3] <= dif[3];
		dif_r[2] <= dif[2];
		dif_r[1] <= dif[1];
		dif_r[0] <= dif[0];
	end
end


//==========================================================================
// Line Buffer Instance and Control
//==========================================================================
// mask out invalid data duration
assign v_mask = drd_vin & ~mask;

// indicates first/last word contained in drd_din.
assign first = drd_vin & (wcnt == 0);
assign last  = drd_vin & (wcnt == wdt[9:2] - 1'b1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		first_r <= 4'b0;
		last_r  <= 4'b0;
	end
	else begin
		first_r <= {first_r[2:0], first};
		last_r  <= {last_r[2:0], last};
	end
end

// shift time window backward by 1
assign lbuf_enb = ~first & v_mask | last_r[0];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		lbuf_enb_r <= 4'b0;
	end
	else begin
		lbuf_enb_r <= {lbuf_enb_r[2:0], lbuf_enb};
	end
end

assign lbuf_rden = lbuf_enb;
assign lbuf_wren = lbuf_enb_r[1];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		lbuf_rdaddr <= 8'b0;
	end
	else begin
		if (xsbl_start | line_end) begin
			lbuf_rdaddr <= 0;
		end
		else if (lbuf_rden) begin
			lbuf_rdaddr <= lbuf_rdaddr + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		lbuf_wraddr <= 8'b0;
	end
	else begin
		if (xsbl_start | line_end) begin
			lbuf_wraddr <= 0;
		end
		else if (lbuf_wren) begin
			lbuf_wraddr <= lbuf_wraddr + 1'b1;
		end
	end
end

assign line_phase = hcnt[0];

assign lbuf_wr[0] = ~line_phase & lbuf_wren;
assign lbuf_wr[1] =  line_phase & lbuf_wren;

assign lbuf_din[35:0] = {dif_r[3], dif_r[2], dif_r[1], dif_r[0]};

dpram_36_512 ram_0 (
  .clka  (clk),
  .ena   (enb),
  .wea   (lbuf_wr[0]),
  .addra ({lr, lbuf_wraddr}),
  .dina  (lbuf_din),
  .clkb  (clk),
  .enb   (enb),
  .addrb ({lr, lbuf_rdaddr}),
  .doutb (lbuf_dout[0])
);

dpram_36_512 ram_1 (
  .clka  (clk),
  .ena   (enb),
  .wea   (lbuf_wr[1]),
  .addra ({lr, lbuf_wraddr}),
  .dina  (lbuf_din),
  .clkb  (clk),
  .enb   (enb),
  .addrb ({lr, lbuf_rdaddr}),
  .doutb (lbuf_dout[1])
);


//==========================================================================
// X-Sobel Calculation
//==========================================================================
assign line_1st[3][8:0] = (~line_phase) ? lbuf_dout[0][35:27] : lbuf_dout[1][35:27];
assign line_1st[2][8:0] = (~line_phase) ? lbuf_dout[0][26:18] : lbuf_dout[1][26:18];
assign line_1st[1][8:0] = (~line_phase) ? lbuf_dout[0][17: 9] : lbuf_dout[1][17: 9];
assign line_1st[0][8:0] = (~line_phase) ? lbuf_dout[0][ 8: 0] : lbuf_dout[1][ 8: 0];

assign line_2nd[3][8:0] = (~line_phase) ? lbuf_dout[1][35:27] : lbuf_dout[0][35:27];
assign line_2nd[2][8:0] = (~line_phase) ? lbuf_dout[1][26:18] : lbuf_dout[0][26:18];
assign line_2nd[1][8:0] = (~line_phase) ? lbuf_dout[1][17: 9] : lbuf_dout[0][17: 9];
assign line_2nd[0][8:0] = (~line_phase) ? lbuf_dout[1][ 8: 0] : lbuf_dout[0][ 8: 0];

// (s8.0) + (s9.0) = (s10.0)
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		add_a[3] <= 11'b0;
		add_a[2] <= 11'b0;
		add_a[1] <= 11'b0;
		add_a[0] <= 11'b0;
	end
	else begin
		// bit ext 1bit + doubled
		add_a[3] <= {line_1st[3][8], ~line_1st[3][8], line_1st[3][7:0]} + {line_2nd[3], 1'b0};
		add_a[2] <= {line_1st[2][8], ~line_1st[2][8], line_1st[2][7:0]} + {line_2nd[2], 1'b0};
		add_a[1] <= {line_1st[1][8], ~line_1st[1][8], line_1st[1][7:0]} + {line_2nd[1], 1'b0};
		add_a[0] <= {line_1st[0][8], ~line_1st[0][8], line_1st[0][7:0]} + {line_2nd[0], 1'b0};
	end
end

// (s10.0) + (s8.0) = (s11.0)
// doubles the center
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		add_b[3] <= 12'b0;
		add_b[2] <= 12'b0;
		add_b[1] <= 12'b0;
		add_b[0] <= 12'b0;
	end
	else begin
		add_b[3] <= add_a[3] + {dif_r[3][8], {2{~dif_r[3][8]}}, dif_r[3][7:0]}; // bit ext 2bits
		add_b[2] <= add_a[2] + {dif_r[2][8], {2{~dif_r[2][8]}}, dif_r[2][7:0]};
		add_b[1] <= add_a[1] + {dif_r[1][8], {2{~dif_r[1][8]}}, dif_r[1][7:0]};
		add_b[0] <= add_a[0] + {dif_r[0][8], {2{~dif_r[0][8]}}, dif_r[0][7:0]};
	end
end

// upper limit
// (s11.0) -> (s7.0)
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		xsbl[3] <= 6'b0;
		xsbl[2] <= 6'b0;
		xsbl[1] <= 6'b0;
		xsbl[0] <= 6'b0;
	end
	else begin
		xsbl[3] <= (lbuf_enb_r[2] & ~first_r[3]) ? limit(add_b[3]) : 6'h20;
		xsbl[2] <= (lbuf_enb_r[2]              ) ? limit(add_b[2]) : 6'h20;
		xsbl[1] <= (lbuf_enb_r[2]              ) ? limit(add_b[1]) : 6'h20;
		xsbl[0] <= (lbuf_enb_r[2] & ~last_r[3] ) ? limit(add_b[0]) : 6'h20;
	end
end


//==========================================================================
// Output Buffer Instace
//==========================================================================
assign obuf_din[31:0] = {2'b0, xsbl[3], 2'b0, xsbl[2], 2'b0, xsbl[1], 2'b0, xsbl[0]};
assign obuf_wr = henb_r[3] & lbuf_enb_r[3];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		obuf_wraddr <= 8'b0;
	end
	else begin
		if (xsbl_start | line_end) begin
			obuf_wraddr <= 0;
		end
		else if (obuf_wr) begin
			obuf_wraddr <= obuf_wraddr + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		obuf_rd_on <= 1'b0;
	end
	else begin
		if (xsbl_start | line_end) begin
			obuf_rd_on <= 1'b0;
		end
		else if (state == 8) begin
			obuf_rd_on <= 1'b1;
		end
		else if (bst_end) begin
			obuf_rd_on <= 1'b0;
		end
	end
end

assign obuf_rd = obuf_rd_on & dwr_ack;

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		obuf_rd_r <= 2'b0;
	end
	else begin
		obuf_rd_r <= {obuf_rd_r[0], obuf_rd};
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		obuf_rd_cnt <= 9'b0;
	end
	else begin
		if (xsbl_start | line_end) begin
			obuf_rd_cnt <= 0;
		end
		else if (obuf_rd) begin
			obuf_rd_cnt <= obuf_rd_cnt + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		obuf_dphase <= 2'b0;
	end
	else begin
		obuf_dphase <= {obuf_dphase[0], obuf_rd_cnt[0]};
	end
end

assign obuf_rdaddr[8:0] = {obuf_rd_cnt[0], obuf_rd_cnt[8:1]};

dpram_32_512 obuf (
	.clka  (clk),
	.ena   (1'b1),
	.wea   (obuf_wr),
	.addra ({lr, obuf_wraddr[7:0]}),
	.dina  (obuf_din[31:0]),
	.clkb  (clk),
	.enb   (1'b1),
	.addrb (obuf_rdaddr[8:0]),
	.doutb (obuf_dout[31:0])
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		obuf_dout_r2 <= 32'b0;
		obuf_dout_r1 <= 32'b0;
	end
	else begin
		obuf_dout_r2 <= obuf_dout_r1;
		obuf_dout_r1 <= obuf_dout;
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
		if (state == 8) begin
			dwr_req <= 1'b1;
		end
		else if (bst_end_r[1]) begin
			dwr_req <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dwr_ack_r <= 1'b0;
	end
	else begin
		dwr_ack_r <= dwr_ack;
	end
end

assign dwr_ack_pos = (dwr_ack & ~dwr_ack_r);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dwr_ack_pos_r <= 1'b0;
	end
	else begin
		dwr_ack_pos_r <= dwr_ack_pos;
	end
end

assign dwr_cmd[31:0] = {
	22'h00_0000,
	last_bst, // last
	1'b0, // rd_wrn
	bst_len_m1[7:0]
};

assign dwr_base_sel[11:0] = (~bank) ? dwr_base_a[11:0] : dwr_base_b[11:0];

assign dwr_addr[31:0] = {
	dwr_base_sel[11:0],
	hcnt_r[8:0], // hcnt - 1
	wcnt_r[8:0],
	2'b0
};

assign obuf_dout_sel[31:0] = (
	(~obuf_dphase[1]) ? {
		obuf_dout_r1[31:24],
		obuf_dout[31:24],
		obuf_dout_r1[23:16],
		obuf_dout[23:16]
	} :  {
		obuf_dout_r2[15:8],
		obuf_dout_r1[15:8],
		obuf_dout_r2[ 7:0],
		obuf_dout_r1[ 7:0]
	}
);

assign dwr_dout[31:0] = (
	(dwr_ack_pos)   ? dwr_cmd   :
	(dwr_ack_pos_r) ? dwr_addr  :
	(mask_r)        ? 32'b0     :
	(obuf_rd_r[1])  ? obuf_dout_sel :
	                  32'b0
);

assign dwr_strb[3:0] = 4'hF;

assign dwr_vout = dwr_req & dwr_ack;


endmodule
