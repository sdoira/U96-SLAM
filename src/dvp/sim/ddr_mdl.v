`timescale 1ns / 1ps

module ddr_mdl (
	// AXI I/F
	axi_aclk,
	axi_awaddr,
	axi_awlen,
	axi_awvalid,
	axi_awready,
	axi_wdata,
	axi_wstrb,
	axi_wvalid,
	axi_wready,
	axi_wlast,
	axi_araddr,
	axi_arlen,
	axi_arvalid,
	axi_arready,
	axi_rdata,
	axi_rvalid,
	axi_rready,
	axi_rlast,
	axi_bvalid,
	axi_bready
);


//==========================================================================
// Port Declaration
//==========================================================================
// AXI I/F
input			axi_aclk;
input	[31:0]	axi_awaddr;
input	[7:0]	axi_awlen;
input			axi_awvalid;
output			axi_awready;
input	[31:0]	axi_wdata;
input	[3:0]	axi_wstrb;
input			axi_wvalid;
output			axi_wready;
input			axi_wlast;
input	[31:0]	axi_araddr;
input	[7:0]	axi_arlen;
input			axi_arvalid;
output			axi_arready;
output	[31:0]	axi_rdata;
output			axi_rvalid;
input			axi_rready;
output			axi_rlast;
output			axi_bvalid;
input			axi_bready;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Memory Instance
reg		[31:0]		mem[0:4194304-1]; // 2^22 * 32bits = 16MB

// Control
integer				wr_state;
reg					axi_wready;
wire				axi_bvalid;
integer				rd_state;
integer				bst_cnt;
reg		[23:0]		addr;
wire				axi_rvalid, axi_rlast;
wire	[31:0]		axi_rdata;


//==========================================================================
// Memory Instance
//--------------------------------------------------------------------------
// LR : 000ooooo_ooolllll_lllllaaa_aaaaaaaa
// SAD: 000ooooo_oollllll_llllaaaa_aaaaaaaa
// use: --------_********_********_********
//--------------------------------------------------------------------------
// lr_addr_a  = 32'h0000_0000
// lr_addr_b  = 32'h0020_0000
// sad_addr_a = 32'h0040_0000
// sad_addr_b = 32'h0080_0000
//==========================================================================
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


//==========================================================================
// Control
//==========================================================================
initial begin
	wr_state <= 0;
	forever @(posedge axi_aclk) begin
		case (wr_state)
			0: if (axi_awvalid) wr_state <= 1;
			1: if (axi_wlast)   wr_state <= 2;
			2:                  wr_state <= 3;
			3:                  wr_state <= 4;
			4: if (axi_bready)  wr_state <= 0;
		endcase
	end
end

assign axi_awready = 1'b1;
//assign axi_wready  = 1'b1;
initial begin
	axi_wready <= 1'b0;
	forever @(posedge axi_aclk) begin
		if (axi_wvalid) begin
			axi_wready <= 1'b1;
		end
		else begin
			axi_wready <= 1'b0;
		end
	end
end
assign axi_bvalid = (wr_state == 4);

initial begin
	rd_state <= 0;
	forever @(posedge axi_aclk) begin
		case (rd_state)
			0: if (axi_arvalid) rd_state <= 1;
			1:                  rd_state <= 2;
			2:                  rd_state <= 3;
			3:                  rd_state <= 4;
			4: if (axi_rlast)   rd_state <= 0;
		endcase
	end
end

assign axi_arready = 1'b1;

initial begin
	bst_cnt <= 0;
	forever @(posedge axi_aclk) begin
		if ((wr_state == 1) | (rd_state == 4)) begin
			bst_cnt <= bst_cnt +1;
		end
		else begin
			bst_cnt <= 0;
		end
	end
end

initial begin
	addr <= 24'b0;
	forever @(posedge axi_aclk) begin
		if (axi_awvalid) begin
			if (axi_wvalid) begin
				addr <= axi_awaddr[23:0] + 4;
			end
			else begin
				addr <= axi_awaddr[23:0];
			end
		end
		else if (axi_arvalid) begin
			if (axi_rvalid) begin
				addr <= axi_araddr[23:0] + 4;
			end
			else begin
				addr <= axi_araddr[23:0];
			end
		end
		else if ((axi_wvalid & axi_wready) | (axi_rvalid & axi_rready)) begin
			addr <= addr + 4;
		end
	end
end

assign axi_rvalid = (rd_state == 4);
assign axi_rlast  = (rd_state == 4) & (bst_cnt == axi_arlen);
assign axi_rdata  = (axi_rvalid) ? mem[addr[23:2]] : 32'b0;


endmodule
