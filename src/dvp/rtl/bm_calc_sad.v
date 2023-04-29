//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module bm_calc_sad (
	// Global Control
	rst_n, clk,
	
	// Parameter
	wdt,
	ndisp,
	hwsz,
	hsad_wdt,
	
	// Buffer Status
	first_line,
	dphase,
	op_type,
	
	// Control
	enb,
	lr_rdy,
	sad_rdy,
	lr_done,
	sad_line_end,
	
	// IBUF I/F
	lr_rdaddr,
	lr_din,
	
	// Output
	sad_out,
	vout
);


//==========================================================================
// Parameter
//==========================================================================
parameter		PARALLEL = 32;


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Parameter
input	[9:0]	wdt;
input	[8:0]	ndisp;
input	[3:0]	hwsz;
input	[9:0]	hsad_wdt;

// Buffer Status
input			first_line;
input	[3:0]	dphase;
input	[1:0]	op_type;

// Control
input			enb;
input			lr_rdy, sad_rdy;
output			lr_done;
input			sad_line_end;

// IBUF I/F
output	[9:0]	lr_rdaddr;
input	[15:0]	lr_din;

// Output
output	[543:0]	sad_out;
output			vout;


//==========================================================================
// Function
//==========================================================================
// dif6 = in1 - in2
// (s5.0) - (s5.0) = (s6.0), offset binary, no largest negative value
function [6:0] dif6;
	input	[5:0]	in1, in2;
	begin
		dif6[6:0] = {1'b1, in1[5:0]} - in2[5:0];
	end
endfunction

// abs7 = |in|
// (s6.0) -> (u6.0), offset binary, no largest negative value
function [5:0] abs7;
	input	[6:0]	in;
	begin
		if (in[6]) begin
			abs7[5:0] = in[5:0];
		end
		else begin
			abs7[5:0] = 7'h40 - in[5:0];
		end
	end
endfunction

function [9:0]	upper_lim10;
	input	[10:0]	in;
	begin
		if (in[10]) begin
			upper_lim10 = 10'h3FF;
		end
		else begin
			upper_lim10 = in[9:0];
		end
	end
endfunction

function [9:0]	lower_lim10;
	input	[10:0]	in;
	begin
		if (in[10]) begin
			lower_lim10 = 10'h000;
		end
		else begin
			lower_lim10 = in[9:0];
		end
	end
endfunction

function [15:0] limit16;
	input	[17:0]	in;
	begin
		if (~in[17]) begin
			if (|in[16:15]) begin
				limit16 = 16'hFFFF; // upper limit
			end
			else begin
				limit16 = in[15:0];
			end
		end
		else begin
			limit16 = 16'h0000; // lower limit
		end
	end
endfunction

function [10:0]	inv11;
	input	[10:0]	in;
	begin
		inv11 = ~in[10:0] + 1'b1;
	end
endfunction


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
integer			i;
genvar			j;

// Buffer Status
wire			last_dphase, first_line;
wire	[3:0]	dphase;
wire	[1:0]	op_type;

// L-R Line Read Controller
reg		[2:0]	lr_state;
wire			lr_rdaddr_on, dly_on, hsad_on;
reg				dly_on_r;
reg		[1:0]	hsad_on_r;
reg		[9:0]	dly_wr_cnt;
reg		[9:0]	lr_rdaddr;
wire			lr_line_end;
reg				lr_done;

// Delay FIFO
wire			dly_wr;
wire	[5:0]	dly_dout;
wire			dly_full, dly_empty;
reg				dly_rd_on;
wire			dly_rd;

// Absolute Difference
reg		[5:0]	l_buf;
wire	[5:0]	r_din;
reg		[5:0]	r_buf[32:0];
wire	[6:0]	dif[33:0];
wire	[5:0]	abs[33:0];
reg		[5:0]	abs_r[33:0];

// HSAD Update
reg		[9:0]	hsad_addra, hsad_addrb;
wire			hsad_web;
wire	[9:0]	hsad_dout[33:0];
wire	[10:0]	hsad_add_sel[33:0];
wire	[10:0]	hsad_add[33:0];
wire	[9:0]	hsad_add_lim[33:0];
wire	[10:0]	hsad_sub[33:0];
wire	[9:0]	hsad_sub_lim[33:0];
wire	[9:0]	hsad_din[33:0];
wire	[339:0]	hsad_dinb;

// HSAD Buffer Instance
wire	[9:0]	ram_addra, ram_addrb;
wire	[339:0]	hsad_douta, hsad_doutb;

// SAD Update
reg		[2:0]	sad_state;
wire			sad_addra_enb, sad_addrb_enb, min_enb;
reg				sad_addra_enb_r, sad_addrb_enb_r;
reg		[1:0]	min_enb_r;
reg		[9:0]	sad_addrb, sad_addra;
wire	[9:0]	hsad_old[33:0], hsad_new[33:0];
wire	[10:0]	hsad_dif[33:0];
wire	[17:0]	sad_add[33:0];
wire	[15:0]	sad_add_lim[33:0];
reg		[15:0]	sad[33:0];
wire	[543:0]	sad_out;
wire			vout;


//==========================================================================
// L-R Line Read Controller
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		lr_state <= 3'b0;
	end
	else begin
		if (~enb) begin
			lr_state <= 0;
		end
		else begin
			case (lr_state)
				0: if ((op_type != 3) & lr_rdy)                           lr_state <= 1;
				1:                                                        lr_state <= 2;
				2: if (lr_rdaddr == ndisp)                                lr_state <= 3;
				3: begin
					if (dphase == 0)                                      lr_state <= 4;
					else if (dly_wr_cnt == wdt - ndisp + PARALLEL - 1'b1) lr_state <= 4;
				end
				4: if (lr_rdaddr == wdt - 1'b1)                           lr_state <= 5;
				5: if (lr_done)                                           lr_state <= 0;
			endcase
		end
	end
end

assign lr_rdaddr_on = (lr_state == 2) | (lr_state == 3) | (lr_state == 4);
assign dly_on       = (lr_state == 2) | (lr_state == 3);
assign hsad_on      = (lr_state == 3) | (lr_state == 4);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dly_on_r  <= 1'b0;
		hsad_on_r <= 2'b0;
	end
	else begin
		dly_on_r  <= dly_on;
		hsad_on_r <= {hsad_on_r[0], hsad_on};
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dly_wr_cnt <= 10'b0;
	end
	else begin
		if ((lr_state == 2) | (lr_state == 3)) begin
			dly_wr_cnt <= dly_wr_cnt + 1'b1;
		end
		else begin
			dly_wr_cnt <= 0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		lr_rdaddr <= 10'b0;
	end
	else begin
		if (lr_state == 1) begin
			lr_rdaddr <= (ndisp[8:5] - dphase - 1) * PARALLEL;
		end
		else if (lr_rdaddr_on) begin
			lr_rdaddr <= lr_rdaddr + 1'b1;
		end
		else begin
			lr_rdaddr <= 0;
		end
	end
end

assign lr_line_end = hsad_on_r[1] & ~hsad_on_r[0];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		lr_done <= 1'b0;
	end
	else begin
		if ((op_type == 0) & lr_line_end) begin
			lr_done <= 1'b1;
		end
		else if ((op_type == 1) & lr_line_end) begin
			lr_done <= 1'b1;
		end
		else if ((op_type == 2) & sad_line_end) begin
			lr_done <= 1'b1;
		end
		else begin
			lr_done <= 1'b0;
		end
	end
end


//==========================================================================
// Delay FIFO
//==========================================================================
assign dly_wr = (dphase != 0) & dly_on_r;

bm_calc_dly bm_calc_dly (
	.clk   (clk),
	.srst  (~rst_n),
	.din   (lr_din[5:0]),
	.wr_en (dly_wr),
	.rd_en (dly_rd),
	.dout  (dly_dout[5:0]),
	.full  (dly_full),
	.empty (dly_empty),
	.wr_rst_busy (),
	.rd_rst_busy ()
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dly_rd_on <= 1'b0;
	end
	else begin
		if (lr_line_end) begin
			dly_rd_on <= 1'b0;
		end
		else if (lr_rdaddr == ndisp - PARALLEL - 1'b1) begin
			dly_rd_on <= 1'b1;
		end
	end
end

assign dly_rd = dly_rd_on & ~dly_empty;


//==========================================================================
// Absolute Difference
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		l_buf <= 6'b0;
	end
	else begin
		if (lr_state != 0) begin
			l_buf <= lr_din[13:8];
		end
		else begin
			l_buf <= 0;
		end
	end
end

assign r_din[5:0] = (dphase == 0) ? lr_din[5:0] : dly_dout;

// 8-bit x 31-stage shift register
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 33; i = i + 1) begin
			r_buf[i] <= 6'b0;
		end
	end
	else begin
		if (lr_state != 0) begin
			for (i = 1; i < 33; i = i + 1) begin
				r_buf[i] <= r_buf[i-1];
			end
			r_buf[0] <= r_din;
		end
		else begin
			for (i = 0; i < 33; i = i + 1) begin
				r_buf[i] <= 6'b0;
			end
		end
	end
end

assign dif[ 0] = dif6(r_din[5:0], l_buf[5:0]);
for (j = 1; j < 34; j = j + 1) begin
	assign dif[j] = dif6(r_buf[j-1][5:0], l_buf[5:0]);
end

for (j = 0; j < 34; j = j + 1) begin
	assign abs[j] = abs7(dif[j]);
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 34; i = i + 1) begin
			abs_r[i] <= 6'b0;
		end
	end
	else begin
		if (hsad_on_r[0]) begin
			for (i = 0; i < 34; i = i + 1) begin
				abs_r[i] <= abs[i];
			end
		end
		else begin
			for (i = 0; i < 34; i = i + 1) begin
				abs_r[i] <= 6'b0;
			end
		end
	end
end


//==========================================================================
// HSAD Update
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		hsad_addra <= 10'b0;
	end
	else begin
		if (hsad_on_r[0]) begin
			hsad_addra <= hsad_addra + 1'b1;
		end
		else begin
			hsad_addra <= 0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		hsad_addrb <= 10'b0;
	end
	else begin
		hsad_addrb <= hsad_addra;
	end
end

assign hsad_web = hsad_on_r[1];

// vertical sliding
for (j = 0; j < 34; j = j + 1) begin
	// separate
	assign hsad_dout[j][9:0] = hsad_douta[10*j+9:10*j];
	
	// add new row
	// u6.0 + u10.0 -> u11.0 -> u10.0
	assign hsad_add_sel[j][10:0] = (first_line) ? abs_r[j][5:0] : hsad_dout[j][9:0] + abs_r[j][5:0];
	assign hsad_add[j][10:0]  = ((op_type == 0) | (op_type == 2)) ? hsad_add_sel[j][10:0] : 11'b0;
	assign hsad_add_lim[j][9:0] = upper_lim10(hsad_add[j][10:0]);
	
	// subtract old row
	// u10.0 - u6.0 -> s10.0 -> u10.0
	assign hsad_sub[j][10:0] = (op_type == 1) ? hsad_dout[j][9:0] - abs_r[j][5:0] : 11'b0;
	assign hsad_sub_lim[j][9:0] = lower_lim10(hsad_sub[j][10:0]);
	
	// selector
	assign hsad_din[j][9:0] = hsad_add_lim[j][9:0] | hsad_sub_lim[j][9:0];
	
	// concatenate
	assign hsad_dinb[10*j+9:10*j] = hsad_din[j][9:0];
end


//==========================================================================
// HSAD Buffer Instance
//==========================================================================
assign ram_addra[9:0] = (sad_state != 0) ? sad_addra[9:0] : hsad_addra[9:0];
assign ram_addrb[9:0] = (sad_state != 0) ? sad_addrb[9:0] : hsad_addrb[9:0];

tdpram_340_1024 hsad (
	// A port (read)
	.clka  (clk),
	.ena   (enb),
	.wea   (1'b0),
	.addra (ram_addra[9:0]),
	.dina  (340'b0),
	.douta (hsad_douta[339:0]),
	
	// B port (write/read)
	.clkb  (clk),
	.enb   (enb),
	.web   (hsad_web),
	.addrb (ram_addrb[9:0]),
	.dinb  (hsad_dinb[339:0]),
	.doutb (hsad_doutb[339:0])
);


//==========================================================================
// SAD Update
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		sad_state <= 3'b0;
	end
	else begin
		case (sad_state)
			0: begin
				if ((op_type == 2) & lr_line_end) begin
					sad_state <= 1;
				end
				else if ((op_type == 3) & sad_rdy) begin
					sad_state <= 1;
				end
			end
			1: if (sad_addrb == {hwsz, 1'b0} - 1'b1) sad_state <= 2;
			2:                                       sad_state <= 3;
			3: if (sad_addrb == hsad_wdt - 1'b1)     sad_state <= 4;
			4: if (sad_line_end)                     sad_state <= 0;
		endcase
	end
end

assign sad_addra_enb = (sad_state == 3);
assign sad_addrb_enb = (sad_state == 1) | (sad_state == 2) | (sad_state == 3);
assign min_enb = (sad_state == 2) | (sad_state == 3);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		sad_addra_enb_r <= 1'b0;
		sad_addrb_enb_r <= 1'b0;
		min_enb_r       <= 2'b0;
	end
	else begin
		sad_addra_enb_r <= sad_addra_enb;
		sad_addrb_enb_r <= sad_addrb_enb;
		min_enb_r       <= {min_enb_r[0], min_enb};
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		sad_addrb <= 10'b0;
	end
	else begin
		if (sad_addrb_enb) begin
			sad_addrb <= sad_addrb + 1'b1;
		end
		else begin
			sad_addrb <= 0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		sad_addra <= 10'b0;
	end
	else begin
		if (sad_addra_enb) begin
			sad_addra <= sad_addra + 1'b1;
		end
		else begin
			sad_addra <= 0;
		end
	end
end

// horizontal sliding
for (j = 0; j < 34; j = j + 1) begin
	// separate
	assign hsad_old[j][9:0] = (sad_addra_enb_r) ? hsad_douta[j*10+9:j*10] : 10'b0;
	assign hsad_new[j][9:0] = (sad_addrb_enb_r) ? hsad_doutb[j*10+9:j*10] : 10'b0;
	
	// difference between new and old
	// u10.0 - u10.0 -> u10.0 + s10.0 = s11.0 (two's complement)
	assign hsad_dif[j][10:0] = hsad_new[j][9:0] + inv11({1'b0, hsad_old[j][9:0]});
	
	// horizontal sliding
	// u16.0 + s10.0 -> u17.0 + s17.0 -> s17.0
	assign sad_add[j][17:0] = {2'b0, sad[j][15:0]} + {{7{hsad_dif[j][10]}}, hsad_dif[j][10:0]};
	
	// upper limit
	// s17.0 -> u16.0
	assign sad_add_lim[j][15:0] = limit16(sad_add[j][17:0]);
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 34; i = i + 1) begin
			sad[i] <= 16'b0;
		end
	end
	else begin
		if (sad_addrb_enb_r) begin
			for (i = 0; i < 34; i = i + 1) begin
				sad[i] <= sad_add_lim[i][15:0];
			end
		end
		else begin
			for (i = 0; i < 34; i = i + 1) begin
				sad[i] <= 16'b0;
			end
		end
	end
end

// concatenate
for (j = 0; j < 34; j = j + 1) begin
	assign sad_out[16*j+15:16*j] = sad[j][15:0];
end

assign vout = min_enb_r[1];


endmodule
