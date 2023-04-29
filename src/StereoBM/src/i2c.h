//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file i2c.h
//!
//! Header file for i2c.c
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#ifndef I2C_H
#define I2C_H

#include "xparameters_ps.h"
#include "xparameters.h"
#include "xil_printf.h"

#include "main.h"


//=============================================================================
// Define
//=============================================================================
// I2C Device Address (7-bit)
#define I2C_ADDR_TCA9548A	0x75 // I2C Expander

// Target SCL Frequency [Hz]
#define I2C_TARGET_SCL	100000

#define I2C_FIFO_DEPTH	16


//=============================================================================
// TCA9548A Channel Selection Bits
//-----------------------------------------------------------------------------
//        [Ultra96-V2] : [U96-SVM]
// CH[7]: USB5744      : N.C
// CH[6]: N.C          : N.C
// CH[5]: TP[32/33]    : N.C
// CH[4]: PMIC         : N.C
// CH[3]: HSEXP I2C3   : OV5640(R)
// CH[2]: HSEXP I2C2   : OV5640(L)
// CH[1]: LSEXP I2C1   : MikroBUS1
// CH[0]: LSEXP I2C0   : MikroBUS0
//-----------------------------------------------------------------------------
// Multiple bits can be set simultaneously.
//=============================================================================
#define I2C_SEL_CH7	0x80
#define I2C_SEL_CH6	0x40
#define I2C_SEL_CH5	0x20
#define I2C_SEL_CH4	0x10
#define I2C_SEL_CH3	0x08
#define I2C_SEL_CH2	0x04
#define I2C_SEL_CH1	0x02
#define I2C_SEL_CH0	0x01


//=============================================================================
// I2C Module Register Map
//-----------------------------------------------------------------------------
// Defined in [Zynq Ultrascale+ Devices Register Reference]
// https://www.xilinx.com/html_docs/registers/ug1087/ug1087-zynq-ultrascale-registers.html
//=============================================================================
struct I2C_REG {
	volatile unsigned int ctrl;
	volatile unsigned int sts;
	volatile unsigned int addr;
	volatile unsigned int data;
	volatile unsigned int intr_sts;
	volatile unsigned int size;
	volatile unsigned int pause;
	volatile unsigned int time_out;
	volatile unsigned int intr_mask;
	volatile unsigned int intr_enb;
	volatile unsigned int intr_dis;
	volatile unsigned int gf;
};


//=============================================================================
// Bit Field Definition
//=============================================================================
// CTRL Register
// [15:14]: divisor_a
// [13: 8]: divisor_b
// [    7]: Reserved
// [    6]: CLR_FIFO
// [    5]: SLVMON
// [    4]: HOLD
// [    3]: ACK_EN
// [    2]: NEA
// [    1]: MS
// [    0]: RW
#define I2C_CTRL_DIV_A_OFFSET	14
#define I2C_CTRL_DIV_B_OFFSET	8
#define I2C_CTRL_CLR_FIFO	0x0040
#define I2C_CTRL_SLVMON		0x0020
#define I2C_CTRL_HOLD		0x0010
#define I2C_CTRL_ACK_EN		0x0008
#define I2C_CTRL_NEA		0x0004
#define I2C_CTRL_MS			0x0002
#define I2C_CTRL_RW			0x0001

// Status Register
// [15: 9]: Reserved
// [    8]: Bus Active
// [    7]: Receiver Overflow
// [    6]: Transmit Data Valid
// [    5]: Receiver Data Valid
// [    4]: Reserved
// [    3]: RX read_write
// [ 2: 0]: Reserved
#define I2C_STS_BA			0x0100
#define I2C_STS_RXOVF		0x0080
#define I2C_STS_TXDV		0x0040
#define I2C_STS_RXDV		0x0020
#define I2C_STS_RXRW		0x0004

// Interrupt Status Register
// [15:10]: Reserved
// [    9]: Arbitration Lost
// [    8]: Reserved
// [    7]: FIFO Receive Underflow
// [    6]: FIFO Transmit Overflow
// [    5]: Receive Overflow
// [    4]: Monitored Slave Ready
// [    3]: Transfer Time Out
// [    2]: Transfer not Acknowledged
// [    1]: More Data
// [    0]: Transfer Complete
#define I2C_INTR_STS_ARB_LOST	0x0200
#define I2C_INTR_STS_RX_UNF		0x0080
#define I2C_INTR_STS_TX_OVF		0x0040
#define I2C_INTR_STS_RX_OVF		0x0020
#define I2C_INTR_STS_SLV_RDY	0x0010
#define I2C_INTR_STS_TO			0x0008
#define I2C_INTR_STS_NACK		0x0004
#define I2C_INTR_STS_DATA		0x0002
#define I2C_INTR_STS_COMP		0x0001


//=============================================================================
// Function Prototypes
//=============================================================================
void I2c_ReadCh (unsigned char *ch);
void I2c_SetCh (unsigned char ch);

void I2c_ClearFifo (void);
unsigned int I2c_Read (unsigned char addr, unsigned char *data, int length);
unsigned int I2c_Write (unsigned char addr, unsigned char *data, int length);
int I2c_IsTransferComplete (unsigned int sts);
void I2c_DiagnoseError (unsigned int sts);
unsigned int I2c_WaitForComp (void);
void I2c_SetMasterTransmitter (void);
void I2c_SetMasterReceiver (void);
int I2c_SetSclFreq (int freq_scl, int *div_a, int *div_b);
int I2c_Init (void);
void I2c_ShowStatus (void);
int I2c_GetFifoLength (void);

#endif // I2C_H
