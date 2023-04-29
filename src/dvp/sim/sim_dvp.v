`timescale 1ns / 1ps

module sim_dvp;


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
parameter ADR_COM_BLINK     = 14'h0030;
parameter ADR_COM_SW_HOLD   = 14'h0034;

parameter ADR_CSI_CTRL        = 14'h1100;
parameter ADR_CSI_STS         = 14'h1104;
parameter ADR_CSI_SIZE1       = 14'h1108;
parameter ADR_CSI_PTN_SEL1    = 14'h110C;
parameter ADR_CSI_FRAME_LEN1  = 14'h1110;
parameter ADR_CSI_FRAME_CNT1  = 14'h1114;
parameter ADR_CSI_SIZE2       = 14'h1118;
parameter ADR_CSI_PTN_SEL2    = 14'h111C;
parameter ADR_CSI_FRAME_LEN2  = 14'h1120;
parameter ADR_CSI_FRAME_CNT2  = 14'h1124;
parameter ADR_CSI_FRAME_DECIM = 14'h1128;

parameter ADR_FRM_CMD      = 14'h1200;
parameter ADR_FRM_ADDR_A   = 14'h1204;
parameter ADR_FRM_ADDR_B   = 14'h1208;
parameter ADR_FRM_BST_LEN  = 14'h120C;
parameter ADR_FRM_STS      = 14'h1210;
parameter ADR_FRM_NUM      = 14'h1214;

parameter ADR_FFM_CMD      = 14'h1300;
parameter ADR_FFM_STS      = 14'h1304;
parameter ADR_FFM_V_PER    = 14'h1308;
parameter ADR_FFM_H_PER    = 14'h130C;
parameter ADR_FFM_V_ACT    = 14'h1310;
parameter ADR_FFM_H_ACT    = 14'h1314;
parameter ADR_FFM_H_BEG    = 14'h1318;
parameter ADR_FFM_V_REF    = 14'h131C;

parameter ADR_SEL_INSEL    = 14'h1400;
parameter ADR_SEL_MODE     = 14'h1404;

parameter ADR_GRB_CTRL     = 14'h1500;
parameter ADR_GRB_STS      = 14'h1504;
parameter ADR_GRB_ADDR_A   = 14'h1508;
parameter ADR_GRB_ADDR_B   = 14'h150C;
parameter ADR_GRB_BST_LEN  = 14'h1510;
parameter ADR_GRB_SIZE     = 14'h1514;

parameter ADR_XSBL_CTRL    = 14'h1600;
parameter ADR_XSBL_STS     = 14'h1604;
parameter ADR_XSBL_RADDR_A = 14'h1608;
parameter ADR_XSBL_RADDR_B = 14'h160C;
parameter ADR_XSBL_WADDR_A = 14'h1610;
parameter ADR_XSBL_WADDR_B = 14'h1614;
parameter ADR_XSBL_BST_LEN = 14'h1618;
parameter ADR_XSBL_SIZE    = 14'h161C;

parameter ADR_BM_CTRL       = 14'h1700;
parameter ADR_BM_STS        = 14'h1704;
parameter ADR_BM_SIZE       = 14'h1708;
parameter ADR_BM_BM_PARM    = 14'h170C;
parameter ADR_BM_LR_ADDR_A  = 14'h1710;
parameter ADR_BM_LR_ADDR_B  = 14'h1714;
parameter ADR_BM_SAD_ADDR_A = 14'h1718;
parameter ADR_BM_SAD_ADDR_B = 14'h171C;
parameter ADR_BM_BST_LEN    = 14'h1720;
parameter ADR_BM_SAD_SIZE   = 14'h1724;
parameter ADR_BM_DISP2_ADDR_A = 14'h172C;
parameter ADR_BM_DISP2_ADDR_B = 14'h1730;

parameter ADR_ARB_CTRL      = 14'h1800;
parameter ADR_ARB_STS       = 14'h1804;

parameter ADR_RECT_CTRL     = 14'h1900;
parameter ADR_RECT_STS      = 14'h1904;
parameter ADR_RECT_CMD_CTRL = 14'h1908;
parameter ADR_RECT_CMD_STS  = 14'h190C;
parameter ADR_RECT_CMD_WR   = 14'h1910;
parameter ADR_RECT_L_FX     = 14'h1914;
parameter ADR_RECT_L_FY     = 14'h1918;
parameter ADR_RECT_R_FX     = 14'h191C;
parameter ADR_RECT_R_FY     = 14'h1920;
parameter ADR_RECT_CXY      = 14'h1924;
parameter ADR_RECT_FX2_INV  = 14'h1928;
parameter ADR_RECT_FY2_INV  = 14'h192C;
parameter ADR_RECT_CX2_FX2  = 14'h1930;
parameter ADR_RECT_CY2_FY2  = 14'h1934;
parameter ADR_RECT_L_R11    = 14'h1938;
parameter ADR_RECT_L_R12    = 14'h193C;
parameter ADR_RECT_L_R13    = 14'h1940;
parameter ADR_RECT_L_R21    = 14'h1944;
parameter ADR_RECT_L_R22    = 14'h1948;
parameter ADR_RECT_L_R23    = 14'h194C;
parameter ADR_RECT_L_R31    = 14'h1950;
parameter ADR_RECT_L_R32    = 14'h1954;
parameter ADR_RECT_L_R33    = 14'h1958;
parameter ADR_RECT_R_R11    = 14'h195C;
parameter ADR_RECT_R_R12    = 14'h1960;
parameter ADR_RECT_R_R13    = 14'h1964;
parameter ADR_RECT_R_R21    = 14'h1968;
parameter ADR_RECT_R_R22    = 14'h196C;
parameter ADR_RECT_R_R23    = 14'h1970;
parameter ADR_RECT_R_R31    = 14'h1974;
parameter ADR_RECT_R_R32    = 14'h1978;
parameter ADR_RECT_R_R33    = 14'h197C;
parameter ADR_RECT_BASE_A   = 14'h1980;
parameter ADR_RECT_BASE_B   = 14'h1984;

parameter ADR_GFTT_CTRL     = 14'h1A00;
parameter ADR_GFTT_STS      = 14'h1A04;
parameter ADR_GFTT_RADDR_A  = 14'h1A08;
parameter ADR_GFTT_RADDR_B  = 14'h1A0C;
parameter ADR_GFTT_WADDR_A  = 14'h1A10;
parameter ADR_GFTT_WADDR_B  = 14'h1A14;
parameter ADR_GFTT_BST_LEN  = 14'h1A18;
parameter ADR_GFTT_SIZE     = 14'h1A1C;


//==========================================================================
// Definition
//==========================================================================
parameter SimMode = (
	0 // I2C
	//1 // Frame Format Measurement
	//2 // Frame Grabber
	//3 // Rect
	//4 // X-Sobel
	//5 // BM
	//6 // Rect + X-Sobel + BM
	//7 // GFTT
);
// uncomment `define directives in dvp.v to speed up simulation
// SimMode			0 1 2 3 4 5 6 7
// USE_DUMMY_RECT	+ + + - + + - +
// USE_DUMMY_XSBL2	+ + + + - + - +
// USE_DUMMY_BM		+ + + + + - - +
// USE_DUMMY_GFTT	+ + + + + + + -
// '+': dummy modules can be used
// '-': necessary to use actual modules

