
`timescale 1 ns / 1 ps

module dummy_bm (
	// Global Control
	rst_n, clk,
	
	// Internal Bus I/F
	ibus_cs,
	ibus_wr,
	ibus_addr_7_2,
	ibus_wrdata,
	ibus_rddata,
	
	// Control
	xsbl_done,
	bm_done,
	xsbl_fcnt,
	bm_fcnt,
	enb,
	
	// DDR Read Arbiter I/F
	drd_req,
	drd_ack,
	drd_vin,
	drd_din,
	drd_vout,
	drd_dout,
	
	// DDR Write Arbiter I/F
	dwr_req,
	dwr_ack,
	dwr_vout,
	dwr_dout,
	dwr_strb
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

// Control
input			xsbl_done, bm_done;
input	[3:0]	xsbl_fcnt;
output	[3:0]	bm_fcnt;
output			enb;

// DDR Read Arbiter I/F
output			drd_req;
input			drd_ack;
input			drd_vin;
input	[31:0]	drd_din;
output			drd_vout;
output	[31:0]	drd_dout;

// DDR Write Arbiter I/F
output			dwr_req;
input			dwr_ack;
output			dwr_vout;
output	[31:0]	dwr_dout;
output	[3:0]	dwr_strb;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
reg		[8:0]	lr_addr_a, lr_addr_b;
wire	[6:0]	sad_addr_a, sad_addr_b;
wire	[9:0]	sad_wdt;
wire	[8:0]	sad_hgt;
wire			bm_start;
reg				bm_sim_start;
reg		[8:0]	disp2_addr_a, disp2_addr_b;
reg		[8:0]	hgt;
reg		[9:0]	wdt;


//==========================================================================
// Dummy Output
//==========================================================================
assign ibus_rddata = 32'b0;
assign bm_fcnt     =  4'b0;
assign enb         =  1'b0;
assign drd_req     =  1'b0;
assign drd_vout    =  1'b0;
assign drd_dout    = 32'b0;
assign dwr_req     =  1'b0;
assign dwr_vout    =  1'b0;
assign dwr_dout    = 32'b0;
assign dwr_strb    =  4'hF;


endmodule
