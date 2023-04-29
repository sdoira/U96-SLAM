//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file fpga.c
//!
//! C source for FPGA related functions.
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#include "fpga.h"

//! FPGA register space
volatile struct FPGA_REG *fpga = (struct FPGA_REG *)XPAR_DVP_0_BASEADDR;

//=============================================================================
//! FPGA Read Version
//-----------------------------------------------------------------------------
//! @return FPGA version number
//=============================================================================
int Fpga_ReadVersion (void) {
#ifdef _WIN32
	return 0;
#else
	return (fpga->com.Version);
#endif
}


//==========================================================================
//! Initialize FPGA
//--------------------------------------------------------------------------
//! @param remoteSetting	Parameter settings
//--------------------------------------------------------------------------
//! @brief This function initializes FPGA and start DMA transfer of image
//! data through MIPI CSI interface.
//==========================================================================
void Fpga_Init (struct REMOTE_SETTING *remoteSetting)
{
	//=================================================================
	// determine which module should be activated
	//=================================================================
	int activateGrb = (
		((remoteSetting->returnData & RETURN_DATA_RAW_FRAME) == RETURN_DATA_RAW_FRAME) ||
		(remoteSetting->usbOutput == USB_OUTPUT_RAW_FRAME)
	);
	int activateRect = (
		((remoteSetting->returnData & RETURN_DATA_STEREO_RECT) == RETURN_DATA_STEREO_RECT) ||
		(remoteSetting->usbOutput == USB_OUTPUT_STEREO_RECT)
	);
	int activateXsbl = (
		((remoteSetting->returnData & RETURN_DATA_STEREO_BM) == RETURN_DATA_STEREO_BM) ||
		(remoteSetting->usbOutput == USB_OUTPUT_STEREO_XSBL) ||
		(remoteSetting->usbOutput == USB_OUTPUT_STEREO_BM)
	);
	int activateBm = (
		((remoteSetting->returnData & RETURN_DATA_STEREO_BM) == RETURN_DATA_STEREO_BM) ||
		(remoteSetting->usbOutput == USB_OUTPUT_STEREO_BM)
	);
	int activateGftt = ((remoteSetting->returnData & RETURN_DATA_GFTT) == RETURN_DATA_GFTT);

	//=================================================================
	// select input data type
	//=================================================================
	fpga->csi.PatternSelect1 = remoteSetting->inputData;
	fpga->csi.PatternSelect2 = remoteSetting->inputData;

	//=================================================================
	// interrupt
	//-----------------------------------------------------------------
	// enable only the most time consuming process
	//=================================================================
	if (activateBm) {
		fpga->com.InterruptEnable |= FPGA_INTR_BM;
	} else if (activateXsbl) {
		fpga->com.InterruptEnable |= FPGA_INTR_XSBL;
	} else if (activateGftt) {
		fpga->com.InterruptEnable |= FPGA_INTR_GFTT;
	} else if (activateRect) {
		fpga->com.InterruptEnable |= FPGA_INTR_RECT;
	} else if (activateGrb) {
		fpga->com.InterruptEnable |= FPGA_INTR_GRB;
	} else {
		fpga->com.InterruptEnable = 0;
	}

	//=================================================================
	// frame rate
	//-----------------------------------------------------------------
	// sensor input is assumed to be 30fps.
	//=================================================================
	fpga->csi.FrameDecimation = 2; // 30 / (2 + 1) = 10fps

	//=================================================================
	// clear work memory
	//=================================================================
	memset((void*)BUF_DISP_A, 0xFF, IMAGE_HEIGHT * IMAGE_WIDTH * 2);
	memset((void*)BUF_DISP_B, 0xFF, IMAGE_HEIGHT * IMAGE_WIDTH * 2);
	memset((void*)BUF_GFTT_A, 0x00, GFTT_MAX_SIZE);
	memset((void*)BUF_GFTT_B, 0x00, GFTT_MAX_SIZE);

	// for debug
	memset((void*)BUF_RECT_A, 0x00, RECT_MAX_SIZE);
	memset((void*)BUF_RECT_B, 0x00, RECT_MAX_SIZE);
	memset((void*)BUF_XSBL_A, 0x00, XSBL_MAX_SIZE);
	memset((void*)BUF_XSBL_B, 0x00, XSBL_MAX_SIZE);
	memset((void*)BUF_BM_A, 0x00, BM_MAX_SIZE);
	memset((void*)BUF_BM_B, 0x00, BM_MAX_SIZE);

	//=================================================================
	// initialize memory mapped registers
	//=================================================================
	// Frame Grabber
	if (activateGrb)
	{
		fpga->grb.Address_A = (unsigned int)BUF_GRB_A;
		fpga->grb.Address_B = (unsigned int)BUF_GRB_B;
		fpga->grb.BurstLength = 63;
		fpga->grb.Size = (IMAGE_HEIGHT << 16) + IMAGE_WIDTH;
		fpga->grb.Control = FPGA_GRB_CTRL_ENABLE;
	}

	// Stereo Rectifier
	if (activateRect){
		rect_init ();
		fpga->rect.Address_A = (unsigned int)BUF_RECT_A;
		fpga->rect.Address_B = (unsigned int)BUF_RECT_B;
		fpga->rect.Control = FPGA_RECT_CTRL_ENABLE;
	}

	// Stereo X-Sobel
	if (activateXsbl){
		fpga->xsbl.Address_In_A = (unsigned int)BUF_RECT_A;
		fpga->xsbl.Address_In_B = (unsigned int)BUF_RECT_B;
		fpga->xsbl.Address_Out_A = (unsigned int)BUF_XSBL_A;
		fpga->xsbl.Address_Out_B = (unsigned int)BUF_XSBL_B;
		fpga->xsbl.Size = (IMAGE_HEIGHT << 16) + IMAGE_WIDTH;
		fpga->xsbl.Control = FPGA_XSBL_CTRL_ENABLE;
	}

	// Stereo Block Matching
	if (activateBm){
		fpga->bm.LR_Address_A = (unsigned int)BUF_XSBL_A;
		fpga->bm.LR_Address_B = (unsigned int)BUF_XSBL_B;
		fpga->bm.SAD_Address_A = (unsigned int)BUF_BM_A;
		fpga->bm.SAD_Address_B = (unsigned int)BUF_BM_B;
		fpga->bm.ImageSize = (IMAGE_HEIGHT << 16) + IMAGE_WIDTH;
		fpga->bm.DISP_Address_A = (unsigned int)BUF_DISP_A;
		fpga->bm.DISP_Address_B = (unsigned int)BUF_DISP_B;
		fpga->bm.BmSetting = 0x00150040;
		fpga->bm.Control   = FPGA_BM_CTRL_ENABLE;
	}

	// GFTT
	if (
		((remoteSetting->returnData & RETURN_DATA_GFTT) == RETURN_DATA_GFTT)
	){
		fpga->gftt.Address_In_A = (unsigned int)BUF_RECT_A;
		fpga->gftt.Address_In_B = (unsigned int)BUF_RECT_B;
		fpga->gftt.Address_Out_A = (unsigned int)BUF_GFTT_A;
		fpga->gftt.Address_Out_B = (unsigned int)BUF_GFTT_B;
		fpga->gftt.Size = (IMAGE_HEIGHT << 16) + IMAGE_WIDTH;
		fpga->gftt.Control = FPGA_GFTT_CTRL_ENABLE;
	}

	//=================================================================
	// start the image processing pipeline
	//=================================================================
	fpga->arb.Control = FPGA_ARB_CTRL_ENABLE;
	fpga->csi.Control = FPGA_CSI_CTRL_ENABLE;
}

