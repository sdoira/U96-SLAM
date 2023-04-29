//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module gftt_box (
	// Global Control
	rst_n, clk,
	
	// Parameter
	wdt_m1,
	
	// Control
	start,
	enb,
	
	// Frontend I/F
	din,
	vin,
	
	// Backend I/F
	dout,
	vout
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Parameter
input	[10:0]	wdt_m1;

// Control
input			start;
input			enb;

// Frontend I/F
input	[15:0]	din;
input			vin;

// Backend I/F
output	[15:0]	dout;
output			vout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Timing Control
reg		[3:0]	vin_r;
reg		[9:0]	col;
wire			col_start, col_end;
reg				col_start_r, col_end_r;
reg		[1:0]	row;
wire			line_ready;
reg		[2:0]	line_ready_r;
reg				line_phase;

// Line Buffer Instance and Control
wire	[1:0]	wr;
reg		[9:0]	wraddr;
wire	[17:0]	wrdata;
reg		[17:0]	wrdata_r;
wire	[17:0]	ram0_dout, ram1_dout;
wire			rd;
wire	[9:0]	rdaddr;

// Calculation Pipeline
reg		[15:0]	din_r;
reg		[16:0]	add1;
reg		[17:0]	add2;
reg		[18:0]	add3;
reg		[19:0]	add4;
wire	[15:0]	lim;
wire	[15:0]	dout;
wire			vout;


//==========================================================================
// Timing Control
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vin_r <= 4'b0;
	end
	else begin
		vin_r <= {vin_r[2:0], vin};
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		col <= 10'b0;
	end
	else begin
		if (~enb | start) begin
			col <= 0;
		end
		else if (vin_r[0]) begin
			if (col_end) begin
				col <= 0;
			end
			else begin
				col <= col + 1'b1;
			end
		end
	end
end

assign col_start = vin_r[0] & (col == 0);
assign col_end = (col == wdt_m1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		col_start_r <= 1'b0;
		col_end_r <= 1'b0;
	end
	else begin
		col_start_r <= col_start;
		col_end_r <= col_end;
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		row <= 2'b0;
	end
	else begin
		if (~enb | start) begin
			row <= 0;
		end
		else if (col_end) begin
			if (row == 2) begin
				row <= row;
			end
			else begin
				row <= row + 1'b1;
			end
		end
	end
end

assign line_ready = (row == 2);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		line_ready_r <= 3'b0;
	end
	else begin
		line_ready_r <= {line_ready_r[1:0], line_ready};
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		line_phase <= 1'b0;
	end
	else begin
		if (~enb | start) begin
			line_phase <= 0;
		end
		else if (col_end_r) begin
			line_phase <= ~line_phase;
		end
	end
end


//==========================================================================
// Line Buffer Instance and Control
//==========================================================================
// Write Control
assign wr[0] = ~line_phase & vin_r[1];
assign wr[1] =  line_phase & vin_r[1];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		wraddr <= 10'b0;
	end
	else begin
		wraddr <= col;
	end
end

assign wrdata[17:0] = (col_start_r | col_end_r) ? 18'b0 : add2[17:0];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		wrdata_r <= 18'b0;
	end
	else begin
		wrdata_r <= wrdata[17:0];
	end
end

gftt_box_ram ram0 (
	.clka  (clk),
	.ena   (1'b1),
	.addra (wraddr[9:0]),
	.dina  (wrdata[17:0]),
	.wea   (wr[0]),
	
	.clkb  (clk),
	.enb   (1'b1),
	.addrb (rdaddr[9:0]),
	.doutb (ram0_dout[17:0])
);

gftt_box_ram ram1 (
	.clka  (clk),
	.ena   (1'b1),
	.addra (wraddr[9:0]),
	.dina  (wrdata[17:0]),
	.wea   (wr[1]),
	
	.clkb  (clk),
	.enb   (1'b1),
	.addrb (rdaddr[9:0]),
	.doutb (ram1_dout[17:0])
);

// Read Control
assign rd = vin_r[0];
assign rdaddr = col;


//==========================================================================
// Calculation Pipeline
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		din_r <= 16'b0;
	end
	else begin
		din_r <= din[15:0];
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		add1 <= 17'b0;
	end
	else begin
		add1 <= din_r[15:0] + din[15:0];
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		add2 <= 18'b0;
	end
	else begin
		add2 <= add1[16:0] + din[15:0];
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		add3 <= 19'b0;
	end
	else begin
		add3 <= ram0_dout[17:0] + ram1_dout[17:0];
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		add4 <= 20'b0;
	end
	else begin
		add4 <= wrdata_r[17:0] + add3[18:0];
	end
end

assign lim[15:0] = |add4[19:16] ? 16'hFFFF : add4[15:0];

assign dout[15:0] = lim[15:0];
assign vout = line_ready_r[2] & vin_r[3];


endmodule
