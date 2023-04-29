//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module bm_calc (
	// Global Control
	rst_n, clk,
	
	// Parameter
	wdt,
	ndisp,
	hwsz,
	hsad_wdt,
	uni_enb,
	uni_mode,
	uni_thr,
	
	// Control
	enb,
	start,
	lr_rdy,
	sad_rdy,
	lr_done,
	sad_done,
	buf_info,
	
	// IBUF I/F
	lr_rdaddr,
	lr_din,
	sad_rdaddr,
	sad_din,
	
	// OBUF I/F
	obuf_wr,
	obuf_wrdata,
	obuf2_wr,
	obuf2_wrdata
);


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
input			uni_enb, uni_mode;
input	[9:0]	uni_thr;

// Control
input			enb, start;
input			lr_rdy, sad_rdy;
output			lr_done, sad_done;
input	[15:0]	buf_info;

// IBUF I/F
output	[9:0]	lr_rdaddr;
input	[15:0]	lr_din;
output	[9:0]	sad_rdaddr;
input	[63:0]	sad_din;

// OBUF I/F
output			obuf_wr;
output	[65:0]	obuf_wrdata;
output			obuf2_wr;
output	[15:0]	obuf2_wrdata;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
integer			i;

// Buffer Status
wire			bank;
wire			last_dphase;
wire			first_line, last_line;
wire	[3:0]	dphase;
wire	[5:0]	pdisp;
wire	[1:0]	op_type;

// SAD Calculation
wire			lr_done;
wire	[9:0]	lr_rdaddr;
wire	[543:0]	sad_dout;
wire			sad_vout;

// Minimum Detection
wire	[15:0]	det_min1, det_min2;
wire	[4:0]	det_idx1, det_idx2;
wire	[15:0]	det_l, det_r;
wire			det_vout, det_vout_m1;

// Disp Fraction Calculation
wire	[7:0]	frac_dout;
wire			frac_vout;
reg		[7:0]	frac_dout_r[1:0];

// SAD Update and Uniqueness Filter
wire			upd_mode;
wire	[9:0]	sad_rdaddr;
wire			upd;
wire	[15:0]	upd_min1, upd_min2;
wire	[7:0]	upd_disp1, upd_disp2;
wire	[7:0]	upd_frac;
wire			upd_vout;
wire			uni_on;
wire			uni_upd;
wire	[15:0]	uni_min1, uni_min2;
wire	[7:0]	uni_disp1, uni_disp2;
wire	[7:0]	uni_frac;
wire	[9:0]	uni_ratio;
wire			uni_vout;
wire	[7:0]	uni_frac_upd;
wire			uni_act;
wire	[7:0]	uni_disp1_mask, uni_frac_mask;
reg				uni_vout_r;

// OBUF Data Generation
wire	[31:0]	obuf_word1, obuf_word2;
reg				obuf_wr_phase;
reg		[31:0]	obuf_word1_r;
reg		[65:0]	obuf_wrdata_i;
wire	[65:0]	obuf_wrdata;
wire			obuf_wr;
wire			sad_line_end;
wire			sad_done;
wire	[15:0]	obuf2_wrdata;
wire			obuf2_wr;


//==========================================================================
// Buffer Status
//==========================================================================
assign bank         = buf_info[15   ];
assign last_dphase  = buf_info[14   ];
assign first_line   = buf_info[13   ];
assign last_line    = buf_info[12   ];
assign dphase[3:0]  = buf_info[11: 8];
assign pdisp[5:0]   = buf_info[ 7: 2];
assign op_type[1:0] = buf_info[ 1: 0];


//==========================================================================
// SAD Calculation
//==========================================================================
bm_calc_sad bm_calc_sad (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Parameter
	.wdt      (wdt),
	.ndisp    (ndisp),
	.hwsz     (hwsz),
	.hsad_wdt (hsad_wdt),
	
	// Buffer Status
	.first_line (first_line),
	.dphase     (dphase),
	.op_type    (op_type),
	
	// Control
	.enb (enb),
	.lr_rdy   (lr_rdy),
	.sad_rdy  (sad_rdy),
	.lr_done  (lr_done),
	.sad_line_end (sad_line_end),
	
	// IBUF I/F
	.lr_rdaddr  (lr_rdaddr),
	.lr_din     (lr_din),
	
	// Output
	.sad_out (sad_dout),
	.vout    (sad_vout)
);


//==========================================================================
// Minimum Detection
//==========================================================================
bm_calc_det bm_calc_det (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Input
	.sad_con (sad_dout),
	.vin     (sad_vout),
	
	// Output
	.det_min1 (det_min1),
	.det_min2 (det_min2),
	.det_idx1 (det_idx1),
	.det_idx2 (det_idx2),
	.det_l    (det_l),
	.det_r    (det_r),
	.vout     (det_vout),
	.vout_m1  (det_vout_m1)
);


//==========================================================================
// Disp Fraction Calculation
//==========================================================================
bm_calc_frac bm_calc_frac (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Input
	.det_min (det_min1),
	.det_idx (det_idx1),
	.det_l   (det_l),
	.det_r   (det_r),
	.vin     (det_vout),
	
	// Output
	.frac_out (frac_dout),
	.vout     (frac_vout)
);

// delay matching for uniqueness filter path
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		for (i = 0; i < 2; i = i + 1) begin
			frac_dout_r[i] <= 8'b0;
		end
	end
	else begin
		for (i = 1; i < 2; i = i + 1) begin
			frac_dout_r[i] <= frac_dout_r[i-1];
		end
		frac_dout_r[0] <= frac_dout;
	end
