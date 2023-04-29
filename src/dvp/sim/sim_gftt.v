`timescale 1ns / 1ps

module sim_gftt;

//==========================================================================
// Parameter
//==========================================================================
//parameter FILE_REF_RECT_L = "../../../../../../src/dvp/sim/ref/ref_rect_l.dat";
//parameter FILE_REF_RECT_L = "../../../../../../src/dvp/sim/out_220308/out_rect_l.dat";
parameter FILE_REF_RECT_L = "../../../../../../src/dvp/sim/src.dat";
parameter FILE_REF_RECT_R = "../../../../../../src/dvp/sim/ref/ref_rect_r.dat";
parameter FILE_EIG_A      = "../../../../../../src/dvp/sim/out_gftt_eig_a.csv";
parameter FILE_EIG_B      = "../../../../../../src/dvp/sim/out_gftt_eig_b.csv";
parameter FILE_XSBL       = "../../../../../../src/dvp/sim/out_gftt_xsbl.csv";
parameter FILE_YSBL       = "../../../../../../src/dvp/sim/out_gftt_ysbl.csv";
parameter FILE_BOX_A      = "../../../../../../src/dvp/sim/out_gftt_box_a.csv";
parameter FILE_BOX_B      = "../../../../../../src/dvp/sim/out_gftt_box_b.csv";
parameter FILE_BOX_C      = "../../../../../../src/dvp/sim/out_gftt_box_c.csv";

parameter HGT = 480;
parameter WDT = 640;
parameter BUF_RECT_A = 32'h0000_0000;
parameter BUF_RECT_B = 32'h0010_0000;
parameter BUF_GFTT_A = 32'h00A0_0000;
parameter BUF_GFTT_B = 32'h00B0_0000;


//==========================================================================
// Register Map
//==========================================================================
parameter ADR_ARB_CTRL     = 14'h1800;
parameter ADR_GFTT_CTRL    = 14'h1A00;
parameter ADR_GFTT_STS     = 14'h1A04;
parameter ADR_GFTT_RADDR_A = 14'h1A08;
parameter ADR_GFTT_RADDR_B = 14'h1A0C;
parameter ADR_GFTT_WADDR_A = 14'h1A10;
parameter ADR_GFTT_WADDR_B = 14'h1A14;
parameter ADR_GFTT_BST_LEN = 14'h1A18;
parameter ADR_GFTT_SIZE    = 14'h1A1C;
parameter ADR_GFTT_MAX     = 14'h1A20;
parameter ADR_GFTT_STOP    = 14'h1A24;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
integer			ADDR, DATA;
integer			i, lr, row, col;
integer			mcd_eig;
integer			mcd_xsbl, mcd_ysbl, sbl_col, sbl_row, sbl_done;
integer			mcd_box_a, mcd_box_b, mcd_box_c, box_col, box_row, box_done;

// Reset/Clock Generation
reg				clk, rst_n;

// Internal Bus
reg				ibus_cs;
reg				ibus_wr;
reg		[5:0]	ibus_addr_7_2;
reg		[31:0]	ibus_wrdata;

// gftt
wire			gftt_done;
wire	[31:0]	ibus_rddata_gftt;
reg				rect_done;
wire			drd_req;
wire			drd_vin;
wire	[31:0]	drd_din;
wire			drd_vout;
wire	[31:0]	drd_dout;
wire			dwr_req;
wire			dwr_vout;
wire	[31:0]	dwr_dout;

// arb
wire	[5:0]	arb_req;
wire			drd_ack, dwr_ack;
wire	[3:0]	arb_strb;
wire			arb_vin;
wire	[31:0]	arb_din;
wire	[31:0]	ibus_rddata_arb;
wire	[5:0]	arb_done;
wire	[5:0]	arb_ack;
wire			arb_vin;
wire	[31:0]	arb_din;
wire	[3:0]	arb_strb;
wire			arb_vout;
wire	[31:0]	arb_dout;
wire			axi_aclk;
wire	[31:0]	axi_awaddr;
wire	[7:0]	axi_awlen;
wire			axi_awvalid;
wire	[31:0]	axi_wdata;
wire	[3:0]	axi_wstrb;
wire			axi_wvalid;
wire			axi_wlast;
wire	[31:0]	axi_araddr;
wire	[7:0]	axi_arlen;
wire			axi_arvalid;
wire			axi_rready;
wire			axi_bready;

// ddr_mdl
reg		[7:0]	tmp_mem[WDT*HGT-1:0];
wire			axi_awready;
wire			axi_wready;
wire			axi_arready;
wire	[31:0]	axi_rdata;
wire			axi_rvalid;
wire			axi_rlast;
wire			axi_bvalid;


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

assign axi_aclk = clk;


//==========================================================================
// Target Module
//==========================================================================
assign gftt_done = arb_done[1];
assign drd_vin = arb_vout;
assign drd_din[31:0] = arb_dout[31:0];

gftt gftt (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Internal Bus I/F
	.ibus_cs (ibus_cs),
	.ibus_wr (ibus_wr),
	.ibus_addr_7_2 (ibus_addr_7_2[5:0]),
	.ibus_wrdata (ibus_wrdata[31:0]),
	.ibus_rddata (ibus_rddata_gftt[31:0]),
	
	// Control
	.rect_done (rect_done),
	.gftt_done (gftt_done),
	.rect_fcnt (4'b0),
	.gftt_fcnt (),
	
	// DDR Read Arbiter I/F
	.drd_req  (drd_req),
	.drd_ack  (drd_ack),
	.drd_vin  (drd_vin),
	.drd_din  (drd_din),
	.drd_vout (drd_vout),
	.drd_dout (drd_dout),
	
	// DDR Write Arbiter I/F
	.dwr_req  (dwr_req),
	.dwr_ack  (dwr_ack),
	.dwr_vout (dwr_vout),
	.dwr_dout (dwr_dout)
);


//==========================================================================
// Arbiter
//--------------------------------------------------------------------------
// [5]: xsblw
// [4]: rect
// [3]: dwr
// [2]: drd
// [1]: xsblr
// [0]: grb
//==========================================================================
assign arb_req  = {4'b0, dwr_req, drd_req};
assign drd_ack  = arb_ack[0];
assign dwr_ack  = arb_ack[1];
assign arb_strb = 4'hF;
assign arb_vin  = dwr_vout | drd_vout;
assign arb_din  = dwr_dout[31:0] | drd_dout[31:0];

arb arb (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Internal Bus I/F
	.ibus_cs (ibus_cs),
	.ibus_wr (ibus_wr),
	.ibus_addr_7_2 (ibus_addr_7_2[5:0]),
	.ibus_wrdata   (ibus_wrdata[31:0]),
	.ibus_rddata   (ibus_rddata_arb[31:0]),
	
	// Status
	.done (arb_done[5:0]),
	
	// FIFO I/F
	.req (arb_req),
	.ack (arb_ack),
	.valid_in  (arb_vin),
	.data_in   (arb_din),
	.strb_in   (arb_strb),
	.valid_out (arb_vout),
	.data_out  (arb_dout),
	
	// AXI I/F
	.axi_aclk    (axi_aclk),
	.axi_awaddr  (axi_awaddr),
	.axi_awlen   (axi_awlen),
	.axi_awvalid (axi_awvalid),
	.axi_awready (axi_awready),
	.axi_wdata   (axi_wdata),
	.axi_wstrb   (axi_wstrb),
	.axi_wvalid  (axi_wvalid),
	.axi_wready  (axi_wready),
	.axi_wlast   (axi_wlast),
	.axi_araddr  (axi_araddr),
	.axi_arlen   (axi_arlen),
	.axi_arvalid (axi_arvalid),
	.axi_arready (axi_arready),
	.axi_rdata   (axi_rdata),
	.axi_rvalid  (axi_rvalid),
	.axi_rready  (axi_rready),
	.axi_rlast   (axi_rlast),
	.axi_bvalid  (axi_bvalid),
	.axi_bready  (axi_bready)
);

//==========================================================================
// DDR Memory Model
//  16MB Space: 0000_0000 - 00FF_FFFF (16MB)
//--------------------------------------------------------------------------
//  BUF_GRB_A : 0000_0000 - 001F_FFFF (2MB)
//  BUF_GRB_B : 0020_0000 - 003F_FFFF (2MB)
//  BUF_RECT_A: 0000_0000 - 001F_FFFF (2MB)
//  BUF_RECT_B: 0020_0000 - 003F_FFFF (2MB)
//  BUF_XSBL_A: 0040_0000 - 005F_FFFF (2MB)
//  BUF_XSBL_B: 0060_0000 - 007F_FFFF (2MB)
//  BUF_BM_A  : 0080_0000 - 00BF_FFFF (4MB)
//  BUF_BM_B  : 00C0_0000 - 00FF_FFFF (4MB)
//--------------------------------------------------------------------------
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
	.axi_awaddr  (axi_awaddr),
	.axi_awlen   (axi_awlen),
	.axi_awvalid (axi_awvalid),
	.axi_awready (axi_awready),
	.axi_wdata   (axi_wdata),
	.axi_wstrb   (axi_wstrb),
	.axi_wvalid  (axi_wvalid),
	.axi_wready  (axi_wready),
	.axi_wlast   (axi_wlast),
	.axi_araddr  (axi_araddr),
	.axi_arlen   (axi_arlen),
	.axi_arvalid (axi_arvalid),
	.axi_arready (axi_arready),
	.axi_rdata   (axi_rdata),
	.axi_rvalid  (axi_rvalid),
	.axi_rready  (axi_rready),
	.axi_rlast   (axi_rlast),
	.axi_bvalid  (axi_bvalid),
	.axi_bready  (axi_bready)
);
/*
reg		[31:0]	mem_3300;
initial begin
	forever @(posedge ddr_mdl.axi_aclk) begin
		if (ddr_mdl.axi_wvalid & (ddr_mdl.addr[23:0] == 24'h00_3300)) begin
			mem_3300 <= ddr_mdl.axi_wdata[31:0];
		end
	end
end
*/

//==========================================================================
// Simulation Control
//==========================================================================
/*
task LOAD_RECT_A;
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
					DATA = {
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

task LOAD_RECT_B;
	begin
		for (lr = 0; lr < 2; lr = lr + 1) begin
			if (lr == 0) $readmemh(FILE_REF_RECT_L, tmp_mem);
			else         $readmemh(FILE_REF_RECT_R, tmp_mem);
			
			i = 0;
			for (row = 0; row < HGT; row = row + 1) begin
				for (col = 0; col < WDT; col = col + 4) begin
					ADDR = {
						2'b0,
						BUF_RECT_B[29:20],
						lr[0],
						row[8:0],
						col[9:0]
					};
					DATA = {
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
task LOAD_RECT_A;
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

task LOAD_RECT_B;
	begin
		for (lr = 0; lr < 2; lr = lr + 1) begin
			if (lr == 0) $readmemh(FILE_REF_RECT_L, tmp_mem);
			else         $readmemh(FILE_REF_RECT_R, tmp_mem);
			
			i = 0;
			ADDR = {2'b0, BUF_RECT_B[29:20], lr[0], 19'b0};
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

task DUMP_EIG_B;
	begin
		mcd_eig = $fopen(FILE_EIG_B, "w");
		
		ADDR = BUF_GFTT_B;
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

task INIT;
	begin
		ibus_cs <= 1'b0;
		ibus_wr <= 1'b0;
		ibus_addr_7_2 <= 6'b0;
		ibus_wrdata   <= 32'b0;
		rect_done <= 1'b0;
	end
endtask

task REG_WR;
	begin
		ibus_cs <= 1'b1;
		ibus_wrdata   <= DATA;
		ibus_addr_7_2 <= ADDR[7:2];
		ibus_wr <= 1'b1;
		@(posedge clk);
		ibus_cs <= 1'b0;
		ibus_wr <= 1'b0;
	end
endtask

initial begin
	mcd_xsbl = $fopen(FILE_XSBL, "w");
	mcd_ysbl = $fopen(FILE_YSBL, "w");
	sbl_col = 0;
	sbl_row = 0;
	sbl_done = 0;
	while (sbl_done == 0) begin
		@(posedge clk);
		if (gftt.sbl_vout) begin
			$fwrite(mcd_xsbl, "%d,", gftt.xsbl[11:0]);
			$fwrite(mcd_ysbl, "%d,", gftt.ysbl[11:0]);
			if (sbl_col == WDT - 1) begin
				$fwrite(mcd_xsbl, "\n");
				$fwrite(mcd_ysbl, "\n");
				if (sbl_row == HGT - 3) begin
					sbl_done = 1;
				end
				else begin
					sbl_row = sbl_row + 1;
					sbl_col = 0;
				end
			end
			else begin
				sbl_col = sbl_col + 1;
			end
		end
	end
	$fclose(mcd_xsbl);
	$fclose(mcd_ysbl);
end

initial begin
	mcd_box_a = $fopen(FILE_BOX_A, "w");
	mcd_box_b = $fopen(FILE_BOX_B, "w");
	mcd_box_c = $fopen(FILE_BOX_C, "w");
	box_col = 0;
	box_row = 0;
	box_done = 0;
	while (box_done == 0) begin
		@(posedge clk);
		if (gftt.gftt_eig.box_vout) begin
			$fwrite(mcd_box_a, "%d,", gftt.gftt_eig.a[15:0]);
			$fwrite(mcd_box_b, "%d,", gftt.gftt_eig.b[15:0]);
			$fwrite(mcd_box_c, "%d,", gftt.gftt_eig.c[15:0]);
			if (box_col == WDT - 1) begin
				$fwrite(mcd_box_a, "\n");
				$fwrite(mcd_box_b, "\n");
				$fwrite(mcd_box_c, "\n");
				if (box_row == HGT - 5) begin
					box_done = 1;
				end
				else begin
					box_row = box_row + 1;
					box_col = 0;
				end
			end
			else begin
				box_col = box_col + 1;
			end
		end
	end
	$fclose(mcd_box_a);
	$fclose(mcd_box_b);
	$fclose(mcd_box_c);
end

initial begin
	INIT;
	LOAD_RECT_A;
	LOAD_RECT_B;
	repeat (10) @(posedge clk);
	
	ADDR = ADR_GFTT_RADDR_A;	DATA = BUF_RECT_A;				REG_WR;
	ADDR = ADR_GFTT_RADDR_B;	DATA = BUF_RECT_B;				REG_WR;
	ADDR = ADR_GFTT_WADDR_A;	DATA = BUF_GFTT_A;				REG_WR;
	ADDR = ADR_GFTT_WADDR_B;	DATA = BUF_GFTT_B;				REG_WR;
	ADDR = ADR_GFTT_BST_LEN;	DATA = 32'd31;					REG_WR;
	ADDR = ADR_GFTT_SIZE;		DATA = {HGT[15:0], WDT[15:0]};	REG_WR;
	ADDR = ADR_ARB_CTRL;		DATA = 32'h0000_0001;			REG_WR;
	ADDR = ADR_GFTT_CTRL;		DATA = 32'h0000_0001;			REG_WR;
	//ADDR = ADR_GFTT_STOP;		DATA = 32'h0000_0001;			REG_WR;
	
	repeat (10) @(posedge clk);
	rect_done <= 1'b1;
	@(posedge clk);
	rect_done <= 1'b0;
	
	@(posedge arb_done[1]);
	DUMP_EIG_A;
	
	//////////////////////////////////
	// bank B
	repeat (1000) @(posedge clk);
	rect_done <= 1'b1;
	@(posedge clk);
	rect_done <= 1'b0;
	
	@(posedge arb_done[1]);
	DUMP_EIG_B;
	//////////////////////////////////
	
	repeat (100) @(posedge clk);
	$finish;
end


endmodule
