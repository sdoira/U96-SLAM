//==========================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//==========================================================================
`timescale 1 ns / 1 ps

module axi_if #(
	// Width of S_AXI data bus
	parameter integer C_S_AXI_DATA_WIDTH = 32,
	// Width of S_AXI address bus
	parameter integer C_S_AXI_ADDR_WIDTH = 14,
	
	parameter integer VERSION = 32'h0000
)(
	// AXI Interface
	axi_aclk,
	axi_aresetn,
	axi_awaddr,
	axi_awprot,
	axi_awvalid,
	axi_awready,
	axi_wdata,
	axi_wstrb,
	axi_wvalid,
	axi_wready,
	axi_bresp,
	axi_bvalid,
	axi_bready,
	axi_araddr,
	axi_arprot,
	axi_arvalid,
	axi_arready,
	axi_rdata,
	axi_rresp,
	axi_rvalid,
	axi_rready,
	
	// Memory Mapped Register Interface
	led,
	slide_sw,
	push_sw,
	ptn_sel,
	gpio_in,
	gpio_out,
	
	// Internal Bus
	addr,
	cs,
	wr,
	wrdata,
	rddata,
	
	// Interrupt
	arb_done,
	vsync,
	intr
);


//==========================================================================
// Parameter Settings
//==========================================================================
// Register Address
localparam ADDR_VERSION    = 12'h000; // 0000h
localparam ADDR_TESTPAD    = 12'h001; // 0004h
localparam ADDR_LED        = 12'h002; // 0008h
localparam ADDR_SW         = 12'h003; // 000Ch
localparam ADDR_TIMER      = 12'h004; // 0010h
localparam ADDR_INTR_ENB   = 12'h005; // 0014h
localparam ADDR_INTR_STS   = 12'h006; // 0018h
localparam ADDR_PTN_SEL    = 12'h007; // 001Ch
localparam ADDR_GPIO_IN    = 12'h008; // 0020h
localparam ADDR_GPIO_OUT   = 12'h009; // 0024h
localparam ADDR_HOLD_POS   = 12'h00A; // 0028h
localparam ADDR_HOLD_NEG   = 12'h00B; // 002Ch
localparam ADDR_BLINK      = 12'h00C; // 0030h
localparam ADDR_SW_HOLD    = 12'h00D; // 0034h
                                      // 0038h - 003Ch
localparam ADDR_IPC_MSG1   = 12'h010; // 0040h
localparam ADDR_IPC_MSG2   = 12'h011; // 0044h
localparam ADDR_IPC_PARAM1 = 12'h012; // 0048h
localparam ADDR_IPC_PARAM2 = 12'h013; // 004Ch
localparam ADDR_IPC_PARAM3 = 12'h014; // 0050h
localparam ADDR_IPC_PARAM4 = 12'h015; // 0054h
                                      // 0058h - 0FFCh


//==========================================================================
// Port Declaration
//==========================================================================
// AXI Interface
input			axi_aclk;
input			axi_aresetn;
input	[13:0]	axi_awaddr;
input	[2:0]	axi_awprot;
input			axi_awvalid;
output			axi_awready;
input	[31:0]	axi_wdata;
input	[3:0]	axi_wstrb;
input			axi_wvalid;
output			axi_wready;
output	[1:0]	axi_bresp;
output			axi_bvalid;
input			axi_bready;
input	[13:0]	axi_araddr;
input	[2:0]	axi_arprot;
input			axi_arvalid;
output			axi_arready;
output	[31:0]	axi_rdata;
output	[1:0]	axi_rresp;
output			axi_rvalid;
input			axi_rready;

// Memory Mapped Register Interface
output	[1:0]	led;
input	[7:0]	slide_sw;
input	[4:0]	push_sw;
output	[1:0]	ptn_sel;
input	[1:0]	gpio_in;
output	[1:0]	gpio_out;

// Internal Bus
output	[11:0]	addr;
output	[3:0]	cs;
output			wr;
output	[31:0]	wrdata;
input	[31:0]	rddata;

// Interrupt
input	[7:0]	arb_done;
input			vsync;
output			intr;


//==========================================================================
// Reg/Wire Declaration
//==========================================================================
// Write Process
reg				axi_awready;
reg				aw_en;
reg		[13:0] 	axi_awaddr_r;
reg				axi_wready;
wire			slv_reg_wren;
reg		[31:0]	testpad;
reg		[1:0]	led;
reg		[31:0]	intr_enb;
reg		[1:0]	ptn_sel;
reg		[1:0]	gpio_out_r;
reg		[7:0]	ipc_msg1, ipc_msg2;
reg		[31:0]	ipc_param1, ipc_param2, ipc_param3, ipc_param4;
reg		[2:0]	blink2, blink1;
reg				axi_bvalid;
reg		[1:0] 	axi_bresp;

// Timer
reg		[6:0]	timer;
wire			us;
reg		[31:0]	us_timer;

// GPIO Input Hold
reg		[1:0]	gpio_in_r, gpio_pos, gpio_neg;
reg		[13:0]	chatter_count;
wire			chatter_pulse;
reg		[4:0]	sw_r;
reg				sw_smooth, sw_smooth_r;
reg				sw_pos, sw_neg;

// LED Blink
reg		[23:0]	led_timer;
wire			epoch_125ms;
reg		[2:0]	blink1_timer;
wire			blink1_epoch;
reg				blink1_led;
reg		[2:0]	blink2_timer;
wire			blink2_epoch;
reg				blink2_led;
wire	[1:0]	gpio_out;

// Interrupt
reg		[1:0]	vsync_r;
wire			vsync_pos;
reg		[31:0]	intr_sts;
reg		[31:0]	intr_sts_r;
wire			intr;

// Read Process
reg				axi_arready;
reg		[13:0] 	axi_araddr_r;
wire			rd_trig;
reg		[3:0]	rd_trig_r;
reg				axi_rvalid;
reg		[1:0] 	axi_rresp;
wire			slv_reg_rden;
wire	[31:0]	reg_data_out;
reg		[31:0]	axi_rdata;

// Internal Bus Converter
reg		[13:0]	S_AXI_ARADDR_1;
wire	[13:0]	addr_full;
wire	[11:0]	addr;
wire	[31:0]	wrdata;
wire			wr;


//==========================================================================
// Write Process
//==========================================================================
// Write Address
always @(posedge axi_aclk) begin
	if	(~axi_aresetn) begin
		axi_awready <= 1'b0;
		aw_en       <= 1'b1;
	end 
	else begin
		if (~axi_awready && axi_awvalid && axi_wvalid && aw_en) begin
			axi_awready <= 1'b1;
			aw_en       <= 1'b0;
		end
		else if (axi_bready && axi_bvalid) begin
			aw_en       <= 1'b1;
			axi_awready <= 1'b0;
		end
		else begin
			axi_awready <= 1'b0;
		end
	end
end

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		axi_awaddr_r <= 0;
	end 
	else begin
    	if (~axi_awready && axi_awvalid && axi_wvalid && aw_en) begin
			axi_awaddr_r <= axi_awaddr;
		end
	end
end

// Write Data
always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		axi_wready <= 1'b0;
	end 
	else begin
		if (~axi_wready && axi_wvalid && axi_awvalid && aw_en) begin
			axi_wready <= 1'b1;
		end
		else begin
			axi_wready <= 1'b0;
		end
	end
end

assign slv_reg_wren = (axi_wready & axi_wvalid & axi_awready & axi_awvalid);

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		testpad    <= 32'b0;
		led        <= 2'b01;
		intr_enb   <= 32'b0;
		ptn_sel    <=  2'b0;
		gpio_out_r <=  2'b0;
		ipc_msg1   <=  8'b0;
		ipc_msg2   <=  8'b0;
		ipc_param1 <= 32'b0;
		ipc_param2 <= 32'b0;
		ipc_param3 <= 32'b0;
		ipc_param4 <= 32'b0;
		blink2     <=  3'b0;
		blink1     <=  3'b0;
	end 
	else begin
		if (slv_reg_wren) begin
			case (axi_awaddr_r[13:2])
				ADDR_TESTPAD   : testpad    <= axi_wdata;
				ADDR_LED       : led        <= axi_wdata[1:0];
				ADDR_INTR_ENB  : intr_enb   <= axi_wdata;
				ADDR_PTN_SEL   : ptn_sel    <= axi_wdata[1:0];
				ADDR_GPIO_OUT  : gpio_out_r <= axi_wdata[1:0];
				ADDR_IPC_MSG1  : ipc_msg1   <= axi_wdata[7:0];
				ADDR_IPC_MSG2  : ipc_msg2   <= axi_wdata[7:0];
				ADDR_IPC_PARAM1: ipc_param1 <= axi_wdata[31:0];
				ADDR_IPC_PARAM2: ipc_param2 <= axi_wdata[31:0];
				ADDR_IPC_PARAM3: ipc_param3 <= axi_wdata[31:0];
				ADDR_IPC_PARAM4: ipc_param4 <= axi_wdata[31:0];
				ADDR_BLINK     : begin
					blink2 <= axi_wdata[10:8];
					blink1 <= axi_wdata[ 2:0];
				end
				default: begin
					testpad    <= testpad;
					led        <= led;
					intr_enb   <= intr_enb;
					ptn_sel    <= ptn_sel;
					gpio_out_r <= gpio_out_r;
					ipc_msg1   <= ipc_msg1;
					ipc_msg2   <= ipc_msg2;
					ipc_param1 <= ipc_param1;
					ipc_param2 <= ipc_param2;
					ipc_param3 <= ipc_param3;
					ipc_param4 <= ipc_param4;
					blink2     <= blink2;
					blink1     <= blink1;
				end
			endcase
		end
	end
end

// Write Response
always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		axi_bvalid  <= 0;
		axi_bresp   <= 2'b0;
	end 
	else begin
		if (axi_awready && axi_awvalid && ~axi_bvalid && axi_wready && axi_wvalid) begin
			// indicates a valid write response is available
			axi_bvalid <= 1'b1;
			axi_bresp  <= 2'b0; // 'OKAY' response 
		end
		else begin
			if (axi_bready && axi_bvalid) begin
				axi_bvalid <= 1'b0; 
			end
		end
	end
end


//==========================================================================
// Timer
//==========================================================================
always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		timer <= 7'b0;
	end 
	else begin
		if (us) begin
			timer <= 0;
		end
		else begin
			timer <= timer + 1'b1;
		end
	end
end

// 1 microsecond epoch pulse
// clock cycle is assumed to be 100MHz
assign us = (timer[6:0] == 7'd99);

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		us_timer <= 32'b0;
	end
	else begin
		if (us) begin
			us_timer <= us_timer + 1'b1;
		end
	end
end


//==========================================================================
// GPIO Input Hold
//==========================================================================
always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		gpio_in_r <= 2'b0;
	end
	else begin
		gpio_in_r <= gpio_in;
	end
end

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		gpio_pos <= 2'b0;
	end
	else if (slv_reg_wren & (axi_awaddr_r[13:2] == ADDR_HOLD_POS)) begin
		// write '1' to clear
		gpio_pos <= (gpio_pos[1:0] & ~axi_wdata[1:0]);
	end
	else begin
		// hold positive edge event
		gpio_pos <= gpio_pos[1:0] | (~gpio_in_r[1:0] & gpio_in[1:0]);
	end
end

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		gpio_neg <= 2'b0;
	end
	else if (slv_reg_wren & (axi_awaddr_r[13:2] == ADDR_HOLD_NEG)) begin
		gpio_neg <= (gpio_neg[1:0] & ~axi_wdata[1:0]);
	end
	else begin
		gpio_neg <= gpio_neg[1:0] | (gpio_in_r[1:0] & ~gpio_in[1:0]);
	end
end


//==========================================================================
// Switch Input
//==========================================================================
always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		chatter_count <= 14'b0;
	end
	else begin
		if (chatter_pulse) begin
			chatter_count <= 0;
		end
		else begin
			chatter_count <= chatter_count + 1'b1;
		end
	end
end

assign chatter_pulse = (chatter_count == 14'd9999); // 100us
//assign chatter_pulse = (chatter_count == 14'd99);

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		sw_r <= 5'b0;
	end
	else begin
		if (chatter_pulse) begin
			sw_r <= {sw_r[3:0], gpio_in[1]};
		end
	end
end

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		sw_smooth <= 1'b0;
	end
	else begin
		if (sw_r[4:0] == 5'b11111) begin
			sw_smooth <= 1'b1;
		end
		else if (sw_r[4:0] == 5'b00000) begin
			sw_smooth <= 1'b0;
		end
		else begin
			sw_smooth <= sw_smooth;
		end
	end
end

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		sw_smooth_r <= 1'b0;
	end
	else begin
		sw_smooth_r <= sw_smooth;
	end
end

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		sw_pos <= 1'b0;
	end
	else begin
		if (~sw_smooth_r & sw_smooth) begin
			sw_pos <= 1'b1;
		end
		else if (slv_reg_wren & (axi_awaddr_r[13:2] == ADDR_SW_HOLD) & axi_wdata[0]) begin
			sw_pos <= 1'b0;
		end
	end
end

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		sw_neg <= 1'b0;
	end
	else begin
		if (sw_smooth_r & ~sw_smooth) begin
			sw_neg <= 1'b1;
		end
		else if (slv_reg_wren & (axi_awaddr_r[13:2] == ADDR_SW_HOLD) & axi_wdata[1]) begin
			sw_neg <= 1'b0;
		end
	end
end


//==========================================================================
// LED Blink
//==========================================================================
always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		led_timer <= 24'b0;
	end
	else begin
		if (epoch_125ms) begin
			led_timer <= 0;
		end
		else begin
			led_timer <= led_timer + 1'b1;
		end
	end
end

assign epoch_125ms = (led_timer[23:0] == 24'd12500000);
//assign epoch_125ms = (led_timer[23:0] == 24'd125);

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		blink1_timer <= 3'd1;
	end
	else begin
		if (epoch_125ms) begin
			if (blink1_epoch) begin
				blink1_timer <= 1;
			end
			else begin
			blink1_timer <= blink1_timer + 1'b1;
			end
		end
	end
end

assign blink1_epoch = epoch_125ms & (blink1_timer[2:0] == blink1[2:0]);

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		blink1_led <= 1'b0;
	end
	else begin
		if (blink1[2:0] == 0) begin
			blink1_led <= 1'b0;
		end
		else if (blink1_epoch) begin
			blink1_led <= ~blink1_led;
		end
	end
end

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		blink2_timer <= 3'd1;
	end
	else begin
		if (epoch_125ms) begin
			if (blink2_epoch) begin
				blink2_timer <= 1;
			end
			else begin
			blink2_timer <= blink2_timer + 1'b1;
			end
		end
	end
end

assign blink2_epoch = epoch_125ms & (blink2_timer[2:0] == blink2[2:0]);

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		blink2_led <= 1'b0;
	end
	else begin
		if (blink2[2:0] == 0) begin
			blink2_led <= 1'b0;
		end
		else if (blink2_epoch) begin
			blink2_led <= ~blink2_led;
		end
	end
end

assign gpio_out[1] = gpio_out_r[1] | blink2_led;
assign gpio_out[0] = gpio_out_r[0] | blink1_led;


//==========================================================================
// Interrupt
//==========================================================================
always @(negedge axi_aresetn or posedge axi_aclk) begin
	if (~axi_aresetn) begin
		vsync_r <= 2'b0;
	end
	else begin
		vsync_r <= {vsync_r[1:0], vsync};
	end
end

assign vsync_pos = vsync_r[0] & ~vsync_r[1];

always @(negedge axi_aresetn or posedge axi_aclk) begin
	if (~axi_aresetn) begin
		intr_sts <= 32'b0;
	end 
	else begin
		if (slv_reg_wren & axi_awaddr_r[13:2] == ADDR_INTR_STS) begin
			intr_sts <= intr_sts & ~axi_wdata; // write '1' to clear
		end
		else begin
			intr_sts[31:6] <= 27'b0;
			intr_sts[5] <= (arb_done[7]) ? 1'b1 : intr_sts[5]; // gftt
			intr_sts[4] <= (vsync_pos)   ? 1'b1 : intr_sts[4]; // vsync
			intr_sts[3] <= (arb_done[5]) ? 1'b1 : intr_sts[3]; // xsbl2
			intr_sts[2] <= (arb_done[4]) ? 1'b1 : intr_sts[2]; // rect
			intr_sts[1] <= (arb_done[3]) ? 1'b1 : intr_sts[1]; // bm
			intr_sts[0] <= (arb_done[0]) ? 1'b1 : intr_sts[0]; // grb
		end
	end
end

always @(negedge axi_aresetn or posedge axi_aclk) begin
	if (~axi_aresetn) begin
		intr_sts_r <= 32'b0;
	end 
	else begin
		intr_sts_r <= intr_sts;
	end
end

//assign intr = |(intr_enb[31:0] & intr_sts[31:0] & ~intr_sts_r[31:0]);
assign intr = |(intr_enb[31:0] & intr_sts[31:0]); // level


//==========================================================================
// Read Process
//==========================================================================
// Read Address
always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		axi_arready  <= 1'b0;
		axi_araddr_r <= 0;
	end 
	else begin
		if (~axi_arready && axi_arvalid) begin
			axi_arready  <= 1'b1;
			axi_araddr_r <= axi_araddr;
		end
		else begin
			axi_arready <= 1'b0;
		end
	end
end

assign rd_trig = (axi_arready && axi_arvalid && ~axi_rvalid);

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		rd_trig_r <= 0;
	end
	else begin
		rd_trig_r <= {rd_trig_r[2:0], rd_trig};
	end
end

always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		axi_rvalid <= 0;
		axi_rresp  <= 0;
	end 
	else begin
		if (rd_trig_r[3]) begin
			// Valid read data is available at the read data bus
			axi_rvalid <= 1'b1;
			axi_rresp  <= 2'b0; // 'OKAY' response
		end   
		else if (axi_rvalid && axi_rready) begin
			// Read data is accepted by the master
			axi_rvalid <= 1'b0;
		end
	end
end

assign slv_reg_rden = rd_trig_r[2];

// CS0 [0000h-0FFCh] 00_0000_0000_0000 - 00_1111_1111_1100
// CS1 [1000h-1FFCh] 01_0000_0000_0000 - 01_1111_1111_1100
// CS2 [2000h-2FFCh] 10_0000_0000_0000 - 10_1111_1111_1100
// CS3 [3000h-3FFCh] 11_0000_0000_0000 - 11_1111_1111_1100
assign reg_data_out[31:0] = (
	(axi_araddr_r[13:2] == ADDR_VERSION)    ? VERSION :
	(axi_araddr_r[13:2] == ADDR_TESTPAD)    ? testpad :
	(axi_araddr_r[13:2] == ADDR_LED)        ? {30'b0, led[1:0]} :
	(axi_araddr_r[13:2] == ADDR_SW)         ? {19'b0, push_sw[4:0], slide_sw[7:0]} :
	(axi_araddr_r[13:2] == ADDR_TIMER)      ? us_timer[31:0] :
	(axi_araddr_r[13:2] == ADDR_INTR_ENB)   ? intr_enb[31:0] :
	(axi_araddr_r[13:2] == ADDR_INTR_STS)   ? intr_sts[31:0] :
	(axi_araddr_r[13:2] == ADDR_PTN_SEL)    ? ptn_sel[1:0] :
	(axi_araddr_r[13:2] == ADDR_GPIO_IN)    ? {30'b0, gpio_in[1:0]} :
	(axi_araddr_r[13:2] == ADDR_GPIO_OUT)   ? {30'b0, gpio_out_r[1:0]} :
	(axi_araddr_r[13:2] == ADDR_HOLD_POS)   ? {30'b0, gpio_pos[1:0]} :
	(axi_araddr_r[13:2] == ADDR_HOLD_NEG)   ? {30'b0, gpio_neg[1:0]} :
	(axi_araddr_r[13:2] == ADDR_IPC_MSG1)   ? {24'b0, ipc_msg1[7:0]} :
	(axi_araddr_r[13:2] == ADDR_IPC_MSG2)   ? {24'b0, ipc_msg2[7:0]} :
	(axi_araddr_r[13:2] == ADDR_IPC_PARAM1) ? ipc_param1[31:0] :
	(axi_araddr_r[13:2] == ADDR_IPC_PARAM2) ? ipc_param2[31:0] :
	(axi_araddr_r[13:2] == ADDR_IPC_PARAM3) ? ipc_param3[31:0] :
	(axi_araddr_r[13:2] == ADDR_IPC_PARAM4) ? ipc_param4[31:0] :
	(axi_araddr_r[13:2] == ADDR_BLINK)      ? {21'b0, blink2[2:0], 5'b0, blink1[2:0]} :
	(axi_araddr_r[13:2] == ADDR_SW_HOLD)    ? {30'b0, sw_neg, sw_pos} :
	                                          rddata[31:0]
);

// Output register or memory read data
always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		axi_rdata <= 0;
	end 
	else begin
		if (slv_reg_rden) begin
			axi_rdata <= reg_data_out;
		end
	end
end


//==========================================================================
// Internal Bus Converter
//==========================================================================
always @(posedge axi_aclk) begin
	if (~axi_aresetn) begin
		S_AXI_ARADDR_1 <= 0;
	end
	else begin
		if (axi_arvalid & axi_arready) begin
			S_AXI_ARADDR_1 <= axi_araddr;
		end
	end
end
   
assign addr_full[13:0] = (
	(axi_wvalid)  ? axi_awaddr[13:0] : 
	(axi_arvalid) ? axi_araddr[13:0] :
	                S_AXI_ARADDR_1[13:0]
);
assign addr = addr_full[11:0];
assign wrdata[31:0] = axi_wdata[31:0];
assign cs[0] = (addr_full[13:12] == 2'b00);
assign cs[1] = (addr_full[13:12] == 2'b01);
assign cs[2] = (addr_full[13:12] == 2'b10);
assign cs[3] = (addr_full[13:12] == 2'b11);
assign wr = axi_wready;


endmodule
