//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file lsm9ds1.c
//!
//! C source for MikroElektronika 9DOF Click (LSM9DS1)
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#include "lsm9ds1.h"


//=============================================================================
//! LSM9DS1 Read Magnetometer X/Y/Z Values
//-----------------------------------------------------------------------------
//! @brief This function reads out and dumps magnetometer X/Y/Z-axis data.
//=============================================================================
void Lsm9ds1_ReadMagValues (void) {
	unsigned char reg_value_h, reg_value_l;
	Lsm9ds1_MagRead(MAG_ADDR_OUT_X_L_M, &reg_value_l);
	Lsm9ds1_MagRead(MAG_ADDR_OUT_X_H_M, &reg_value_h);
	xil_printf("X:%02X%02X ", reg_value_h, reg_value_l);
	Lsm9ds1_MagRead(MAG_ADDR_OUT_Y_L_M, &reg_value_l);
	Lsm9ds1_MagRead(MAG_ADDR_OUT_Y_H_M, &reg_value_h);
	xil_printf("Y:%02X%02X ", reg_value_h, reg_value_l);
	Lsm9ds1_MagRead(MAG_ADDR_OUT_Z_L_M, &reg_value_l);
	Lsm9ds1_MagRead(MAG_ADDR_OUT_Z_H_M, &reg_value_h);
	xil_printf("Z:%02X%02X\r\n", reg_value_h, reg_value_l);
}

//=============================================================================
//! LSM9DS1 Read Magnetometer
//-----------------------------------------------------------------------------
//! @param reg_addr		Register address
//! @param *reg_value	Read-out data is stored here
//-----------------------------------------------------------------------------
//! @return	Value of I2C interrupt status register
//-----------------------------------------------------------------------------
//! @brief This function reads out 1 byte from the specified register address
//! of magnetometer.
//=============================================================================
unsigned int Lsm9ds1_MagRead (unsigned char reg_addr, unsigned char *reg_value) {
	unsigned int sts;

	// register address write
	sts = I2c_Write(I2C_ADDR_9DOF_MAG, &reg_addr, 1);
	if ((sts & I2C_INTR_STS_COMP) != I2C_INTR_STS_COMP) {
		// COMP bit not set
		return sts;
	}

	// register read
	sts = I2c_Read(I2C_ADDR_9DOF_MAG, reg_value, 1);

	return sts;
}

//=============================================================================
//! LSM9DS1 Write Magnetometer
//-----------------------------------------------------------------------------
//! @param reg_addr		Register address
//! @param reg_value	Data to be written
//-----------------------------------------------------------------------------
//! @return	Value of I2C interrupt status register
//-----------------------------------------------------------------------------
//! @brief This function writes 1 byte of data to the specified register
//! address of magnetometer.
//=============================================================================
unsigned int Lsm9ds1_MagWrite (unsigned char reg_addr, unsigned char reg_value) {
	unsigned int sts;
	unsigned char tmp[2];
	tmp[0] = reg_addr;
	tmp[1] = reg_value;

	sts = I2c_Write(I2C_ADDR_9DOF_MAG, tmp, 2);

	return sts;
}

//=============================================================================
//! LSM9DS1 Read Accelerometer/Gyroscope
//-----------------------------------------------------------------------------
//! @param reg_addr		Register address
//! @param *reg_value	Read-out data is stored here
//-----------------------------------------------------------------------------
//! @return	Value of I2C interrupt status register
//-----------------------------------------------------------------------------
//! @brief This function reads out 1 byte from the specified register address
//! of accelerometer and gyroscope.
//=============================================================================
unsigned int Lsm9ds1_XlgRead (unsigned char reg_addr, unsigned char *reg_value) {
	unsigned int sts;

	// register address write
	sts = I2c_Write(I2C_ADDR_9DOF_XLG, &reg_addr, 1);
	if ((sts & I2C_INTR_STS_COMP) != I2C_INTR_STS_COMP) {
		// COMP bit not set
		return sts;
	}

	// register read
	sts = I2c_Read(I2C_ADDR_9DOF_XLG, reg_value, 1);

	return sts;
}

//=============================================================================
//! LSM9DS1 Write Accelerometer/Gyroscope
//-----------------------------------------------------------------------------
//! @param reg_addr		Register address
//! @param reg_value	Data to be written
//-----------------------------------------------------------------------------
//! @return	Value of I2C interrupt status register
//-----------------------------------------------------------------------------
//! @brief This function writes 1 byte of data to the specified register
//! address of accelerometer and gyroscope.
//=============================================================================
unsigned int Lsm9ds1_XlgWrite(unsigned char reg_addr, unsigned char reg_value) {
	unsigned int sts;
	unsigned char tmp[2];
	tmp[0] = reg_addr;
	tmp[1] = reg_value;

	sts = I2c_Write(I2C_ADDR_9DOF_XLG, tmp, 2);

	return sts;
}
