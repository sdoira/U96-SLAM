//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

//`define USE_DUMMY_RECT
//`define USE_DUMMY_XSBL2
//`define USE_DUMMY_BM
//`define USE_DUMMY_GFTT

module dvp # (
	// Parameters of Axi Slave Bus Interface S00_AXI
	parameter integer C_S_AXI_DATA_WIDTH = 32,
	parameter integer C_S_AXI_ADDR_WIDTH = 14,
	
	// Parameters of Axi Master Bus Interface M00_AXI
	parameter  C_M00_AXI_TARGET_SLAVE_BASE_ADDR = 32'h40000000,
	parameter integer C_M00_AXI_BURST_LEN    = 16,
	parameter integer C_M00_AXI_ID_WIDTH     =  1,
	parameter integer C_M00_AXI_ADDR_WIDTH   = 32,
	parameter integer C_M00_AXI_DATA_WIDTH   = 32,
	parameter integer C_M00_AXI_AWUSER_WIDTH =  0,
	parameter integer C_M00_AXI_ARUSER_WIDTH =  0,
	parameter integer C_M00_AXI_WUSER_WIDTH  =  0,
	parameter integer C_M00_AXI_RUSER_WIDTH  =  0,
	parameter integer C_M00_AXI_BUSER_WIDTH  =  0
) (
	// CSI Receiver Control
	vrst_n,
	
	// AXI Stream Video Input (Ch1)
	v1_tvalid,
	v1_tready,
	v1_tuser,
	v1_tlast,
	v1_tdata,
	v1_tdest,
	
	// AXI Stream Video Input (Ch2)
	v2_tvalid,
	v2_tready,
	v2_tuser,
	v2_tlast,
	v2_tdata,
	v2_tdest,
	
	// Debug
	led,
	intr,
	gpio_in,
	gpio_out,
	
	// AXI Interface
	s00_axi_aclk,
	s00_axi_aresetn,
	s00_axi_awaddr,
	s00_axi_awprot,
	s00_axi_awvalid,
	s00_axi_awready,
	s00_axi_wdata,
	s00_axi_wstrb,
	s00_axi_wvalid,
	s00_axi_wready,
	s00_axi_bresp,
	s00_axi_bvalid,
	s00_axi_bready,
	s00_axi_araddr,
	s00_axi_arprot,
	s00_axi_arvalid,
	s00_axi_arready,
	s00_axi_rdata,
	s00_axi_rresp,
	s00_axi_rvalid,
	s00_axi_rready,
	
	m00_axi_aclk,
	m00_axi_aresetn,
	m00_axi_awid,
	m00_axi_awaddr,
	m00_axi_awlen,
	m00_axi_awsize,
	m00_axi_awburst,
	m00_axi_awlock,
	m00_axi_awcache,
	m00_axi_awprot,
	m00_axi_awqos,
	m00_axi_awuser,
	m00_axi_awvalid,
	m00_axi_awready,
	m00_axi_wdata,
	m00_axi_wstrb,
	m00_axi_wlast,
	m00_axi_wuser,
	m00_axi_wvalid,
	m00_axi_wready,
	m00_axi_bid,
	m00_axi_bresp,
	m00_axi_buser,
	m00_axi_bvalid,
	m00_axi_bready,
	m00_axi_arid,
	m00_axi_araddr,
	m00_axi_arlen,
	m00_axi_arsize,
	m00_axi_arburst,
	m00_axi_arlock,
	m00_axi_arcache,
	m00_axi_arprot,
	m00_axi_arqos,
	m00_axi_aruser,
	m00_axi_arvalid,
	m00_axi_arready,
	m00_axi_rid,
	m00_axi_rdata,
	m00_axi_rresp,
	m00_axi_rlast,
	m00_axi_ruser,
	m00_axi_rvalid,
	m00_axi_rready
);


//==========================================================================
// Parameter Declaration
//==========================================================================
parameter VERSION = 32'h0103;


