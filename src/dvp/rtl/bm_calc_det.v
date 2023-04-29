//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module bm_calc_det (
	// Global Control
	rst_n, clk,
	
	// Input
	sad_con,
	vin,
	
	// Output
	det_min1,
	det_min2,
	det_idx1,
	det_idx2,
	det_l,
	det_r,
	vout,
	vout_m1
);


//==========================================================================
// Parameters
//==========================================================================
localparam L = 0; // Left
localparam C = 1; // Center
localparam R = 2; // Right


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Input
input	[543:0]	sad_con;
input			vin;

// Output
output	[15:0]	det_min1, det_min2;
output	[4:0]	det_idx1, det_idx2;
output	[15:0]	det_l, det_r;
output			vout, vout_m1;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
integer			i;
genvar			j;

// Input Stage
reg		[5:0]	vin_r;
wire	[15:0]	sad[33:0];

// 1st Stage
reg		[15:0]	cmp1;
reg		[15:0]	sad_r1[33:0];
wire	[15:0]	min1_sel[2:0][15:0];

// 2nd Stage
wire	[7:0]	cmp2;
reg		[4:0]	idx_r2[7:0];
reg		[15:0]	min1_r2[2:0][7:0];

// 3rd Stage
reg		[15:0]	min1_r3[2:0][3:0];
reg		[4:0]	idx_r3[3:0];

// 4th Stage
reg		[15:0]	min1_r4[2:0][1:0];
reg		[4:0]	idx1_r4[1:0];
reg		[15:0]	min2_r4[1:0];
reg		[4:0]	idx2_r4[1:0];

// 5th Stage
reg		[15:0]	min1_r5[2:0];
reg		[4:0]	idx1_r5;
reg		[15:0]	min2_r5[1:0];
reg		[4:0]	idx2_r5[1:0];

// 6th Stage
wire	[5:0]	adj_idx1;
wire	[5:0]	adj_idx2[1:0];
wire			min2_adj[1:0];
reg		[15:0]	min1_r6[2:0];
reg		[4:0]	idx1_r6;
reg		[15:0]	min2_r6;
reg		[4:0]	idx2_r6;

// Output Stage
wire	[15:0]	det_min1, det_min2;
wire	[4:0]	det_idx1, det_idx2;
wire	[15:0]	det_l, det_r;
wire			vout, vout_m1;


//==========================================================================
// Input Stage
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vin_r <= 6'b0;
	end
	else begin
		vin_r <= {vin_r[4:0], vin};
	end
end

for (j = 0; j < 34; j = j + 1) begin
	assign sad[j][15:0] = sad_con[16*j+15:16*j];
end


//==========================================================================
// 1st Stage
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		cmp1 <= 16'b0;
	end
	else begin
		if (vin) begin
			for (i = 0; i < 16; i = i + 1) begin
				// compare sad[1..32]
				cmp1[i] <= (sad[i*2+2] < sad[i*2+1]);
			end
		end
		else begin
			cmp1 <= 16'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 34; i = i + 1) begin
			sad_r1[i] <= 16'b0;
		end
	end
	else begin
		if (vin) begin
			for (i = 0; i < 34; i = i + 1) begin
				sad_r1[i] <= sad[i];
			end
		end
		else begin
			for (i = 0; i < 34; i = i + 1) begin
				sad_r1[i] <= 16'b0;
			end
		end
	end
end

//==========================================================================
// 2nd Stage
//==========================================================================
for (j = 0; j < 16; j = j + 1) begin
	assign min1_sel[L][j] = (cmp1[j]) ? sad_r1[j*2+1] : sad_r1[j*2  ]; // left
	assign min1_sel[C][j] = (cmp1[j]) ? sad_r1[j*2+2] : sad_r1[j*2+1]; // center
	assign min1_sel[R][j] = (cmp1[j]) ? sad_r1[j*2+3] : sad_r1[j*2+2]; // right
end

