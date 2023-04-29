//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module gftt_ibuf (
	// Global Control
	rst_n, clk,
	
	// Parameter
	enb,
	addr_a,
	addr_b,
	bst_len_m1,
	hgt,
	hgt_m1,
	wdt,
	wdt_m1,
	
	// Control
	start,
	
	// DDR Arbiter I/F
	drd_req,
	drd_ack,
	drd_vin,
	drd_din,
	drd_vout,
	drd_dout,
	
	// IBUF I/F
	line0,
	line1,
	line2,
	vout,
	first_smpl,
	last_smpl
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Parameter
input			enb;
input	[11:0]	addr_a, addr_b;
input	[7:0]	bst_len_m1;
input	[8:0]	hgt, hgt_m1;
input	[9:0]	wdt, wdt_m1;

// Control
input			start;

// DDR Arbiter I/F
output			drd_req;
input			drd_ack;
input			drd_vin;
input	[31:0]	drd_din;
output			drd_vout;
output	[31:0]	drd_dout;

// IBUF I/F
output	[7:0]	line0, line1, line2;
output			vout;
output			first_smpl, last_smpl;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Statemachine
reg		[3:0]	state;
reg		[9:0]	st_cnt;
wire			bst_end, line_end;
reg				line_end_r;
reg		[7:0]	col;
wire			col_end;
reg		[8:0]	row;
reg		[1:0]	row2;
reg				bank;

// DDR Read Request & Line Buffer Write
reg				drd_req;
wire			drd_vout;
reg		[16:0]	addr_ofs;
wire	[31:0]	drd_cmd, drd_addr, drd_dout;
wire	[7:0]	wraddr;
wire	[31:0]	wrdata;
wire	[2:0]	wr;

// Line Buffer Instance
wire	[7:0]	ram0_dout, ram1_dout, ram2_dout;

// Line Buffer Read
wire			buf_rd;
wire	[9:0]	rdaddr;
wire			row_end;
reg		[1:0]	line_phase;
reg				vout;
wire	[7:0]	line0, line1, line2;
reg				first_smpl, last_smpl;


//==========================================================================
// Statemachine
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
				0: state <= (start)    ? 1 : 0;
				1: state <= 2;
				2: state <= (drd_ack)  ? 3 : 2;
				3: state <= 4;
				4: begin
					state <= (col_end) ? 6 :
						     (bst_end) ? 5 : 4;
				end
				5: state <= 1;
				6: state <= 7;
				7: begin
					state <= (row_end)  ? 9 :
					         (line_end) ? 8 : 7;
				end
				8: state <= 1;
				9: state <= 0;
			endcase
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		st_cnt <= 10'b0;
	end
	else begin
		if (state == 4) begin
			if (drd_vin) begin
				if (bst_end) begin
					st_cnt <= 0;
				end
				else begin
					st_cnt <= st_cnt + 1'b1;
				end
			end
		end
		else if (state == 7) begin
			if (line_end) begin
				st_cnt <= 0;
			end
			else begin
				st_cnt <= st_cnt + 1'b1;
			end
		end
		else begin
			st_cnt <= 0;
		end
	end
end

assign bst_end = (state == 4) & (st_cnt == bst_len_m1);
assign line_end = (state == 7) & (st_cnt == wdt_m1);

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
		col <= 8'b0;
	end
	else begin
		if (~enb | start) begin
			col <= 0;
		end
		else if (drd_vin) begin
			if (col_end) begin
				col <= 0;
			end
			else begin
				col <= col + 1'b1;
			end
		end
	end
end

