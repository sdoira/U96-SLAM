
`timescale 1 ns / 1 ps

module dummy_xsbl2 (
	// Global Control
	rst_n, clk,
	
	// Internal Bus I/F
	ibus_cs,
	ibus_wr,
	ibus_addr_7_2,
	ibus_wrdata,
	ibus_rddata,
	
	// Control
	rect_done,
	xsbl_done,
	rect_fcnt,
	xsbl_fcnt,
	
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
input			rect_done;
input			xsbl_done;
input	[3:0]	rect_fcnt;
output	[3:0]	xsbl_fcnt;

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
reg				xsbl_sim_start;
reg		[9:0]	drd_base_a, drd_base_b;
reg		[9:0]	dwr_base_a, dwr_base_b;


//==========================================================================
// Dummy Output
//==========================================================================
assign ibus_rddata[31:0] = 32'b0;
assign drd_req  =  1'b0;
assign drd_vout =  1'b0;
assign drd_dout = 32'b0;
assign dwr_req  =  1'b0;
assign dwr_vout =  1'b0;
assign dwr_dout = 32'b0;
assign dwr_strb =  4'hF;


endmodule
