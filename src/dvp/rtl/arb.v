//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module arb (
	// Global Control
	rst_n, clk,
	
	// Internal Bus I/F
	ibus_cs,
	ibus_wr,
	ibus_addr_7_2,
	ibus_wrdata,
	ibus_rddata,
	
	// Status
	done,
	
	// FIFO I/F
	req,
	ack,
	valid_in,
	data_in,
	strb_in,
	valid_out,
	data_out,
	
	// AXI I/F
	axi_aclk,
	axi_awaddr,
	axi_awlen,
	axi_awvalid,
	axi_awready,
	axi_wdata,
	axi_wstrb,
	axi_wvalid,
	axi_wready,
	axi_wlast,
	axi_araddr,
	axi_arlen,
	axi_arvalid,
	axi_arready,
	axi_rdata,
	axi_rvalid,
	axi_rready,
	axi_rlast,
	axi_bvalid,
	axi_bready
);

//==========================================================================
// Parameter Settings
//==========================================================================
parameter NR = 8; // number of requests


//==========================================================================
// Port Declaration
//==========================================================================
// Global Control
input				rst_n, clk;

// Internal Bus I/F
input				ibus_cs;
input				ibus_wr;
input	[5:0]		ibus_addr_7_2;
input	[31:0]		ibus_wrdata;
output	[31:0]		ibus_rddata;

// Status
output	[NR-1:0]	done;

// FIFO I/F
input	[NR-1:0]	req;
output	[NR-1:0]	ack;
input				valid_in;
input	[31:0]		data_in;
input	[3:0]		strb_in;
output				valid_out;
output	[31:0]		data_out;

// AXI I/F
input				axi_aclk;
output	[31:0]		axi_awaddr;
output	[7:0]		axi_awlen;
output				axi_awvalid;
input				axi_awready;
output	[31:0]		axi_wdata;
output	[3:0]		axi_wstrb;
output				axi_wvalid;
input				axi_wready;
output				axi_wlast;
output	[31:0]		axi_araddr;
output	[7:0]		axi_arlen;
output				axi_arvalid;
input				axi_arready;
input	[31:0]		axi_rdata;
input				axi_rvalid;
output				axi_rready;
input				axi_rlast;
input				axi_bvalid;
output				axi_bready;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Register Interface
reg		[3:0]		ctrl;
wire	[31:0]		ibus_rddata;
wire				fifo_enb;

// Arbiter
reg		[NR-1:0]	ack;
wire				busy;
reg		[NR-1:0]	ack_r;

// FIFO Instance
wire				fifo_wr;
wire	[35:0]		fifo_din;
wire	[35:0]		fifo_dout;
wire				fifo_full;
wire				fifo_empty;
reg					fifo_ovf;
reg					fifo_udf;

// FIFO Read
wire				fifo_rd;
reg		[3:0]		state;
wire				last_bit, rd_wrn;
wire	[7:0]		bst_len_m1;
reg					last_bit_r, rd_wrn_r;
reg		[7:0]		bst_len_m1_r;
reg		[7:0]		bst_cnt;
wire				bst_end;
reg		[NR-1:0]	done;

// AXI Write Interface
wire				axi_awvalid;
wire	[31:0]		axi_awaddr;
wire				axi_wvalid;
wire	[3:0]		axi_wstrb;
wire	[31:0]		axi_wdata;
wire	[7:0]		axi_awlen;
wire				axi_wlast;
reg					axi_bready;

// AXI Read Interface
wire				axi_arvalid;
wire	[31:0]		axi_araddr;
wire				axi_rready;
wire	[7:0]		axi_arlen;
wire	[31:0]		data_out;
wire				valid_out;


//==========================================================================
// Register Interface
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ctrl <= 4'b0;
	end
	else begin
		if (ibus_cs & ibus_wr) begin
			case (ibus_addr_7_2)
				6'h00: ctrl <= ibus_wrdata[3:0];
				default: begin
					ctrl <= ctrl;
				end
			endcase
		end
	end
end

assign ibus_rddata[31:0] = (
	(~ibus_cs)              ?  32'b0 :
	(ibus_addr_7_2 == 6'h0) ? {28'b0, ctrl[3:0]} :
	(ibus_addr_7_2 == 6'h1) ? {30'b0, fifo_udf, fifo_ovf} :
	                           32'b0
);

assign fifo_enb = ctrl[0];


