//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1ns / 1ps

module frame_meas (
	// Global Control
	rst_n, clk,
	
	// Internal Bus I/F
	ibus_cs,
	ibus_wr,
	ibus_addr_7_2,
	ibus_wrdata,
	ibus_rddata,
	
	// DVP I/F
	pclk,
	href,
	vsync
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

// DVP I/F
input			pclk;
input			href, vsync;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Register Interface
reg		[1:0]	pol;

// Measurement Control
reg				meas_trig;
wire			meas_trig_p;
reg				meas_wait, meas_on;
reg				cmpl;

// Frame Measurement
wire			href_pol, vsync_pol;
reg				href_1, vsync_1;
wire			h_pos, v_pos;
reg		[2:0]	state;
wire			cmpl_p;
reg		[23:0]	v_period;
reg		[13:0]	h_period, v_active;
reg		[13:0]	h_active;
reg		[23:0]	h_begin;

// Reference
reg		[2:0]	vsync_i;
wire			v_pos_i;
reg		[1:0]	state_i;
reg		[31:0]	v_period_ref;
reg		[15:0]	dbg_cnt;


//==========================================================================
// Register Interface
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		pol <= 2'b0;
	end
	else begin
		if (ibus_cs & ibus_wr) begin
			case (ibus_addr_7_2)
				6'h00: pol <= ibus_wrdata[2:1];
				default: begin
					pol <= pol;
				end
			endcase
		end
	end
end

assign ibus_rddata[31:0] = (
	(~ibus_cs)              ?  32'b0 :
	(ibus_addr_7_2 == 6'h0) ? {29'b0, pol[1:0], 1'b0} :
	(ibus_addr_7_2 == 6'h1) ? {31'b0, cmpl} :
	(ibus_addr_7_2 == 6'h2) ? { 8'b0, v_period[23:0]} :
	(ibus_addr_7_2 == 6'h3) ? {18'b0, h_period[13:0]} :
	(ibus_addr_7_2 == 6'h4) ? {18'b0, v_active[13:0]} :
	(ibus_addr_7_2 == 6'h5) ? {18'b0, h_active[13:0]} :
	(ibus_addr_7_2 == 6'h6) ? { 8'b0, h_begin[23:0]} :
	(ibus_addr_7_2 == 6'h7) ?  v_period_ref[31:0] :
	(ibus_addr_7_2 == 6'h8) ? {16'b0, dbg_cnt[15:0]} :
	                           32'b0
);


//==========================================================================
// Measurement Control
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		meas_trig <= 1'b0;
	end
	else begin
		if (ibus_cs & ibus_wr & (ibus_addr_7_2 == 6'h00)) begin
			meas_trig <= ibus_wrdata[0];
		end
		else begin
			meas_trig <= 1'b0;
		end
	end
end

clkx clkx_meas_trig (
	.rst_n (rst_n),
	.clk1  (clk),  .in  (meas_trig),
	.clk2  (pclk), .out (meas_trig_p)
);

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		meas_wait <= 1'b0;
	end
	else begin
		if (meas_trig_p) begin
			meas_wait <= 1'b1;
		end
		else if (meas_on) begin
			meas_wait <= 1'b0;
		end
	end
end

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		meas_on <= 1'b0;
	end
	else begin
		if (meas_on) begin
			if (v_pos) begin
				meas_on <= 1'b0;
			end
		end
		else begin
			if (meas_wait & v_pos) begin
				meas_on <= 1'b1;
			end
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		cmpl <= 1'b0;
	end
	else begin
		if (meas_trig) begin
			cmpl <= 1'b0;
		end
		else if (cmpl_p) begin
			cmpl <= 1'b1;
		end
	end
end


//==========================================================================
// Frame Measurement
//==========================================================================
assign href_pol  = (pol[0]) ? ~href  : href;
assign vsync_pol = (pol[1]) ? ~vsync : vsync;

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		href_1  <= 1'b0;
		vsync_1 <= 1'b0;
	end
	else begin
		href_1  <= href_pol;
		vsync_1 <= vsync_pol;
	end
end

assign h_pos = (href_pol & ~href_1);
assign v_pos = (vsync_pol & ~vsync_1);

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		state <= 3'b0;
	end
	else begin
		case (state)
			0: if (meas_trig_p)       state <= 1;
			1: if (meas_wait & v_pos) state <= 2;
			2: if (h_pos)             state <= 3;
			3: if (h_pos)             state <= 4;
			4: if (v_pos)             state <= 5;
			5:                        state <= 0;
			default:                  state <= state;
		endcase
	end
end

assign cmpl_p = (state == 5);

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		v_period <= 24'b0;
	end
	else begin
		if (meas_trig_p) begin
			v_period <= 0;
		end
		else if (meas_on) begin
			v_period <= v_period + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		h_period <= 14'b0;
	end
	else begin
		if (meas_trig_p) begin
			h_period = 0;
		end
		else if (state == 3) begin
			h_period <= h_period + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		v_active <= 14'b0;
	end
	else begin
		if (meas_trig_p) begin
			v_active = 0;
		end
		else if (meas_on & h_pos) begin
			v_active <= v_active + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		h_active <= 14'b0;
	end
	else begin
		if (meas_trig_p) begin
			h_active = 0;
		end
		else if ((state == 3) & href_pol) begin
			h_active <= h_active + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge pclk) begin
	if (~rst_n) begin
		h_begin <= 24'b0;
	end
	else begin
		if (meas_trig_p) begin
			h_begin = 0;
		end
		else if (state == 2) begin
			h_begin <= h_begin + 1'b1;
		end
	end
end


//==========================================================================
// Reference
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		vsync_i <= 3'b0;
	end
	else begin
		vsync_i <= {vsync_i[1:0], vsync_pol};
	end
end

assign v_pos_i = (~vsync_i[2] & vsync_i[1]);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		state_i <= 2'b0;
	end
	else begin
		case (state_i)
			0: if (meas_trig) state_i = 1;
			1: if (v_pos_i)   state_i = 2;
			2: if (v_pos_i)   state_i = 0;
			default:          state_i = state_i;
		endcase
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		v_period_ref <= 32'b0;
	end
	else begin
		if (meas_trig) begin
			v_period_ref = 0;
		end
		else if (state_i == 2) begin
			v_period_ref <= v_period_ref + 1'b1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		dbg_cnt <= 16'b0;
	end
	else begin
		dbg_cnt <= dbg_cnt + 1'b1;
	end
end



endmodule
