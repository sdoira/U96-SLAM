`timescale 1ns / 1ps

module sim_gpio;


//==========================================================================
// Register Map
//==========================================================================
parameter ADR_COM_VER       = 14'h0000;
parameter ADR_COM_TEST_PAD  = 14'h0004;
parameter ADR_COM_LED       = 14'h0008;
parameter ADR_COM_SW        = 14'h000C;
parameter ADR_COM_TIMER     = 14'h0010;
parameter ADR_COM_INTR_ENB  = 14'h0014;
parameter ADR_COM_INTR_STS  = 14'h0018;
parameter ADR_GPIO_IN       = 14'h0028;
parameter ADR_GPIO_OUT      = 14'h002C;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Simulator Setting
integer			ADDR, DATA;
integer			mcd_grb;
integer			mcd_rect, mcd_rect2;
integer			mcd_xsbl_l, mcd_xsbl_r, mcd_xsbl2_l, mcd_xsbl2_r;
integer			mcd_disp, mcd_frac, mcd_cost;
integer			mcd_eig;
integer			i, j, lr, row, col, hcnt, pcnt;
integer			mcd_disp2_short, mcd_disp2_int;

// Reset/Clock Generation
reg				clk, rst_n;
wire			s00_axi_aclk;
wire			s00_axi_aresetn;
wire			m00_axi_aclk;
wire			m00_axi_aresetn;

// dvp
reg		[1:0]	gpio_in;
wire			tready_l, tready_r;
wire			intr;
reg		[13:0]	s00_axi_awaddr;
reg		[2:0]	s00_axi_awprot;
reg				s00_axi_awvalid;
wire			s00_axi_awready;
reg		[31:0]	s00_axi_wdata;
reg		[3:0]	s00_axi_wstrb;
reg				s00_axi_wvalid;
wire			s00_axi_wready;
wire	[1:0]	s00_axi_bresp;
wire			s00_axi_bvalid;
reg				s00_axi_bready;
reg		[13:0]	s00_axi_araddr;
reg		[2:0]	s00_axi_arprot;
reg				s00_axi_arvalid;
wire			s00_axi_arready;
wire	[31:0]	s00_axi_rdata;
wire	[1:0]	s00_axi_rresp;
wire			s00_axi_rvalid;
reg				s00_axi_rready;

wire			m00_axi_awid;
wire	[31:0]	m00_axi_awaddr;
wire	[7:0]	m00_axi_awlen;
wire	[2:0]	m00_axi_awsize;
wire	[1:0]	m00_axi_awburst;
wire			m00_axi_awlock;
wire	[3:0]	m00_axi_awcache;
wire	[2:0]	m00_axi_awprot;
wire	[3:0]	m00_axi_awqos;
wire			m00_axi_awuser; // [0-1:0]
wire			m00_axi_awvalid;
wire			m00_axi_awready;
wire	[31:0]	m00_axi_wdata;
wire	[3:0]	m00_axi_wstrb;
wire			m00_axi_wlast;
wire			m00_axi_wuser; // [0-1:0]
wire			m00_axi_wvalid;
wire			m00_axi_wready;
reg				m00_axi_bid;
reg		[1:0]	m00_axi_bresp;
reg				m00_axi_buser; //[0-1:0]
wire			m00_axi_bvalid;
wire			m00_axi_bready;
wire			m00_axi_arid;
wire	[31:0]	m00_axi_araddr;
wire	[7:0]	m00_axi_arlen;
wire	[2:0]	m00_axi_arsize;
wire	[1:0]	m00_axi_arburst;
wire			m00_axi_arlock;
wire	[3:0]	m00_axi_arcache;
wire	[2:0]	m00_axi_arprot;
wire	[3:0]	m00_axi_arqos;
wire			m00_axi_aruser; // [0-1:0]
wire			m00_axi_arvalid;
wire			m00_axi_arready;
reg				m00_axi_rid;
wire	[31:0]	m00_axi_rdata;
reg		[1:0]	m00_axi_rresp;
wire			m00_axi_rlast;
reg				m00_axi_ruser; // [0-1:0]
wire			m00_axi_rvalid;
wire			m00_axi_rready;


//==========================================================================
// Reset/Clock Generation
//==========================================================================
initial begin
	clk <= 1'b0;
	forever begin
		#5; // 100MHz
		clk <= ~clk;
	end
end

initial begin
	rst_n <= 1'b0;
	repeat (5) @(posedge clk);
	rst_n <= 1'b1;
end

assign s00_axi_aclk    = clk;
assign s00_axi_aresetn = rst_n;
assign m00_axi_aclk    = clk;
assign m00_axi_aresetn = rst_n;


//==========================================================================
// Target Instance
//==========================================================================
dvp dvp (
	// AXI Stream Video Input (Ch1)
	.v1_tvalid (1'b0),
	.v1_tready (tready_l),
	.v1_tuser  (1'b0),
	.v1_tlast  (1'b0),
	.v1_tdata  (16'b0),
	.v1_tdest  (4'b0),
	
	// AXI Stream Video Input (Ch2)
	.v2_tvalid (1'b0),
	.v2_tready (tready_r),
	.v2_tuser  (1'b0),
	.v2_tlast  (1'b0),
	.v2_tdata  (16'b0),
	.v2_tdest  (4'b0),
	
	// Debug
	.led      (),
	.intr     (intr),
	.gpio_in  (gpio_in[1:0]),
	.gpio_out (),

	// AXI Interface
	.s00_axi_aclk    (s00_axi_aclk),
	.s00_axi_aresetn (s00_axi_aresetn),
	.s00_axi_awaddr  (s00_axi_awaddr),
	.s00_axi_awprot  (s00_axi_awprot),
	.s00_axi_awvalid (s00_axi_awvalid),
	.s00_axi_awready (s00_axi_awready),
	.s00_axi_wdata   (s00_axi_wdata),
	.s00_axi_wstrb   (s00_axi_wstrb),
	.s00_axi_wvalid  (s00_axi_wvalid),
	.s00_axi_wready  (s00_axi_wready),
	.s00_axi_bresp   (s00_axi_bresp),
	.s00_axi_bvalid  (s00_axi_bvalid),
	.s00_axi_bready  (s00_axi_bready),
	.s00_axi_araddr  (s00_axi_araddr),
	.s00_axi_arprot  (s00_axi_arprot),
	.s00_axi_arvalid (s00_axi_arvalid),
	.s00_axi_arready (s00_axi_arready),
	.s00_axi_rdata   (s00_axi_rdata),
	.s00_axi_rresp   (s00_axi_rresp),
	.s00_axi_rvalid  (s00_axi_rvalid),
	.s00_axi_rready  (s00_axi_rready),
	
	.m00_axi_aclk    (m00_axi_aclk),
	.m00_axi_aresetn (m00_axi_aresetn),
	.m00_axi_awid    (m00_axi_awid),
	.m00_axi_awaddr  (m00_axi_awaddr),
	.m00_axi_awlen   (m00_axi_awlen),
	.m00_axi_awsize  (m00_axi_awsize),
	.m00_axi_awburst (m00_axi_awburst),
	.m00_axi_awlock  (m00_axi_awlock),
	.m00_axi_awcache (m00_axi_awcache),
	.m00_axi_awprot  (m00_axi_awprot),
	.m00_axi_awqos   (m00_axi_awqos),
	.m00_axi_awuser  (m00_axi_awuser),
	.m00_axi_awvalid (m00_axi_awvalid),
	.m00_axi_awready (m00_axi_awready),
	.m00_axi_wdata   (m00_axi_wdata),
	.m00_axi_wstrb   (m00_axi_wstrb),
	.m00_axi_wlast   (m00_axi_wlast),
	.m00_axi_wuser   (m00_axi_wuser),
	.m00_axi_wvalid  (m00_axi_wvalid),
	.m00_axi_wready  (m00_axi_wready),
	.m00_axi_bid     (m00_axi_bid),
	.m00_axi_bresp   (m00_axi_bresp),
	.m00_axi_buser   (m00_axi_buser),
	.m00_axi_bvalid  (m00_axi_bvalid),
	.m00_axi_bready  (m00_axi_bready),
	.m00_axi_arid    (m00_axi_arid),
	.m00_axi_araddr  (m00_axi_araddr),
	.m00_axi_arlen   (m00_axi_arlen),
	.m00_axi_arsize  (m00_axi_arsize),
	.m00_axi_arburst (m00_axi_arburst),
	.m00_axi_arlock  (m00_axi_arlock),
	.m00_axi_arcache (m00_axi_arcache),
	.m00_axi_arprot  (m00_axi_arprot),
	.m00_axi_arqos   (m00_axi_arqos),
	.m00_axi_aruser  (m00_axi_aruser),
	.m00_axi_arvalid (m00_axi_arvalid),
	.m00_axi_arready (m00_axi_arready),
	.m00_axi_rid     (m00_axi_rid),
	.m00_axi_rdata   (m00_axi_rdata),
	.m00_axi_rresp   (m00_axi_rresp),
	.m00_axi_rlast   (m00_axi_rlast),
	.m00_axi_ruser   (m00_axi_ruser),
	.m00_axi_rvalid  (m00_axi_rvalid),
	.m00_axi_rready  (m00_axi_rready)
);


//==========================================================================
// Simulation Control
//==========================================================================
task INIT;
	begin
		gpio_in <= 2'b0;
		
		s00_axi_awaddr  <= 14'b0;
		s00_axi_awprot  <=  3'b0;
		s00_axi_awvalid <=  1'b0;
		s00_axi_wdata   <= 32'b0;
		s00_axi_wstrb   <=  8'hF;
		s00_axi_wvalid  <=  1'b0;
		s00_axi_bready  <=  1'b0;
		s00_axi_araddr  <= 14'b0;
		s00_axi_arprot  <=  3'b0;
		s00_axi_arvalid <=  1'b0;
		s00_axi_rready  <=  1'b0;
		
		m00_axi_bid     <=  1'b0;
		m00_axi_bresp   <=  2'b0;
		m00_axi_buser   <=  1'b0; //[0-1:0]
		m00_axi_rid     <=  1'b0;
		m00_axi_rresp   <=  2'b0;
		m00_axi_ruser   <=  1'b0; // [0-1:0]
	end
endtask

task REG_WR;
	begin
		repeat (5) @(posedge s00_axi_aclk);
		s00_axi_wvalid <= 1'b1;
		s00_axi_wdata <= DATA;
		repeat (2) @(posedge s00_axi_aclk);
		s00_axi_awvalid <= 1'b1;
		s00_axi_awaddr <= ADDR[13:0];
		
		@(posedge s00_axi_awready);
		@(posedge s00_axi_aclk);
		s00_axi_wvalid <= 1'b0;
		s00_axi_wdata <= 32'h0;
		s00_axi_awvalid <= 1'b0;
		s00_axi_awaddr <= 0;
		
		@(posedge s00_axi_bvalid);
		@(posedge s00_axi_aclk);
		s00_axi_bready <= 1'b1;
		@(posedge s00_axi_aclk);
		s00_axi_bready <= 1'b0;
		
		repeat (5) @(posedge s00_axi_aclk);
	end
endtask

task REG_RD;
	begin
		repeat (5) @(posedge s00_axi_aclk);
		s00_axi_arvalid <= 1'b1;
		s00_axi_araddr <= ADDR[13:0];
		@(posedge s00_axi_arready);
		@(posedge s00_axi_aclk);
		s00_axi_arvalid <= 1'b0;
		s00_axi_araddr <= 0;
		
		@(posedge s00_axi_rvalid);
		@(posedge s00_axi_aclk);
		s00_axi_rready <= 1'b1;
		@(posedge s00_axi_aclk);
		s00_axi_rready <= 1'b0;
		
		repeat (5) @(posedge s00_axi_aclk);
	end
endtask


initial begin
	INIT;
	
	repeat (10) @(posedge clk);
	
	gpio_in[0] <= 1'b1;
	@(posedge clk);
	gpio_in[0] <= 1'b0;
	repeat (100) @(posedge clk);
	ADDR = ADR_GPIO_IN;								REG_RD;
	repeat (10) @(posedge clk);
	ADDR = ADR_GPIO_IN;		DATA = 32'h0000_0001;	REG_WR;
	
	repeat (100) @(posedge clk);
	
	gpio_in[1] <= 1'b1;
	@(posedge clk);
	gpio_in[1] <= 1'b0;
	repeat (100) @(posedge clk);
	ADDR = ADR_GPIO_IN;								REG_RD;
	repeat (10) @(posedge clk);
	ADDR = ADR_GPIO_IN;		DATA = 32'h0000_0002;	REG_WR;
	
	repeat (100) @(posedge clk);
	
	$finish;
end



endmodule