parameter WDT = 640;
parameter HGT = 480;

parameter BUF_GRB_A  = 32'h0000_0000;
parameter BUF_GRB_B  = 32'h0020_0000;
parameter BUF_RECT_A = 32'h0000_0000;
parameter BUF_RECT_B = 32'h0010_0000;
parameter BUF_XSBL_A = 32'h0020_0000;
parameter BUF_XSBL_B = 32'h0030_0000;
parameter BUF_BM_A   = 32'h0040_0000;
parameter BUF_BM_B   = 32'h0080_0000;
parameter BUF_GFTT_A = 32'h00A0_0000;
parameter BUF_GFTT_B = 32'h00B0_0000;

// Input Data
parameter FILE_IN_L       = "../../../../../../src/dvp/sim/img_001_l.dat";
parameter FILE_IN_R       = "../../../../../../src/dvp/sim/img_001_r.dat";

parameter FILE_REF_RECT_L = "../../../../../../src/dvp/sim/ref/ref_rect_l.dat";
parameter FILE_REF_RECT_R = "../../../../../../src/dvp/sim/ref/ref_rect_r.dat";
parameter FILE_REF_XSBL_L = "../../../../../../src/dvp/sim/ref/ref_xsbl_l.dat";
parameter FILE_REF_XSBL_R = "../../../../../../src/dvp/sim/ref/ref_xsbl_r.dat";

// Rectification Command Input
//parameter FILE_CMD  = "../../../../dvp.srcs/sim_1/new/cmd.dat";
parameter FILE_CMD  = "../../../../../../src/dvp/sim/cmd.dat";
//parameter CMD_LEN = 13284;
parameter CMD_LEN = 17104;

// Output Data
parameter FILE_GRB_A   = "../../../../../../src/dvp/sim/out_grb_a.csv";
parameter FILE_GRB_B   = "../../../../../../src/dvp/sim/out_grb_b.csv";
parameter FILE_RECT_L  = "../../../../../../src/dvp/sim/out_rect_l.csv";
parameter FILE_RECT_R  = "../../../../../../src/dvp/sim/out_rect_r.csv";
parameter FILE_RECT_L2 = "../../../../../../src/dvp/sim/out_rect_l.dat";
parameter FILE_RECT_R2 = "../../../../../../src/dvp/sim/out_rect_r.dat";
parameter FILE_XSBL_L  = "../../../../../../src/dvp/sim/out_xsbl_l.csv";
parameter FILE_XSBL_R  = "../../../../../../src/dvp/sim/out_xsbl_r.csv";
parameter FILE_XSBL_L2 = "../../../../../../src/dvp/sim/out_xsbl_l.dat";
parameter FILE_XSBL_R2 = "../../../../../../src/dvp/sim/out_xsbl_r.dat";
parameter FILE_DISP    = "../../../../../../src/dvp/sim/out_disp.csv";
parameter FILE_FRAC    = "../../../../../../src/dvp/sim/out_disp_frac.csv";
parameter FILE_COST    = "../../../../../../src/dvp/sim/out_sad.csv";

parameter FILE_DISP2_SHORT = "../../../../../../src/dvp/sim/out_disp2_short.csv";
parameter FILE_DISP2_INT   = "../../../../../../src/dvp/sim/out_disp2_int.csv";
parameter FILE_DISP2_SHORT_B = "../../../../../../src/dvp/sim/out_disp2_short_b.csv";
parameter FILE_DISP2_INT_B   = "../../../../../../src/dvp/sim/out_disp2_int_b.csv";

parameter FILE_EIG_A = "../../../../../../src/dvp/sim/out_gftt_eig_a.csv";
parameter FILE_EIG_B = "../../../../../../src/dvp/sim/out_gftt_eig_b.csv";


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

// DVP Generation
reg				imread_enb;
wire			tuser_l, tvalid_;
wire	[15:0]	tdata_l;
wire			tlast_l;
wire			tuser_r, tvalid_r;
wire	[15:0]	tdata_r;
wire			tlast_r;

// DDR Memory Model
reg		[7:0]	tmp_mem[WDT*HGT-1:0];

// Rectification Command
reg		[17:0]	cmd_mem[0:CMD_LEN-1];

// dvp
wire			tready_l, tready_r;
wire			intr;
reg		[1:0]	gpio_in;
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
// DVP Generation
//==========================================================================
imread_csi #(FILE_IN_L) imread_csi_l (
	.clk    (clk),
	.en     (imread_enb),
	.tuser  (tuser_l),
	.tvalid (tvalid_l),
	.tready (tready_l),
	.tdata  (tdata_l),
	.tlast  (tlast_l)
);

imread_csi #(FILE_IN_R) imread_csi_r (
	.clk    (clk),
	.en     (imread_enb),
	.tuser  (tuser_r),
	.tvalid (tvalid_r),
	.tready (tready_r),
	.tdata  (tdata_r),
	.tlast  (tlast_r)
);