void rect_init ()
{
	struct RECT_PARAM param;

	//=================================================================
	// Camera Parameters
	//-----------------------------------------------------------------
	// Generated by Visual Studio "rect" program.
	//=================================================================
	// Data set 220426// Left Camera
	param.ch[0].f[0] = 40419817;
	param.ch[0].f[1] = 40382910;
	param.ch[0].c[0] = 320;
	param.ch[0].c[1] = 240;
	param.ch[0].f2inv[0] = 6338213;
	param.ch[0].f2inv[1] = 6338213;
	param.ch[0].c2_f2[0] = 4984405;
	param.ch[0].c2_f2[1] = 5932596;
	param.ch[0].rot[0][0] = 16598538;
	param.ch[0].rot[0][1] = -120818;
	param.ch[0].rot[0][2] = 2439034;
	param.ch[0].rot[1][0] = 137992;
	param.ch[0].rot[1][1] = 16776300;
	param.ch[0].rot[1][2] = -108069;
	param.ch[0].rot[2][0] = -2438123;
	param.ch[0].rot[2][1] = 126979;
	param.ch[0].rot[2][2] = 16598626;

	// Right Camera
	param.ch[1].f[0] = 39609530;
	param.ch[1].f[1] = 39627967;
	param.ch[1].c[0] = 320;
	param.ch[1].c[1] = 240;
	param.ch[1].f2inv[0] = 6338213;
	param.ch[1].f2inv[1] = 6338213;
	param.ch[1].c2_f2[0] = 4984405;
	param.ch[1].c2_f2[1] = 5932596;
	param.ch[1].rot[0][0] = 16569087;
	param.ch[1].rot[0][1] = -69780;
	param.ch[1].rot[0][2] = 2633522;
	param.ch[1].rot[1][0] = 51223;
	param.ch[1].rot[1][1] = 16776692;
	param.ch[1].rot[1][2] = 122251;
	param.ch[1].rot[2][0] = -2633948;
	param.ch[1].rot[2][1] = -112694;
	param.ch[1].rot[2][2] = 16568783;

	set_rect_param (&param);

	//=================================================================
	// Initialize
	//=================================================================
	struct MAT2S map1;
	map1.rows = 480;
	map1.cols = 640;
	map1.data[0] = (short*)malloc(sizeof(short) * map1.rows * map1.cols);
	Xil_AssertVoid(map1.data[0] != NULL);
	map1.data[1] = (short*)malloc(sizeof(short) * map1.rows * map1.cols);
	Xil_AssertVoid(map1.data[1] != NULL);

	struct MAT2S map2;
	map2.rows = 480;
	map2.cols = 640;
	map2.data[0] = (short*)malloc(sizeof(short) * map2.rows * map2.cols);
	Xil_AssertVoid(map2.data[0] != NULL);
	map2.data[1] = (short*)malloc(sizeof(short) * map2.rows * map2.cols);
	Xil_AssertVoid(map2.data[1] != NULL);

	//=================================================================
	// Generate reverse map
	//=================================================================
	rect_remap(&param, &map1, &map2);

	//=================================================================
	// Generate commands
	//=================================================================
	rect_cmd_gen(map1, map2);

	free(map1.data[0]);
	free(map1.data[1]);
	free(map2.data[0]);
	free(map2.data[1]);

	return;
}

