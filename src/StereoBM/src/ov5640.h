//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file ov5640.h
//!
//! Header file for ov5640.c
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#ifndef OV5640_H
#define OV5640_H

#include "i2c.h"
#include "Parameters.h"
#include "xusb_ch9_video.h"

//=============================================================================
// Define
//=============================================================================
// I2C Device Address (7-bit)
#define OV5640_I2C_ADDR		0x3C

#define OV5640_ADDR_SYSTEM_CTROL0		0x3008

// SYSTEM_CTROL0 Register
// [  7]: Software reset
// [  6]: Software power down
// [5:0]: Debug mode
#define OV5640_SYSTEM_CTROL0_SOFTWARE_RESET		0x80
#define OV5640_SYSTEM_CTROL0_SOFTWARE_POWERDOWN	0x40

#define DPHY_DLSTS_PKT_CNT		0xFFFF0000
#define DPHY_DLSTS_STOP_STATE	0x00000040

enum OV5640_RESET_MODE	{OV5640_RESET_ASSERT, OV5640_RESET_RELEASE};
enum OV5640_POWER_MODE	{OV5640_POWER_DOWN, OV5640_POWER_UP};
enum OV5640_RESOLUTION {
	OV5640_RESOLUTION_1920_1080,
	OV5640_RESOLUTION_1280_720,
	OV5640_RESOLUTION_960_540,
	OV5640_RESOLUTION_640_480,
	OV5640_RESOLUTION_640_360,
	OV5640_RESOLUTION_320_240,
	OV5640_RESOLUTION_320_200
};
enum OV5640_TEST_MODE {
	OV5640_TEST_NORMAL,
	OV5640_TEST_COLORBAR
};


//=============================================================================
// Function Prototypes
//=============================================================================
void Ov5640_SoftwareReset (int mode);
void Ov5640_PowerMode (int mode);
void Ov5640_I2cRead (unsigned int reg_addr, unsigned char *reg_value);
void Ov5640_I2cWrite (unsigned int reg_addr, unsigned char reg_value);
void Ov5640_I2cBurstWrite (unsigned int reg_addr, unsigned char* reg_value, int total_len);
void Ov5640_RegisterInit (int test_mode);
void Ov5640_Format (int resolution, double fps);
int Ov5640_Init(int test_mode, int resolution, double fps);
void Sccb_Write (unsigned int data);
void Sccb_Read(unsigned long WrData, unsigned char *RdData);
void AutoFocusOn(unsigned long cam);
void AutoFocusOff(unsigned long cam);
int Ov5640_Init2();
void Ov5640_PowerUp(void);
void Ov5640_PowerUp2(void);
void Ov5640_DeviceSelect(void);
void Ov5640_YUY2_640_480_60FPS(void);
void Ov5640_YUY2_640_480_60FPS2(void);
void Ov5640_YUY2_640_480_60FPS3(void);

void Ov5640_SetExposureAuto();
void Ov5640_SetExposureManual(int level);
void Ov5640_SetBrightness(int br);
void Ov5640_BrightnessControl(
	struct UVC_APP_DATA *app_data,
	struct REMOTE_SETTING *remoteSetting,
	int wdt, int hgt, int *av, int *br);

#endif // OV5640_H