//==========================================================================
// DDR Memory Model
//  16MB Space: 0000_0000 - 00FF_FFFF (16MB)
//--------------------------------------------------------------------------
// (old)
//  BUF_GRB_A : 0000_0000 - 001F_FFFF (2MB)
//  BUF_GRB_B : 0020_0000 - 003F_FFFF (2MB)
//  BUF_RECT_A: 0000_0000 - 001F_FFFF (2MB)
//  BUF_RECT_B: 0020_0000 - 003F_FFFF (2MB)
//  BUF_XSBL_A: 0040_0000 - 005F_FFFF (2MB)
//  BUF_XSBL_B: 0060_0000 - 007F_FFFF (2MB)
//  BUF_BM_A  : 0080_0000 - 00BF_FFFF (4MB)
//  BUF_BM_B  : 00C0_0000 - 00FF_FFFF (4MB)
//--------------------------------------------------------------------------
// (new)
//  BUF_GRB_A : 0000_0000 - 001F_FFFF (2MB)
//  BUF_GRB_B : 0020_0000 - 003F_FFFF (2MB)
//  BUF_RECT_A: 0000_0000 - 000F_FFFF (1MB)
//  BUF_RECT_B: 0010_0000 - 001F_FFFF (1MB)
//  BUF_XSBL_A: 0020_0000 - 002F_FFFF (1MB)
//  BUF_XSBL_B: 0030_0000 - 003F_FFFF (1MB)
//  BUF_BM_A  : 0040_0000 - 005F_FFFF (2MB)
//  Padding   : 0060_0000 - 007F_FFFF (2MB)
//  BUF_BM_B  : 0080_0000 - 009F_FFFF (2MB)
//  BUF_GFTT_A: 00A0_0000 - 00AF_FFFF (1MB)
//  BUF_GFTT_B: 00B0_0000 - 00BF_FFFF (1MB)
//==========================================================================
ddr_mdl ddr_mdl (
	// AXI I/F
	.axi_aclk    (clk),
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
/*
task LOAD_RECT;
	begin
		for (lr = 0; lr < 2; lr = lr + 1) begin
			if (lr == 0) $readmemh(FILE_REF_RECT_L, tmp_mem);
			else         $readmemh(FILE_REF_RECT_R, tmp_mem);
			
			i = 0;
			for (row = 0; row < HGT; row = row + 1) begin
				for (col = 0; col < WDT; col = col + 4) begin
					ADDR = {
						2'b0,
						BUF_RECT_A[29:20],
						lr[0],
						row[8:0],
						col[9:0]
					};
					// big endian -> little
					DATA = {
					//	tmp_mem[i  ][7:0], tmp_mem[i+1][7:0],
					//	tmp_mem[i+2][7:0], tmp_mem[i+3][7:0]
						tmp_mem[i+3][7:0], tmp_mem[i+2][7:0],
						tmp_mem[i+1][7:0], tmp_mem[i  ][7:0]
					};
					ddr_mdl.mem[ADDR >> 2] = DATA;
					
					i = i + 4;
				end
			end
		end
	end
endtask
*/
task LOAD_RECT;
	begin
		for (lr = 0; lr < 2; lr = lr + 1) begin
			if (lr == 0) $readmemh(FILE_REF_RECT_L, tmp_mem);
			else         $readmemh(FILE_REF_RECT_R, tmp_mem);
			
			i = 0;
			ADDR = {2'b0, BUF_RECT_A[29:20], lr[0], 19'b0};
			for (row = 0; row < HGT; row = row + 1) begin
				for (col = 0; col < WDT; col = col + 4) begin
					// big endian -> little
					DATA = {
						tmp_mem[i+3][7:0], tmp_mem[i+2][7:0],
						tmp_mem[i+1][7:0], tmp_mem[i  ][7:0]
					};
					ddr_mdl.mem[ADDR >> 2] = DATA;
					
					i = i + 4;
					ADDR = ADDR + 4;
				end
			end
		end
	end
endtask

task LOAD_XSBL;
	begin
		for (lr = 0; lr < 2; lr = lr + 1) begin
			if (lr == 0) $readmemh(FILE_REF_XSBL_L, tmp_mem);
			else         $readmemh(FILE_REF_XSBL_R, tmp_mem);
			
			i = 0;
			for (row = 0; row < HGT; row = row + 1) begin
				for (col = 0; col < WDT*2; col = col + 4) begin
					ADDR = {
						3'b0,
						dvp.bm.lr_addr_a[8:0],
						row[8:0],
						col[10:0]
					};
					
					if (lr == 0) begin
						ddr_mdl.mem[ADDR >> 2][31:24] = tmp_mem[i  ][7:0];
						ddr_mdl.mem[ADDR >> 2][15: 8] = tmp_mem[i+1][7:0];
					end
					else begin
						ddr_mdl.mem[ADDR >> 2][23:16] = tmp_mem[i  ][7:0];
						ddr_mdl.mem[ADDR >> 2][ 7: 0] = tmp_mem[i+1][7:0];
					end
					
					i = i + 2;
				end
			end
		end
	end
endtask

task LOAD_XSBL_B;
	begin
		for (lr = 0; lr < 2; lr = lr + 1) begin
			if (lr == 0) $readmemh(FILE_REF_XSBL_L, tmp_mem);
			else         $readmemh(FILE_REF_XSBL_R, tmp_mem);
			
			i = 0;
			for (row = 0; row < HGT; row = row + 1) begin
				for (col = 0; col < WDT*2; col = col + 4) begin
					ADDR = {
						3'b0,
						dvp.bm.lr_addr_b[8:0],
						row[8:0],
						col[10:0]
					};
					
					if (lr == 0) begin
						ddr_mdl.mem[ADDR >> 2][31:24] = tmp_mem[i  ][7:0];
						ddr_mdl.mem[ADDR >> 2][15: 8] = tmp_mem[i+1][7:0];
					end
					else begin
						ddr_mdl.mem[ADDR >> 2][23:16] = tmp_mem[i  ][7:0];
						ddr_mdl.mem[ADDR >> 2][ 7: 0] = tmp_mem[i+1][7:0];
					end
					
					i = i + 2;
				end
			end
		end
	end
endtask

// XSBL output has invalid lines at the top and the bottom of the field.
// This task will fill those lines with zeros.
task INIT_XSBL;
	begin
		for (i = 0; i < 2; i = i + 1) begin
			row = (i == 0) ? 0 : HGT - 1;
			for (col = 0; col < WDT*2; col = col + 4) begin
				ADDR = {
					3'b0,
					dvp.bm.lr_addr_a[8:0],
					row[8:0],
					col[10:0]
				};
				
				ddr_mdl.mem[ADDR >> 2] = 32'b0;
			end
		end
	end
endtask


//==========================================================================
// Rectification Command
//==========================================================================
initial begin
	$readmemh(FILE_CMD, cmd_mem);
end

task CMD_WR;
	begin
		for (i = 0; i < CMD_LEN; i = i + 1) begin
			ADDR = ADR_RECT_CMD_WR;
			DATA = cmd_mem[i];
			REG_WR;
		end
		ADDR = ADR_RECT_CMD_STS;							REG_RD;
		ADDR = ADR_RECT_CMD_CTRL;	DATA = 32'h0000_0002;	REG_WR; // read mode
	end
endtask


//==========================================================================
// Interrupt Handler
//==========================================================================
initial begin
	forever @(posedge clk) begin
		if (intr) begin
			ADDR = ADR_COM_INTR_STS;	DATA = 32'hFFFF_FFFF;	REG_WR;
		end
	end
end


//==========================================================================
// Target Instance
//==========================================================================
dvp dvp (
	// AXI Stream Video Input (Ch1)
	.v1_tvalid (tvalid_l),
	.v1_tready (tready_l),
	.v1_tuser  (tuser_l),
	.v1_tlast  (tlast_l),
	.v1_tdata  (tdata_l),
	.v1_tdest  (4'b0),
	
	// AXI Stream Video Input (Ch2)
	.v2_tvalid (tvalid_r),
	.v2_tready (tready_r),
	.v2_tuser  (tuser_r),
	.v2_tlast  (tlast_r),
	.v2_tdata  (tdata_r),
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
		imread_enb <= 1'b0;
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

task DUMP_GRB_A;
	begin
		mcd_grb = $fopen(FILE_GRB_A, "w");
		
		for (row = 0; row < HGT; row = row + 1) begin
			for (col = 0; col < WDT; col = col + 1) begin
				ADDR = dvp.grb.addr_a[31:0];
				ADDR = ADDR + (row * WDT + col) * 4;
				DATA = ddr_mdl.mem[ADDR >> 2];
				$fwrite(mcd_grb, "%d,", DATA[31:0]);
			end
			$fwrite(mcd_grb, "\n");
		end
		
		$fclose(mcd_grb);
	end
endtask

integer		mcd_grb1, mcd_grb2, mcd_grb3, mcd_grb4;
task DUMP_GRB_A2;
	begin
		mcd_grb1 = $fopen("../../../../../../src/dvp/sim/out_grb_a1.csv", "w");
		mcd_grb2 = $fopen("../../../../../../src/dvp/sim/out_grb_a2.csv", "w");
		mcd_grb3 = $fopen("../../../../../../src/dvp/sim/out_grb_a3.csv", "w");
		mcd_grb4 = $fopen("../../../../../../src/dvp/sim/out_grb_a4.csv", "w");
		
		for (row = 0; row < HGT; row = row + 1) begin
			for (col = 0; col < WDT; col = col + 1) begin
				ADDR = dvp.grb.addr_a[31:0];
				ADDR = ADDR + (row * WDT + col) * 4;
				DATA = ddr_mdl.mem[ADDR >> 2];
				$fwrite(mcd_grb1, "%d,", DATA[7:0]);
				$fwrite(mcd_grb2, "%d,", DATA[15:8]);
				$fwrite(mcd_grb3, "%d,", DATA[23:16]);
				$fwrite(mcd_grb4, "%d,", DATA[31:24]);
			end
			$fwrite(mcd_grb1, "\n");
			$fwrite(mcd_grb2, "\n");
			$fwrite(mcd_grb3, "\n");
			$fwrite(mcd_grb4, "\n");
		end
		
		$fclose(mcd_grb1);
		$fclose(mcd_grb2);
		$fclose(mcd_grb3);
		$fclose(mcd_grb4);
	end
endtask

//wire	[31:0]	addr_0000, addr_0A00, addr_1400;
//assign addr_0000[31:0] = ddr_mdl.mem[0][31:0];
//assign addr_0A00 = ddr_mdl.mem[2560];
//assign addr_1400 = ddr_mdl.mem[5120];

reg		[31:0]	addr_5112, addr_5116, addr_5120;
initial begin
	forever @(posedge ddr_mdl.axi_aclk) begin
		if (ddr_mdl.axi_wvalid & (ddr_mdl.addr[23:0] == 5112)) begin
			addr_5112 <= ddr_mdl.axi_wdata[31:0];
		end
		if (ddr_mdl.axi_wvalid & (ddr_mdl.addr[23:0] == 5116)) begin
			addr_5116 <= ddr_mdl.axi_wdata[31:0];
		end
		if (ddr_mdl.axi_wvalid & (ddr_mdl.addr[23:0] == 5120)) begin
			addr_5120 <= ddr_mdl.axi_wdata[31:0];
		end
	end
end
/*
initial begin
	forever @(posedge axi_aclk) begin
		if (axi_wvalid) begin
			if (axi_wstrb[3]) mem[addr[23:2]][31:24] <= axi_wdata[31:24];
			if (axi_wstrb[2]) mem[addr[23:2]][23:16] <= axi_wdata[23:16];
			if (axi_wstrb[1]) mem[addr[23:2]][15: 8] <= axi_wdata[15: 8];
			if (axi_wstrb[0]) mem[addr[23:2]][ 7: 0] <= axi_wdata[ 7: 0];
		end
	end
end
*/
task DUMP_GRB_B;
	begin
		mcd_grb = $fopen(FILE_GRB_B, "w");
		
		for (row = 0; row < HGT; row = row + 1) begin
			for (col = 0; col < WDT; col = col + 1) begin
				ADDR = dvp.grb.addr_b[28:0];
				ADDR = ADDR + (row * WDT + col) * 4;
				DATA = ddr_mdl.mem[ADDR >> 2];
				$fwrite(mcd_grb, "%d,", DATA[31:0]);
			end
			$fwrite(mcd_grb, "\n");
		end
		
		$fclose(mcd_grb);
	end
endtask

task DUMP_RECT;
	begin
		for (lr = 0; lr < 2; lr = lr + 1) begin
			if (lr == 0) begin
				mcd_rect  = $fopen(FILE_RECT_L,  "w");
				mcd_rect2 = $fopen(FILE_RECT_L2, "w");
			end
			else begin
				mcd_rect  = $fopen(FILE_RECT_R,  "w");
				mcd_rect2 = $fopen(FILE_RECT_R2, "w");
			end
			// little endian + continuous
			ADDR = {dvp.rect.base_a[9:0], lr[0], 19'b0};
			for (row = 0; row < HGT; row = row + 1) begin
				for (col = 0; col < WDT; col = col + 4) begin
					DATA = ddr_mdl.mem[ADDR >> 2];
					// CSV format
					$fwrite(mcd_rect, "%d,", DATA[ 7: 0]);
					$fwrite(mcd_rect, "%d,", DATA[15: 8]);
					$fwrite(mcd_rect, "%d,", DATA[23:16]);
					$fwrite(mcd_rect, "%d,", DATA[31:24]);
					// HEX format
					$fwrite(mcd_rect2, "%02X ", DATA[ 7: 0]);
					$fwrite(mcd_rect2, "%02X ", DATA[15: 8]);
					$fwrite(mcd_rect2, "%02X ", DATA[23:16]);
					$fwrite(mcd_rect2, "%02X ", DATA[31:24]);
					ADDR = ADDR + 4;
				end
				$fwrite(mcd_rect,  "\n");
				$fwrite(mcd_rect2, "\n");
			end
			
			$fclose(mcd_rect);
			$fclose(mcd_rect2);
		end
	end
endtask

task DUMP_XSBL;
	begin
		mcd_xsbl_l  = $fopen(FILE_XSBL_L,  "w");
		mcd_xsbl_r  = $fopen(FILE_XSBL_R,  "w");
		mcd_xsbl2_l = $fopen(FILE_XSBL_L2, "w");
		mcd_xsbl2_r = $fopen(FILE_XSBL_R2, "w");
			
		for (row = 0; row < HGT; row = row + 1) begin
			for (col = 0; col < WDT*2; col = col + 4) begin
				ADDR = {
					2'b0,
					dvp.xsbl2.dwr_base_a[9:0],
					row[8:0],
					col[10:0]
				};
				if ((row == 0) | (row == HGT - 1)) begin
					DATA = 0; // invalid lines
				end
				else begin
					DATA = ddr_mdl.mem[ADDR >> 2];
				end
				// CSV format
				$fwrite(mcd_xsbl_l,  "%d,", DATA[31:24]);
				$fwrite(mcd_xsbl_r,  "%d,", DATA[23:16]);
				$fwrite(mcd_xsbl_l,  "%d,", DATA[15: 8]);
				$fwrite(mcd_xsbl_r,  "%d,", DATA[ 7: 0]);
				// HEX format
				$fwrite(mcd_xsbl2_l, "%02X ", DATA[31:24]);
				$fwrite(mcd_xsbl2_r, "%02X ", DATA[23:16]);
				$fwrite(mcd_xsbl2_l, "%02X ", DATA[15: 8]);
				$fwrite(mcd_xsbl2_r, "%02X ", DATA[ 7: 0]);
			end
			$fwrite(mcd_xsbl_l,  "\n");
			$fwrite(mcd_xsbl_r,  "\n");
			$fwrite(mcd_xsbl2_l, "\n");
			$fwrite(mcd_xsbl2_r, "\n");
		end
		
		$fclose(mcd_xsbl_l);
		$fclose(mcd_xsbl_r);
		$fclose(mcd_xsbl2_l);
		$fclose(mcd_xsbl2_r);
	end
endtask

task DUMP_DISP;
	begin
		mcd_disp = $fopen(FILE_DISP, "w");
		mcd_frac = $fopen(FILE_FRAC, "w");
		mcd_cost = $fopen(FILE_COST, "w");
		
		ADDR = {dvp.bm.sad_addr_a[6:0], 22'b0};
		for (hcnt = 0; hcnt < dvp.bm.sad_hgt; hcnt = hcnt + 1) begin
			for (pcnt = 0; pcnt < dvp.bm.sad_wdt; pcnt = pcnt + 1) begin
				DATA = ddr_mdl.mem[ADDR >> 2];
				$fwrite(mcd_disp, "%d,", DATA[31:24]);
				$fwrite(mcd_frac, "%d,", DATA[23:16]);
				$fwrite(mcd_cost, "%d,", DATA[15: 0]);
				ADDR = ADDR + 4;
			end
			$fwrite(mcd_disp, "\n");
			$fwrite(mcd_frac, "\n");
			$fwrite(mcd_cost, "\n");
		end
		
		$fclose(mcd_disp);
		$fclose(mcd_frac);
		$fclose(mcd_cost);
	end
endtask

task DUMP_DISP2;
	begin
		mcd_disp2_short = $fopen(FILE_DISP2_SHORT, "w");
		mcd_disp2_int = $fopen(FILE_DISP2_INT, "w");
		
		ADDR = {dvp.bm.disp2_addr_a[8:0], 20'b0};
		for (hcnt = 0; hcnt < dvp.bm.hgt; hcnt = hcnt + 1) begin
			for (pcnt = 0; pcnt < dvp.bm.wdt / 2; pcnt = pcnt + 1) begin
				DATA = ddr_mdl.mem[ADDR >> 2];
				$fwrite(mcd_disp2_short, "%d,", DATA[15: 0]);
				$fwrite(mcd_disp2_short, "%d,", DATA[31:16]);
				$fwrite(mcd_disp2_int, "%d,", DATA[11: 4]);
				$fwrite(mcd_disp2_int, "%d,", DATA[27:20]);
				ADDR = ADDR + 4;
			end
			$fwrite(mcd_disp2_short, "\n");
			$fwrite(mcd_disp2_int, "\n");
		end
		
		$fclose(mcd_disp2_short);
		$fclose(mcd_disp2_int);
	end
endtask

task DUMP_DISP2_B;
	begin
		mcd_disp2_short = $fopen(FILE_DISP2_SHORT_B, "w");
		mcd_disp2_int = $fopen(FILE_DISP2_INT_B, "w");
		
		ADDR = {dvp.bm.disp2_addr_b[8:0], 20'b0};
		for (hcnt = 0; hcnt < dvp.bm.hgt; hcnt = hcnt + 1) begin
			for (pcnt = 0; pcnt < dvp.bm.wdt / 2; pcnt = pcnt + 1) begin
				DATA = ddr_mdl.mem[ADDR >> 2];
				$fwrite(mcd_disp2_short, "%d,", DATA[15: 0]);
				$fwrite(mcd_disp2_short, "%d,", DATA[31:16]);
				$fwrite(mcd_disp2_int, "%d,", DATA[11: 4]);
				$fwrite(mcd_disp2_int, "%d,", DATA[27:20]);
				ADDR = ADDR + 4;
			end
			$fwrite(mcd_disp2_short, "\n");
			$fwrite(mcd_disp2_int, "\n");
		end
		
		$fclose(mcd_disp2_short);
		$fclose(mcd_disp2_int);
	end
endtask

task DUMP_EIG_A;
	begin
		mcd_eig = $fopen(FILE_EIG_A, "w");
		
		ADDR = BUF_GFTT_A;
		for (row = 0; row < HGT - 4; row = row + 1) begin
			for (col = 0; col < WDT; col = col + 2) begin
				DATA = ddr_mdl.mem[ADDR >> 2];
				$fwrite(mcd_eig, "%d,", DATA[15: 0]);
				$fwrite(mcd_eig, "%d,", DATA[31:16]);
				ADDR = ADDR + 4;
			end
			$fwrite(mcd_eig, "\n");
		end
		
		$fclose(mcd_eig);
	end
endtask


wire	[31:0]	mem_1980;
assign mem_1980 = ddr_mdl.mem[32'h00001980];

reg		[31:0]	addr_1980;
initial begin
	forever @(posedge ddr_mdl.axi_aclk) begin
		if (ddr_mdl.axi_wvalid & (ddr_mdl.addr[23:0] == 24'h1980)) begin
			addr_1980 <= ddr_mdl.axi_wdata[31:0];
		end
	end
end

initial begin
	INIT;
	
	repeat (10) @(posedge clk);
	
	case (SimMode)
	0: begin // I2C
		/*
		ADDR = ADR_I2C_WR;		DATA = 32'h7831_0311;				REG_WR;
		ADDR = ADR_I2C_CMD;		DATA = 32'h0000_0008;				REG_WR;
		ADDR = ADR_I2C_STS;											REG_RD;
		@(negedge dvp.i2c_if.i2c_on);
		*/
		ADDR = ADR_COM_BLINK;	DATA = 32'h0000_0700;				REG_WR;
		
		repeat (1000) @(posedge clk);
		
		gpio_in[1] <= 1'b1;
		repeat (1000) @(posedge clk);
		ADDR = ADR_COM_SW_HOLD;	DATA = 32'h0000_0001;				REG_WR;
		
		gpio_in[1] <= 1'b0;
		repeat (1000) @(posedge clk);
		ADDR = ADR_COM_SW_HOLD;	DATA = 32'h0000_0002;				REG_WR;
		
		gpio_in[1] <= 1'b1;
		repeat (1000) @(posedge clk);
	end
	1: begin // Frame Format Measurement
		// Control
		ADDR = ADR_FFM_CMD;		DATA = 32'h0000_0001;				REG_WR;
		
		repeat (10) @(posedge clk);
	
		imread_enb <= 1'b1; // start data input
		
		@(posedge dvp.frame_meas.cmpl);
		ADDR = ADR_FFM_STS;											REG_RD;
		ADDR = ADR_FFM_V_PER;										REG_RD;
		ADDR = ADR_FFM_H_PER;										REG_RD;
		ADDR = ADR_FFM_V_ACT;										REG_RD;
		ADDR = ADR_FFM_H_ACT;										REG_RD;
		ADDR = ADR_FFM_H_BEG;										REG_RD;
		ADDR = ADR_FFM_V_REF;										REG_RD;
		
		repeat (10) @(posedge tvalid_l);
		
		repeat (10) @(posedge clk);
	end
	2: begin // Frame Grabber
		// Parameter
		ADDR = ADR_COM_INTR_ENB;	DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_GRB_ADDR_A;		DATA = 32'h0000_0000;			REG_WR;
		ADDR = ADR_GRB_ADDR_B;		DATA = 32'h0020_0000;			REG_WR;
		ADDR = ADR_GRB_BST_LEN;		DATA = 32'h0000_003F;			REG_WR;
		ADDR = ADR_GRB_SIZE;		DATA = {HGT[15:0], WDT[15:0]};	REG_WR;
		
		ADDR = ADR_CSI_PTN_SEL1;	DATA = 32'h0000_0005;	        REG_WR;
		ADDR = ADR_CSI_PTN_SEL2;	DATA = 32'h0000_0005;	        REG_WR;
		ADDR = ADR_CSI_FRAME_DECIM;	DATA = 32'h0000_0002;	        REG_WR;
		
		// Control
		ADDR = ADR_ARB_CTRL;		DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_GRB_CTRL;		DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_CSI_CTRL;		DATA = 32'h0000_0003;			REG_WR;
		repeat (10) @(posedge clk);
		imread_enb <= 1'b1;
		
		@(posedge intr);
		DUMP_GRB_A;
		DUMP_GRB_A2;
		ADDR = ADR_COM_INTR_STS;	DATA = 32'h0000_0001;			REG_WR;
		
		//@(posedge intr);
		//DUMP_GRB_B;
		//ADDR = ADR_COM_INTR_STS;	DATA = 32'h0000_0001;			REG_WR;
	end
	3: begin // Rectification
		// Parameter
		ADDR = ADR_SEL_MODE;		DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_RECT_BASE_A;		DATA = 32'h0000_0000;			REG_WR;
		ADDR = ADR_RECT_BASE_B;		DATA = 32'h0020_0000;			REG_WR;
		ADDR = ADR_COM_INTR_ENB;	DATA = 32'h0000_0004;			REG_WR;
		
		ADDR = ADR_CSI_PTN_SEL1;	DATA = 32'h0000_0000;	        REG_WR;
		ADDR = ADR_CSI_PTN_SEL2;	DATA = 32'h0000_0000;	        REG_WR;
		
		ADDR = ADR_RECT_L_FX;		DATA = 40419817;				REG_WR;
		ADDR = ADR_RECT_L_FY;		DATA = 40382910;				REG_WR;
		ADDR = ADR_RECT_R_FX;		DATA = 39609530;				REG_WR;
		ADDR = ADR_RECT_R_FY;		DATA = 39627967;				REG_WR;
		ADDR = ADR_RECT_CXY;		DATA = {16'd320, 16'd240};		REG_WR;
		ADDR = ADR_RECT_FX2_INV;	DATA =  6338213;				REG_WR;
		ADDR = ADR_RECT_FY2_INV;	DATA =  6338213;				REG_WR;
		ADDR = ADR_RECT_CX2_FX2;	DATA =  4984405;				REG_WR;
		ADDR = ADR_RECT_CY2_FY2;	DATA =  5932596;				REG_WR;
		ADDR = ADR_RECT_L_R11;		DATA = 16598538;				REG_WR;
		ADDR = ADR_RECT_L_R12;		DATA =  -120818;				REG_WR;
		ADDR = ADR_RECT_L_R13;		DATA =  2439034;				REG_WR;
		ADDR = ADR_RECT_L_R21;		DATA =   137992;				REG_WR;
		ADDR = ADR_RECT_L_R22;		DATA = 16776300;				REG_WR;
		ADDR = ADR_RECT_L_R23;		DATA =  -108069;				REG_WR;
		ADDR = ADR_RECT_L_R31;		DATA = -2438123;				REG_WR;
		ADDR = ADR_RECT_L_R32;		DATA =   126979;				REG_WR;
		ADDR = ADR_RECT_L_R33;		DATA = 16598626;				REG_WR;
		ADDR = ADR_RECT_R_R11;		DATA = 16569087;				REG_WR;
		ADDR = ADR_RECT_R_R12;		DATA =   -69780;				REG_WR;
		ADDR = ADR_RECT_R_R13;		DATA =  2633522;				REG_WR;
		ADDR = ADR_RECT_R_R21;		DATA =    51223;				REG_WR;
		ADDR = ADR_RECT_R_R22;		DATA = 16776692;				REG_WR;
		ADDR = ADR_RECT_R_R23;		DATA =   122251;				REG_WR;
		ADDR = ADR_RECT_R_R31;		DATA = -2633948;				REG_WR;
		ADDR = ADR_RECT_R_R32;		DATA =  -112694;				REG_WR;
		ADDR = ADR_RECT_R_R33;		DATA = 16568783;				REG_WR;
		
		// Control
		ADDR = ADR_ARB_CTRL;		DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_RECT_CTRL;		DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_CSI_CTRL;		DATA = 32'h0000_0003;			REG_WR;
		
		repeat (10) @(posedge clk);
		
		CMD_WR;
		
		repeat (10) @(posedge clk);
	
		imread_enb <= 1'b1; // start data input
		
		@(posedge intr);
		ADDR = ADR_COM_INTR_STS;	DATA = 32'h0000_0004;			REG_WR;
		DUMP_RECT;
		repeat (100) @(posedge clk);
	end
	4: begin // X-Sobel
		// Parameter
		ADDR = ADR_XSBL_RADDR_A;	DATA = 32'h0000_0000;			REG_WR;
		ADDR = ADR_XSBL_RADDR_B;	DATA = 32'h0020_0000;			REG_WR;
		ADDR = ADR_XSBL_WADDR_A;	DATA = 32'h0040_0000;			REG_WR;
		ADDR = ADR_XSBL_WADDR_B;	DATA = 32'h0060_0000;			REG_WR;
		ADDR = ADR_XSBL_SIZE;		DATA = {HGT[15:0], WDT[15:0]};	REG_WR;
		ADDR = ADR_COM_INTR_ENB;	DATA = 32'h0000_0008;			REG_WR;
		
		LOAD_RECT;
		
		// Control
		ADDR = ADR_ARB_CTRL;		DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_XSBL_CTRL;		DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_CSI_CTRL;		DATA = 32'h0000_0003;			REG_WR;
		
		repeat (10) @(posedge clk);
		
		dvp.xsbl2.xsbl_sim_start <= 1'b1;
		@(posedge clk);
		dvp.xsbl2.xsbl_sim_start <= 1'b0;
		
		@(posedge intr);
		ADDR = ADR_COM_INTR_STS;	DATA = 32'h0000_0002;			REG_WR;
		DUMP_XSBL;
		repeat (100) @(posedge clk);
	end
	5: begin // Block Matching
		// Parameter
		ADDR = ADR_BM_SIZE;			DATA = {HGT[15:0], WDT[15:0]};	REG_WR;
		ADDR = ADR_BM_LR_ADDR_A;	DATA = 32'h0040_0000;			REG_WR;
		ADDR = ADR_BM_LR_ADDR_B;	DATA = 32'h0060_0000;			REG_WR;
		ADDR = ADR_BM_SAD_ADDR_A;	DATA = 32'h0080_0000;			REG_WR;
		ADDR = ADR_BM_SAD_ADDR_B;	DATA = 32'h00C0_0000;			REG_WR;
		ADDR = ADR_COM_INTR_ENB;	DATA = 32'h0000_0002;			REG_WR;
		ADDR = ADR_BM_DISP2_ADDR_A;	DATA = 32'h0000_0000;			REG_WR;
		ADDR = ADR_BM_DISP2_ADDR_B;	DATA = 32'h0010_0000;			REG_WR;
		
		ADDR = ADR_CSI_PTN_SEL1;	DATA = 32'h0000_0000;	        REG_WR;
		ADDR = ADR_CSI_PTN_SEL2;	DATA = 32'h0000_0000;	        REG_WR;
		ADDR = ADR_CSI_FRAME_DECIM;	DATA = 32'h0000_0000;	        REG_WR;
		
		LOAD_XSBL;
		LOAD_XSBL_B;
		
		// Control
		ADDR = ADR_ARB_CTRL;		DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_BM_CTRL;			DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_CSI_CTRL;		DATA = 32'h0000_0003;			REG_WR;
		
		repeat (10) @(posedge clk);
		
		dvp.bm.bm_sim_start <= 1'b1;
		@(posedge clk);
		dvp.bm.bm_sim_start <= 1'b0;
		
		@(posedge intr);
		ADDR = ADR_COM_INTR_STS;	DATA = 32'h0000_0002;			REG_WR;
		DUMP_DISP;
		DUMP_DISP2;
		///////////////////////////////////////////////////////////////////
		/*
		repeat (10) @(posedge clk);
		dvp.bm.bm_sim_start <= 1'b1;
		@(posedge clk);
		dvp.bm.bm_sim_start <= 1'b0;
		@(posedge intr);
		ADDR = ADR_COM_INTR_STS;	DATA = 32'h0000_0002;			REG_WR;
		DUMP_DISP2_B;
		*/
		///////////////////////////////////////////////////////////////////
		
		repeat (100) @(posedge clk);
	end
	6: begin // Rect + X-Sobel + BM
		// Parameter
		ADDR = ADR_SEL_MODE;		DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_RECT_BASE_A;		DATA = 32'h0000_0000;			REG_WR;
		ADDR = ADR_RECT_BASE_B;		DATA = 32'h0020_0000;			REG_WR;
		ADDR = ADR_XSBL_RADDR_A;	DATA = 32'h0000_0000;			REG_WR;
		ADDR = ADR_XSBL_RADDR_B;	DATA = 32'h0020_0000;			REG_WR;
		ADDR = ADR_XSBL_WADDR_A;	DATA = 32'h0040_0000;			REG_WR;
		ADDR = ADR_XSBL_WADDR_B;	DATA = 32'h0060_0000;			REG_WR;
		ADDR = ADR_XSBL_SIZE;		DATA = {HGT[15:0], WDT[15:0]};	REG_WR;
		ADDR = ADR_BM_SIZE;			DATA = {HGT[15:0], WDT[15:0]};	REG_WR;
		ADDR = ADR_BM_LR_ADDR_A;	DATA = 32'h0040_0000;			REG_WR;
		ADDR = ADR_BM_LR_ADDR_B;	DATA = 32'h0060_0000;			REG_WR;
		ADDR = ADR_BM_SAD_ADDR_A;	DATA = 32'h0080_0000;			REG_WR;
		ADDR = ADR_BM_SAD_ADDR_B;	DATA = 32'h00C0_0000;			REG_WR;
		ADDR = ADR_BM_DISP2_ADDR_A;	DATA = 32'h0000_0000;			REG_WR;
		ADDR = ADR_BM_DISP2_ADDR_B;	DATA = 32'h0020_0000;			REG_WR;
		ADDR = ADR_COM_INTR_ENB;	DATA = 32'h0000_0002;			REG_WR;
		
		ADDR = ADR_RECT_L_FX;		DATA = 40419817;				REG_WR;
		ADDR = ADR_RECT_L_FY;		DATA = 40382910;				REG_WR;
		ADDR = ADR_RECT_R_FX;		DATA = 39609530;				REG_WR;
		ADDR = ADR_RECT_R_FY;		DATA = 39627967;				REG_WR;
		ADDR = ADR_RECT_CXY;		DATA = {16'd320, 16'd240};		REG_WR;
		ADDR = ADR_RECT_FX2_INV;	DATA =  6338213;				REG_WR;
		ADDR = ADR_RECT_FY2_INV;	DATA =  6338213;				REG_WR;
		ADDR = ADR_RECT_CX2_FX2;	DATA =  4984405;				REG_WR;
		ADDR = ADR_RECT_CY2_FY2;	DATA =  5932596;				REG_WR;
		ADDR = ADR_RECT_L_R11;		DATA = 16598538;				REG_WR;
		ADDR = ADR_RECT_L_R12;		DATA =  -120818;				REG_WR;
		ADDR = ADR_RECT_L_R13;		DATA =  2439034;				REG_WR;
		ADDR = ADR_RECT_L_R21;		DATA =   137992;				REG_WR;
		ADDR = ADR_RECT_L_R22;		DATA = 16776300;				REG_WR;
		ADDR = ADR_RECT_L_R23;		DATA =  -108069;				REG_WR;
		ADDR = ADR_RECT_L_R31;		DATA = -2438123;				REG_WR;
		ADDR = ADR_RECT_L_R32;		DATA =   126979;				REG_WR;
		ADDR = ADR_RECT_L_R33;		DATA = 16598626;				REG_WR;
		ADDR = ADR_RECT_R_R11;		DATA = 16569087;				REG_WR;
		ADDR = ADR_RECT_R_R12;		DATA =   -69780;				REG_WR;
		ADDR = ADR_RECT_R_R13;		DATA =  2633522;				REG_WR;
		ADDR = ADR_RECT_R_R21;		DATA =    51223;				REG_WR;
		ADDR = ADR_RECT_R_R22;		DATA = 16776692;				REG_WR;
		ADDR = ADR_RECT_R_R23;		DATA =   122251;				REG_WR;
		ADDR = ADR_RECT_R_R31;		DATA = -2633948;				REG_WR;
		ADDR = ADR_RECT_R_R32;		DATA =  -112694;				REG_WR;
		ADDR = ADR_RECT_R_R33;		DATA = 16568783;				REG_WR;
		
		// Control
		ADDR = ADR_ARB_CTRL;		DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_RECT_CTRL;		DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_XSBL_CTRL;		DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_BM_CTRL;			DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_CSI_CTRL;		DATA = 32'h0000_0003;			REG_WR;
		
		INIT_XSBL;
		
		repeat (10) @(posedge clk);
		
		CMD_WR;
		
		repeat (10) @(posedge clk);
	
		imread_enb <= 1'b1; // start data input
		
		@(posedge intr);
		ADDR = ADR_COM_INTR_STS;	DATA = 32'h0000_0002;			REG_WR;
		DUMP_RECT;
		DUMP_XSBL;
		DUMP_DISP;
		repeat (100) @(posedge clk);
	end
	7: begin
		LOAD_RECT;
		repeat (10) @(posedge clk);
		
		ADDR = ADR_GFTT_RADDR_A;	DATA = BUF_RECT_A;				REG_WR;
		ADDR = ADR_GFTT_RADDR_B;	DATA = BUF_RECT_B;				REG_WR;
		ADDR = ADR_GFTT_WADDR_A;	DATA = BUF_GFTT_A;				REG_WR;
		ADDR = ADR_GFTT_WADDR_B;	DATA = BUF_GFTT_B;				REG_WR;
		ADDR = ADR_GFTT_BST_LEN;	DATA = 32'd31;					REG_WR;
		ADDR = ADR_GFTT_SIZE;		DATA = {HGT[15:0], WDT[15:0]};	REG_WR;
		ADDR = ADR_ARB_CTRL;		DATA = 32'h0000_0001;			REG_WR;
		ADDR = ADR_GFTT_CTRL;		DATA = 32'h0000_0001;			REG_WR;
		
		ADDR = ADR_COM_INTR_ENB;	DATA = 32'h0000_0020;			REG_WR;
		
		repeat (10) @(posedge clk);
		dvp.gftt.gftt_sim_start <= 1'b1;
		@(posedge clk);
		dvp.gftt.gftt_sim_start <= 1'b0;
		
		@(posedge intr);
		DUMP_EIG_A;
	end
	default: begin
	end
	endcase
	
	repeat (10) @(posedge clk);
	
	$finish;
end



endmodule