//==========================================================================
// Arbiter
//--------------------------------------------------------------------------
// [7]: gftt_dwr
// [6]: gftt_drd
// [5]: xsblw
// [4]: rect
// [3]: dwr
// [2]: drd
// [1]: xsblr
// [0]: grb
//==========================================================================
always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ack <= {NR{1'b0}};
	end
	else begin
		if (~busy) begin
			if      (req[0]) ack[0] <= 1'b1; // grb
			else if (req[4]) ack[4] <= 1'b1; // rect
			else if (req[5]) ack[5] <= 1'b1; // xsblw
			else if (req[1]) ack[1] <= 1'b1; // xsblr
			else if (req[3]) ack[3] <= 1'b1; // dwr
			else if (req[2]) ack[2] <= 1'b1; // drd
			else if (req[7]) ack[7] <= 1'b1; // gftt_dwr
			else if (req[6]) ack[6] <= 1'b1; // gftt_drd
		end
		else if (ack[0]) ack[0] <= req[0];
		else if (ack[1]) ack[1] <= req[1];
		else if (ack[2]) ack[2] <= req[2];
		else if (ack[3]) ack[3] <= req[3];
		else if (ack[4]) ack[4] <= req[4];
		else if (ack[5]) ack[5] <= req[5];
		else if (ack[6]) ack[6] <= req[6];
		else if (ack[7]) ack[7] <= req[7];
	end
end

assign busy = |ack[NR-1:0] | (state != 0);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		ack_r <= {NR{1'b0}};
	end
	else begin
		if (state == 1) begin
			ack_r <= ack;
		end
	end
end


//==========================================================================
// FIFO Instance
//==========================================================================
assign fifo_wr  = valid_in;
assign fifo_din = {strb_in, data_in};

fifo_frm fifo_frm (
	.clk   (clk),
	.srst  (~fifo_enb),
	.din   (fifo_din),
	.wr_en (fifo_wr),
	.rd_en (fifo_rd),
	.dout  (fifo_dout),
	.full  (fifo_full),
	.empty (fifo_empty),
	.wr_rst_busy (),
	.rd_rst_busy ()
);

always @(negedge rst_n or posedge axi_aclk) begin
	if (~rst_n) begin
		fifo_ovf <= 1'b0;
	end
	else begin
		if (~fifo_enb) begin
			fifo_ovf <= 1'b0;
		end
		else if (fifo_full & fifo_wr) begin
			fifo_ovf <= 1'b1;
		end
	end
end

always @(negedge rst_n or posedge axi_aclk) begin
	if (~rst_n) begin
		fifo_udf <= 1'b0;
	end
	else begin
		if (~fifo_enb) begin
			fifo_udf <= 1'b0;
		end
		else if (fifo_empty & fifo_rd) begin
			fifo_udf <= 1'b1;
		end
	end
end


//==========================================================================
// FIFO Read
//==========================================================================
assign fifo_rd = (
	(state == 0) ? ~fifo_empty :
	(state == 1) ? 1'b1 :
	(state == 2) ? axi_awvalid & axi_awready :
	(state == 3) ? axi_awvalid & axi_awready :
	(state == 4) ? axi_wvalid & axi_wready :
	               1'b0
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		state <= 4'b0;
	end
	else begin
		case (state)
			0: if (~fifo_empty)               state <= 1;
			1:                                state <= 2;
			2: begin
				if (~rd_wrn_r) begin
					state <= (~axi_awready) ? 3 : 4;
				end
				else begin
					state <= (~axi_arready) ? 7 : 8;
				end
			end
			3: if (axi_awvalid & axi_awready) state <= 4;
			4: if (bst_end)                   state <= 5;
			5: if (axi_wvalid & axi_wready)   state <= 6;
			6: if (axi_bvalid & axi_bready)   state <= 0;
			7: if (axi_arvalid & axi_arready) state <= 8;
			8: if (bst_end)                   state <= 0;
		endcase
	end
end

assign last_bit   = fifo_dout[9];
assign rd_wrn     = fifo_dout[8];
assign bst_len_m1 = fifo_dout[7:0];

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		last_bit_r   <= 1'b0;
		rd_wrn_r     <= 1'b0;
		bst_len_m1_r <= 8'b0;
	end
	else begin
		if (state == 1) begin
			last_bit_r   <= last_bit;
			rd_wrn_r     <= rd_wrn;
			bst_len_m1_r <= bst_len_m1;
		end
	end
end

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		bst_cnt <= 8'b0;
	end
	else begin
		if (~fifo_enb | (state == 0)) begin
			bst_cnt <= 0;
		end
		else if (
			((state == 2) & axi_awvalid & axi_awready) |
			(((state == 3) | (state == 4)) & fifo_rd) |
			((state == 8) & axi_rvalid & axi_rready)
		) begin
			if (bst_end) begin
				bst_cnt <= 0;
			end
			else begin
				bst_cnt <= bst_cnt + 1'b1;
			end
		end
	end
end

assign bst_end = (
	((state == 4) | (state == 8)) & (bst_cnt == bst_len_m1_r)
);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		done <= {NR{1'b0}};
	end
	else begin
		if (
			((state == 6) & last_bit_r & axi_bvalid & axi_bready) ||
			((state == 8) & last_bit_r & bst_end)
		) begin
			done <= ack_r;
		end
		else begin
			done <= {NR{1'b0}};
		end
	end
end


//==========================================================================
// AXI Write Interface
//==========================================================================
assign axi_awvalid      = ~rd_wrn_r & (state == 2) | (state == 3);
assign axi_awaddr[31:0] = (axi_awvalid) ? fifo_dout[31:0] : 32'b0;
assign axi_wvalid       = (state == 4) | (state == 5);
assign axi_wstrb[3:0]   = (axi_wvalid) ? fifo_dout[35:32] : 4'hF;
assign axi_wdata[31:0]  = (axi_wvalid) ? fifo_dout[31: 0] : 32'b0;
assign axi_awlen[7:0]   = bst_len_m1_r[7:0];
assign axi_wlast        = (state == 5);

always @(negedge rst_n or posedge clk) begin
	if (~rst_n) begin
		axi_bready <= 1'b0;
	end
	else begin
		if (axi_bready) begin
			axi_bready <= 1'b0;
		end
		else if (axi_bvalid) begin
			axi_bready <= 1'b1;
		end
	end
end


//==========================================================================
// AXI Read Interface
//==========================================================================
assign axi_arvalid      = rd_wrn_r & (state == 2) | (state == 7);
assign axi_araddr[31:0] = (axi_arvalid) ? fifo_dout[31:0] : 32'b0;
assign axi_rready       = (state == 8);
assign axi_arlen[7:0]   = bst_len_m1_r[7:0];

assign data_out[31:0]  = axi_rdata[31:0];
assign valid_out = axi_rready & axi_rvalid;


endmodule
