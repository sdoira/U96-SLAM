`timescale 1ns / 1ps

module sim_bm_obuf2;

//==========================================================================
// Parameter
//==========================================================================
parameter	img_hgt = 480;
parameter	img_wdt = 640;
parameter	sad_hgt = 460;
parameter	sad_wdt = 491;
parameter	addr_a = 32'h00000000;
parameter	addr_b = 32'h00200000;


//==========================================================================
// Register Map
//==========================================================================
parameter ADR_ARB_CTRL      = 14'h1800;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
integer			ADDR, DATA;
integer			frame, row, col;
integer			i, j, lr, row, col, hcnt, pcnt;
integer			mcd_disp2_short, mcd_disp2_int;

// Reset/Clock Generation
reg				clk, rst_n;

// bm_obuf2
reg				enb;
reg		[15:0]	din;
reg				wr;
wire			dwr_req;
wire			dwr_ack;
wire			dwr_vout;
wire	[31:0]	dwr_dout;

// arb
reg				ibus_cs;
reg				ibus_wr;
reg		[5:0]	ibus_addr_7_2;
reg		[31:0]	ibus_wrdata;
wire	[31:0]	ibus_rddata;
wire	[5:0]	arb_done;
wire	[5:0]	arb_req;
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
bm_obuf2 bm_obuf2 (
	// Global Control
	.rst_n (rst_n),
	.clk (clk),
	
	// Parameter
	.img_hgt (img_hgt[8:0]),
	.img_wdt (img_wdt[9:0]),
	.hwsz (4'd10),
	.ndisp (9'd128),
	.addr_a (addr_a[31:20]),
	.addr_b (addr_b[31:20]),
	
	// Control
	.enb (enb),
	
	// Status
	.ovf (),
	.udf (),
	
	// OBUF I/F
	.din (din),
	.wr (wr),
	
	// DDR Arbiter I/F
	.dwr_req (dwr_req),
	.dwr_ack (dwr_ack),
	.dwr_vout (dwr_vout),
	.dwr_dout (dwr_dout)
);

assign arb_req  = {5'b0, dwr_req};
assign dwr_ack  = arb_ack[0];
assign arb_strb = 4'hF;
assign arb_vin  = dwr_vout;
assign arb_din  = dwr_dout;

arb arb (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Internal Bus I/F
	.ibus_cs (ibus_cs),
	.ibus_wr (ibus_wr),
	.ibus_addr_7_2 (ibus_addr_7_2[5:0]),
	.ibus_wrdata   (ibus_wrdata[31:0]),
	.ibus_rddata   (),
	
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
reg		[31:0]	mem_3304;
reg		[31:0]	mem_3308;
reg		[31:0]	mem_330C;
reg		[31:0]	mem_3310;
reg		[31:0]	mem_3314;
reg		[31:0]	mem_3318;
reg		[31:0]	mem_331C;
initial begin
	forever @(posedge ddr_mdl.axi_aclk) begin
		if (ddr_mdl.axi_wvalid & (ddr_mdl.addr[23:0] == 24'h00_3300)) begin
			mem_3300 <= ddr_mdl.axi_wdata[31:0];
		end
		if (ddr_mdl.axi_wvalid & (ddr_mdl.addr[23:0] == 24'h00_3304)) begin
			mem_3304 <= ddr_mdl.axi_wdata[31:0];
		end
		if (ddr_mdl.axi_wvalid & (ddr_mdl.addr[23:0] == 24'h00_3308)) begin
			mem_3308 <= ddr_mdl.axi_wdata[31:0];
		end
		if (ddr_mdl.axi_wvalid & (ddr_mdl.addr[23:0] == 24'h00_330C)) begin
			mem_330C <= ddr_mdl.axi_wdata[31:0];
		end
		if (ddr_mdl.axi_wvalid & (ddr_mdl.addr[23:0] == 24'h00_3310)) begin
			mem_3310 <= ddr_mdl.axi_wdata[31:0];
		end
		if (ddr_mdl.axi_wvalid & (ddr_mdl.addr[23:0] == 24'h00_3314)) begin
			mem_3314 <= ddr_mdl.axi_wdata[31:0];
		end
		if (ddr_mdl.axi_wvalid & (ddr_mdl.addr[23:0] == 24'h00_3318)) begin
			mem_3318 <= ddr_mdl.axi_wdata[31:0];
		end
		if (ddr_mdl.axi_wvalid & (ddr_mdl.addr[23:0] == 24'h00_331C)) begin
			mem_331C <= ddr_mdl.axi_wdata[31:0];
		end
	end
end
*/

//==========================================================================
// Simulation Control
//==========================================================================
parameter FILE_DISP2_SHORT_A = "../../../../dvp.srcs/sim_1/new/out_disp2_short_a.csv";
parameter FILE_DISP2_INT_A   = "../../../../dvp.srcs/sim_1/new/out_disp2_int_a.csv";
parameter FILE_DISP2_SHORT_B = "../../../../dvp.srcs/sim_1/new/out_disp2_short_b.csv";
parameter FILE_DISP2_INT_B   = "../../../../dvp.srcs/sim_1/new/out_disp2_int_b.csv";


task DUMP_DISP2_A;
	begin
		mcd_disp2_short = $fopen(FILE_DISP2_SHORT_A, "w");
		mcd_disp2_int = $fopen(FILE_DISP2_INT_A, "w");
		
		ADDR = addr_a;
		for (hcnt = 0; hcnt < img_hgt; hcnt = hcnt + 1) begin
			for (pcnt = 0; pcnt < img_wdt / 2; pcnt = pcnt + 1) begin
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
		
		ADDR = addr_b;
		for (hcnt = 0; hcnt < img_hgt; hcnt = hcnt + 1) begin
			for (pcnt = 0; pcnt < img_wdt / 2; pcnt = pcnt + 1) begin
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

task INIT;
	begin
		enb <= 1'b0;
		din <= 16'b0;
		wr  <= 1'b0;
		ibus_cs <= 1'b0;
		ibus_wr <= 1'b0;
		ibus_addr_7_2 <= 6'b0;
		ibus_wrdata   <= 32'b0;
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
	INIT;
	repeat (10) @(posedge clk);
	
	ADDR = ADR_ARB_CTRL;		DATA = 32'h0000_0001;			REG_WR;
	
	enb <= 1'b1;
	repeat (100) @(posedge clk);
	
	frame = 0;
	row = 0;
	col = 0;
	ADDR = 0;
	
	for (i = 0; i < sad_hgt * 2; i = i + 1) begin
		col = 0;
		for (j = 0; j < sad_wdt; j = j + 1) begin
			//din <= {frame[0], row[6:0], col[7:0]};
			din <= {row[1:0], col[5:0], 8'b0};
			wr  <= 1'b1;
			@(posedge clk);
			col = col + 1;
		end
		wr  <= 1'b0;
		@(posedge clk);
		
		if (row == sad_hgt - 1) begin
			row = 0;
			repeat (500) @(posedge clk);
			if (frame == 0) begin
				DUMP_DISP2_A;
			end
			else if (frame == 1) begin
				DUMP_DISP2_B;
			end
			frame = frame + 1;
		end
		else begin
			row = row + 1;
			repeat (100) @(posedge clk);
		end
	end
	
	repeat (100) @(posedge clk);
	$finish;
end


endmodule
