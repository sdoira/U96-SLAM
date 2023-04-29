//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module rect_ibuf (
	// Global Control
	rst_n, clk,
	
	// Control
	enb,
	start_pclk,
	frm_end,
	
	// Sensor Input
	pclk,
	href,
	d_l,
	d_r,
	
	// Internal I/F
	rdy,
	upd,
	lr_sel,
	rdaddr,
	rddata_up,
	rddata_lo
);


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input			rst_n, clk;

// Control
input			enb;
input			start_pclk;
input			frm_end;

// Sensor Input
input			pclk;
input			href;
input	[7:0]	d_l, d_r;

// Internal I/F
output			rdy, upd;
input			lr_sel;
input	[9:0]	rdaddr;
output	[7:0]	rddata_up, rddata_lo;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Write Port Control (PCLK Domain)
wire	[15:0]	din;

//reg				href_r;
reg		[10:0]	href_cnt;
wire	[10:0]	width_m1;
wire			href_end;
reg				href_end_r1, href_end_r2;

wire			href_neg;
wire			frm_end_pclk;
reg				on;
reg				phase;
reg		[2:0]	msel_p;
wire	[2:0]	wr;
reg		[9:0]	wraddr;
reg				hcnt;
reg				rdy_p, upd_p;

// Clock Domain Crossing (PCLK -> CLK)
reg		[2:0]	msel;
reg				rdy;
wire			upd;

// Line Buffer Instance
wire	[9:0]	rdaddr_0, rdaddr_1, rdaddr_2;
wire	[15:0]	dout_0, dout_1, dout_2;

// Read Port Control
wire	[15:0]	rddata_up_lr, rddata_lo_lr;
wire	[7:0]	rddata_up, rddata_lo;


//==========================================================================
// Write Port Control (PCLK Domain)
//==========================================================================
assign din[15:0] = {d_l[7:0], d_r[7:0]};
/*
always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		href_r  <= 1'b0;
	end
	else begin
		if (enb) begin
			href_r  <= href;
		end
		else begin
			href_r  <= 1'b0;
		end
	end
end
*/
always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		href_cnt  <= 11'b0;
	end
	else begin
		if (enb) begin
			if (href) begin
				if (href_end) begin
					href_cnt <= 0;
				end
				else begin
					href_cnt <= href_cnt + 1'b1;
				end
			end
		end
		else begin
			href_cnt  <= 0;
		end
	end
end

assign width_m1[10:0] = 11'd639;
assign href_end = (href_cnt == width_m1[10:0]);

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		href_end_r2 <= 1'b0;
		href_end_r1 <= 1'b0;
	end
	else begin
		href_end_r2 <= href_end_r1;
		href_end_r1 <= href_end;
	end
end

//assign href_neg  = (~href &  href_r );
assign href_neg = (~href_end_r2 & href_end_r1);

