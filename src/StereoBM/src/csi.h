//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file csi.h
//!
//! Header file for csi.c
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#ifndef CSI_H
#define CSI_H


#include "xil_printf.h"
#include "xparameters.h"
#include "sleep.h"

#include "main.h"
#include "uart.h"
#include "fpga.h"


//=============================================================================
// Register Map Defined in
//   MIPI CSI-2 Receiver Subsystem v5.0
//   MIPI D-PHY v4.2
//=============================================================================
// MIPI CSI-2 Receiver Subsystem
struct CSI_REG {
	volatile unsigned int core_config;		// 0x00
	volatile unsigned int prot_config;		// 0x04
	volatile unsigned int rsvd1[2];			// 0x08-0x0C
	volatile unsigned int core_sts;			// 0x10
	volatile unsigned int rsvd2[3];			// 0x14-0x1C
	volatile unsigned int global_intr_enb;	// 0x20
	volatile unsigned int intr_sts;			// 0x24
	volatile unsigned int intr_enb;			// 0x28
	volatile unsigned int rsvd3;			// 0x2C
};

// MIPI D-PHY
struct DPHY_REG {
	volatile unsigned int ctrl;				// 0x00
	volatile unsigned int tap;				// 0x04
	volatile unsigned int init;				// 0x08
	volatile unsigned int rsvd1;			// 0x0C
	volatile unsigned int hs_timeout;		// 0x10
	volatile unsigned int esc_timeout;		// 0x14
	volatile unsigned int cl_sts;			// 0x18
	volatile unsigned int dl0_sts;			// 0x1C
	volatile unsigned int dl1_sts;			// 0x20
	volatile unsigned int dl2_sts;			// 0x24
	volatile unsigned int dl3_sts;			// 0x28
	volatile unsigned int rsvd2;			// 0x2C
	volatile unsigned int hs_settle0;		// 0x30
	volatile unsigned int rsvd3[5];			// 0x34-0x44
	volatile unsigned int hs_settle1;		// 0x48
	volatile unsigned int hs_settle2;		// 0x4C
	volatile unsigned int hs_settle3;		// 0x50
	volatile unsigned int hs_settle4;		// 0x54
	volatile unsigned int hs_settle5;		// 0x58
	volatile unsigned int hs_settle6;		// 0x5C
	volatile unsigned int hs_settle7;		// 0x60
	volatile unsigned int dl4_sts;			// 0x64
	volatile unsigned int dl5_sts;			// 0x68
	volatile unsigned int dl6_sts;			// 0x6C
	volatile unsigned int dl7_sts;			// 0x70
	volatile unsigned int tap2;				// 0x74
};


//=============================================================================
// Register Bit Field Definition for MIPI CSI-2 Receiver
//=============================================================================
// Core Configuration Register
// [    1]: Soft Reset
// [    0]: Core Enable
#define CSI_CORE_CONFIG_RST		0x00000002
#define CSI_CORE_CONFIG_ENB		0x00000001

// Protocol Configuration Register
// [ 4: 3]: Maximum Lanes
// [ 1: 0]: Active Lanes
#define CSI_PROT_CONFIG_MAX_LANES_OFFSET	3
#define CSI_PROT_CONFIG_MAX_LANES	0x0000000C
#define CSI_PROT_CONFIG_ACT_LANES_OFFSET	0
#define CSI_PROT_CONFIG_ACT_LANES	0x00000003

// Core Status Register
// [31:16]: Packet Count
// [    3]: Short Packet FIFO FULL
// [    2]: Short Packet FIFO not Empty
// [    1]: Stream Line Buffer Full
// [    0]: Soft Reset/Core Disable in Progress
#define CSI_CORE_STS_PC_OFFSET				16
#define CSI_CORE_STS_PC				0xFFFF0000
#define CSI_CORE_STS_SP_FULL		0x00000008
#define CSI_CORE_STS_SP_NOT_EMPTY	0x00000004
#define CSI_CORE_STS_SL_FULL		0x00000002
#define CSI_CORE_STS_SR_IN PROG		0x00000001

