//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file csi.c
//!
//! This file contains the implementation of MIPI CSI-2 Receiver Subsystem.
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#include "csi.h"

extern volatile struct FPGA_REG *fpga;

struct CSI_REG  *csi_l  = (struct CSI_REG *)XPAR_CSISS_0_BASEADDR;
struct DPHY_REG *dphy_l = (struct DPHY_REG *)(XPAR_CSISS_0_BASEADDR + 0x1000);
struct CSI_REG  *csi_r  = (struct CSI_REG *)XPAR_CSISS_1_BASEADDR;
struct DPHY_REG *dphy_r = (struct DPHY_REG *)(XPAR_CSISS_1_BASEADDR + 0x1000);


//=============================================================================
//! CSI Status Read-out
//-----------------------------------------------------------------------------
//! @brief This function reads out status of MIPI CSI and DPHY modules
//! then displays them until "ESC" key is hit.
//! Can be used to see how the status changes according to time progress.
//=============================================================================
void Csi_ReadStatus (void) {
	unsigned int cl_sts, dl0_sts, dl1_sts;
	unsigned int core_sts, intr_sts;
	int wcnt, hcnt;
	unsigned char ch;
	while(1) {
		cl_sts  = dphy_l->cl_sts;
		dl0_sts = dphy_l->dl0_sts;
		dl1_sts = dphy_l->dl1_sts;
		core_sts = csi_l->core_sts;
		intr_sts = csi_l->intr_sts;
		hcnt = (fpga->csi.Size1 >> 16) & 0xFFFF;
		wcnt = (fpga->csi.Size1) & 0xFFFF;
		csi_l->intr_sts = intr_sts; // write 1 to clear
		xil_printf("CL:%08X, DL0:%08X, DL1:%08X, CS:%08X, IS:%08X, %d, %d\r\n", cl_sts, dl0_sts, dl1_sts, core_sts, intr_sts, hcnt, wcnt);

		// esc to leave
		if (uart_getc2(&ch) && (ch == 0x1B)) {
			break;
		}
	}
}

