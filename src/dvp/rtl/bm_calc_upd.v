//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module bm_calc_upd (
	// Global Control
	rst_n, clk,
	
	// Control
	dphase,
	mode,
	
	// DET Input
	det_min1,
	det_min2,
	det_idx1,
	det_idx2,
	vin_m1,
	
	// IBUF SAD I/F
	sad_rdaddr,
	sad_din,
	
	// Output
	upd,
	upd_min1,
	upd_min2,
	upd_disp1,
	upd_disp2,
	upd_frac,
	vout
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Control
input	[3:0]	dphase;
input			mode;

// DET Input
input	[15:0]	det_min1, det_min2;
input	[4:0]	det_idx1, det_idx2;
input			vin_m1;

// IBUF SAD I/F
output	[9:0]	sad_rdaddr;
input	[63:0]	sad_din;

// Output
output			upd;
output	[15:0]	upd_min1, upd_min2;
output	[7:0]	upd_disp1, upd_disp2;
output	[7:0]	upd_frac;
output			vout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// IBUF SAD Read
reg		[9:0]	sad_rdaddr;
wire	[7:0]	sad_disp1, sad_disp2, sad_frac;
wire	[15:0]	sad_min1, sad_min2;

// Uniqueness Filter
reg		[1:0]	vin_m1_r;
wire	[7:0]	det_disp1, det_disp2;
wire			d1_lt_s1, d2_lt_s1, d1_lt_s2, d2_lt_s2, d1_adj_s1;
reg		[15:0]	upd_min1, upd_min2;
reg		[7:0]	upd_disp1, upd_disp2;
reg				upd;
reg		[7:0]	upd_frac;
wire			vout;


//==========================================================================
// IBUF SAD Read
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		sad_rdaddr <= 10'b0;
	end
	else begin
		if (mode & vin_m1) begin
			sad_rdaddr <= sad_rdaddr + 1'b1;
		end
		else begin
			sad_rdaddr <= 0;
		end
	end
end

assign sad_disp1[7:0] = sad_din[63:56];
assign sad_frac[7:0]  = sad_din[55:48];
assign sad_min1[15:0] = sad_din[47:32];
assign sad_disp2[7:0] = sad_din[31:24];
assign sad_min2[15:0] = sad_din[15: 0];


//==========================================================================
// Uniqueness Filter
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vin_m1_r <= 2'b0;
	end
	else begin
		vin_m1_r <= {vin_m1_r[0], vin_m1};
	end
end

assign det_disp1[7:5] = dphase[2:0];
assign det_disp1[4:0] = det_idx1[4:0];
assign det_disp2[7:5] = det_disp1[7:5];
assign det_disp2[4:0] = det_idx2[4:0];


//==========================================================================
// Truth Table for Minimum Order
//--------------------------------------------------------------------------
// d1_lt_s1|d2_lt_s1|d1_lt_s2|d2_lt_s2| 1st| 2nd| 3rd| 4th
//        1|       1|       x|       x|  d1|  d2|  s1|  s2
//        1|       0|       x|       1|  d1|  s1|  d2|  s2
//        1|       0|       x|       0|  d1|  s1|  s2|  d2
//        0|       x|       1|       1|  s1|  d1|  d2|  s2
//        0|       x|       1|       0|  s1|  d1|  s2|  d2
//        0|       x|       0|       x|  s1|  s2|  d1|  d2
//--------------------------------------------------------------------------
// Take the 1st and 2nd minimum values. If the 2nd is adjucent to the 1st,
// take the 3rd instead.
//==========================================================================
assign d1_lt_s1 = (det_min1 < sad_min1);
assign d2_lt_s1 = (det_min2 < sad_min1);
assign d1_lt_s2 = (det_min1 < sad_min2);
assign d2_lt_s2 = (det_min2 < sad_min2);
assign d1_adj_s1 = (det_disp1 == sad_disp1 + 1'b1);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		upd_min1  <= 16'b0;
		upd_disp1 <=  8'b0;
		upd_min2  <= 16'b0;
		upd_disp2 <=  8'b0;
		upd  <=  1'b0; // indicates min1 is updated
	end
	else begin
		if (vin_m1_r[0]) begin
			if (~mode) begin
				// initial SAD
				upd_min1  <= det_min1;
				upd_disp1 <= det_disp1;
				upd_min2  <= det_min2;
				upd_disp2 <= det_disp2;
				upd  <= 1'b1;
			end
			else begin
				casex({d1_lt_s1, d2_lt_s1, d1_lt_s2, d2_lt_s2})
					4'b11xx: begin
						upd_min1  <= det_min1;
						upd_disp1 <= det_disp1;
						upd_min2  <= det_min2;
						upd_disp2 <= det_disp2;
						upd  <= 1'b1;
					end
					4'b10x1: begin
						upd_min1  <= det_min1;
						upd_disp1 <= det_disp1;
						upd_min2  <= (~d1_adj_s1) ? sad_min1  : det_min2;
						upd_disp2 <= (~d1_adj_s1) ? sad_disp1 : det_disp2;
						upd  <= 1'b1;
					end
					4'b10x0: begin
						upd_min1  <= det_min1;
						upd_disp1 <= det_disp1;
						upd_min2  <= (~d1_adj_s1) ? sad_min1  : sad_min2;
						upd_disp2 <= (~d1_adj_s1) ? sad_disp1 : sad_disp2;
						upd  <= 1'b1;
					end
					4'b0x11: begin
						upd_min1  <= sad_min1;
						upd_disp1 <= sad_disp1;
						upd_min2  <= (~d1_adj_s1) ? det_min1  : det_min2;
						upd_disp2 <= (~d1_adj_s1) ? det_disp1 : det_disp2;
						upd  <= 1'b0;
					end
					4'b0x10: begin
						upd_min1  <= sad_min1;
						upd_disp1 <= sad_disp1;
						upd_min2  <= (~d1_adj_s1) ? det_min1  : sad_min2;
						upd_disp2 <= (~d1_adj_s1) ? det_disp1 : sad_disp2;
						upd  <= 1'b0;
					end
					default: begin
						upd_min1  <= sad_min1;
						upd_disp1 <= sad_disp1;
						upd_min2  <= sad_min2;
						upd_disp2 <= sad_disp2;
						upd  <= 1'b0;
					end
				endcase
			end
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		upd_frac <= 8'b0;
	end
	else begin
		upd_frac <= sad_frac;
	end
end

assign vout = vin_m1_r[1];


endmodule
