//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file lsm9ds1.h
//!
//! Header file for lsm9ds1.c
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#ifndef LSM9DS1_H
#define LSM9DS1_H

#include "i2c.h"


//=============================================================================
// Define
//=============================================================================
// I2C Device Address (7-bit)
#define I2C_ADDR_9DOF_XLG	0x6A // 9DOF Accelerometer and Gyroscope
#define I2C_ADDR_9DOF_MAG	0x1C // 9DOF Magnetometer


//=============================================================================
// LSM9DS1 Register Map
//=============================================================================
// Accelerometer and Gyroscope
#define XLG_ADDR_ACT_THS			0x04
#define XLG_ADDR_ACT_DUR			0x05
#define XLG_ADDR_INT_GEN_CFG_XL		0x06
#define XLG_ADDR_INT_GEN_THS_X_XL	0x07
#define XLG_ADDR_INT_GEN_THS_Y_XL	0x08
#define XLG_ADDR_INT_GEN_THS_Z_XL	0x09
#define XLG_ADDR_INT_GEN_DUR_XL		0x0A
#define XLG_ADDR_REFERENCE_G		0x0B
#define XLG_ADDR_INT1_CTRL			0x0C
#define XLG_ADDR_INT2_CTRL			0x0D
#define XLG_ADDR_WHO_AM_I			0x0F
#define XLG_ADDR_CTRL_REG1_G		0x10
#define XLG_ADDR_CTRL_REG2_G		0x11
#define XLG_ADDR_CTRL_REG3_G		0x12
#define XLG_ADDR_ORIENT_CFG_G		0x13
#define XLG_ADDR_INT_GEN_SRC_G		0x14
#define XLG_ADDR_OUT_TEMP_L			0x15
#define XLG_ADDR_OUT_TEMP_H			0x16
#define XLG_ADDR_STATUS_REG			0x17
#define XLG_ADDR_OUT_X_L_G			0x18
#define XLG_ADDR_OUT_X_H_G			0x19
#define XLG_ADDR_OUT_Y_L_G			0x1A
#define XLG_ADDR_OUT_Y_H_G			0x1B
#define XLG_ADDR_OUT_Z_L_G			0x1C
#define XLG_ADDR_OUT_Z_H_G			0x1D
#define XLG_ADDR_CTRL_REG4			0x1E
#define XLG_ADDR_CTRL_REG5_XL		0x1F
#define XLG_ADDR_CTRL_REG6_XL		0x20
#define XLG_ADDR_CTRL_REG7_XL		0x21
#define XLG_ADDR_CTRL_REG8			0x22
#define XLG_ADDR_CTRL_REG9			0x23
#define XLG_ADDR_CTRL_REG10			0x24
#define XLG_ADDR_INT_GEN_SRC_XL		0x26
#define XLG_ADDR_STATUS_REG2		0x27
#define XLG_ADDR_OUT_X_L_XL			0x28
#define XLG_ADDR_OUT_X_H_XL			0x29
#define XLG_ADDR_OUT_Y_L_XL			0x2A
#define XLG_ADDR_OUT_Y_H_XL			0x2B
#define XLG_ADDR_OUT_Z_L_XL			0x2C
#define XLG_ADDR_OUT_Z_H_XL			0x2D
#define XLG_ADDR_FIFO_CTRL			0x2E
#define XLG_ADDR_FIFO_SRC			0x2F
#define XLG_ADDR_INT_GEN_CFG_G		0x30
#define XLG_ADDR_INT_GEN_THS_XH_G	0x31
#define XLG_ADDR_INT_GEN_THS_XL_G	0x32
#define XLG_ADDR_INT_GEN_THS_YH_G	0x33
#define XLG_ADDR_INT_GEN_THS_YL_G	0x34
#define XLG_ADDR_INT_GEN_THS_ZH_G	0x35
#define XLG_ADDR_INT_GEN_THS_ZL_G	0x36
#define XLG_ADDR_INT_GEN_DUR_G		0x37

// Magnetometer
#define MAG_ADDR_OFFSET_X_REG_L_M	0x05
#define MAG_ADDR_OFFSET_X_REG_H_M	0x06
#define MAG_ADDR_OFFSET_Y_REG_L_M	0x07
#define MAG_ADDR_OFFSET_Y_REG_H_M	0x08
#define MAG_ADDR_OFFSET_Z_REG_L_M	0x09
#define MAG_ADDR_OFFSET_Z_REG_H_M	0x0A
#define MAG_ADDR_WHO_AM_I_M			0x0F
#define MAG_ADDR_CTRL_REG1_M		0x20
#define MAG_ADDR_CTRL_REG2_M		0x21
#define MAG_ADDR_CTRL_REG3_M		0x22
#define MAG_ADDR_CTRL_REG4_M		0x23
#define MAG_ADDR_CTRL_REG5_M		0x24
#define MAG_ADDR_STATUS_REG_M		0x27
#define MAG_ADDR_OUT_X_L_M			0x28
#define MAG_ADDR_OUT_X_H_M			0x29
#define MAG_ADDR_OUT_Y_L_M			0x2A
#define MAG_ADDR_OUT_Y_H_M			0x2B
#define MAG_ADDR_OUT_Z_L_M			0x2C
#define MAG_ADDR_OUT_Z_H_M			0x2D
#define MAG_ADDR_INT_CFG_M			0x30
#define MAG_ADDR_INT_SRC_M			0x31
#define MAG_ADDR_INT_THS_L_M		0x32
#define MAG_ADDR_INT_THS_H_M		0x33

#define XLG_WHO_AM_I	0x68
#define MAG_WHO_AM_I	0x3D

//=============================================================================
// Function Prototypes
//=============================================================================
void Lsm9ds1_ReadMagValues (void);
unsigned int Lsm9ds1_MagRead (unsigned char reg_addr, unsigned char *reg_value);
unsigned int Lsm9ds1_MagWrite (unsigned char reg_addr, unsigned char reg_value);
unsigned int Lsm9ds1_XlgRead (unsigned char reg_addr, unsigned char *reg_value);
unsigned int Lsm9ds1_XlgWrite(unsigned char reg_addr, unsigned char reg_value);


#endif // LSM9DS1_H
