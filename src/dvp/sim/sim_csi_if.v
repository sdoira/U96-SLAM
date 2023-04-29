`timescale 1ns / 1ps

module sim_csi_if;


//==========================================================================
// Definition
//==========================================================================
// Input Data
parameter FILE_IN_L       = "../../../../../../src/dvp/sim/img_001_l.dat";
parameter FILE_IN_R       = "../../../../../../src/dvp/sim/img_001_r.dat";


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
integer				ADDR, DATA;

// Reset/Clock Generation
reg					clk, rst_n;

// DVP Generation
reg					imread_enb_l;
wire				tuser_l, tvalid_l, tlast_l;
wire	[15:0]		tdata_l;
reg					imread_enb_r;
wire				tuser_r, tvalid_r, tlast_r;
wire	[15:0]		tdata_r;

// csi_if
reg					ibus_cs, ibus_wr;
reg		[7:0]		ibus_addr;
reg		[31:0]		ibus_wrdata;
wire	[31:0]		ibus_rddata;
wire				tready_l, tready_r;
wire				sof_out, vout;
wire	[15:0]		d1_out, d2_out;


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


//==========================================================================
// DVP Generation
//==========================================================================
imread_csi #(FILE_IN_L) imread_csi_l (
	.clk    (clk),
	.en     (imread_enb_l),
	.tuser  (tuser_l),
	.tvalid (tvalid_l),
	.tready (tready_l),
	.tdata  (tdata_l),
	.tlast  (tlast_l)
);

imread_csi #(FILE_IN_R) imread_csi_r (
	.clk    (clk),
	.en     (imread_enb_r),
	.tuser  (tuser_r),
	.tvalid (tvalid_r),
	.tready (tready_r),
	.tdata  (tdata_r),
	.tlast  (tlast_r)
);


//==========================================================================
// Target Module
//==========================================================================
csi_if csi_if (
	// Global Control
	.rst_n (rst_n),
	.clk   (clk),
	
	// Internal Bus I/F
	.ibus_cs     (ibus_cs),
	.ibus_wr     (ibus_wr),
	.ibus_addr   (ibus_addr[7:0]),
	.ibus_wrdata (ibus_wrdata[31:0]),
	.ibus_rddata (ibus_rddata[31:0]),
	
	// CSI Receiver Control
	.vrst_n (),
	
	// AXI Stream Video Input (Ch1)
	.tvalid_ch1 (tvalid_l),
	.tready_ch1 (tready_l),
	.tuser_ch1  (tuser_l),
	.tlast_ch1  (tlast_l),
	.tdata_ch1  (tdata_l[15:0]),
	.tdest_ch1  (4'b0),
	
	// AXI Stream Video Input (Ch2)
	.tvalid_ch2 (tvalid_r),
	.tready_ch2 (tready_r),
	.tuser_ch2  (tuser_r),
	.tlast_ch2  (tlast_r),
	.tdata_ch2  (tdata_r[15:0]),
	.tdest_ch2  (4'b0),
	
	// Parallel Video Output
	.sof_out (sof_out),
	.vout    (vout),
	.d1_out  (d1_out[15:0]),
	.d2_out  (d2_out[15:0])
);


//==========================================================================
// Simulation Control
//==========================================================================
task INIT;
	begin
		imread_enb_l <=  1'b0;
		imread_enb_r <=  1'b0;
		ibus_cs      <=  1'b0;
		ibus_wr      <=  1'b0;
		ibus_addr    <=  8'b0;
		ibus_wrdata  <= 32'b0;
	end
endtask

task REG_WR;
	begin
		@(posedge clk);
		ibus_addr[7:0] <= ADDR[7:0];
		ibus_wrdata[31:0] <= DATA[31:0];
		ibus_wr <= 1'b1;
		ibus_cs <= 1'b1;
		
		@(posedge clk);
		ibus_wr <= 1'b0;
		ibus_cs <= 1'b1;
		
		repeat (5) @(posedge clk);
	end
endtask

task REG_RD;
	begin
		@(posedge clk);
		ibus_addr[7:0] <= ADDR[7:0];
		ibus_cs <= 1'b1;
		
		repeat (5) @(posedge clk);
		ibus_cs <= 1'b1;
		
		repeat (5) @(posedge clk);
	end
endtask

initial begin
	INIT;
	
	repeat (10) @(posedge clk);
	
	ADDR = 8'h00;	DATA = 32'h0000_0003;		REG_WR;
	imread_enb_r <= 1'b1;
	repeat (3000) @(posedge clk);
	imread_enb_l <= 1'b1;
	
	repeat (10000) @(posedge clk);
	
	$finish;
end


endmodule