void set_rect_param (struct RECT_PARAM *param) {

	// Left Camera
	fpga->rect.l_fx  = param->ch[0].f[0];
	fpga->rect.l_fy  = param->ch[0].f[1];
	fpga->rect.l_r11 = param->ch[0].rot[0][0];
	fpga->rect.l_r12 = param->ch[0].rot[0][1];
	fpga->rect.l_r13 = param->ch[0].rot[0][2];
	fpga->rect.l_r21 = param->ch[0].rot[1][0];
	fpga->rect.l_r22 = param->ch[0].rot[1][1];
	fpga->rect.l_r23 = param->ch[0].rot[1][2];
	fpga->rect.l_r31 = param->ch[0].rot[2][0];
	fpga->rect.l_r32 = param->ch[0].rot[2][1];
	fpga->rect.l_r33 = param->ch[0].rot[2][2];

	// Right Camera
	fpga->rect.r_fx  = param->ch[1].f[0];
	fpga->rect.r_fy  = param->ch[1].f[1];
	fpga->rect.r_r11 = param->ch[1].rot[0][0];
	fpga->rect.r_r12 = param->ch[1].rot[0][1];
	fpga->rect.r_r13 = param->ch[1].rot[0][2];
	fpga->rect.r_r21 = param->ch[1].rot[1][0];
	fpga->rect.r_r22 = param->ch[1].rot[1][1];
	fpga->rect.r_r23 = param->ch[1].rot[1][2];
	fpga->rect.r_r31 = param->ch[1].rot[2][0];
	fpga->rect.r_r32 = param->ch[1].rot[2][1];
	fpga->rect.r_r33 = param->ch[1].rot[2][2];

	// Common to LR
	fpga->rect.c       = (param->ch[0].c[0] << 16) + param->ch[0].c[1];
	fpga->rect.fx2_inv = param->ch[0].f2inv[0];
	fpga->rect.fy2_inv = param->ch[0].f2inv[1];
	fpga->rect.cx2_fx2 = param->ch[0].c2_f2[0];
	fpga->rect.cy2_fy2 = param->ch[0].c2_f2[1];
}

