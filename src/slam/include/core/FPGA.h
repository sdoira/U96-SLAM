//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "opencv2/core/core.hpp"
#include "opencv2/highgui.hpp"
#include "core/Parameters.h"
#include "core/SensorData.h"


//==========================================================================
// Memory Map
//--------------------------------------------------------------------------
// DDR Address Space: 2000_0000 - 3FFF_FFFF (2GMB)
//--------------------------------------------------------------------------
// [SLAM Project]
//  MEM_BASE_ADDR  : 7200_0000
//  BUF_GRB_A : 7200_0000 - 721F_FFFF (2MB)
//  BUF_GRB_B : 7220_0000 - 723F_FFFF (2MB)
//  BUF_XSBL_A: 7240_0000 - 724F_FFFF (1MB)
//  BUF_XSBL_B: 7250_0000 - 725F_FFFF (1MB)
//  Padding   : 7260_0000 - 727F_FFFF (2MB)
//  BUF_BM_A  : 7280_0000 - 729F_FFFF (2MB)
//  Padding   : 72A0_0000 - 72BF_FFFF (2MB)
//  BUF_BM_B  : 72C0_0000 - 72DF_FFFF (2MB)
//  BUF_RECT_A: 72E0_0000 - 72EF_FFFF (1MB)
//  BUF_RECT_B: 72F0_0000 - 72FF_FFFF (1MB)
//  BUF_DISP_A: 7300_0000 - 730F_FFFF (1MB)
//  BUF_DISP_B: 7310_0000 - 731F_FFFF (1MB)
//  BUF_GFTT_A: 7320_0000 - 732F_FFFF (1MB)
//  BUF_GFTT_B: 7330_0000 - 733F_FFFF (1MB)
//==========================================================================
#define MEM_BASE_ADDR	0x72000000
#define GRB_MAX_SIZE	0x00200000 // 2MB (512H * 1024W * 2YUV * 2LR)
#define XSBL_MAX_SIZE	0x00100000 // 1MB (512H * 1024W * 2LR)
#define BM_MAX_SIZE		0x00400000 // 2MB (512H * 1024W * 4bytes)->4MB align
#define RECT_MAX_SIZE	0x00100000 // 1MB (512H * 1024W * 2LR)
#define DISP_MAX_SIZE	0x00100000 // 1MB (512H * 1024W * 2bytes)
#define GFTT_MAX_SIZE	0x00100000 // 1MB (512H * 1024W * 2bytes)
#define BUF_GRB_A		MEM_BASE_ADDR
#define BUF_GRB_B		(BUF_GRB_A  + GRB_MAX_SIZE)
#define BUF_XSBL_A		(BUF_GRB_B  + GRB_MAX_SIZE)
#define BUF_XSBL_B		(BUF_XSBL_A + XSBL_MAX_SIZE)
#define BUF_BM_A		(BUF_XSBL_B + XSBL_MAX_SIZE + 0x00200000) // 4MB align
#define BUF_BM_B		(BUF_BM_A   + BM_MAX_SIZE)
#define BUF_RECT_A		(BUF_BM_B   + BM_MAX_SIZE)
#define BUF_RECT_B		(BUF_RECT_A + RECT_MAX_SIZE)
#define BUF_DISP_A		(BUF_RECT_B + RECT_MAX_SIZE)
#define BUF_DISP_B		(BUF_DISP_A + DISP_MAX_SIZE)
#define BUF_GFTT_A		(BUF_DISP_B + DISP_MAX_SIZE)
#define BUF_GFTT_B		(BUF_GFTT_A + GFTT_MAX_SIZE)

#define IMAGE_HEIGHT	480
#define IMAGE_WIDTH		640

#define RECT_ALIGN_HEIGHT	512
#define RECT_ALIGN_WIDTH	1024
#define RECT_FRAME_OFFSET	(RECT_ALIGN_HEIGHT * RECT_ALIGN_WIDTH)