for (j = 0; j < 8; j = j + 1) begin
	assign cmp2[j] = (min1_sel[C][j*2+1] < min1_sel[C][j*2]);
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 8; i = i + 1) begin
			idx_r2[i] <= 5'b0;
		end
	end
	else begin
		if (vin_r[0]) begin
			for (i = 0; i < 8; i = i + 1) begin
				if (~cmp2[i]) begin
					idx_r2[i] <= i * 4 + cmp2[i] * 2 + cmp1[i*2];
				end
				else begin
					idx_r2[i] <= i * 4 + cmp2[i] * 2 + cmp1[i*2+1];
				end
			end
		end
		else begin
			for (i = 0; i < 8; i = i + 1) begin
				idx_r2[i] <= 5'b0;
			end
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 8; i = i + 1) begin
			min1_r2[L][i] <= 16'b0;
			min1_r2[C][i] <= 16'b0;
			min1_r2[R][i] <= 16'b0;
		end
	end
	else begin
		if (vin_r[0]) begin
			for (i = 0; i < 8; i = i + 1) begin
				min1_r2[L][i] <= (cmp2[i]) ? min1_sel[L][i*2+1] : min1_sel[L][i*2];
				min1_r2[C][i] <= (cmp2[i]) ? min1_sel[C][i*2+1] : min1_sel[C][i*2];
				min1_r2[R][i] <= (cmp2[i]) ? min1_sel[R][i*2+1] : min1_sel[R][i*2];
			end
		end
		else begin
			for (i = 0; i < 8; i = i + 1) begin
				min1_r2[L][i] <= 16'b0;
				min1_r2[C][i] <= 16'b0;
				min1_r2[R][i] <= 16'b0;
			end
		end
	end
end

//==========================================================================
// 3rd Stage
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 4; i = i + 1) begin
			min1_r3[L][i] <= 16'b0;
			min1_r3[C][i] <= 16'b0;
			min1_r3[R][i] <= 16'b0;
			idx_r3[i]    <=  5'b0;
		end
	end
	else begin
		if (vin_r[1]) begin
			for (i = 0; i < 4; i = i + 1) begin
				if (min1_r2[C][i*2+1] < min1_r2[C][i*2]) begin
					min1_r3[L][i] <= min1_r2[L][i*2+1];
					min1_r3[C][i] <= min1_r2[C][i*2+1];
					min1_r3[R][i] <= min1_r2[R][i*2+1];
					idx_r3[i]    <= idx_r2[i*2+1];
				end
				else begin
					min1_r3[L][i] <= min1_r2[L][i*2];
					min1_r3[C][i] <= min1_r2[C][i*2];
					min1_r3[R][i] <= min1_r2[R][i*2];
					idx_r3[i]    <= idx_r2[i*2];
				end
			end
		end
		else begin
			for (i = 0; i < 4; i = i + 1) begin
				min1_r3[L][i] <= 16'b0;
				min1_r3[C][i] <= 16'b0;
				min1_r3[R][i] <= 16'b0;
				idx_r3[i]    <=  5'b0;
			end
		end
	end
end

//==========================================================================
// 4th Stage
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 2; i = i + 1) begin
			min1_r4[L][i] <= 16'b0;
			min1_r4[C][i] <= 16'b0;
			min1_r4[R][i] <= 16'b0;
			idx1_r4[i]    <=  5'b0;
			min2_r4[i]    <= 16'b0;
			idx2_r4[i]    <=  5'b0;
		end
	end
	else begin
		if (vin_r[2]) begin
			for (i = 0; i < 2; i = i + 1) begin
				if (min1_r3[C][i*2+1] < min1_r3[C][i*2]) begin
					min1_r4[L][i] <= min1_r3[L][i*2+1];
					min1_r4[C][i] <= min1_r3[C][i*2+1];
					min1_r4[R][i] <= min1_r3[R][i*2+1];
					idx1_r4[i]    <= idx_r3[i*2+1];
					min2_r4[i]    <= min1_r3[C][i*2];
					idx2_r4[i]    <= idx_r3[i*2];
				end
				else begin
					min1_r4[L][i] <= min1_r3[L][i*2];
					min1_r4[C][i] <= min1_r3[C][i*2];
					min1_r4[R][i] <= min1_r3[R][i*2];
					idx1_r4[i]    <= idx_r3[i*2];
					min2_r4[i]    <= min1_r3[C][i*2+1];
					idx2_r4[i]    <= idx_r3[i*2+1];
				end
			end
		end
		else begin
			for (i = 0; i < 2; i = i + 1) begin
				min1_r4[L][i] <= 16'b0;
				min1_r4[C][i] <= 16'b0;
				min1_r4[R][i] <= 16'b0;
				idx1_r4[i]    <=  5'b0;
				min2_r4[i]    <= 16'b0;
				idx2_r4[i]    <=  5'b0;
			end
		end
	end