void rect_remap(struct RECT_PARAM *param, struct MAT2S *map1, struct MAT2S *map2)
{
	struct MAT2S *src;

	for (int lr = 0; lr < 2; lr++) {

		// L/R select
		src = (lr == 0) ? map1 : map2;

		for (int ydst = 0; ydst < src->rows; ydst++) {
			for (int xdst = 0; xdst < src->cols; xdst++) {

				// (u10.0) * (u-8.32) = (u2.32) -> (u1.24)
				long long xdst_fx2 = ((long long)xdst * (long long)param->ch[lr].f2inv[0]) >> 8;
				long long ydst_fy2 = ((long long)ydst * (long long)param->ch[lr].f2inv[1]) >> 8;

				// (u1.24) - (u0.24) = (s1.24)
				long long xd = xdst_fx2 - (long long)param->ch[lr].c2_f2[0];
				long long yd = ydst_fy2 - (long long)param->ch[lr].c2_f2[1];

				// (s0.24) * (s.1.24) = (s1.48) -> (s1.24)
				long long r11x = ((long long)param->ch[lr].rot[0][0] * xd) >> 24;
				long long r21y = ((long long)param->ch[lr].rot[1][0] * yd) >> 24;
				long long r12x = ((long long)param->ch[lr].rot[0][1] * xd) >> 24;
				long long r22y = ((long long)param->ch[lr].rot[1][1] * yd) >> 24;
				long long r13x = ((long long)param->ch[lr].rot[0][2] * xd) >> 24;
				long long r23y = ((long long)param->ch[lr].rot[1][2] * yd) >> 24;

				// (s1.24) + (s1.24) = (s2.24) -> (s1.24)
				long long r11x_21y = r11x + r21y;
				long long r12x_22y = r12x + r22y;
				long long r13x_23y = r13x + r23y;

				// (s1.24) + (s0.24) = (s2.24) -> (s1.24)
				long long lx = r11x_21y + (long long)param->ch[lr].rot[2][0];
				long long ly = r12x_22y + (long long)param->ch[lr].rot[2][1];
				long long lw = r13x_23y + (long long)param->ch[lr].rot[2][2];

				// (s1.24) / (s1.24) = (s25.24) -> (s1.24)
				long long lw_inv = (1ull << 48) / lw;

				// (s1.24) * (s1.24) = (s2.48) -> (s1.24)
				long long x2 = (lx * lw_inv) >> 24;
				long long y2 = (ly * lw_inv) >> 24;

				// (s1.24) * (u10.16) = (s11.40) -> (s10.6)
				long long x_fx = (x2 * (long long)param->ch[lr].f[0]) >> 34;
				long long y_fy = (y2 * (long long)param->ch[lr].f[1]) >> 34;

				// (s10.6) + (u10.0) = (s11.6)
				long long x_fx_cx = x_fx + ((long long)param->ch[lr].c[0] << 6);
				long long y_fy_cy = y_fy + ((long long)param->ch[lr].c[1] << 6);

				long long x_fx_cx_rnd = (x_fx_cx + 1) >> 1;
				long long y_fy_cy_rnd = (y_fy_cy + 1) >> 1;

				// store to Mat structure
				int loc = ydst * src->cols + xdst;
				src->data[0][loc] = (short)x_fx_cx_rnd;
				src->data[1][loc] = (short)y_fy_cy_rnd;
			}
		}
	}
}