//=============================================================================
// Memory Mapped Registers
//=============================================================================
//! Register map for COM (common functions)
struct FPGA_REG_COM {
	volatile unsigned int Version;			//!< [0000h]
	volatile unsigned int Testpad;			//!< [0004h]
	volatile unsigned int LED;				//!< [0008h]
	volatile unsigned int Switch;			//!< [000Ch]
	volatile unsigned int Timer;			//!< [0010h]
	volatile unsigned int InterruptEnable;	//!< [0014h]
	volatile unsigned int InterruptStatus;	//!< [0018h]
	volatile unsigned int PatternSelect;	//!< [001Ch]
	volatile unsigned int GPIO_In;			//!< [0020h]
	volatile unsigned int GPIO_Out;			//!< [0024h]
	volatile unsigned int Hold_Pos;			//!< [0028h]
	volatile unsigned int Hold_Neg;			//!< [002Ch]
	volatile unsigned int Blink;			//!< [0030h]
	volatile unsigned int SwitchHold;		//!< [0034h]
	volatile unsigned int rsvd1[2];			//!< [0038h-003Ch]
	volatile unsigned int IpcMessage1;		//!< [0040h]
	volatile unsigned int IpcMessage2;		//!< [0044h]
	volatile unsigned int IpcParameter1;	//!< [0048h]
	volatile unsigned int IpcParameter2;	//!< [004Ch]
	volatile unsigned int IpcParameter3;	//!< [0050h]
	volatile unsigned int IpcParameter4;	//!< [0054h]
	volatile unsigned int rsvd2[1002];		//!< [0058h-0FFCh]
};

struct FPGA_REG_GLB {
	volatile unsigned int rsvd[64];
};

struct FPGA_REG_CSI {
	volatile unsigned int Control;			//!< [1100h]
	volatile unsigned int Status;			//!< [1104h]
	volatile unsigned int Size1;			//!< [1108h]
	volatile unsigned int PatternSelect1;	//!< [110Ch]
	volatile unsigned int FrameLength1;		//!< [1110h]
	volatile unsigned int FrameCoun1;		//!< [1114h]
	volatile unsigned int Size2;			//!< [1118h]
	volatile unsigned int PatternSelect2;	//!< [111Ch]
	volatile unsigned int FrameLength2;		//!< [1120h]
	volatile unsigned int FrameCount2;		//!< [1124h]
	volatile unsigned int FrameDecimation;	//!< [1128h]
	volatile unsigned int rsvd[53];
};

struct FPGA_REG_FRM {
	volatile unsigned int rsvd[64];
};

struct FPGA_REG_MES {
	volatile unsigned int rsvd[64];
};

struct FPGA_REG_SEL {
	volatile unsigned int rsvd[64];
};

struct FPGA_REG_GRB {
	volatile unsigned int Control;			//!< [1500h]
	volatile unsigned int Status;			//!< [1504h]
	volatile unsigned int Address_A;		//!< [1508h]
	volatile unsigned int Address_B;		//!< [150Ch]
	volatile unsigned int BurstLength;		//!< [1510h]
	volatile unsigned int Size;				//!< [1514h]
	volatile unsigned int rsvd[58];
};

struct FPGA_REG_XSBL {
	volatile unsigned int Control;			//!< [1600h]
	volatile unsigned int Status;			//!< [1604h]
	volatile unsigned int Address_In_A;		//!< [1608h]
	volatile unsigned int Address_In_B;		//!< [160Ch]
	volatile unsigned int Address_Out_A;	//!< [1610h]
	volatile unsigned int Address_Out_B;	//!< [1614h]
	volatile unsigned int BurstLength;		//!< [1618h]
	volatile unsigned int Size;				//!< [161Ch]
	volatile unsigned int rsvd[56];
};

struct FPGA_REG_BM {
	volatile unsigned int Control;			//!< [1700h]
	volatile unsigned int Status;			//!< [1704h]
	volatile unsigned int ImageSize;		//!< [1708h]
	volatile unsigned int BmSetting;		//!< [170Ch]
	volatile unsigned int LR_Address_A;		//!< [1710h]
	volatile unsigned int LR_Address_B;		//!< [1714h]
	volatile unsigned int SAD_Address_A;	//!< [1718h]
	volatile unsigned int SAD_Address_B;	//!< [171Ch]
	volatile unsigned int BurstLength;		//!< [1720h]
	volatile unsigned int SAD_Size;			//!< [1724h]
	volatile unsigned int UniFiltCtrl;		//!< [1728h]
	volatile unsigned int DISP_Address_A;	//!< [172Ch]
	volatile unsigned int DISP_Address_B;	//!< [1730h]
	volatile unsigned int rsvd[51];
};

//! Register map for DDR Arbiter
struct FPGA_REG_ARB {
	volatile unsigned int Control;
	volatile unsigned int Status;
	volatile unsigned int rsvd[62];
};

