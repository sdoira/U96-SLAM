//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file i2c.c
//!
//! C source for PS I2C module.
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#include "i2c.h"

//! I2C register space
struct I2C_REG *i2c = (struct I2C_REG *)XPAR_PSU_I2C_1_BASEADDR;


//=============================================================================
//! I2C Read Channel
//-----------------------------------------------------------------------------
//! @param	*ch	Read-out channel index
//-----------------------------------------------------------------------------
//! @brief This function reads out the channel index that is currently set
//! inside TCS9548A.
//=============================================================================
void I2c_ReadCh (unsigned char *ch) {
	I2c_Read(I2C_ADDR_TCA9548A, ch, 1);
}

//=============================================================================
//! I2C Set Channel
//-----------------------------------------------------------------------------
//! @param	ch	Channel index to be written
//-----------------------------------------------------------------------------
//! @brief This function writes the specified channel index to TCS9548A.
//=============================================================================
void I2c_SetCh (unsigned char ch) {
	I2c_Write(I2C_ADDR_TCA9548A, &ch, 1);
}

//=============================================================================
//! I2C Clear FIFO
//-----------------------------------------------------------------------------
//! @brief This function clears FIFO inside I2C module.
//=============================================================================
void I2c_ClearFifo (void) {
	i2c->ctrl |= I2C_CTRL_CLR_FIFO;
	while((i2c->ctrl & I2C_CTRL_CLR_FIFO) == I2C_CTRL_CLR_FIFO) {}
}

//=============================================================================
//! I2C Read
//-----------------------------------------------------------------------------
//! @param	addr	I2C device address
//! @param	*data	Read-out data
//! @param	length	Length of the data to be read in bytes
//-----------------------------------------------------------------------------
//! @return	Values of interrupt status register
//-----------------------------------------------------------------------------
//! @brief This function reads the specified number of bytes of data from
//! the specified I2C device address.
//=============================================================================
unsigned int I2c_Read (unsigned char addr, unsigned char *data, int length) {
	unsigned int sts;

	// set to Master-Receiver mode
	I2c_SetMasterReceiver();

	// # of bytes to read
	i2c->size = length;

	// set device address to start the operation
	i2c->addr = addr;

	// wait for transfer complete
	sts = I2c_WaitForComp();

	// read data
	if (I2c_IsTransferComplete(sts)) {
		// transfer complete
		for (int i = 0; i < length; i++) {
			data[i] = i2c->data;
		}
	} else {
		// communication error
		I2c_ClearFifo();
	}

	return sts;
}

//=============================================================================
//! I2C Write
//-----------------------------------------------------------------------------
//! @param	addr	I2C device address
//! @param	*data	Pointer to data to be written
//! @param	length	Length of the data to be written in bytes
//! @return	Values of interrupt status register
//-----------------------------------------------------------------------------
//! @brief This function writes the specified number of bytes of data to
//! the specified I2C device address.
//=============================================================================
unsigned int I2c_Write (unsigned char addr, unsigned char *data, int length) {
	unsigned int sts;

	// set to Master-Transmitter mode
	I2c_SetMasterTransmitter();

	// write data to transmit
	for (int i = 0; i < length; i++) {
		i2c->data = data[i];
	}

	// set device address to start the operation
	i2c->addr = addr;

	// wait for transfer complete
	sts = I2c_WaitForComp();
	if (I2c_IsTransferComplete(sts)) {
		// transfer complete
	} else {
		// communication error
		I2c_ClearFifo();
	}

	return sts;
}

//=============================================================================
//! This function checks if the previous transfer is completed.
//-----------------------------------------------------------------------------
//! @return	"1" if completed else "0".
//=============================================================================
int I2c_IsTransferComplete (unsigned int sts) {
	if ((sts & I2C_INTR_STS_COMP) == I2C_INTR_STS_COMP) {
		return 1;
	} else {
		return 0;
	}
}

//=============================================================================
//! This function diagnoses the given status register and displays errors
//! if there are any.
//-----------------------------------------------------------------------------
//! @param	sts		Value of interrupt status register
//=============================================================================
void I2c_DiagnoseError (unsigned int sts) {
	if ((sts & 0x02EC) != 0) {
		// I2C communication error
		if ((sts & I2C_INTR_STS_ARB_LOST) == I2C_INTR_STS_ARB_LOST) {
			xil_printf("Arbitration lost\r\n");
		}
		if ((sts & I2C_INTR_STS_RX_UNF) == I2C_INTR_STS_RX_UNF) {
			xil_printf("FIFO receive underflow\r\n");
		}
		if ((sts & I2C_INTR_STS_TX_OVF) == I2C_INTR_STS_TX_OVF) {
			xil_printf("FIFO transmit overflow\r\n");
		}
		if ((sts & I2C_INTR_STS_RX_OVF) == I2C_INTR_STS_RX_OVF) {
			xil_printf("Receive overflow\r\n");
		}
		if ((sts & I2C_INTR_STS_TO) == I2C_INTR_STS_TO) {
			xil_printf("Transfer time out\r\n");
		}
		if ((sts & I2C_INTR_STS_NACK) == I2C_INTR_STS_NACK) {
			xil_printf("Transfer not acknowledged\r\n");
		}
	}
	else {
		xil_printf("No error\r\n");
	}
}