//==========================================================================
// Port Declaration
//==========================================================================
// CSI Receiver Control
output			vrst_n;

// AXI Stream Video Input (Ch1)
input			v1_tvalid;
output			v1_tready;
input			v1_tuser;
input			v1_tlast;
input	[15:0]	v1_tdata;
input	[3:0]	v1_tdest;

// AXI Stream Video Input (Ch2)
input			v2_tvalid;
output			v2_tready;
input			v2_tuser;
input			v2_tlast;
input	[15:0]	v2_tdata;
input	[3:0]	v2_tdest;

// Debug
output	[1:0]	led;
output			intr;
input	[1:0]	gpio_in;
output	[1:0]	gpio_out;

// AXI Interface
input			s00_axi_aclk;
input			s00_axi_aresetn;
input	[13:0]	s00_axi_awaddr;
input	[2:0]	s00_axi_awprot;
input			s00_axi_awvalid;
output			s00_axi_awready;
input	[31:0]	s00_axi_wdata;
input	[3:0]	s00_axi_wstrb;
input			s00_axi_wvalid;
output			s00_axi_wready;
output	[1:0]	s00_axi_bresp;
output			s00_axi_bvalid;
input			s00_axi_bready;
input	[13:0]	s00_axi_araddr;
input	[2:0]	s00_axi_arprot;
input			s00_axi_arvalid;
output			s00_axi_arready;
output	[31:0]	s00_axi_rdata;
output	[1:0]	s00_axi_rresp;
output			s00_axi_rvalid;
input			s00_axi_rready;

input			m00_axi_aclk;
input			m00_axi_aresetn;
output			m00_axi_awid;
output	[31:0]	m00_axi_awaddr;
output	[7:0]	m00_axi_awlen;
output	[2:0]	m00_axi_awsize;
output	[1:0]	m00_axi_awburst;
output			m00_axi_awlock;
output	[3:0]	m00_axi_awcache;
output	[2:0]	m00_axi_awprot;
output	[3:0]	m00_axi_awqos;
output			m00_axi_awuser;
output			m00_axi_awvalid;
input			m00_axi_awready;
output	[31:0]	m00_axi_wdata;
output	[3:0]	m00_axi_wstrb;
output			m00_axi_wlast;
output			m00_axi_wuser;
output			m00_axi_wvalid;
input			m00_axi_wready;
input			m00_axi_bid;
input	[1:0]	m00_axi_bresp;
input			m00_axi_buser;
input			m00_axi_bvalid;
output			m00_axi_bready;
output			m00_axi_arid;
output	[31:0]	m00_axi_araddr;
output	[7:0]	m00_axi_arlen;
output	[2:0]	m00_axi_arsize;
output	[1:0]	m00_axi_arburst;
output			m00_axi_arlock;
output	[3:0]	m00_axi_arcache;
output	[2:0]	m00_axi_arprot;
output	[3:0]	m00_axi_arqos;
output			m00_axi_aruser;
output			m00_axi_arvalid;
input			m00_axi_arready;
input			m00_axi_rid;
input	[31:0]	m00_axi_rdata;
input	[1:0]	m00_axi_rresp;
input			m00_axi_rlast;
input			m00_axi_ruser;
input			m00_axi_rvalid;
output			m00_axi_rready;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// AXI Interface
wire			clk, rst_n;
wire	[1:0]	led;
wire	[1:0]	ptn_sel;
wire	[1:0]	gpio_out;
wire	[11:0]	ibus_addr;
wire	[3:0]	ibus_cs;
wire			ibus_wr;
wire	[31:0]	ibus_wrdata;
wire			ibus_cs_csi, ibus_cs_frm, ibus_cs_mes, ibus_cs_sel;
wire			ibus_cs_grb, ibus_cs_xsbl, ibus_cs_bm, ibus_cs_arb;
wire			ibus_cs_rect, ibus_cs_gftt;
wire	[31:0]	ibus_rddata;
wire			intr;