struct FPGA_REG_RECT {
	volatile unsigned int Control;			//!< [1900h]
	volatile unsigned int Status;			//!< [1904h]
	volatile unsigned int CmdControl;		//!< [1908h]
	volatile unsigned int CmdStatus;		//!< [190Ch]
	volatile unsigned int CmdWritePort;		//!< [1910h]
	volatile unsigned int l_fx;				//!< [1914h]
	volatile unsigned int l_fy;				//!< [1918h]
	volatile unsigned int r_fx;				//!< [191Ch]
	volatile unsigned int r_fy;				//!< [1920h]
	volatile unsigned int c;				//!< [1924h]
	volatile unsigned int fx2_inv;			//!< [1928h]
	volatile unsigned int fy2_inv;			//!< [192Ch]
	volatile unsigned int cx2_fx2;			//!< [1930h]
	volatile unsigned int cy2_fy2;			//!< [1934h]
	volatile unsigned int l_r11;			//!< [1938h]
	volatile unsigned int l_r12;			//!< [193Ch]
	volatile unsigned int l_r13;			//!< [1940h]
	volatile unsigned int l_r21;			//!< [1944h]
	volatile unsigned int l_r22;			//!< [1948h]
	volatile unsigned int l_r23;			//!< [194Ch]
	volatile unsigned int l_r31;			//!< [1950h]
	volatile unsigned int l_r32;			//!< [1954h]
	volatile unsigned int l_r33;			//!< [1958h]
	volatile unsigned int r_r11;			//!< [195Ch]
	volatile unsigned int r_r12;			//!< [1960h]
	volatile unsigned int r_r13;			//!< [1964h]
	volatile unsigned int r_r21;			//!< [1968h]
	volatile unsigned int r_r22;			//!< [196Ch]
	volatile unsigned int r_r23;			//!< [1970h]
	volatile unsigned int r_r31;			//!< [1974h]
	volatile unsigned int r_r32;			//!< [1978h]
	volatile unsigned int r_r33;			//!< [197Ch]
	volatile unsigned int Address_A;		//!< [1980h]
	volatile unsigned int Address_B;		//!< [1984h]
	volatile unsigned int rsvd[30];
};

//! Register map for GFTT Detector
struct FPGA_REG_GFTT {
	volatile unsigned int Control;			//!< [1A00h]
	volatile unsigned int Status;			//!< [1A04h]
	volatile unsigned int Address_In_A;		//!< [1A08h]
	volatile unsigned int Address_In_B;		//!< [1A0Ch]
	volatile unsigned int Address_Out_A;	//!< [1A10h]
	volatile unsigned int Address_Out_B;	//!< [1A14h]
	volatile unsigned int BurstLength;		//!< [1A18h]
	volatile unsigned int Size;				//!< [1A1Ch]
	volatile unsigned int Max;				//!< [1A20h]
	volatile unsigned int rsvd[55];
};

//! FPGA register space
struct FPGA_REG {
	struct FPGA_REG_COM  com;  // [0000h - 0FFEh]
	struct FPGA_REG_GLB  glb;  // [1000h - 10FEh]
	struct FPGA_REG_CSI  csi;  // [1100h - 11FEh]
	struct FPGA_REG_FRM  frm;  // [1200h - 12FEh]
	struct FPGA_REG_MES  mes;  // [1300h - 13FEh]
	struct FPGA_REG_SEL  sel;  // [1400h - 14FEh]
	struct FPGA_REG_GRB  grb;  // [1500h - 15FEh]
	struct FPGA_REG_XSBL xsbl; // [1600h - 16FEh]
	struct FPGA_REG_BM   bm;   // [1700h - 17FEh]
	struct FPGA_REG_ARB  arb;  // [1800h - 18FEh]
	struct FPGA_REG_RECT rect; // [1900h - 19FEh]
	struct FPGA_REG_GFTT gftt; // [1A00h - 1AFEh]
};


//=============================================================================
// Rectification Parameters
//=============================================================================
struct RECT_PARAM_CH {
	long f[2]; // x(f[0]) and y(f[1])
	short c[2];
	long f2inv[2]; // 1 / f2
	long c2_f2[2]; // c2 / f2
	long rot[3][3]; // inverted R
};

struct RECT_PARAM {
	struct RECT_PARAM_CH ch[2]; // L(ch[0]) and R(ch[1])
};

struct MAT2S {
	int rows;
	int cols;
	short *data[2];
};

struct trn_elem {
	int lr; // L = 0, R = 1
	int ysrc;
	int ydst;
	int xdst;
	int len;
};

struct cmd_elem {
	int lr;
	int ydst;
	int xdst;
	int ydif;
	int len;
};