assign col_end = (col == (wdt[9:2] - 1'b1));

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		row <= 9'b0;
	end
	else begin
		if (~enb | start) begin
			row <= 0;
		end
		else if (line_end) begin
			if (row_end) begin
				row <= 0;
			end
			else begin
				row <= row + 1'b1;
			end
		end
	end
end

// counts the first two lines
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		row2 <= 2'b0;
	end
	else begin
		if (~enb | start) begin
			row2 <= 0;
		end
		else if (line_end) begin
			if (row_end) begin
				row2 <= 0;
			end
			else if (row2 == 2) begin
				row2 <= row2;
			end
			else begin
				row2 <= row2 + 1'b1;
			end
		end
	end
end

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


//==========================================================================
// DDR Read Request & Line Buffer Write
//==========================================================================
/*
	ADDR = {
		2'b0,
		BUF_RECT_A[29:20],
		lr[0],
		row[8:0],
		col[9:0]
	};
*/
// ADDR = {2'b0, BUF_RECT_A[29:20], lr[0], 19'b0};
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		drd_req <= 1'b0;
	end
	else begin
		if (state == 1) begin
			drd_req <= 1'b1;
		end
		else if (bst_end) begin
			drd_req <= 1'b0;
		end
	end
end

assign drd_vout = (state == 2 & drd_ack) | (state == 3);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		addr_ofs <= 17'b0;
	end
	else begin
		if (~enb | start) begin
			addr_ofs <= 0;
		end
		else if (drd_vin) begin
			addr_ofs <= addr_ofs + 1'b1;
		end
	end
end

assign drd_cmd[31:0] = {23'h00_0000, 1'b1, bst_len_m1[7:0]};

assign drd_addr[31:20] = (~bank) ? addr_a[11:0] : addr_b[11:0];
//assign drd_addr[19: 0] = {1'b0, row[8:0], col[7:0], 2'b0};
assign drd_addr[19: 0] = {1'b0, addr_ofs[16:0], 2'b0};

assign drd_dout[31:0] = (
	((state == 2) & (drd_ack)) ? drd_cmd[31:0] :
	(state == 3)               ? drd_addr[31:0] :
	                             32'b0
);

assign wraddr[7:0] = col[7:0];
assign wrdata[31:0] = drd_din[31:0];
assign wr[0] = (line_phase == 0) & drd_vin;
assign wr[1] = (line_phase == 1) & drd_vin;
assign wr[2] = (line_phase == 2) & drd_vin;


//==========================================================================
// Line Buffer Instance
//==========================================================================
gftt_ibuf_ram ram0 (
	.clka  (clk),
	.ena   (1'b1),
	.addra (wraddr[7:0]),
	.dina  (wrdata[31:0]),
	.wea   (wr[0]),
	
	.clkb  (clk),
	.enb   (1'b1),
	.addrb (rdaddr[9:0]),
	.doutb (ram0_dout[7:0])
);

gftt_ibuf_ram ram1 (
	.clka  (clk),
	.ena   (1'b1),
	.addra (wraddr[7:0]),
	.dina  (wrdata[31:0]),
	.wea   (wr[1]),
	
	.clkb  (clk),
	.enb   (1'b1),
	.addrb (rdaddr[9:0]),
	.doutb (ram1_dout[7:0])
);

gftt_ibuf_ram ram2 (
	.clka  (clk),
	.ena   (1'b1),
	.addra (wraddr[7:0]),
	.dina  (wrdata[31:0]),
	.wea   (wr[2]),
	
	.clkb  (clk),
	.enb   (1'b1),
	.addrb (rdaddr[9:0]),
	.doutb (ram2_dout[7:0])
);


//==========================================================================
// Line Buffer Read
//==========================================================================
assign buf_rd = (state == 7);
assign rdaddr = st_cnt;

assign row_end = line_end & (row == hgt_m1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		line_phase <= 2'b0;
	end
	else begin
		if (~enb | start) begin
			line_phase <= 0;
		end
		else if (line_end_r) begin
			if (line_phase == 2) begin
				line_phase <= 0;
			end
			else begin
				line_phase <= line_phase + 1'b1;
			end
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vout <= 1'b0;
	end
	else begin
		if (row2 == 2) begin
			vout <= buf_rd;
		end
		else begin
			vout <= 1'b0;
		end
	end
end

assign line0[7:0] = (
	(~vout)                ? 8'b0 :
	(line_phase[1:0] == 0) ? ram1_dout[7:0] :
	(line_phase[1:0] == 1) ? ram2_dout[7:0] :
	                         ram0_dout[7:0]
);

assign line1[7:0] = (
	(~vout)                ? 8'b0 :
	(line_phase[1:0] == 0) ? ram2_dout[7:0] :
	(line_phase[1:0] == 1) ? ram0_dout[7:0] :
	                         ram1_dout[7:0]
);

assign line2[7:0] = (
	(~vout)                ? 8'b0 :
	(line_phase[1:0] == 0) ? ram0_dout[7:0] :
	(line_phase[1:0] == 1) ? ram1_dout[7:0] :
	                         ram2_dout[7:0]
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		first_smpl <= 1'b0;
	end
	else begin
		if (buf_rd & (rdaddr == 0)) begin
			first_smpl <= 1'b1;
		end
		else begin
			first_smpl <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		last_smpl <= 1'b0;
	end
	else begin
		if (buf_rd & (rdaddr == wdt_m1)) begin
			last_smpl <= 1'b1;
		end
		else begin
			last_smpl <= 1'b0;
		end
	end
end


endmodule