end


//==========================================================================
// SAD Update and Uniqueness Filter
//==========================================================================
assign upd_mode = (op_type == 3);

bm_calc_upd bm_calc_upd (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Control
	.mode   (upd_mode),
	.dphase (dphase),
	
	// DET Input
	.det_min1 (det_min1),
	.det_min2 (det_min2),
	.det_idx1 (det_idx1),
	.det_idx2 (det_idx2),
	.vin_m1   (det_vout_m1),
	
	// IBUF SAD I/F
	.sad_rdaddr (sad_rdaddr),
	.sad_din    (sad_din),
	
	// Output
	.upd       (upd),
	.upd_min1  (upd_min1),
	.upd_min2  (upd_min2),
	.upd_disp1 (upd_disp1),
	.upd_disp2 (upd_disp2),
	.upd_frac  (upd_frac),
	.vout      (upd_vout)
);

assign uni_on = last_dphase; // uniqueness filter works only on the last dphase

bm_calc_uni bm_calc_uni (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Control
	.uni_on (uni_on),
	
	// Input
	.upd_in   (upd),
	.min1_in  (upd_min1),
	.min2_in  (upd_min2),
	.disp1_in (upd_disp1),
	.disp2_in (upd_disp2),
	.frac_in  (upd_frac),
	.vin      (upd_vout),
	
	// Output
	.upd_out   (uni_upd),
	.min1_out  (uni_min1),
	.min2_out  (uni_min2),
	.disp1_out (uni_disp1),
	.disp2_out (uni_disp2),
	.frac_out  (uni_frac),
    .uni_ratio (uni_ratio),
    .vout      (uni_vout)
);

// update fraction part
assign uni_frac_upd[7:0] = (uni_upd) ? frac_dout_r[1][7:0] : uni_frac[7:0];

// apply uniqueness filter
assign uni_act = (uni_enb & last_dphase & (uni_ratio[9:0] > uni_thr[9:0]));

assign uni_disp1_mask[7:0] = (
	(~uni_act)  ? uni_disp1[7:0] :
	(~uni_mode) ? 8'h00 :
	              8'hFF
);

assign uni_frac_mask[7:0] = (
	(~uni_act)  ? uni_frac_upd[7:0] :
	(~uni_mode) ? 8'h00 :
	              8'hFF
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		uni_vout_r <= 1'b0;
	end
	else begin
		uni_vout_r <= uni_vout;
	end
end


//==========================================================================
// OBUF Data Generation
//--------------------------------------------------------------------------
// 2:1 convertion is required in the last dphase.
//==========================================================================
reg		[9:0]	hcnt;
reg		[8:0]	vcnt;
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		hcnt <= 10'b0;
	end
	else begin
		if (last_dphase) begin
			if (sad_line_end) begin
				hcnt <= 0;
			end
			else if (uni_vout) begin
				hcnt <= hcnt + 1'b1;
			end
		end
		else begin
			hcnt <= 0;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vcnt <= 9'b0;
	end
	else begin
		if (last_dphase) begin
			if (sad_line_end) begin
				if (last_line) begin
					vcnt <= 0;
				end
				else begin
					vcnt <= vcnt + 1'b1;
				end
			end
		end
		else begin
			vcnt <= 0;
		end
	end
end

assign obuf_word1[31:0] = {
	uni_disp1_mask[7:0],
	//uni_disp1[7:0],
	//hcnt[7:0],
	//vcnt[7:0],
	uni_frac_mask[7:0],
	uni_min1[15:0]
};

assign obuf_word2[31:0] = {
	uni_disp2[7:0],
	8'b0,
	uni_min2[15:0]
};

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		obuf_wr_phase <= 1'b0;
	end
	else begin
		if (start) begin
			obuf_wr_phase <= 1'b0;
		end
		else if (last_dphase & uni_vout) begin
			obuf_wr_phase <= ~obuf_wr_phase;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		obuf_word1_r <= 32'b0;
	end
	else begin
		if (uni_vout & last_dphase & ~obuf_wr_phase) begin
			obuf_word1_r <= obuf_word1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		obuf_wrdata_i <= 66'b0;
	end
	else begin
		if (uni_vout) begin
			obuf_wrdata_i <= {last_dphase, last_line, obuf_word1[31:0], obuf_word2[31:0]};
		end
	end
end

assign obuf_wrdata[65:64] = obuf_wrdata_i[65:64];
assign obuf_wrdata[63:32] = (~last_dphase) ? obuf_wrdata_i[63:32] : obuf_word1_r[31:0];
assign obuf_wrdata[31: 0] = (~last_dphase) ? obuf_wrdata_i[31: 0] : obuf_word1[31:0];

//assign obuf_wr = (~last_dphase) ? uni_vout_r : (uni_vout & obuf_wr_phase); // org
assign obuf_wr = (~last_dphase) ? uni_vout_r : 1'b0; // obuf2

assign sad_line_end = uni_vout_r & ~uni_vout;

assign sad_done = (op_type == 3) ? sad_line_end : 1'b0;

// added
assign obuf2_wr = (~last_dphase) ? 1'b0 : uni_vout;
//assign obuf2_wr = 1'b0;
assign obuf2_wrdata[15: 0] = (~last_dphase) ? 16'b0 : obuf_word1[31:16];



endmodule
