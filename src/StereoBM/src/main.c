//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file main.c
//!
//! Main C source file for usb_grabber project.
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#include "main.h"

extern volatile struct FPGA_REG *fpga;
extern struct XUSB_TX_INFO tx_info;
extern struct Usb_DevData UsbInstance;

extern struct I2C_REG *i2c;

struct REMOTE_SETTING remoteSetting;

//=============================================================================
//! PL Interrupt Handler
//-----------------------------------------------------------------------------
//! @param *CallBackRef	Pointer to UVC_APP_DATA
//-----------------------------------------------------------------------------
//! @brief Interrupt handler for PL interrupt. This handler will be called
//! every time a frame of image data is DMA-transferred.
//=============================================================================
void PL_IntrHandler(void *CallBackRef)
{
	static int do_once = 0;
	if (do_once == 0) {
		xil_printf("PL Intr\r\n");
		do_once = 1;
	}

	struct UVC_APP_DATA *app_data = Uvc_GetAppData(CallBackRef);
	int received = 0;
	if ((fpga->com.InterruptStatus & FPGA_INTR_GRB) == FPGA_INTR_GRB) {
		fpga->com.InterruptStatus = FPGA_INTR_GRB;
		app_data->grb_received += 1;
		received = 1;
	}
	if ((fpga->com.InterruptStatus & FPGA_INTR_RECT) == FPGA_INTR_RECT) {
		fpga->com.InterruptStatus = FPGA_INTR_RECT;
		app_data->rect_received += 1;
		received = 1;
	}
	if ((fpga->com.InterruptStatus & FPGA_INTR_XSBL) == FPGA_INTR_XSBL) {
		fpga->com.InterruptStatus = FPGA_INTR_XSBL;
		app_data->xsbl_received += 1;
		received = 1;
	}
	if ((fpga->com.InterruptStatus & FPGA_INTR_BM) == FPGA_INTR_BM) {
		fpga->com.InterruptStatus = FPGA_INTR_BM;
		app_data->bm_received += 1;
		received = 1;
	}
	if ((fpga->com.InterruptStatus & FPGA_INTR_GFTT) == FPGA_INTR_GFTT) {
		fpga->com.InterruptStatus = FPGA_INTR_GFTT;
		app_data->gftt_received += 1;
		received = 1;
	}

	if (received) {
		app_data->intr_received++;
	}
}

