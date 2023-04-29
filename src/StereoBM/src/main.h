//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file main.h
//!
//! Header file for main.c
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#ifndef MAIN_H
#define MAIN_H

#include "xil_printf.h"
#include "sleep.h"
#include "xil_cache.h"

#include "uart.h"
#include "i2c.h"
#include "ov5640.h"
#include "csi.h"
#include "fpga.h"
#include "xusb_main.h"
#include "lsm9ds1.h"

//! Command structure
struct HWTP_CMD {
	char parm[10][16];
	int num;
};

//! @name Buffer Address for Bank A/B/C
//! @{
#define IMG_BUF_A			0x40000000
#define IMG_BUF_B			0x41000000
#define IMG_BUF_C			0x42000000
//! @}


//=============================================================================
// Function Prototypes
//=============================================================================
void PL_IntrHandler (void *CallBackRef);
s32 SetupInterruptSystem (u16 IntcDeviceID, void *IntcPtr);
s32 PL_SetupInterrupt (
	XScuGic_Config *IntcConfig,
	void *IntcPtr,
	struct Usb_DevData *UsbInstance
);
void AssertCallBack (const char8 *File, s32 Line);
int main (void);

// Application level functions
void App_UsbGrabber (struct REMOTE_SETTING *remoteSetting);
void App_9DofHwTest (void);

// Command related functions
void Prompt (void);
void CommandProcess (void);
void CommandReceive (char* buf, int len);
void CommandAnalyze (struct HWTP_CMD *cmd);
void CommandExecute (struct HWTP_CMD cmd);

// Helper functions
void hex_to_num (char* hex, unsigned long* num);
void dec_to_num (char* dec, unsigned long* num);
void set_bit (unsigned int *reg, int bit_mask);
void clear_bit (unsigned int *reg, int bit_mask);
int get_bit (unsigned int reg, int bit_mask);


#endif
