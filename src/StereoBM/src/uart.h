//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file uart.h
//!
//! Header file for uart.c
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#ifndef UART_H
#define UART_H

#include "xparameters.h"
#include "xil_printf.h"
#include "xuartps_hw.h"
#include "xil_types.h"


//=============================================================================
// Define
//=============================================================================
#define UART_BASEADDR		XPAR_XUARTPS_1_BASEADDR
#define UART_CLOCK_HZ		XPAR_XUARTPS_1_CLOCK_HZ


//=============================================================================
// Function Prototypes
//=============================================================================
unsigned char uart_getc(void);
int uart_getc2(unsigned char* data);
void uart_init(void);

#endif // UART_H