//=============================================================================
//! Setup Interrupt System
//-----------------------------------------------------------------------------
//! @param IntcDeviceID	Device ID of SCU GIC
//! @param *IntcPtr		Pointer to the instance of the interrupt controller
//-----------------------------------------------------------------------------
//! @return XST_SUCCESS when succeed else XST_FAILURE
//-----------------------------------------------------------------------------
//! @brief This function sets up the interrupt system for this application.
//! There are two interrupt sources in this design, USB and PL.
//=============================================================================
s32 SetupInterruptSystem (
	u16 IntcDeviceID,
	void *IntcPtr
){
	s32 Status;

	//-----------------------------------------------------------
	// Initialize GIC
	//-----------------------------------------------------------
	XScuGic_Config *IntcConfig;

	XScuGic *IntcInstancePtr = (XScuGic*)IntcPtr;

	// Initialize the interrupt controller driver
	IntcConfig = XScuGic_LookupConfig(IntcDeviceID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(
		IntcInstancePtr,
		IntcConfig,
		IntcConfig->CpuBaseAddress
	);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//-----------------------------------------------------------
	// Setup Individual Interrupt Source
	//-----------------------------------------------------------
	// USB
	Status = Xusb_SetupInterrupt (IntcPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// PL
	Status = PL_SetupInterrupt (IntcConfig, IntcPtr, &UsbInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//-----------------------------------------------------------
	// Enable Interrupt for ARM
	//-----------------------------------------------------------
	// Register interrupt handlers to ARM core.
	Xil_ExceptionRegisterHandler(
		XIL_EXCEPTION_ID_INT,
		(Xil_ExceptionHandler)XScuGic_InterruptHandler,
		IntcInstancePtr
	);

	// Enable interrupts in ARM
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

//=============================================================================
//! PL Setup Interrupt
//-----------------------------------------------------------------------------
//! @param *IntcConfig	Pointer to the configuration information of the
//! XScuGic driver.
//! @param *IntcPtr		Pointer to the instance of the interrupt controller
//! @param *UsbInstance Pointer to the Usb_DevData
//-----------------------------------------------------------------------------
//! @return XST_SUCCESS when succeed else XST_FAILURE
//-----------------------------------------------------------------------------
//! @brief This function sets up the PL interrupt.
//=============================================================================
s32 PL_SetupInterrupt (
	XScuGic_Config *IntcConfig,
	void *IntcPtr,
	struct Usb_DevData *UsbInstance
) {
	s32 Status;

	// Connect to the interrupt controller
	Status = XScuGic_Connect(
		(XScuGic*)IntcPtr,
		XPS_FPGA0_INT_ID,
		(Xil_ExceptionHandler)PL_IntrHandler,
		UsbInstance
	);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// set sensitivity type and priority
	XScuGic_SetPriTrigTypeByDistAddr(
		IntcConfig->DistBaseAddress,
		XPS_FPGA0_INT_ID,		// Int_Id
		32,		// Priority
		//2		// Trigger, edge-triggered
		0		// Trigger, level-sensitive
	);

	// Enable the interrupt for the USB
	XScuGic_Enable((XScuGic*)IntcPtr, XPS_FPGA0_INT_ID);

	return XST_SUCCESS;
}

//=============================================================================
//! Assertion Callback Function
//-----------------------------------------------------------------------------
//! @param *File	File name of the source code where the assertion was
//! triggered.
//! @param Line		Line number of the source code where the assertion was
//! triggered.
//-----------------------------------------------------------------------------
//! @brief This function will be called when assertion is triggered.
//=============================================================================
void AssertCallBack (const char8 *File, s32 Line) {
	xil_printf("[Assert] %s, %d\r\n", File, Line);
}

//=============================================================================
//! Main Function
//-----------------------------------------------------------------------------
//! @brief Main function of this application.
//=============================================================================
int main (void)
{
	//Xil_ICacheDisable();
	//Xil_DCacheDisable();

	Xil_AssertSetCallback((Xil_AssertCallback)AssertCallBack);

	// waits message from linux app
	remoteSetting.opMode = REMOTE_OP_MODE_REMOTE;
	while(fpga->com.IpcMessage1 != IPC_MSG1_APP_START) {}
	fpga->com.IpcMessage1 = 0x00000000;

	// parameter set
	if (remoteSetting.opMode == REMOTE_OP_MODE_AUTO) {
		// obsolete
		xil_printf("remote proc operates in autonomous mode\r\n");
		remoteSetting.inputData = INPUT_DATA_SENSOR_INPUT;
		remoteSetting.returnData = RETURN_DATA_NONE;
		remoteSetting.usbOutput = USB_OUTPUT_STEREO_BM;
	} else {
		int param = fpga->com.IpcParameter1;
		remoteSetting.inputData = param & 0xFF;
		remoteSetting.returnData = (param >> 8) & 0xFF;
		remoteSetting.usbOutput = (param >> 16) & 0xFF;
	}

	xil_printf("start USB Frame Grabber[%d,%d]\r\n", remoteSetting.returnData, remoteSetting.usbOutput);
	App_UsbGrabber(&remoteSetting);

	while(1){
		CommandProcess();
	}

	return 0;
}

//=============================================================================
//! Prompt
//-----------------------------------------------------------------------------
//! Shows prompt to screen.
//=============================================================================
void Prompt (void) {
	xil_printf(">");
	return;
}

//=============================================================================
//! Command Process
//-----------------------------------------------------------------------------
//! Main loop of command process. This function receive a new command and
//! execute it sequentially.
//=============================================================================
void CommandProcess (void)
{
	struct HWTP_CMD cmd;

	while(1){
		Prompt();
		CommandAnalyze(&cmd);
		CommandExecute(cmd);
	}

	return;
}

//=============================================================================
//! Command Receive
//-----------------------------------------------------------------------------
//! Receives a new command.
//=============================================================================
void CommandReceive (
	char* buf, int len
){
	int n = 0;
	while(1){
		buf[n] = uart_getc();
		n++;

		if ((buf[n-1] == 0x0d) || (n == len)) {
			xil_printf("\r\n");
			buf[n-1] = 0x00;
			break;
		} else if (buf[n-1] == 0x08) { // BS
			if (n >= 2) n -= 2;
		}
	}

	return;
}

//=============================================================================
//! Command Analyze
//-----------------------------------------------------------------------------
//! Decompose a received command into a set of parameters.
//=============================================================================
void CommandAnalyze (
	struct HWTP_CMD *cmd
) {
	char buf[128];

	// Receive 1 command line
	CommandReceive(buf, sizeof(buf));
	int len = strlen(buf);

	// Decompose into set of parameters
	int n_parm = 0;
	int n = 0;
	for (int i = 0; i <= len; i++) {
		if (buf[i] == 0x20) {
			if (n != 0) {
				cmd->parm[n_parm][n] = 0x00;
				n = 0;
				n_parm++;
			}
			else {
				// skip successive space
			}
		} else {
			if (n < sizeof(cmd->parm[n_parm])) {
				cmd->parm[n_parm][n] = tolower(buf[i]);
				n++;
			}
		}
	}

	// number of parameters - 1
	cmd->num = n_parm;

	return;
}

//=============================================================================
//! Command Execute
//-----------------------------------------------------------------------------
//! Executes a receive command.
//=============================================================================
void CommandExecute (
	struct HWTP_CMD cmd
) {
	if (strcmp(cmd.parm[0], "ol") == 0) {
		// Output LONG
		unsigned long addr, data;
		hex_to_num(cmd.parm[1], &addr);
		hex_to_num(cmd.parm[2], &data);
		volatile unsigned int* ptr = (volatile unsigned int*)addr;
		*ptr = data;
	} else if (strcmp(cmd.parm[0], "ow") == 0) {
		// Output WORD
		unsigned long addr, data;;
		hex_to_num(cmd.parm[1], &addr);
		hex_to_num(cmd.parm[2], &data);
		volatile unsigned short* ptr = (volatile unsigned short*)addr;
		*ptr = (unsigned short)(data & 0xFFFF);
	} else if (strcmp(cmd.parm[0], "oc") == 0) {
		// Output CHAR
		unsigned long addr, data;;
		hex_to_num(cmd.parm[1], &addr);
		hex_to_num(cmd.parm[2], &data);
		volatile unsigned char* ptr = (volatile unsigned char*)addr;
		*ptr = (unsigned char)(data & 0xFF);
	} else if (strcmp(cmd.parm[0], "il") == 0) {
		// Input LONG
		unsigned long addr;
		hex_to_num(cmd.parm[1], &addr);
		volatile unsigned int* ptr = (volatile unsigned int*)addr;
		xil_printf("%08lX\r\n", *ptr);
	} else if (strcmp(cmd.parm[0], "iw") == 0) {
		// Input WORD
		unsigned long addr;
		hex_to_num(cmd.parm[1], &addr);
		volatile unsigned short* ptr = (volatile unsigned short*)addr;
		xil_printf("%04X\r\n", *ptr);
	} else if (strcmp(cmd.parm[0], "ic") == 0) {
		// Input CHAR
		unsigned long addr;
		hex_to_num(cmd.parm[1], &addr);
		volatile unsigned char* ptr = (volatile unsigned char*)addr;
		xil_printf("%02X\r\n", *ptr);
	} else if (strcmp(cmd.parm[0], "dl") == 0) {
		// Dump LONG
		unsigned long addr;
		hex_to_num(cmd.parm[1], &addr);
		volatile unsigned int* ptr = (volatile unsigned int*)addr;
		for (int i = 0; i < 16; i++) {
			if (i % 4 == 0) xil_printf("%08lX: ", ptr);
			xil_printf("%08lX ", *ptr++);
			if (i % 4 == 3) xil_printf("\r\n");
		}
	} else if (strcmp(cmd.parm[0], "dw") == 0) {
		// Dump WORD
		unsigned long addr;
		hex_to_num(cmd.parm[1], &addr);
		volatile unsigned short* ptr = (volatile unsigned short*)addr;
		for (int i = 0; i < 32; i++) {
			if (i % 8 == 0) xil_printf("%08lX: ", ptr);
			xil_printf("%04X ", *ptr++);
			if (i % 8 == 7) xil_printf("\r\n");
		}
	} else if (strcmp(cmd.parm[0], "dc") == 0) {
		// Dump CHAR
		unsigned long addr;
		hex_to_num(cmd.parm[1], &addr);
		volatile unsigned char* ptr = (volatile unsigned char*)addr;
		for (int i = 0; i < 64; i++) {
			if (i % 16 == 0) xil_printf("%08lX: ", ptr);
			xil_printf("%02X ", *ptr++);
			if (i % 16 == 15) xil_printf("\r\n");
		}
	} else if (strcmp(cmd.parm[0], "r") == 0) {
		unsigned long addr;
		unsigned char data;
		if(cmd.num != 1) {
			xil_printf("Invalid parameters\r\n");
		} else {
			hex_to_num(cmd.parm[1], &addr);
			Ov5640_I2cRead((unsigned int)addr, &data);
			xil_printf("%02X\r\n", data);
		}
	} else if (strcmp(cmd.parm[0], "w") == 0) {
		unsigned long addr, data;
		if(cmd.num != 2) {
			xil_printf("Invalid parameters\r\n");
		} else {
			hex_to_num(cmd.parm[1], &addr);
			hex_to_num(cmd.parm[2], &data);
			Ov5640_I2cWrite((unsigned int)addr, (unsigned char)data);
		}
	} else if (strcmp(cmd.parm[0], "t1") == 0) {
		I2c_Init();
	} else if (strcmp(cmd.parm[0], "t2") == 0) {
		I2c_ShowStatus();
	} else if (strcmp(cmd.parm[0], "t3") == 0) {
		I2c_SetCh(I2C_SEL_CH2 | I2C_SEL_CH3);
	} else if (strcmp(cmd.parm[0], "t4") == 0) {
		unsigned char ch;
		I2c_ReadCh(&ch);
		xil_printf("ch %d\r\n", ch);
	} else if (strcmp(cmd.parm[0], "t5") == 0) {
		Ov5640_I2cWrite(0x3000, 0xAA);
	} else if (strcmp(cmd.parm[0], "t6") == 0) {
		unsigned char val;
		Ov5640_I2cRead(0x3000, &val);
		xil_printf("val %02X\r\n", val);
	} else if (strcmp(cmd.parm[0], "t7") == 0) {
	} else if (strcmp(cmd.parm[0], "t8") == 0) {
	} else if (strcmp(cmd.parm[0], "t9") == 0) {
	} else if (strcmp(cmd.parm[0], "t10") == 0) {
		unsigned int *addr = (unsigned int *)0x40000000;
		for (int i = 0; i < 0x10000000; i+=4) {
			*addr = 0;
			addr++;
		}
	} else if (strcmp(cmd.parm[0], "t11") == 0) {
		unsigned int *addr = (unsigned int *)0x40000000;
		Xil_DCacheInvalidateRange((UINTPTR)addr, 0x1000);
	} else if (strcmp(cmd.parm[0], "t12") == 0) {
		fpga->arb.Control   = 0x00000001;
		fpga->grb.Control   = 0x00000001;
		fpga->grb.Address_A = 0x40000000;
	} else if (strcmp(cmd.parm[0], "t13") == 0) {
		// dump image buffer
		unsigned int *addr;
		addr = (unsigned int *)0x40000000;

		for (int h = 0; h < 480; h++) {
			for (int w = 0; w < 640; w++) {
				xil_printf("%08X,", *addr);
				//xil_printf("%03X,", *addr);
				addr++;
			}
		}
	} else if (strcmp(cmd.parm[0], "t14") == 0) {
		// read WHO_AM_I register from LSM9DS1 Accel & Gyro
		I2c_Init();
		I2c_SetCh(I2C_SEL_CH0);

		unsigned char reg_addr = 0x0F;
		I2c_Write(0x6A, &reg_addr, 1);

		unsigned char reg_value;
		I2c_Read(0x6A, &reg_value, 1);

		xil_printf("%X\r\n", reg_value);
	} else if (strcmp(cmd.parm[0], "t15") == 0) {
		unsigned char reg_value[2];
		unsigned long tmpl;

		// Register Address
		hex_to_num(cmd.parm[1], &tmpl);
		reg_value[0] = (unsigned char)tmpl;

		// Register Value
		hex_to_num(cmd.parm[2], &tmpl);
		reg_value[1] = (unsigned char)tmpl;

		Lsm9ds1_XlgWrite(reg_value[0], reg_value[1]);
	} else if (strcmp(cmd.parm[0], "t16") == 0) {
		unsigned char reg_addr, reg_value;
		unsigned long tmpl;

		// Register Address
		hex_to_num(cmd.parm[1], &tmpl);
		reg_addr = (unsigned char)tmpl;

		Lsm9ds1_XlgRead(reg_addr, &reg_value);
		xil_printf("read %x\r\n", reg_value);
	} else if (strcmp(cmd.parm[0], "t17") == 0) {
		unsigned char reg_value[2];
		unsigned long tmpl;

		// Register Address
		hex_to_num(cmd.parm[1], &tmpl);
		reg_value[0] = (unsigned char)tmpl;

		// Register Value
		hex_to_num(cmd.parm[2], &tmpl);
		reg_value[1] = (unsigned char)tmpl;

		Lsm9ds1_MagWrite(reg_value[0], reg_value[1]);
	} else if (strcmp(cmd.parm[0], "t18") == 0) {
		unsigned char reg_addr, reg_value;
		unsigned long tmpl;

		// Register Address
		hex_to_num(cmd.parm[1], &tmpl);
		reg_addr = (unsigned char)tmpl;

		Lsm9ds1_MagRead(reg_addr, &reg_value);
		xil_printf("read %x\r\n", reg_value);
	} else if (strcmp(cmd.parm[0], "t19") == 0) {
		I2c_Init();
		//I2c_SetCh(I2C_SEL_CH0);
		I2c_SetCh(I2C_SEL_CH1);

		Lsm9ds1_MagWrite(MAG_ADDR_CTRL_REG3_M,		0x00);
		Lsm9ds1_MagWrite(MAG_ADDR_INT_CFG_M,		0xE1);
	} else if (strcmp(cmd.parm[0], "t20") == 0) {
		unsigned char reg_value;
		Lsm9ds1_MagRead(MAG_ADDR_STATUS_REG_M, &reg_value);
		xil_printf("MAG_ADDR_STATUS_REG_M: %08X\r\n", reg_value);
		Lsm9ds1_MagRead(MAG_ADDR_INT_SRC_M, &reg_value);
		xil_printf("MAG_ADDR_INT_SRC_M: %08X\r\n", reg_value);
	} else if (strcmp(cmd.parm[0], "t21") == 0) {
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
	} else if (strcmp(cmd.parm[0], "t23") == 0) {
		//App_UsbGrabber(0, 0);
	} else if (strcmp(cmd.parm[0], "t24") == 0) {
		App_9DofHwTest();
	} else if (strcmp(cmd.parm[0], "t25") == 0) {
		xil_printf("\n\r");
		xil_printf("Monitor INT pin\r\n");
		xil_printf("Press ESC to leave\r\n");
		unsigned char ch;
		while(1) {
			// press "esc" to leave
			if (uart_getc2(&ch)) {
				if (ch == 0x1B) {
					return XST_SUCCESS;
				}
			}
			//unsigned int tmpi = fpga->com.GPIO_In;
			//xil_printf("%1d", tmpi);
			xil_printf("%1d", Fpga_ReadSwitch());
		}
	} else if (strcmp(cmd.parm[0], "t26") == 0) {
		unsigned int tmpi = fpga->com.GPIO_Out;
		unsigned int tmpi2 = 0;
		if ((tmpi & 0x00000001) == 0x00000000) {
			tmpi2 += 0x00000001;
		}
		if ((tmpi & 0x00000002) == 0x00000000) {
			tmpi2 += 0x00000002;
		}
		fpga->com.GPIO_Out = tmpi2;
	} else if (strcmp(cmd.parm[0], "t27") == 0) {
		Fpga_LedOn();
	} else if (strcmp(cmd.parm[0], "t28") == 0) {
		Fpga_LedOff();
	}

	return;
}

//=============================================================================
// Application Level Functions
//=============================================================================
//=============================================================================
//! USB Frame Grabber
//-----------------------------------------------------------------------------
//! @brief Main function of USB frame grabber application.
//=============================================================================
void App_UsbGrabber (struct REMOTE_SETTING *remoteSetting) {
	s32 Status;

	// Initialize FPGA
	Fpga_Init(remoteSetting);

	// Initialize MIPI CSI Receiver
	Csi_Init(CSI_TWO_LANE);

	// Initialize OV5640
	int actual_scl = I2c_Init();
	xil_printf("SCL frequency is set to %d[Hz] (target %d)\r\n", actual_scl, I2C_TARGET_SCL);
	I2c_SetCh(I2C_SEL_CH2 | I2C_SEL_CH3);

	Ov5640_SoftwareReset (OV5640_RESET_ASSERT);
	usleep(1000);
	Ov5640_SoftwareReset (OV5640_RESET_RELEASE);

	int param_set = 0;
	if (param_set == 0) {
		Ov5640_Init(
			OV5640_TEST_NORMAL,
			//OV5640_TEST_COLORBAR,
			OV5640_RESOLUTION_640_480,
			30.0
		);
		//Ov5640_SetExposureManual(0);
	} else if (param_set == 1) {
		// e-con parameter
		Ov5640_PowerUp();
		Ov5640_YUY2_640_480_60FPS2();
	} else {
		// e-con parameter modified
		Ov5640_PowerUp();
		Ov5640_YUY2_640_480_60FPS3();
		fpga->csi.FrameDecimation = 11;
	}

	// Wait some time for PLLs to be stabilized
	usleep(1000000);

	// Check status
	Status = Csi_CheckStatus();
	//Xil_AssertVoid(Status == XST_SUCCESS);

	// Initialize USB driver
	Status = Xusb_Init();
	Xil_AssertVoid(Status == XST_SUCCESS);

	// Setup interrupt
	Status = SetupInterruptSystem(
		INTC_DEVICE_ID,
		(void *)&InterruptController
	);
	Xil_AssertVoid(Status == XST_SUCCESS);

	// Start
	xil_printf("start USB driver\r\n");
	Xusb_Main(remoteSetting);
}

//=============================================================================
//! 9DOF Hardware Test
//-----------------------------------------------------------------------------
//! @brief Performs hardware test on MikroElektronika 9DOF click module
//! mounted on MB1/2 sites.
//=============================================================================
void App_9DofHwTest (void) {

	int found;
	unsigned char reg_value;
	unsigned int intr_sts, gpio;

	I2c_Init();

	found = 0;
	for (int ch = 0; ch < 2; ch++) {
		xil_printf("H/W test on MB site %d\r\n", ch + 1);

		// set channel
		if (ch == 0) {
			I2c_SetCh(I2C_SEL_CH0);
		} else {
			I2c_SetCh(I2C_SEL_CH1);
		}

		// read WHO_AM_I registers
		found = 1;
		intr_sts = Lsm9ds1_XlgRead (XLG_ADDR_WHO_AM_I, &reg_value);
		if (!I2c_IsTransferComplete(intr_sts) || (reg_value != XLG_WHO_AM_I)) {
			I2c_DiagnoseError(intr_sts);
			found = 0;
		} else {
			xil_printf("XLG WHO_AM_I: %02X\r\n", reg_value);
		}

		intr_sts = Lsm9ds1_MagRead (MAG_ADDR_WHO_AM_I_M, &reg_value);
		if (!I2c_IsTransferComplete(intr_sts) || (reg_value != MAG_WHO_AM_I)) {
			I2c_DiagnoseError(intr_sts);
			found = 0;
		} else {
			xil_printf("MAG WHO_AM_I: %02X\r\n", reg_value);
		}

		// interrupt signal test
		if (found == 1) {
			// enable interrupt
			Lsm9ds1_MagWrite(MAG_ADDR_CTRL_REG3_M,		0x00);
			Lsm9ds1_MagWrite(MAG_ADDR_INT_CFG_M,		0xE1);

			int count = 0;
			while(1) {
				if (ch == 0) {
					gpio = fpga->com.GPIO_In & 0x00000001;
				} else {
					gpio = (fpga->com.GPIO_In >> 1) & 0x00000001;
				}

				if (gpio == 0x00000001) {
					xil_printf("Interrupt from MB%d detected\r\n", ch + 1);
					Lsm9ds1_ReadMagValues();
					break;
				}

				if (count == 0x1000) {
					xil_printf("Interrupt from MB%d not detected\r\n", ch + 1);
					break;
				}

				count++;
			}
		}
		else {
			xil_printf("Device not found on MB%d\r\n", ch + 1);
		}
	}
}

//=============================================================================
// Helper Functions
//=============================================================================
//=============================================================================
//! Hexadecimal to Numerical
//-----------------------------------------------------------------------------
//! @param *hex		Pointer to an input hexadecimal number
//! @param *num		Converted data is stored here.
//-----------------------------------------------------------------------------
//! Converts hexadecimal number expressed in ASCII code string into numerical
//! value.
//=============================================================================
void hex_to_num (char* hex, unsigned long* num) {
	int id = 0;
	unsigned long val = 0;
	while(hex[id] != 0) {
		val *= 16;
		if ((0x30 <= hex[id]) && (hex[id] <= 0x39)) {
			// 0 - 9
			val += (hex[id] - 0x30);
		}
		else if ((0x41 <= hex[id]) && (hex[id] <= 0x46)) {
			// A - F
			val += (hex[id] - 0x37);
		}
		else if ((0x61 <= hex[id]) && (hex[id] <= 0x66)) {
			// a - f
			val += (hex[id] - 0x57);
		}
		id++;
	}

	*num = val;
}

//=============================================================================
//! Decimal to Numerical
//-----------------------------------------------------------------------------
//! @param *dec		Pointer to an input decimal number
//! @param *num		Converted data is stored here.
//-----------------------------------------------------------------------------
//! Converts decimal number expressed in ASCII code string into numerical
//! value.
//=============================================================================
void dec_to_num (char* dec, unsigned long* num) {
	int id = 0;
	unsigned long val = 0;
	while(dec[id] != 0) {
		val *= 10;
		if ((0x30 <= dec[id]) && (dec[id] <= 0x39)) {
			// 0 - 9
			val += (dec[id] - 0x30);
		}
		id++;
	}

	*num = val;
}

//=============================================================================
//! Set Bits
//-----------------------------------------------------------------------------
//! @param *reg		Pointer to a register to be manipulated
//! @param bit_mask	Bit mask pattern
//-----------------------------------------------------------------------------
//! This function sets bits in the register where corresponding bits in the
//! mask pattern is 1.
//=============================================================================
void set_bit (unsigned int* reg, int bit_mask) {
	*reg |= bit_mask;
}

//=============================================================================
//! Clear Bits
//-----------------------------------------------------------------------------
//! @param *reg		Pointer to a register to be manipulated
//! @param bit_mask	Bit mask pattern
//-----------------------------------------------------------------------------
//! This function clears bits in the register where corresponding bits in the
//! mask pattern is 1.
//=============================================================================
void clear_bit (unsigned int* reg, int bit_mask) {
	*reg &= ~bit_mask;
}

//=============================================================================
//! Get Bit
//-----------------------------------------------------------------------------
//! @param reg		Pointer to a register to be read
//! @param bit_mask	Bit mask pattern
//-----------------------------------------------------------------------------
//! @return "1" if all bits specified by mask patter is set in the register,
//! else "0".
//=============================================================================
int get_bit (unsigned int reg, int bit_mask) {
	if ((reg & bit_mask) == bit_mask) {
		return 1;
	} else {
		return 0;
	}
}