void rect_cmd_gen(struct MAT2S map1, struct MAT2S map2)
{
	int ysrc_r = -1;
	int len = 0;

	int trn_num, trn_cnt;

	// transition information
	struct trn_elem* trn;
	struct trn_elem* trn2;

	//================================================================
	// Calculate transition information
	//----------------------------------------------------------------
	// remap coordinates are searched and grouped into same integer
	// part of ysrc. It's start coordinate and length are recorded
	// into trn structure.
	//================================================================
	// calculate number of transitions
	trn_num = 0;
	for (int lr = 0; lr < 2; lr++) {
		struct MAT2S map = (lr == 0) ? map1 : map2;

		for (int y = 0; y < map.rows; y++) {
			for (int x = 0; x < map.cols; x++) {
				int ysrc = (int)(map.data[1][y * map.cols + x] / 32); // integer part of ysrc
				if ((x != 0) && (ysrc != ysrc_r)) {
					trn_num++;
				}
				ysrc_r = ysrc;
			}
			trn_num++;
		}
	}

	// array of transition information
	trn = (struct trn_elem*)malloc(sizeof(struct trn_elem) * trn_num);

	trn_cnt = 0;
	for (int lr = 0; lr < 2; lr++) {

		// L/R select
		struct MAT2S map = (lr == 0) ? map1 : map2;

		for (int y = 0; y < map.rows; y++) {
			for (int x = 0; x < map.cols; x++) {
				// coordinates of source image correspond to [y,x] of
				// destination image.
				int ysrc = (int)map.data[1][y * map.cols + x] / 32;

				if (x == 0) {
					// left-most pixel
					// -> initialize new trn_elem
					trn[trn_cnt].lr   = lr;
					trn[trn_cnt].ysrc = ysrc;
					trn[trn_cnt].ydst = y;
					trn[trn_cnt].xdst = x;
				}
				else if (ysrc != ysrc_r) {
					// src column is changed
					// -> len is fixed, record to trn_elem
					trn[trn_cnt].len = len;
					trn_cnt++;
					len = 0;

					// initialize the next trn_elem
					trn[trn_cnt].lr   = lr;
					trn[trn_cnt].ysrc = ysrc;
					trn[trn_cnt].ydst = y;
					trn[trn_cnt].xdst = x;
				}
				len++;
				ysrc_r = ysrc;
			}

			// end of line
			// -> len is fixed, record to trn_elem
			trn[trn_cnt].len = len;
			trn_cnt++;
			len = 0;
		}
	}

	//================================================================
	// sort transition information in ascending order according to
	// ysrc
	//================================================================
	trn_cnt = 0;
	trn2 = (struct trn_elem*)malloc(sizeof(struct trn_elem) * trn_num);
	for (int y = 0; y < map1.rows; y++) {
		for (int i = 0; i < trn_num; i++) {
			if (trn[i].ysrc == y) {
				trn2[trn_cnt] = trn[i];
				trn_cnt++;
			}
		}
	}

	//================================================================
	// Generates a sequence of command which is to be written to FPGA
	//----------------------------------------------------------------
	// Y command
	//  cmd[   17] : LR bit（L:0、R:1）
	//     [16: 8] : Y coordinate
	//     [    7] : Not used
	//     [ 6: 0] : Length（fixed to 0）
	// X command
	//  cmd[17: 8] : X coordinate
	//     [    7] : ydif
	//     [ 6: 0] : Length（1-127）
	//================================================================
	fpga->rect.CmdControl |= 0x00000001; // reset command counter
	fpga->rect.CmdControl &= 0xFFFFFFFD; // command write mode

	// replica of information stored in FPGA
	int int_ysrc = -1;
	int int_lr = 0;
	int int_ydst = 0;
	for (int i = 0; i < trn_num; i++) {

		// ith transition
		struct trn_elem trn_elem = trn2[i];

		// ysrc or lr changed
		// -> return to the start of line
		// -> issue Y command
		if ((int_ysrc != trn_elem.ysrc) || (int_lr != trn_elem.lr)) {
			cmd_y(&trn_elem, &int_ysrc, &int_lr, &int_ydst);
		}

		// issue X command
		cmd_x(&trn_elem, &int_ydst);
	}

	xil_printf("%d commands written.", fpga->rect.CmdStatus);

	fpga->rect.CmdControl |= 0x00000002; // command read mode

	free (trn);
	free (trn2);
}