//=============================================================================
//! This function waits for the completion of the transfer.
//-----------------------------------------------------------------------------
//! @param	sts		Value of interrupt status register
//-----------------------------------------------------------------------------
//! @note This function returns when the transfer completed or any errors
//! happened. Caller of this function should check the returned status
//! register.
//=============================================================================
unsigned int I2c_WaitForComp (void) {
	unsigned int sts;

	while(1) {
		sts = i2c->intr_sts;
		if ((sts & 0x02ED) != 0) {
			i2c->intr_sts = sts; // write 1 to clear the bits
			return sts;
		}
	}
}

//=============================================================================
//! I2C Set Master-Transmitter Mode
//-----------------------------------------------------------------------------
//! @brief This function sets I2C module to the master-transmitter mode.
//=============================================================================
void I2c_SetMasterTransmitter (void) {
	i2c->ctrl &= ~I2C_CTRL_RW;
}

//=============================================================================
//! I2C Set Master-Receiver Mode
//-----------------------------------------------------------------------------
//! @brief This function sets I2C module to the master-receiver mode.
//=============================================================================
void I2c_SetMasterReceiver (void) {
	i2c->ctrl |= I2C_CTRL_RW;
}

//=============================================================================
//! I2C Set SCL Frequency
//-----------------------------------------------------------------------------
//! @param freq_scl	Desired SCL frequency in Hz.
//! @param *div_a	Calculated value of divisor A.
//! @param *div_b	Calculated value of divisor B.
//-----------------------------------------------------------------------------
//! @return Achievable actual SCL frequency in Hz.
//-----------------------------------------------------------------------------
//! @brief This function calculates values of divisors to achieve the specified
//! SCL frequency.
//-----------------------------------------------------------------------------
//! @note
//! Formula for SCL divisor
//! <pre>
//! fSCL = fCPU / (22 * (divisor_a + 1) * (divisor_b + 1)
//! divisor_a = [0..3]
//! divisor_b = [0..63]
//! </pre>
//=============================================================================
int I2c_SetSclFreq (int freq_scl, int *div_a, int *div_b) {
	double C = XPAR_PSU_I2C_1_I2C_CLK_FREQ_HZ / (freq_scl * 22);

	int tmp_a = (int)(C / 64.0);
	*div_a = (tmp_a > 3) ? 3 : tmp_a;

	// div_b is added by 1 so that the calculated frequency will not exceed
	// the target value.
	int tmp_b = (int)(C - *div_a) + 1;
	*div_b = (tmp_b > 63) ? 63 : tmp_b;

	int actual_scl = (int)(XPAR_PSU_I2C_1_I2C_CLK_FREQ_HZ / (22 * (*div_a + 1) * (*div_b + 1)));

	return actual_scl;
}

//=============================================================================
//! I2C Initialize
//-----------------------------------------------------------------------------
//! @brief Initializes I2C module.
//=============================================================================
int I2c_Init (void) {
	// set divisor
	int div_a, div_b;
	int actual_scl = I2c_SetSclFreq(I2C_TARGET_SCL, &div_a, &div_b);

	// set CTRL register
	i2c->ctrl = (
		(div_a << I2C_CTRL_DIV_A_OFFSET) +
		(div_b << I2C_CTRL_DIV_B_OFFSET) +
		I2C_CTRL_CLR_FIFO +
		I2C_CTRL_ACK_EN +
		I2C_CTRL_NEA +
		I2C_CTRL_MS
	);

	return actual_scl;
}

//=============================================================================
//! I2C Show Status
//-----------------------------------------------------------------------------
//! @brief Dumps all status relevant to I2C transfer.
//=============================================================================
void I2c_ShowStatus (void) {
	unsigned int st = i2c->sts;
	xil_printf("Status_Reg: %08X\r\n", st);
	xil_printf("Bus Active         : %1d\r\n", get_bit(st, I2C_STS_BA));
	xil_printf("Receiver Overflow  : %1d\r\n", get_bit(st, I2C_STS_RXOVF));
	xil_printf("Transmit Data Valid: %1d\r\n", get_bit(st, I2C_STS_TXDV));
	xil_printf("Receiver Data Valid: %1d\r\n", get_bit(st, I2C_STS_RXDV));
	xil_printf("RX read_write      : %1d\r\n", get_bit(st, I2C_STS_RXRW));
	xil_printf("\r\n");

	st = i2c->intr_sts;
	xil_printf("Interrupt_Status: %08X\r\n", st);
	xil_printf("Arbitration Lost         : %1d\r\n", get_bit(st, I2C_INTR_STS_ARB_LOST));
	xil_printf("FIFO receive underflow   : %1d\r\n", get_bit(st, I2C_INTR_STS_RX_UNF));
	xil_printf("FIFO transmit underflow  : %1d\r\n", get_bit(st, I2C_INTR_STS_TX_OVF));
	xil_printf("FIFO receive overflow    : %1d\r\n", get_bit(st, I2C_INTR_STS_RX_OVF));
	xil_printf("Monitored slave ready    : %1d\r\n", get_bit(st, I2C_INTR_STS_SLV_RDY));
	xil_printf("Transfer time out        : %1d\r\n", get_bit(st, I2C_INTR_STS_TO));
	xil_printf("Transfer not acknowledged: %1d\r\n", get_bit(st, I2C_INTR_STS_NACK));
	xil_printf("More data                : %1d\r\n", get_bit(st, I2C_INTR_STS_DATA));
	xil_printf("Transfer complete        : %1d\r\n", get_bit(st, I2C_INTR_STS_COMP));
}

// return available FIFO space
int I2c_GetFifoLength (void) {
	return (I2C_FIFO_DEPTH - i2c->size);
}
