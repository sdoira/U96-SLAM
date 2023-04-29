
`timescale 1 ns / 1 ps

module dummy_rect (
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
	rect_fcnt,
	bm_done,
	bm_enb,
	
	// Sensor Input
	pclk,
	vsync,
	href,
	d_l,
	d_r,
	
	// DDR I/F
	ddr_req,
	ddr_ack,
	ddr_dout,
	ddr_strb,
	ddr_vout
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
output	[3:0]	rect_fcnt;
input			bm_done;
input			bm_enb;

// Sensor Input
input			pclk, vsync, href;
input	[7:0]	d_l, d_r;

// DDR I/F
output			ddr_req;
input			ddr_ack;
output	[31:0]	ddr_dout;
output	[3:0]	ddr_strb;
output			ddr_vout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
reg		[9:0]	base_a, base_b;


//==========================================================================
// Dummy Output
//==========================================================================
assign ibus_rddata[31:0] = 32'b0;
assign rect_fcnt[3:0]    =  4'b0;
assign ddr_req           =  1'b0;
assign ddr_dout[31:0]    = 32'b0;
assign ddr_strb[3:0]     =  4'hF;
assign ddr_vout          =  1'b0;


endmodule