// CSI Interface
wire	[31:0]	ibus_rddata_csi;
wire			vrst_n;
wire			v1_tready;
wire			v2_tready;
wire			sof_csi, v_csi;
wire	[15:0]	d1_csi, d2_csi;

// Frame Measurement
wire	[31:0]	ibus_rddata_mes;

// grb
wire	[31:0]	ibus_rddata_grb;
wire			grb_req;
wire	[31:0]	grb_dout;
wire			grb_vout;

// rect
wire	[3:0]	rect_fcnt;
wire	[31:0]	ibus_rddata_rect;
wire			rect_req;
wire	[31:0]	rect_dout;
wire	[3:0]	rect_strb;
wire			rect_vout;

// xsbl
wire			xsblr_vin;
wire	[3:0]	xsbl_fcnt;
wire	[31:0]	ibus_rddata_xsbl;
wire			xsblr_req;
wire			xsblr_vout;
wire	[31:0]	xsblr_dout;
wire			xsblw_req;
wire			xsblw_vout;
wire	[31:0]	xsblw_dout;
wire	[3:0]	xsblw_strb;

// bm
wire			drd_vin;
wire			bm_enb;
wire	[31:0]	ibus_rddata_bm;
wire			drd_req;
wire			drd_vout;
wire	[31:0]	drd_dout;
wire			dwr_req;
wire			dwr_vout;
wire	[31:0]	dwr_dout;
wire	[3:0]	dwr_strb;

// gftt
wire			gftt_drd_vin;
wire	[31:0]	ibus_rddata_gftt;
wire			gftt_drd_req;
wire			gftt_drd_vout;
wire	[31:0]	gftt_drd_dout;
wire			gftt_dwr_req;
wire			gftt_dwr_vout;
wire	[31:0]	gftt_dwr_dout;

// arb
wire	[7:0]	arb_req;
wire			arb_vin;
wire	[31:0]	arb_din;
wire	[3:0]	arb_strb;
wire			xsblw_ack, rect_ack, dwr_ack, drd_ack, xsblr_ack, grb_ack;
wire			gftt_drd_ack, gftt_dwr_ack;
wire	[31:0]	ibus_rddata_arb;
wire	[7:0]	arb_done;
wire	[7:0]	arb_ack;
wire			arb_vout;
wire	[31:0]	arb_dout;
wire			grb_done, rect_done, xsbl_done, bm_done, gftt_done;
wire	[31:0]	axi_awaddr;
wire	[7:0]	axi_awlen;
wire			axi_awvalid;
wire	[31:0]	axi_wdata;
wire			axi_wvalid;
wire			axi_wlast;
wire	[31:0]	axi_araddr;
wire	[7:0]	axi_arlen;
wire			axi_arvalid;
wire			axi_rready;
wire			axi_bready;


//==========================================================================
// AXI Interface
//==========================================================================
assign clk   = s00_axi_aclk;
assign rst_n = s00_axi_aresetn;