void issue_cmd(struct cmd_elem cmd) {
	unsigned long val;

	if (cmd.len == 0)
	{
		// Y command
		val = (cmd.lr << 17) + (cmd.ydst << 8);
		fpga->rect.CmdWritePort = val;
	}
	else
	{
		// X command
		// start address when the image data is written in continuous memory space
		int start_address = cmd.ydst * IMAGE_WIDTH + cmd.xdst;
		// offset in the current 4kB bank
		int mod_4kb = start_address % 4096;
		// number of bytes remain in the current bank
		int rem_4kb = 4096 - mod_4kb;
		if (cmd.len > rem_4kb)
		{
			// burst access cross 4kB boundary
			// -> split into two burst accesses
			int len1 = rem_4kb;
			val = (cmd.xdst << 8) + (cmd.ydif << 7) + len1;
			fpga->rect.CmdWritePort = val;

			int xdst2 = cmd.xdst + len1;
			int len2 = cmd.len - len1;
			val = (xdst2 << 8) + len2; // y doesn't change
			fpga->rect.CmdWritePort = val;
		}
		else
		{
			val = (cmd.xdst << 8) + (cmd.ydif << 7) + cmd.len;
			fpga->rect.CmdWritePort = val;
		}
	}
}

void cmd_y(
	struct trn_elem *trn_elem,
	int *int_ysrc,
	int *int_lr,
	int *int_ydst
) {
	struct cmd_elem cmd_elem;

	cmd_elem.lr   = trn_elem->lr;
	cmd_elem.ydst = trn_elem->ydst;
	cmd_elem.xdst = 0;
	cmd_elem.ydif = 0;
	cmd_elem.len  = 0;
	issue_cmd(cmd_elem);

	// update internal state
	*int_ysrc = trn_elem->ysrc;
	*int_lr   = trn_elem->lr;
	*int_ydst = trn_elem->ydst;

	return;
}

void cmd_x(
	struct trn_elem *trn_elem,
	int *int_ydst
) {
	struct cmd_elem cmd_elem;
	cmd_elem.lr   = 0;
	cmd_elem.ydst = trn_elem->ydst;

	// split to burst length
	while (trn_elem->len > 127) {
		cmd_elem.ydif = trn_elem->ydst - *int_ydst;
		if (cmd_elem.ydif > 1) xil_printf("error: ydif greater than 1");
		cmd_elem.xdst = trn_elem->xdst;
		cmd_elem.len = 127;
		issue_cmd(cmd_elem);
		trn_elem->xdst += 127;
		trn_elem->len -= 127;

		// update internal state
		*int_ydst = trn_elem->ydst;
	}

	// normal operation
	cmd_elem.ydif = trn_elem->ydst - *int_ydst;
	if (cmd_elem.ydif > 1) xil_printf("error: ydif greater than 1");
	cmd_elem.xdst = trn_elem->xdst;
	cmd_elem.len = trn_elem->len;
	issue_cmd(cmd_elem);

	// update internal state
	*int_ydst = trn_elem->ydst;

	return;
}

int Fpga_ReadSwitch(void) {
	// returns the state of the switch
	// 1 if pressed.
	if ((fpga->com.GPIO_In & 0x00000002) == 0x00000002) {
		return 1;
	}
	else {
		return 0;
	}
}

int Fpga_IsSwitchPressed(void) {
	// returns 1 if the switch has been pressed
	if ((fpga->com.Hold_Pos & 0x00000002) == 0x00000002) {
		fpga->com.Hold_Pos = 0x00000002; // write 1 to clear
		return 1;
	}
	else {
		return 0;
	}
}

void Fpga_LedOn (void) {
	unsigned int tmpi;
	tmpi = fpga->com.GPIO_Out;
	tmpi |= 0x00000002;
	fpga->com.GPIO_Out = tmpi;
}

void Fpga_LedOff (void) {
	unsigned int tmpi;
	tmpi = fpga->com.GPIO_Out;
	tmpi &= ~0x00000002;
	fpga->com.GPIO_Out = tmpi;
}

unsigned int Fpga_TimeElapsedUs (int start_time) {
	unsigned int current_time = fpga->com.Timer;
	unsigned int elapsed_time;
	if (current_time < start_time) {
		// timer roll over
		elapsed_time = (current_time + 4294967296) - start_time;
	}
	else {
		elapsed_time = current_time - start_time;
	}

	return elapsed_time;
}

void Fpga_WaitUs (int us) {
	unsigned int start_time = fpga->com.Timer;

	while(1) {
		if (Fpga_TimeElapsedUs(start_time) > us) {
			break;
		}
	}
}

unsigned int Fpga_TimeElapsedMs (int start_time) {
	unsigned int elapsed_time = Fpga_TimeElapsedUs(start_time);
	return elapsed_time / 1000;
}

void Fpga_WaitMs (int ms) {
	Fpga_WaitUs(ms * 1000);
}

