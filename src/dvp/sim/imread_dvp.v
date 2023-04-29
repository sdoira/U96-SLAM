`timescale 1ns / 1ps

module imread_dvp #(
	parameter integer WDT = 64,
	parameter integer HGT = 48,
	parameter integer D = 8,
	parameter FILE = "filename.txt",
	parameter BLK_WDT_B = 8,
	parameter BLK_WDT_E = 4,
	parameter BLK_HGT_B = 2,
	parameter BLK_HGT_E = 1
)(
	en,
	pclk,
	vsync,
	href,
	dout
);


//==========================================================================
// Parameter Declaration
//==========================================================================
parameter TOT_WDT = WDT + BLK_WDT_B + BLK_WDT_E;
parameter TOT_HGT = HGT + BLK_HGT_B + BLK_HGT_E;
parameter TCK  = 25; // clock period [ns]


//==========================================================================
// Port Declaration
//==========================================================================
input				en;
output				pclk, vsync, href;
output	[D-1:0]		dout;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
reg		[D-1:0]		mem[0:WDT*HGT-1];
reg					pclk;
reg					phase;
integer				hcnt, vcnt;
wire				valid;
wire				vsync, href;
integer				addr;
wire	[D-1:0]		dout;


//==========================================================================
// Memory Implementation
//==========================================================================
initial begin
	$readmemh(FILE, mem);
end

initial begin
	pclk <= 1'b0;
	forever begin
		#TCK;
		pclk <= ~pclk;
	end
end

initial begin
	phase <= 1'b0;
	forever @(posedge pclk) begin
		if (en) begin
			phase <= ~phase;
		end
	end
end

initial begin
	hcnt <= WDT + BLK_WDT_E;
	vcnt <= HGT + BLK_HGT_E;
	forever @(posedge pclk) begin
		if (en) begin
			if (phase == 1) begin
				if (hcnt == TOT_WDT - 1'b1) begin
					if (vcnt == TOT_HGT - 1'b1) begin
						vcnt <= 0;
					end
					else begin
						vcnt <= vcnt + 1'b1;
					end
					hcnt <= 0;
				end
				else begin
					hcnt <= hcnt + 1'b1;
				end
			end
		end
	end
end

assign valid = (BLK_HGT_B <= vcnt) & (vcnt < BLK_HGT_B + HGT) & (BLK_WDT_B <= hcnt) & (hcnt < BLK_WDT_B + WDT);

assign vsync = (vcnt == 0);
assign href = valid;

initial begin
	addr <= 0;
	forever @(posedge pclk) begin
		if (valid) begin
			if (phase == 1) begin
				if (addr == WDT*HGT-1) begin
					addr <= 0;
				end
				else begin
					addr <= addr + 1'b1;
				end
			end
		end
	end
end

//assign dout = (~valid) ? 8'h00 : (phase == 0) ? mem[addr] : 8'h80;
assign dout = (~valid) ? 8'h00 : (phase == 0) ? 8'h80 : mem[addr]; // UYVY order


endmodule
