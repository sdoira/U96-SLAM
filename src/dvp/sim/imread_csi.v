`timescale 1ns / 1ps

module imread_csi #(
	parameter FILE = "filename.txt",
	parameter integer WDT = 640,
	parameter integer HGT = 480,
	parameter TOT_WDT = 1640,
	parameter TOT_HGT = 677
)(
	clk,
	en,
	tuser,
	tvalid,
	tready,
	tdata,
	tlast
);


//==========================================================================
// Port Declaration
//==========================================================================
input				clk;
input				en;
output				tuser, tvalid;
input				tready;
output	[15:0]		tdata;
output				tlast;

//==========================================================================
// Reg/Wire Declaration
//==========================================================================
reg		[7:0]		mem[0:WDT*HGT-1]; // Y only
integer				phase;
wire				phase_end;
integer				col, row;
wire				data_phase, advance_counter;
wire				tvalid;
integer				addr;
wire				tuser;
wire	[15:0]		tdata; // UYVY
wire				tlast;


//==========================================================================
// Memory Implementation
//==========================================================================
initial begin
	$readmemh(FILE, mem);
end

initial begin
	phase <= 0;
	forever @(posedge clk) begin
		if (en) begin
			if (advance_counter) begin
				if (phase_end) begin
					phase <= 0;
				end
				else begin
					phase <= phase + 1;
				end
			end
		end
		else begin
			phase <= 0;
		end
	end
end

// read 4 samples followed by 8 cycles wait
assign phase_end = (phase == 11);

initial begin
	col <= TOT_WDT / 2;
	row <= TOT_HGT - 1;
	forever @(posedge clk) begin
		if (en) begin
			if (advance_counter) begin
				if (phase_end) begin
					if (col == TOT_WDT - 4) begin
						if (row == TOT_HGT - 1) begin
							row <= 0;
						end
						else begin
							row <= row + 1;
						end
						col <= 0;
					end
					else begin
						col <= col + 4; // read 4 samples per cycle
					end
				end
			end
		end
		else begin
			col <= 0;
			row <= 0;
		end
	end
end

assign data_phase = (
	(0 <= phase) & (phase <= 3) &
	(0 <= col)   & (col < WDT)  &
	(0 <= row)   & (row < HGT)
);

assign advance_counter = ~data_phase | tready;

assign tvalid = (
	(en) &
	(0 <= phase) & (phase <= 3) &
	(0 <= col)   & (col < WDT)  &
	(0 <= row)   & (row < HGT)
);

initial begin
	addr <= 0;
	forever @(posedge clk) begin
		if (tvalid & tready) begin
			if (addr == WDT*HGT-1) begin
				addr <= 0;
			end
			else begin
				addr <= addr + 1;
			end
		end
	end
end

assign tuser = tvalid & (addr == 0);
assign tdata = (~tvalid) ? 16'h0080 : {mem[addr], 8'h80};
assign tlast = (col == WDT - 4) & (phase == 3) & tvalid;


endmodule