// CLK -> PCLK
clkx clkx_frm_end (
	.rst_n (rst_n),
	.clk1  (clk),  .in  (frm_end),
	.clk2  (pclk), .out (frm_end_pclk)
);

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		on <= 1'b0;
	end
	else begin
		if (enb) begin
			if (start_pclk) begin
				on <= 1'b1;
			end
			else if (frm_end_pclk) begin
				on <= 1'b0;
			end
		end
		else begin
			on <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		phase <= 1'b0;
	end
	else begin
		if (on & href) begin
			phase <= ~phase;
		end
		else begin
			phase <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		msel_p <= 3'b0;
	end
	else begin
		if (on) begin
			if (msel_p == 3'b000) begin
				msel_p <= 3'b001;
			end
			else if (href_neg) begin
				msel_p <= {msel_p[1:0], msel_p[2]};
			end
		end
		else begin
			msel_p <= 0;
		end
	end
end

//assign wr[2:0] = (enb & on & href & phase) ? msel_p[2:0] : 3'b0; // UYVY order
assign wr[2:0] = (enb & on & href) ? msel_p[2:0] : 3'b0;

/*
always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		wraddr <= 10'b0;
	end
	else begin
		if (on & href) begin
			if (|wr[2:0]) begin
				wraddr <= wraddr + 1'b1;
			end
		end
		else begin
			wraddr <= 0;
		end
	end
end
*/
always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		wraddr <= 10'b0;
	end
	else begin
		if (on) begin
			if (href_neg) begin
				wraddr <= 0;
			end
			else if (|wr[2:0]) begin
				wraddr <= wraddr + 1'b1;
			end
		end
		else begin
			wraddr <= 0;
		end
	end
end

// counts only the first line
always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		hcnt <= 1'b0;
	end
	else begin
		if (on) begin
			if (href_neg) begin
				hcnt <= 1'b1;
			end
		end
		else begin
			hcnt <= 0;
		end
	end
end

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		rdy_p <= 1'b0;
	end
	else begin
		if (on) begin
			if (href_neg & (hcnt == 1)) begin
				rdy_p <= 1'b1;
			end
		end
		else begin
			rdy_p <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		upd_p <= 1'b0;
	end
	else begin
		if (on) begin
			if (rdy_p & href_neg) begin
				upd_p <= 1'b1;
			end
			else begin
				upd_p <= 1'b0;
			end
		end
		else begin
			upd_p <= 1'b0;
		end
	end
end


//==========================================================================
// Clock Domain Crossing (PCLK -> CLK)
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		msel <= 3'b0;
	end
	else begin
		msel <= msel_p;
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		rdy <= 1'b0;
	end
	else begin
		//if (~on) begin // 'on' is in PCLK domain
		if (frm_end) begin
			rdy <= 1'b0;
		end
		else begin
			rdy <= rdy_p;
		end
	end
end

clkx2 clkx2_upd (
	.rst_n (rst_n),
	.clk1 (pclk), .in  (upd_p),
	.clk2 (clk),  .out (upd)
);


//==========================================================================
// Line Buffer Instance
//==========================================================================
rect_line_buf buf_0 (
	.clka  (pclk),
	.ena   (1'b1),
	.wea   (wr[0]),
	.addra (wraddr[9:0]),
	.dina  (din[15:0]),
	.clkb  (clk),
	.enb   (1'b1),
	.addrb (rdaddr[9:0]),
	.doutb (dout_0[15:0])
);

rect_line_buf buf_1 (
	.clka  (pclk),
	.ena   (1'b1),
	.wea   (wr[1]),
	.addra (wraddr[9:0]),
	.dina  (din[15:0]),
	.clkb  (clk),
	.enb   (1'b1),
	.addrb (rdaddr[9:0]),
	.doutb (dout_1[15:0])
);

rect_line_buf buf_2 (
	.clka  (pclk),
	.ena   (1'b1),
	.wea   (wr[2]),
	.addra (wraddr[9:0]),
	.dina  (din[15:0]),
	.clkb  (clk),
	.enb   (1'b1),
	.addrb (rdaddr[9:0]),
	.doutb (dout_2[15:0])
);


//==========================================================================
// Read Port Control
//==========================================================================
assign rddata_up_lr[15:0] = (
	(msel[2]) ? dout_0[15:0] :
	(msel[0]) ? dout_1[15:0] :
	(msel[1]) ? dout_2[15:0] :
	            16'b0
);

assign rddata_lo_lr[15:0] = (
	(msel[2]) ? dout_1[15:0] :
	(msel[0]) ? dout_2[15:0] :
	(msel[1]) ? dout_0[15:0] :
	            16'b0
);

assign rddata_up[7:0] = (~lr_sel) ? rddata_up_lr[15:8] : rddata_up_lr[7:0];
assign rddata_lo[7:0] = (~lr_sel) ? rddata_lo_lr[15:8] : rddata_lo_lr[7:0];


endmodule
