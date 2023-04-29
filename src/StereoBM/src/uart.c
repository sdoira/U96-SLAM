//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file uart.c
//!
//! C source for PS UART module.
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#include "uart.h"

//=============================================================================
//! UART Get Char
//-----------------------------------------------------------------------------
//! @return One byte of received data
//-----------------------------------------------------------------------------
//! @brief This function waits until there is any data received then returns
//! 1 byte of receive data.
//=============================================================================
unsigned char uart_getc(void){
	// Wait until there is data
	while (!XUartPs_IsReceiveData(UART_BASEADDR));

	unsigned char RecvChar = XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET);

	return RecvChar;
}

//=============================================================================
//! UART Get Char2
//-----------------------------------------------------------------------------
//! @param *data	Recieve data is stored here
//-----------------------------------------------------------------------------
//! @return "1" if data is received, else "-1"
//-----------------------------------------------------------------------------
//! @brief Non-blocking variant of uart_getc function. This function returns
//! immediately regardless of there is any data or not.
//=============================================================================
// non-blocking type
int uart_getc2(unsigned char* data){
	if (XUartPs_IsReceiveData(UART_BASEADDR)) {
		*data = XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET);
		return 1;
	} else {
		return -1;
	}
}

//=============================================================================
//! UART Initialize
//-----------------------------------------------------------------------------
//! @brief This function initializes UART module
//=============================================================================
void uart_init(void) {
	unsigned int CntrlRegister = XUartPs_ReadReg(UART_BASEADDR, XUARTPS_CR_OFFSET);

	/// Enable TX and RX for the device
	XUartPs_WriteReg(
		UART_BASEADDR,
		XUARTPS_CR_OFFSET,
		((CntrlRegister & ~XUARTPS_CR_EN_DIS_MASK) | XUARTPS_CR_TX_EN | XUARTPS_CR_RX_EN)
	);
}