// CSI Interrupt Source
#define CSI_INTR_FRAME_RCVD			0x80000000
#define CSI_INTR_FRAME_ERR			0x40000000
#define CSI_INTR_RX_SKEWCALHS		0x20000000
#define CSI_INTR_WC_CORRUPT			0x00400000
#define CSI_INTR_INCORRECT_LANE		0x00200000
#define CSI_INTR_SP_FIFO_FULL		0x00100000
#define CSI_INTR_SP_FIFO_NOT_EMPTY	0x00080000
#define CSI_INTR_SL_BUFFER_FULL		0x00040000
#define CSI_INTR_STOP_STATE			0x00020000
#define CSI_INTR_SOT_ERR			0x00002000
#define CSI_INTR_SOT_SYNC_ERR		0x00001000
#define CSI_INTR_ECC_2BIT_ERR		0x00000800
#define CSI_INTR_ECC_1BIT_ERR		0x00000400
#define CSI_INTR_CRC_ERR			0x00000200
#define CSI_INTR_UNSUPPORTED_DT		0x00000100
#define CSI_INTR_FRAME_SYNC_ERR_VC3	0x00000080
#define CSI_INTR_FRAME_LVL_ERR_VC3	0x00000040
#define CSI_INTR_FRAME_SYNC_ERR_VC2	0x00000020
#define CSI_INTR_FRAME_LVL_ERR_VC2	0x00000010
#define CSI_INTR_FRAME_SYNC_ERR_VC1	0x00000008
#define CSI_INTR_FRAME_LVL_ERR_VC1	0x00000004
#define CSI_INTR_FRAME_SYNC_ERR_VC0	0x00000002
#define CSI_INTR_FRAME_LVL_ERR_VC0	0x00000001


//=============================================================================
// Register Bit Field Definition for MIPI D-PHY
//=============================================================================
// CONTROL Register
// [    1]: DPHY_EN
// [    0]: SRST
#define DPHY_CTRL_DPHY_EN		0x00000002
#define DPHY_CTRL_SRST			0x00000001

// CL_STATUS Register
// [    5]: ERR_CONTROL
// [    4]: STOP_STATE
// [    3]: INIT_DONE
// [    2]: ULPS
// [ 1: 0]: MODE
#define DPHY_CLSTS_ERR_CTRL		0x00000020
#define DPHY_CLSTS_STOP_STATE	0x00000010
#define DPHY_CLSTS_INIT_DONE	0x00000008
#define DPHY_CLSTS_ULPS			0x00000004
#define DPHY_CLSTS_MODE			0x00000003

// DL_STATUS Register
// [31:16]: PKT_CNT
// [    6]: STOP_STATE
// [    5]: ESC_ABORT
// [    4]: HS_ABORT
// [    3]: INIT_DONE
// [    2]: ULPS
// [ 1: 0]: MODE
#define DPHY_DLSTS_PKT_CNT_OFFSET		16
#define DPHY_DLSTS_PKT_CNT		0xFFFF0000
#define DPHY_DLSTS_STOP_STATE	0x00000040
#define DPHY_DLSTS_ESC_ABORT	0x00000020
#define DPHY_DLSTS_HS_ABORT		0x00000010
#define DPHY_DLSTS_INIT_DONE	0x00000008
#define DPHY_DLSTS_ULPS			0x00000004
#define DPHY_DLSTS_MODE			0x00000003


//=============================================================================
// Other Definitions
//=============================================================================
enum CSI_RESET_MODE		{CSI_RESET_ASSERT, CSI_RESET_RELEASE};
enum CSI_LANE_MODE		{CSI_ONE_LANE, CSI_TWO_LANE};
enum CAMERA_POSITION	{CAM_L, CAM_R};


//=============================================================================
// Function Prototypes
//=============================================================================
void Csi_ReadStatus (void);
s32 Csi_CheckStatus (void);
void Csi_VideoReset (int mode);
void Csi_SoftReset (int mode);
void Csi_DphyReset (int mode);
void Csi_SetLaneMode (int num);
void Csi_Init (int lane_mode);

#endif // CSI_H