//=============================================================================
// Register Bit Field Definition
//=============================================================================
#define FPGA_CSI_CTRL_VRSTN		0x00000002
#define FPGA_CSI_CTRL_ENABLE	0x00000001

#define FPGA_ARB_CTRL_ENABLE	0x00000001

#define FPGA_GRB_CTRL_ENABLE	0x00000001

#define FPGA_RECT_CTRL_ENABLE	0x00000001

#define FPGA_XSBL_CTRL_ENABLE	0x00000001
#define FPGA_XSBL_SW_START		0x00000002

#define FPGA_BM_CTRL_ENABLE		0x00000001

#define FPGA_GFTT_CTRL_ENABLE	0x00000001
#define FPGA_GFTT_SW_START		0x00000002


//=============================================================================
// Other Definition
//=============================================================================
// Frequency of pl_clk0 in Hz
#define FPGA_CLOCK_FREQUENCY	100000000

// Interrupt
#define FPGA_INTR_GRB	0x00000001
#define FPGA_INTR_BM	0x00000002
#define FPGA_INTR_RECT	0x00000004
#define FPGA_INTR_XSBL	0x00000008
#define FPGA_INTR_VSYNC	0x00000010
#define FPGA_INTR_ALL   0xFFFFFFFF


//=============================================================================
// Inter-Prpcessor Communication
//=============================================================================
#define IPC_MSG_NONE			0x00000000

// linux system to remote app
#define IPC_MSG1_APP_START		0x00000001

// remote system to linux system
#define IPC_MSG2_DATA_READY		0x00000001

#define IPC_MSG2_DATA_GRB		0x00000100
#define IPC_MSG2_DATA_XSBL		0x00000101
#define IPC_MSG2_DATA_RECT		0x00000102
#define IPC_MSG2_DATA_BM		0x00000103

#define IPC_PARM_NONE			0x00000000

#define IPC_PARM_FRAME_GRAB		0x00000000
#define IPC_PARM_STEREO_RECT	0x00000001
#define IPC_PARM_STEREO_XSBL	0x00000002
#define IPC_PARM_STEREO_BM		0x00000003
#define IPC_PARM_STEREO_GFTT	0x00000004

#define IPC_PARM_SENSOR_INPUT	0x00000000
#define IPC_PARM_HORIZ_INCR		0x00000001
#define IPC_PARM_VERT_INCR		0x00000002
#define IPC_PARM_FRAME_INCR		0x00000003
#define IPC_PARM_COLOR_BAR		0x00000004
#define IPC_PARM_GRID			0x00000005

//=============================================================================
// Function Prototypes
//=============================================================================

class Fpga
{
public:
	Fpga();
	~Fpga();
	int registerOpen(void);
	int registerClose(void);
	int memoryOpen(void);
	int memoryClose(void);

	void uioIrqOn(int uio_fd);
	void uioIrqOff(int uio_fd);
	int uioIrqWait(int uio_fd);

	int readVersion(void);
	void sendIpcMessage(
		unsigned int Message,
		unsigned int Parameter1 = IPC_PARM_NONE,
		unsigned int Parameter2 = IPC_PARM_NONE,
		unsigned int Parameter3 = IPC_PARM_NONE,
		unsigned int Parameter4 = IPC_PARM_NONE
		);
	void waitIpcMessage(unsigned int Message);
	void waitIpcMessage_Perf(unsigned int message);
	void startRemoteApp(unsigned int mode, unsigned int pattern);
	void setRectImage(int bank, cv::Mat imageLeft, cv::Mat imageRight);
	void receiveRectImages(int bank, cv::Mat &matLeft, cv::Mat &matRight);
	void receiveDepthMap(int bank, cv::Mat &matDepth);
	void receiveEigen(int bank, cv::Mat &matEigen, unsigned short *maxEigen);
	void receiveData(SensorData &data, APP_SETTING appSetting);
	int readSwitch(void);
	int isSwitchPressed(void);
	void ledOn(void);
	void ledOff(void);
	void ledBlink(int rate);
	unsigned int readTimer(void);

	volatile struct FPGA_REG *reg;

private:
	int fd_dvp;
	long long address;
	int fd_mem_rect;
	int fd_mem_bm;
	int fd_mem_disp;
	int fd_mem_gftt;
	volatile unsigned char *iomap_rect;
	volatile unsigned char *iomap_bm;
	volatile unsigned char *iomap_disp;
	volatile unsigned char *iomap_gftt;
};
