//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module bm_calc_frac (
	// Global Control
	rst_n, clk,
	
	// Input
	det_min,
	det_idx,
	det_l,
	det_r,
	vin,
	
	// Output
	frac_out,
	vout
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Input
input	[15:0]	det_min;
input	[4:0]	det_idx;
input	[15:0]	det_l, det_r;
input			vin;

// Output
output	[7:0]	frac_out;
output			vout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Disp _Fraction Calculation
wire	[16:0]	dif_lr;
wire	[16:0]	dif_lc, dif_rc;
wire			cmp, neg_val;
reg		[9:0]	cmp_r;
reg		[17:0]	dividend, divisor;
wire			div_zero;
reg		[8:0]	div_zero_r;
reg				vin_r;
wire	[7:0]	div_dout;
wire			div_vout;
reg		[7:0]	frac_out;
reg				vout;


//==========================================================================
// Disp _Fraction Calculation
//==========================================================================
// u16.0 - u16.0 -> s16.0
assign dif_lr[16:0] = det_l[15:0] - det_r[15:0]; // L - R
assign dif_lc[16:0] = det_l[15:0] - det_min[15:0]; // L - C
assign dif_rc[16:0] = det_r[15:0] - det_min[15:0]; // R - C

assign cmp = (det_l[15:0] < det_r[15:0]); // if L is lesser than R
assign neg_val = dif_lc[16] | dif_rc[16]; // negative value detected

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		cmp_r <= 10'b0;
	end
	else begin
		cmp_r <= {cmp_r[8:0], cmp};
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dividend <= 18'b0;
	end
	else begin
		if (vin) begin
			if (neg_val) begin
				dividend <= 0;
			end
			else begin
				dividend <= {dif_lr[16], dif_lr[16:0]};
			end
		end
		else begin
			dividend <= 0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		divisor <= 18'b0;
	end
	else begin
		if (vin) begin
			if (~cmp) begin
				divisor <= {dif_lc[16:0], 1'b0};
			end
			else begin
				divisor <= {dif_rc[16:0], 1'b0};
			end
		end
		else begin
			divisor <= 0;
		end
	end
end

assign div_zero = (divisor == 0);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		div_zero_r <= 9'b0;
	end
	else begin
		div_zero_r <= {div_zero_r[7:0], div_zero};
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vin_r <= 1'b0;
	end
	else begin
		vin_r <= vin;
	end
end

diven #(18, 18, 8, 17) diven (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	.cke   (1'b1),
	
	// Divider I/F
	.vin      (vin_r),
	.dividend (dividend[17:0]),
	.divisor  (divisor[17:0]),
	.quotient (div_dout[7:0]),
	.vout     (div_vout)
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		frac_out <= 8'b0;
	end
	else begin
		if (div_vout) begin
			if (div_zero_r[8]) begin
				if (~cmp_r[9]) begin
					frac_out <= 8'hC0;
				end
				else begin
					frac_out <= 8'h40;
				end
			end
			else begin
				frac_out <= div_dout[7:0];
			end
		end
		else begin
			frac_out <= 0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vout <= 1'b0;
	end
	else begin
		vout <= div_vout;
	end
end


endmodule