//=============================================================================
//! CSI Status Check
//-----------------------------------------------------------------------------
//! @return	XST_SUCCESS if everything is okay else XST_FAILURE
//-----------------------------------------------------------------------------
//! @brief This function checks status of MIPI CSI and DPHY modules for
//! any errors.
//=============================================================================
s32 Csi_CheckStatus (void) {
	s32 status = XST_SUCCESS;

	struct CSI_REG  *csi;
	struct DPHY_REG *dphy;
	for (int lr = 0; lr < 2; lr++) {
		//------------------------------------------------------------------
		// Camera Select
		//------------------------------------------------------------------
		if (lr == 0) {
			xil_printf("Left camera status check\r\n");
			csi  = (struct CSI_REG*)XPAR_CSISS_0_BASEADDR;
			dphy = (struct DPHY_REG*)(XPAR_CSISS_0_BASEADDR + 0x1000);
		} else {
			xil_printf("Right camera status check\r\n");
			csi  = (struct CSI_REG*)XPAR_CSISS_1_BASEADDR;
			dphy = (struct DPHY_REG*)(XPAR_CSISS_1_BASEADDR + 0x1000);
		}

		//------------------------------------------------------------------
		// Clock Lane Status
		//------------------------------------------------------------------
		unsigned int cl_sts  = dphy->cl_sts;
		if ((cl_sts & DPHY_CLSTS_INIT_DONE) != DPHY_CLSTS_INIT_DONE) {
			xil_printf("clock lane not initialized.\r\n");
			status = XST_FAILURE;
		}
		if ((cl_sts & DPHY_CLSTS_ERR_CTRL) == DPHY_CLSTS_ERR_CTRL) {
			xil_printf("clock lane control error.\r\n");
			status = XST_FAILURE;
		}

		//------------------------------------------------------------------
		// Data Lane [0..1] Status
		//------------------------------------------------------------------
		unsigned int dl_sts;
		for (int lane = 0; lane < 2; lane++) {
			dl_sts = (lane == 0) ? dphy_l->dl0_sts : dphy_l->dl1_sts;

			if ((dl_sts & DPHY_DLSTS_INIT_DONE) != DPHY_DLSTS_INIT_DONE) {
				xil_printf("data lane %d not initialized.\r\n", lane);
				status = XST_FAILURE;
			}
			if ((dl_sts & DPHY_DLSTS_ESC_ABORT) == DPHY_DLSTS_ESC_ABORT) {
				xil_printf("data lane %d escape mode timeout\r\n", lane);
				status = XST_FAILURE;
			}
			if ((dl_sts & DPHY_DLSTS_HS_ABORT) == DPHY_DLSTS_HS_ABORT) {
				xil_printf("data lane %d high-speed timeout\r\n", lane);
				status = XST_FAILURE;
			}
		}

		//------------------------------------------------------------------
		// Core Status
		//------------------------------------------------------------------
		unsigned int core_sts = csi->core_sts;
		if ((core_sts & CSI_CORE_STS_SP_FULL) == CSI_CORE_STS_SP_FULL) {
			xil_printf("short packet FIFO full\r\n");
			status = XST_FAILURE;
		}
		if ((core_sts & CSI_CORE_STS_SL_FULL) == CSI_CORE_STS_SL_FULL) {
			xil_printf("stream line buffer full\r\n");
			status = XST_FAILURE;
		}

		//------------------------------------------------------------------
		// Interrupt Status
		//------------------------------------------------------------------
		unsigned int intr_sts = csi->intr_sts;
		if ((intr_sts & CSI_INTR_FRAME_ERR) == CSI_INTR_FRAME_ERR) {
			xil_printf("VCX frame error\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_WC_CORRUPT) == CSI_INTR_WC_CORRUPT) {
			xil_printf("WC corruption\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_INCORRECT_LANE) == CSI_INTR_INCORRECT_LANE) {
			xil_printf("incorrect lane configuration\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_SP_FIFO_FULL) == CSI_INTR_SP_FIFO_FULL) {
			xil_printf("short packet FIFO full\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_SL_BUFFER_FULL) == CSI_INTR_SL_BUFFER_FULL) {
			xil_printf("stream line buffer full\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_SOT_ERR) == CSI_INTR_SOT_ERR) {
			xil_printf("SoT error\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_SOT_SYNC_ERR) == CSI_INTR_SOT_SYNC_ERR) {
			xil_printf("SoT sync error\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_ECC_2BIT_ERR) == CSI_INTR_ECC_2BIT_ERR) {
			xil_printf("ECC 2-bit error\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_ECC_1BIT_ERR) == CSI_INTR_ECC_1BIT_ERR) {
			xil_printf("ECC 1-bit error\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_CRC_ERR) == CSI_INTR_CRC_ERR) {
			xil_printf("CRC error\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_UNSUPPORTED_DT) == CSI_INTR_UNSUPPORTED_DT) {
			xil_printf("unsupported data type\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_FRAME_SYNC_ERR_VC3) == CSI_INTR_FRAME_SYNC_ERR_VC3) {
			xil_printf("frame synchronization error for VC3\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_FRAME_LVL_ERR_VC3) == CSI_INTR_FRAME_LVL_ERR_VC3) {
			xil_printf("frame level error for VC3\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_FRAME_SYNC_ERR_VC2) == CSI_INTR_FRAME_SYNC_ERR_VC2) {
			xil_printf("frame synchronization error for VC2\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_FRAME_LVL_ERR_VC2) == CSI_INTR_FRAME_LVL_ERR_VC2) {
			xil_printf("frame level error for VC2\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_FRAME_SYNC_ERR_VC1) == CSI_INTR_FRAME_SYNC_ERR_VC1) {
			xil_printf("frame synchronization error for VC1\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_FRAME_LVL_ERR_VC1) == CSI_INTR_FRAME_LVL_ERR_VC1) {
			xil_printf("frame level error for VC1\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_FRAME_SYNC_ERR_VC0) == CSI_INTR_FRAME_SYNC_ERR_VC0) {
			xil_printf("frame synchronization error for VC0\r\n");
			status = XST_FAILURE;
		}
		if ((intr_sts & CSI_INTR_FRAME_LVL_ERR_VC0) == CSI_INTR_FRAME_LVL_ERR_VC0) {
			xil_printf("frame level error for VC0\r\n");
			status = XST_FAILURE;
		}

		//------------------------------------------------------------------
		// Measured Frame Format
		//------------------------------------------------------------------
		//int size = (lr == 0) ? fpga->csi_l.Size : fpga->csi_r.Size;
		//int frame_len = (lr == 0) ? fpga->csi_l.FrameLength : fpga->csi_r.FrameLength;
		int size;
		int frame_len;
		if (lr == 0) {
			size = fpga->csi.Size1;
			frame_len = fpga->csi.FrameLength1;
		} else {
			size = fpga->csi.Size2;
			frame_len = fpga->csi.FrameLength2;
		}
		int height = (size >> 16) & 0xFFFF;
		int width = size & 0xFFFF;
		double fps = (double)FPGA_CLOCK_FREQUENCY / (double)frame_len;
		int fps_i = fps;
		int fps_f = (fps - fps_i) * 100;
		xil_printf("  width %d, height %d, %d.%02d fps\r\n", width, height, fps_i, fps_f);
		if ((width == 0) || (height == 0) || (fps == 0)) {
			xil_printf("frame format error\r\n");
			status = XST_FAILURE;
		}
	} // lr loop

	return status;
}

//=============================================================================
//! CSI Video Reset
//-----------------------------------------------------------------------------
//! @param mode		CSI_RESET_ASSERT: assert reset,
//!            		CSI_RESET_RELEASE: release reset
//-----------------------------------------------------------------------------
//! @brief This function asserts or de-asserts "video_aresetn" signal of CSI
//! receiver module according to "mode" parameter.
//=============================================================================
void Csi_VideoReset (int mode) {
	if (mode == CSI_RESET_ASSERT) {
		// assert reset
		fpga->csi.Control &= ~FPGA_CSI_CTRL_VRSTN;
	} else {
		// release reset
		fpga->csi.Control |= FPGA_CSI_CTRL_VRSTN;
	}
}

//=============================================================================
//! CSI Soft Reset
//-----------------------------------------------------------------------------
//! @param mode		CSI_RESET_ASSERT: assert reset,
//!            		CSI_RESET_RELEASE: release reset
//-----------------------------------------------------------------------------
//! @brief This function puts CSI receiver module into reset state or puts it
//! out of reset according to "mode" parameter.
//=============================================================================
void Csi_SoftReset (int mode) {
	if (mode == CSI_RESET_ASSERT) {
		// assert reset
		csi_l->core_config |= CSI_CORE_CONFIG_RST;
		csi_r->core_config |= CSI_CORE_CONFIG_RST;
	} else {
		// release reset
		csi_l->core_config &= ~CSI_CORE_CONFIG_RST;
		csi_r->core_config &= ~CSI_CORE_CONFIG_RST;
	}
}

//=============================================================================
//! CSI D-PHY Reset
//-----------------------------------------------------------------------------
//! @param mode		CSI_RESET_ASSERT: assert reset,
//!            		CSI_RESET_RELEASE: release reset
//-----------------------------------------------------------------------------
//! @brief This function puts D-PHY module inside CSI receiver into reset
//! state or puts it out of reset according to "mode" parameter.
//=============================================================================
void Csi_DphyReset (int mode) {
	if (mode == CSI_RESET_ASSERT) {
		// assert reset
		dphy_l->ctrl |= DPHY_CTRL_SRST;
		dphy_r->ctrl |= DPHY_CTRL_SRST;
	} else {
		// release reset
		dphy_l->ctrl &= ~DPHY_CTRL_SRST;
		dphy_r->ctrl &= ~DPHY_CTRL_SRST;
	}
}

//=============================================================================
//! CSI Set Lane Mode
//-----------------------------------------------------------------------------
//! @param lane_mode	CSI_ONE_LANE: set to one lane mode,
//!            			CSI_TWO_LANE: set to two lane mode
//-----------------------------------------------------------------------------
//! @brief This function sets the number of active lanes of MIPI CSI interface.
//=============================================================================
void Csi_SetLaneMode (int lane_mode) {
	switch (lane_mode) {
	case CSI_ONE_LANE:
		csi_l->prot_config = 0;
		csi_r->prot_config = 0;
		break;
	case CSI_TWO_LANE:
		csi_l->prot_config = 1;
		csi_r->prot_config = 1;
		break;
	default:
		csi_l->prot_config = 0;
		csi_r->prot_config = 0;
		break;
	}

}

//=============================================================================
//! CSI Initialize
//-----------------------------------------------------------------------------
//! @brief This function initializes modules regarding to video input through
//! MIPI CSI interface.
//=============================================================================
void Csi_Init(int lane_mode) {
	// assert reset
	Csi_DphyReset(CSI_RESET_ASSERT);
	Csi_SoftReset(CSI_RESET_ASSERT);
	Csi_VideoReset(CSI_RESET_ASSERT);

	// video_aresetn should be asserted for more than
	// 40 dphy_clk_200M cycles (200ns)
	usleep(1);

	// release reset
	Csi_VideoReset(CSI_RESET_RELEASE);

	// set lane mode
	Csi_SetLaneMode(lane_mode);

	Csi_SoftReset(CSI_RESET_RELEASE);
	Csi_DphyReset(CSI_RESET_RELEASE);
}