end

//==========================================================================
// 5th Stage
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		min1_r5[L] <= 16'b0;
		min1_r5[C] <= 16'b0;
		min1_r5[R] <= 16'b0;
		idx1_r5    <=  5'b0;
		min2_r5[0] <= 16'b0;
		idx2_r5[0] <=  5'b0;
	end
	else begin
		if (vin_r[3]) begin
			if (min1_r4[C][1] < min1_r4[C][0]) begin
				min1_r5[L] <= min1_r4[L][1];
				min1_r5[C] <= min1_r4[C][1];
				min1_r5[R] <= min1_r4[R][1];
				idx1_r5    <= idx1_r4[1];
				min2_r5[0] <= min1_r4[C][0];
				idx2_r5[0] <= idx1_r4[0];
			end
			else begin
				min1_r5[L] <= min1_r4[L][0];
				min1_r5[C] <= min1_r4[C][0];
				min1_r5[R] <= min1_r4[R][0];
				idx1_r5    <= idx1_r4[0];
				min2_r5[0] <= min1_r4[C][1];
				idx2_r5[0] <= idx1_r4[1];
			end
		end
		else begin
			min1_r5[0] <= 16'b0;
			min1_r5[1] <= 16'b0;
			min1_r5[2] <= 16'b0;
			idx1_r5    <=  5'b0;
			min2_r5[0] <= 16'b0;
			idx2_r5[0] <=  5'b0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		min2_r5[1] <= 16'b0;
		idx2_r5[1] <=  5'b0;
	end
	else begin
		if (vin_r[3]) begin
			if (min2_r4[1] < min2_r4[0]) begin
				min2_r5[1] <= min2_r4[1];
				idx2_r5[1] <= idx2_r4[1];
			end
			else begin
				min2_r5[1] <= min2_r4[0];
				idx2_r5[1] <= idx2_r4[0];
			end
		end
		else begin
			min2_r5[1] <= 16'b0;
			idx2_r5[1] <=  5'b0;
		end
	end
end

//==========================================================================
// 6th Stage
//==========================================================================
// next index number. roll over should be considered.
assign adj_idx1[5:0   ] = idx1_r5[4:0]    + 1'b1;
assign adj_idx2[0][5:0] = idx2_r5[0][4:0] + 1'b1;
assign adj_idx2[1][5:0] = idx2_r5[1][4:0] + 1'b1;

// indicates if the two candidates for min2 are adjucent to min1.
assign min2_adj[0] = (idx2_r5[0] == adj_idx1) | (idx1_r5 == adj_idx2[0]);
assign min2_adj[1] = (idx2_r5[1] == adj_idx1) | (idx1_r5 == adj_idx2[1]);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		min1_r6[L] <= 16'b0;
		min1_r6[C] <= 16'b0;
		min1_r6[R] <= 16'b0;
		idx1_r6    <=  5'b0;
		min2_r6    <= 16'b0;
		idx2_r6    <=  5'b0;
	end
	else begin
		if (vin_r[4]) begin
			min1_r6[L] <= min1_r5[L];
			min1_r6[C] <= min1_r5[C];
			min1_r6[R] <= min1_r5[R];
			idx1_r6    <= idx1_r5;
			// condition to select min2_r5[1] is
			//  1. [1] < [0], and [1] is not adjucent, or
			//  2. [0] is adjucent
			if ((min2_r5[1] < min2_r5[0] & ~min2_adj[1]) | min2_adj[0]) begin
				min2_r6 <= min2_r5[1];
				idx2_r6 <= idx2_r5[1];
			end
			else begin
				min2_r6 <= min2_r5[0];
				idx2_r6 <= idx2_r5[0];
			end
		end
		else begin
			min1_r6[L] <= 16'b0;
			min1_r6[C] <= 16'b0;
			min1_r6[R] <= 16'b0;
			idx1_r6    <=  5'b0;
			min2_r6    <= 16'b0;
			idx2_r6    <=  5'b0;
		end
	end
end


//==========================================================================
// Output Stage
//==========================================================================
assign det_min1 = min1_r6[C];
assign det_min2 = min2_r6;
assign det_idx1 = idx1_r6;
assign det_idx2 = idx2_r6;
assign det_l    = min1_r6[L];
assign det_r    = min1_r6[R];
assign vout     = vin_r[5];
assign vout_m1  = vin_r[4];


endmodule