axi_if #(
	C_S_AXI_DATA_WIDTH,
	C_S_AXI_ADDR_WIDTH,
	VERSION
) axi_if (
	.axi_aclk    (s00_axi_aclk),
	.axi_aresetn (s00_axi_aresetn),
	.axi_awaddr  (s00_axi_awaddr),
	.axi_awprot  (s00_axi_awprot),
	.axi_awvalid (s00_axi_awvalid),
	.axi_awready (s00_axi_awready),
	.axi_wdata   (s00_axi_wdata),
	.axi_wstrb   (s00_axi_wstrb),
	.axi_wvalid  (s00_axi_wvalid),
	.axi_wready  (s00_axi_wready),
	.axi_bresp   (s00_axi_bresp),
	.axi_bvalid  (s00_axi_bvalid),
	.axi_bready  (s00_axi_bready),
	.axi_araddr  (s00_axi_araddr),
	.axi_arprot  (s00_axi_arprot),
	.axi_arvalid (s00_axi_arvalid),
	.axi_arready (s00_axi_arready),
	.axi_rdata   (s00_axi_rdata),
	.axi_rresp   (s00_axi_rresp),
	.axi_rvalid  (s00_axi_rvalid),
	.axi_rready  (s00_axi_rready),
	
	.led      (led),
	.slide_sw (8'b0),
	.push_sw  (5'b0),
	.ptn_sel  (ptn_sel),
	.gpio_in  (gpio_in[1:0]),
	.gpio_out (gpio_out[1:0]),
	
	.addr   (ibus_addr),
	.cs     (ibus_cs),
	.wr     (ibus_wr),
	.wrdata (ibus_wrdata),
	.rddata (ibus_rddata),
	
	// Interrupt
	.arb_done (arb_done[7:0]),
	.vsync    (sof_csi),
	.intr     (intr)
);

assign ibus_cs_csi  = (ibus_cs[1] & (ibus_addr[11:8] == 4'h1));
assign ibus_cs_frm  = (ibus_cs[1] & (ibus_addr[11:8] == 4'h2));
assign ibus_cs_mes  = (ibus_cs[1] & (ibus_addr[11:8] == 4'h3));
assign ibus_cs_sel  = (ibus_cs[1] & (ibus_addr[11:8] == 4'h4));
assign ibus_cs_grb  = (ibus_cs[1] & (ibus_addr[11:8] == 4'h5));
assign ibus_cs_xsbl = (ibus_cs[1] & (ibus_addr[11:8] == 4'h6));
assign ibus_cs_bm   = (ibus_cs[1] & (ibus_addr[11:8] == 4'h7));
assign ibus_cs_arb  = (ibus_cs[1] & (ibus_addr[11:8] == 4'h8));
assign ibus_cs_rect = (ibus_cs[1] & (ibus_addr[11:8] == 4'h9));
assign ibus_cs_gftt = (ibus_cs[1] & (ibus_addr[11:8] == 4'hA));

assign ibus_rddata[31:0] = (
	ibus_rddata_csi[31:0]  |
	ibus_rddata_mes[31:0]  |
	ibus_rddata_grb[31:0]  |
	ibus_rddata_xsbl[31:0] |
	ibus_rddata_bm[31:0]   |
	ibus_rddata_arb[31:0]  |
	ibus_rddata_rect[31:0] |
	ibus_rddata_gftt[31:0]
);


//==========================================================================
// CSI Interface
//==========================================================================
csi_if csi_if (
	// Global Control
	.rst_n (s00_axi_aresetn),
	.clk   (s00_axi_aclk),
	
	// Internal Bus I/F
	.ibus_cs     (ibus_cs_csi),
	.ibus_wr     (ibus_wr),
	.ibus_addr   (ibus_addr[7:0]),
	.ibus_wrdata (ibus_wrdata[31:0]),
	.ibus_rddata (ibus_rddata_csi[31:0]),
	
	// CSI Receiver Control
	.vrst_n (vrst_n),
	
	// AXI Stream Video Input (Ch1)
	.tvalid_ch1 (v1_tvalid),
	.tready_ch1 (v1_tready),
	.tuser_ch1  (v1_tuser),
	.tlast_ch1  (v1_tlast),
	.tdata_ch1  (v1_tdata[15:0]),
	.tdest_ch1  (v1_tdest[3:0]),
	
	// AXI Stream Video Input (Ch2)
	.tvalid_ch2 (v2_tvalid),
	.tready_ch2 (v2_tready),
	.tuser_ch2  (v2_tuser),
	.tlast_ch2  (v2_tlast),
	.tdata_ch2  (v2_tdata[15:0]),
	.tdest_ch2  (v2_tdest[3:0]),
	
	// Parallel Video Output
	.sof_out (sof_csi),
	.vout    (v_csi),
	.d1_out  (d1_csi[15:0]),
	.d2_out  (d2_csi[15:0])
);


//==========================================================================
// Frame Measurement
//==========================================================================
frame_meas frame_meas (
	// Global Control
	.rst_n (s00_axi_aresetn),
	.clk   (s00_axi_aclk),
	
	// Internal Bus I/F
	.ibus_wr (ibus_wr),
	.ibus_cs (ibus_cs_mes),
	.ibus_addr_7_2 (ibus_addr[7:2]),
	.ibus_wrdata (ibus_wrdata),
	.ibus_rddata (ibus_rddata_mes),
	
	// DVP I/F
	.pclk  (clk),
	.href  (v_csi),
	.vsync (sof_csi)
);


//==========================================================================
// Frame Grabber
//==========================================================================
grb grb (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// DVP Interface
	.pclk  (clk),
	.vsync (sof_csi),
	.href  (v_csi),
	.d1    (d1_csi[15:0]),
	.d2    (d2_csi[15:0]),
	
	// Internal Bus I/F
	.ibus_cs (ibus_cs_grb),
	.ibus_wr (ibus_wr),
	.ibus_addr_7_2 (ibus_addr[7:2]),
	.ibus_wrdata   (ibus_wrdata[31:0]),
	.ibus_rddata   (ibus_rddata_grb[31:0]),
	
	// Arbiter I/F
	.req  (grb_req),
	.ack  (grb_ack),
	.dout (grb_dout[31:0]),
	.vout (grb_vout)
);


//==========================================================================
// Stereo Rectifier
//==========================================================================
`ifdef USE_DUMMY_RECT
dummy_rect rect (
`else
rect rect (
`endif
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Internal Bus I/F
	.ibus_cs (ibus_cs_rect),
	.ibus_wr (ibus_wr),
	.ibus_addr_7_2 (ibus_addr[7:2]),
	.ibus_wrdata   (ibus_wrdata[31:0]),
	.ibus_rddata   (ibus_rddata_rect[31:0]),
	
	// Control
	.rect_done (rect_done),
	.rect_fcnt (rect_fcnt[3:0]),
	.bm_done   (bm_done),
	.bm_enb    (bm_enb),
	
	// Sensor Input
	.pclk  (clk),
	.vsync (sof_csi),
	.href  (v_csi),
	.d_l   (d1_csi[15:8]), // upper 8bit, Y
	.d_r   (d2_csi[15:8]),
	
	// DDR I/F
	.ddr_req  (rect_req),
	.ddr_ack  (rect_ack),
	.ddr_dout (rect_dout[31:0]),
	.ddr_strb (rect_strb[3:0]),
	.ddr_vout (rect_vout)
);


//==========================================================================
// Block Matching
//==========================================================================
assign xsblr_vin = xsblr_ack & arb_vout;

`ifdef USE_DUMMY_XSBL2
dummy_xsbl2 xsbl2 (
`else
xsbl2 xsbl2 (
`endif
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Internal Bus I/F
	.ibus_cs (ibus_cs_xsbl),
	.ibus_wr (ibus_wr),
	.ibus_addr_7_2 (ibus_addr[7:2]),
	.ibus_wrdata   (ibus_wrdata[31:0]),
	.ibus_rddata   (ibus_rddata_xsbl[31:0]),
	
	// Control
	.rect_done (rect_done),
	.xsbl_done (xsbl_done),
	.rect_fcnt (rect_fcnt[3:0]),
	.xsbl_fcnt (xsbl_fcnt),
	
	// DDR Read Arbiter I/F
	.drd_req  (xsblr_req),
	.drd_ack  (xsblr_ack),
	.drd_vin  (xsblr_vin),
	.drd_din  (arb_dout[31:0]),
	.drd_vout (xsblr_vout),
	.drd_dout (xsblr_dout),
	
	// DDR Write Arbiter I/F
	.dwr_req  (xsblw_req),
	.dwr_ack  (xsblw_ack),
	.dwr_vout (xsblw_vout),
	.dwr_dout (xsblw_dout),
	.dwr_strb (xsblw_strb)
);

assign drd_vin = drd_ack & arb_vout;

`ifdef USE_DUMMY_BM
dummy_bm bm (
`else
bm bm (
`endif
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Internal Bus I/F
	.ibus_cs (ibus_cs_bm),
	.ibus_wr (ibus_wr),
	.ibus_addr_7_2 (ibus_addr[7:2]),
	.ibus_wrdata   (ibus_wrdata[31:0]),
	.ibus_rddata   (ibus_rddata_bm[31:0]),
	
	// Control
	.xsbl_done (xsbl_done),
	.bm_done   (bm_done),
	.xsbl_fcnt (xsbl_fcnt),
	.bm_fcnt   (),
	.enb       (bm_enb),
	
	// DDR Read Arbiter I/F
	.drd_req  (drd_req),
	.drd_ack  (drd_ack),
	.drd_vin  (drd_vin),
	.drd_din  (arb_dout[31:0]),
	.drd_vout (drd_vout),
	.drd_dout (drd_dout[31:0]),
	
	// DDR Write Arbiter I/F
	.dwr_req  (dwr_req),
	.dwr_ack  (dwr_ack),
	.dwr_vout (dwr_vout),
	.dwr_dout (dwr_dout[31:0]),
	.dwr_strb (dwr_strb[3:0])
);


//==========================================================================
// GFTT
//==========================================================================
assign gftt_drd_vin = gftt_drd_ack & arb_vout;

`ifdef USE_DUMMY_GFTT
dummy_gftt gftt (
`else
gftt gftt (
`endif
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Internal Bus I/F
	.ibus_cs (ibus_cs_gftt),
	.ibus_wr (ibus_wr),
	.ibus_addr_7_2 (ibus_addr[7:2]),
	.ibus_wrdata   (ibus_wrdata[31:0]),
	.ibus_rddata   (ibus_rddata_gftt[31:0]),
	
	// Control
	.rect_done (rect_done),
	.gftt_done (gftt_done),
	.rect_fcnt (rect_fcnt[3:0]),
	.gftt_fcnt (),
	
	// DDR Read Arbiter I/F
	.drd_req  (gftt_drd_req),
	.drd_ack  (gftt_drd_ack),
	.drd_vin  (gftt_drd_vin),
	.drd_din  (arb_dout[31:0]),
	.drd_vout (gftt_drd_vout),
	.drd_dout (gftt_drd_dout[31:0]),
	
	// DDR Write Arbiter I/F
	.dwr_req  (gftt_dwr_req),
	.dwr_ack  (gftt_dwr_ack),
	.dwr_vout (gftt_dwr_vout),
	.dwr_dout (gftt_dwr_dout[31:0])
);
/*
assign ibus_rddata_gftt[31:0] = 32'b0;
assign gftt_drd_req           =  1'b0;
assign gftt_drd_vout          =  1'b0;
assign gftt_drd_dout[31:0]    = 32'b0;
assign gftt_dwr_req           =  1'b0;
assign gftt_dwr_vout          =  1'b0;
assign gftt_dwr_dout[31:0]    = 32'b0;
*/


//==========================================================================
// DDR Arbiter
//==========================================================================
assign arb_req[7:0] = {
	gftt_dwr_req,
	gftt_drd_req,
	xsblw_req,
	rect_req,
	dwr_req,
	drd_req,
	xsblr_req,
	grb_req
};
assign arb_vin = (
	gftt_dwr_vout |
	gftt_drd_vout |
	xsblw_vout    |
	rect_vout     |
	dwr_vout      |
	drd_vout      |
	xsblr_vout    |
	grb_vout
);
assign arb_din[31:0] = (
	gftt_dwr_dout[31:0] |
	gftt_drd_dout[31:0] |
	xsblw_dout[31:0]    |
	rect_dout[31:0]     |
	dwr_dout[31:0]      |
	drd_dout[31:0]      |
	xsblr_dout[31:0]    |
	grb_dout[31:0]
);
//assign arb_strb[3:0] = xsblw_strb & rect_strb[3:0]; // default high
assign arb_strb[3:0] = xsblw_strb & rect_strb[3:0] & dwr_strb[3:0];

assign gftt_dwr_ack = arb_ack[7];
assign gftt_drd_ack = arb_ack[6];
assign xsblw_ack    = arb_ack[5];
assign rect_ack     = arb_ack[4];
assign dwr_ack      = arb_ack[3];
assign drd_ack      = arb_ack[2];
assign xsblr_ack    = arb_ack[1];
assign grb_ack      = arb_ack[0];

arb arb (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Internal Bus I/F
	.ibus_cs (ibus_cs_arb),
	.ibus_wr (ibus_wr),
	.ibus_addr_7_2 (ibus_addr[7:2]),
	.ibus_wrdata   (ibus_wrdata[31:0]),
	.ibus_rddata   (ibus_rddata_arb[31:0]),
	
	// Status
	.done (arb_done[7:0]),
	
	// FIFO I/F
	.req (arb_req[7:0]),
	.ack (arb_ack[7:0]),
	.valid_in  (arb_vin),
	.data_in   (arb_din),
	.strb_in   (arb_strb),
	.valid_out (arb_vout),
	.data_out  (arb_dout),
	
	// AXI I/F
	.axi_aclk    (m00_axi_aclk),
	.axi_awaddr  (m00_axi_awaddr),
	.axi_awlen   (m00_axi_awlen),
	.axi_awvalid (m00_axi_awvalid),
	.axi_awready (m00_axi_awready),
	.axi_wdata   (m00_axi_wdata),
	.axi_wstrb   (m00_axi_wstrb),
	.axi_wvalid  (m00_axi_wvalid),
	.axi_wready  (m00_axi_wready),
	.axi_wlast   (m00_axi_wlast),
	.axi_araddr  (m00_axi_araddr),
	.axi_arlen   (m00_axi_arlen),
	.axi_arvalid (m00_axi_arvalid),
	.axi_arready (m00_axi_arready),
	.axi_rdata   (m00_axi_rdata),
	.axi_rvalid  (m00_axi_rvalid),
	.axi_rready  (m00_axi_rready),
	.axi_rlast   (m00_axi_rlast),
	.axi_bvalid  (m00_axi_bvalid),
	.axi_bready  (m00_axi_bready)
);

// rename
assign grb_done  = arb_done[0];
assign rect_done = arb_done[4];
assign xsbl_done = arb_done[5];
assign bm_done   = arb_done[3];
assign gftt_done = arb_done[7];

assign m00_axi_awid    =  1'b0;
assign m00_axi_awsize  =  3'd2; // 4 bytes
assign m00_axi_awburst =  2'd1; // INCR
assign m00_axi_awlock  =  1'b0;
assign m00_axi_awcache =  4'd2;
assign m00_axi_awprot  =  3'd0; // Data Secure Unprivileged
assign m00_axi_awqos   =  4'd0;
assign m00_axi_awuser  =  1'b0;
//assign m00_axi_wstrb   =  4'hF; // arb controlled
assign m00_axi_wuser   =  1'b0;
assign m00_axi_arid    =  1'b0;
assign m00_axi_arsize  =  3'd2; // 4 bytes
assign m00_axi_arburst =  2'd1; // INCR
assign m00_axi_arlock  =  1'b0;
assign m00_axi_arcache =  4'd2;
assign m00_axi_arprot  =  3'd0; // Data Secure Unprivileged
assign m00_axi_arqos   =  4'd0;
assign m00_axi_aruser  =  1'b0;


endmodule
