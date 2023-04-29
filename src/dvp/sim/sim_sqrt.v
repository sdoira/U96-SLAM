`timescale 1ns / 1ps

module sim_sqrt;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Reset/Clock Generation
reg					clk, rst_n;

// sqrt
reg		[15:0]		din;
reg					vin;
wire	[15:0]		dout;
wire				vout;


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
// Target Module
//==========================================================================
sqrt sqrt (
	.aclk (clk),
	.s_axis_cartesian_tdata (din[15:0]),
	.s_axis_cartesian_tvalid (vin),
	.m_axis_dout_tdata (dout[15:0]),
	.m_axis_dout_tvalid (vout)
);


//==========================================================================
// Simulation Control
//==========================================================================
task INIT;
	begin
		din <= 16'b0;
		vin <=  1'b0;
	end
endtask

initial begin
	INIT;
	
	repeat (10) @(posedge clk);
	
	din <= 16'd144;
	vin <= 1'b1;
	@(posedge clk);
	din <= 16'd169;
	vin <= 1'b1;
	@(posedge clk);
	vin <= 1'b0;
	
	repeat (100) @(posedge clk);
	
	$finish;
end


endmodule
