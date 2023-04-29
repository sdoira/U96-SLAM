//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file xusb_main.c
//!
//! C source file for the implementation of USB main loop.
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.08 First release
//! </pre>
//=============================================================================
#include "xusb_main.h"

// Initialize a DFU data structure
static USBCH9_DATA storage_data = {
	.ch9_func = {
		// Set the chapter9 hooks
		.Usb_Ch9SetupDevDescReply = Usb_Ch9SetupDevDescReply,
		.Usb_Ch9SetupCfgDescReply = Usb_Ch9SetupCfgDescReply,
		.Usb_Ch9SetupBosDescReply = Usb_Ch9SetupBosDescReply,
		.Usb_Ch9SetupStrDescReply = Usb_Ch9SetupStrDescReply,
		.Usb_SetConfiguration = Usb_SetConfiguration,
		.Usb_SetConfigurationApp = Usb_SetConfigurationApp,
		.Usb_SetInterfaceHandler = Usb_SetInterfaceHandler,
		.Usb_ClassReq = Usb_ClassReq,
		.Usb_GetDescReply = NULL,
	},
	.data_ptr = (void *)NULL,
};

extern volatile struct FPGA_REG *fpga;
extern struct UVC_APP_DATA app_data;

//=============================================================================
//! XUSB Initialize
//-----------------------------------------------------------------------------
//! @return XST_SUCCESS else XST_FAILURE
//-----------------------------------------------------------------------------
//! @brief This function initializes USB driver.
//=============================================================================
int Xusb_Init (void)
{
	s32 Status;

	// Initialize the USB driver so that it's ready to use,
	// specify the controller ID that is generated in xparameters.h
	UsbConfigPtr = LookupConfig(USB_DEVICE_ID);
	if (NULL == UsbConfigPtr) {
		return XST_FAILURE;
	}

	// We are passing the physical base address as the third argument
	// because the physical and virtual base address are the same in our
	// example.  For systems that support virtual memory, the third
	// argument needs to be the virtual base address.
	Status = CfgInitialize(&UsbInstance, UsbConfigPtr, UsbConfigPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		return XST_FAILURE;
	}

	// hook up chapter9 handler
	Set_Ch9Handler(UsbInstance.PrivateData, Ch9Handler);

	// Assign the data to usb driver
	Set_DrvData(UsbInstance.PrivateData, &storage_data);

	// Assign user-defined data to usb driver
	storage_data.data_ptr = (void*)&app_data;

	// set endpoint handlers
	SetEpHandler(UsbInstance.PrivateData, 2, USB_EP_DIR_IN, IntrRxHandler);
	SetEpHandler(UsbInstance.PrivateData, 3, USB_EP_DIR_IN, BulkInHandler);

	return XST_SUCCESS;
}

//=============================================================================
//! XUSB Main
//-----------------------------------------------------------------------------
//! @return XST_SUCCESS else XST_FAILURE
//-----------------------------------------------------------------------------
//! @brief Main loop of USB function.
//=============================================================================
int Xusb_Main (struct REMOTE_SETTING *remoteSetting)
{
	s32 Status;
	int remain;
	int payload_size, buf_size;
	u8 *data_ptr;

	// application specific data structure
	struct UVC_APP_DATA *app_data = Uvc_GetAppData(&UsbInstance);

	// start the controller so that Host can see our device
	Usb_Start(UsbInstance.PrivateData);

	int num_frame = 0;
	int usb_tx = 0;
	if (remoteSetting->opMode == REMOTE_OP_MODE_AUTO) {
		usb_tx = 1;
	}
	while(1) {
		// press "esc" to leave
		if (remoteSetting->opMode == REMOTE_OP_MODE_AUTO) {
			unsigned char ch;
			if (uart_getc2(&ch)) {
				if (ch == 0x1B) {
					return XST_SUCCESS;
				}
			}
		}

		// wait for new frame
		while(app_data->intr_received == 0) {}
		unsigned int param = 0;
		if (app_data->grb_received != 0) {
			param += RETURN_DATA_RAW_FRAME;
			app_data->grb_received = 0;
		}
		if (app_data->rect_received != 0) {
			param += RETURN_DATA_STEREO_RECT;
			app_data->rect_received = 0;
		}
		if (app_data->xsbl_received != 0) {
			app_data->xsbl_received = 0;
		}
		if (app_data->bm_received != 0) {
			param += RETURN_DATA_STEREO_BM;
			app_data->bm_received = 0;
		}
		if (app_data->gftt_received != 0) {
			param += RETURN_DATA_GFTT;
			app_data->gftt_received = 0;
		}
		app_data->intr_received = 0;
		fpga->com.IpcParameter1 = param;
		fpga->com.IpcParameter2 = app_data->bank;

		// brightness control
		/*
		if (num_frame % 10 == 0) {
			Ov5640_BrightnessControl(app_data, remoteSetting, 640, 480, &av, &br);
		}
		*/
		fpga->com.IpcMessage2 = IPC_MSG2_DATA_READY;

		// USB transfer control
		if (fpga->com.IpcMessage1 == IPC_MSG1_OP_START) {
			// start transfer
			fpga->com.IpcMessage1 = 0;
			usb_tx = 1;
		} else if (fpga->com.IpcMessage1 == IPC_MSG1_OP_STOP) {
			// stop transfer
			fpga->com.IpcMessage1 = 0;
			usb_tx = 0;
		}

		// transfer data only after video streaming parameters are committed
		if ((app_data->state == UVC_STATE_COMMIT) && (usb_tx == 1))
		{
			// rearrange the received data into TX buffer for USB transfer
			Xusb_ReceiveData (app_data, remoteSetting, &data_ptr);

			remain = app_data->commit.dwMaxVideoFrameSize;
			app_data->busy = 0;
			while(remain > 0) {
				if (remain > app_data->max_payload_size) {
					payload_size = app_data->max_payload_size;
				} else {
					payload_size = remain;
				}
				buf_size = payload_size + app_data->header_size;
				remain -= payload_size;

				// embed header in image buffer
				memcpy (data_ptr, (u8*)&app_data->header, app_data->header_size);

				// data transfer
				app_data->busy = 1;
				Status = EpBufferSend(UsbInstance.PrivateData, 3, data_ptr, buf_size);
				Xil_AssertNonvoid(Status == XST_SUCCESS);

				// wait for transfer complete
				while(app_data->busy == 1) {}

				// next transfer
				data_ptr += payload_size;
			}

			// toggle field bit
			app_data->header.bmHeaderInfo ^= UVC_PAYLOAD_HEADER_FID;
		}

		// switch active bank
		if (app_data->bank == 0) {
			app_data->bank = 1;
		} else {
			app_data->bank = 0;
		}

		num_frame++;
	}

	return XST_SUCCESS;
}

//=============================================================================
//! Interrupt RX Handler
//-----------------------------------------------------------------------------
//! @param *CallBackRef
//! @param RequestedBytes
//! @param BytesTxed
//-----------------------------------------------------------------------------
//! @brief This handler is called when data is received from interrupt
//! endpoint. Not used in this design.
//=============================================================================
void IntrRxHandler(void *CallBackRef, u32 RequestedBytes, u32 BytesTxed)
{
	xil_printf("IntrRxHandler\r\n");
}

//=============================================================================
//! Bulk Input Handler
//-----------------------------------------------------------------------------
//! @param *CallBackRef
//! @param RequestedBytes
//! @param BytesTxed
//-----------------------------------------------------------------------------
//! @brief This handler is called when data transfer through bulk endpoint is
//! completed.
//=============================================================================
void BulkInHandler(void *CallBackRef, u32 RequestedBytes, u32 BytesTxed)
{
	struct UVC_APP_DATA *app_data = Uvc_GetAppData((struct Usb_DevData*)CallBackRef);
	app_data->busy = 0;
}

//=============================================================================
//! XUSB Setup Interrupt
//-----------------------------------------------------------------------------
//! @param *IntcPtr		Pointer to the instance of the interrupt controller
//-----------------------------------------------------------------------------
//! @return XST_SUCCESS else XST_FAILURE
//-----------------------------------------------------------------------------
//! @brief This function sets up the USB interrupt.
//=============================================================================
s32 Xusb_SetupInterrupt (void *IntcPtr)
{
	s32 Status;

	// Connect to the interrupt controller
	Status = XScuGic_Connect(
		(XScuGic*)IntcPtr,
		USB_INT_ID,
		(Xil_ExceptionHandler)XUsbPsu_IntrHandler,
		(void*)UsbInstance.PrivateData
	);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	// Enable the interrupt for the USB
	XScuGic_Enable((XScuGic*)IntcPtr, USB_INT_ID);

	// Enable interrupts for Reset, Disconnect, ConnectionDone, Link State
	// Wakeup and Overflow events.
	XUsbPsu_EnableIntr(
		UsbInstance.PrivateData,
		XUSBPSU_DEVTEN_VNDRDEVTSTRCVEDEN | // added
		XUSBPSU_DEVTEN_EVNTOVERFLOWEN |
		XUSBPSU_DEVTEN_CMDCMPLTEN | // added
		XUSBPSU_DEVTEN_ERRTICERREN | // added
		//XUSBPSU_DEVTEN_SOFEN | // added
		XUSBPSU_DEVTEN_EOPFEN | // added
		XUSBPSU_DEVTEN_HIBERNATIONREQEVTEN | // added
		XUSBPSU_DEVTEN_WKUPEVTEN |
		XUSBPSU_DEVTEN_ULSTCNGEN |
		XUSBPSU_DEVTEN_CONNECTDONEEN |
		XUSBPSU_DEVTEN_USBRSTEN |
		XUSBPSU_DEVTEN_DISCONNEVTEN
	);

	return XST_SUCCESS;
}

void Xusb_ReceiveData (
	struct UVC_APP_DATA *app_data,
	struct REMOTE_SETTING *remoteSetting,
	u8 **data_ptr)
{
	if (remoteSetting->usbOutput == USB_OUTPUT_RAW_FRAME) {
		// Frame Grabber
		u8 *src = (app_data->bank == 0) ? (u8*)BUF_GRB_A : (u8*)BUF_GRB_B;
		u8 *dst = Buffer + app_data->header_size;
		for (int row = 0; row < 480; row++) {
			for (int col = 0; col < 640; col++) {
				for (int lr = 0; lr < 2; lr++) {
					for (int yuv = 1; yuv >= 0; yuv--) {
						dst[(row * 1280 + col + lr * 640) * 2 + yuv] = *src;
						src++;
					}
				}
			}
		}
		*data_ptr = Buffer;
	} else if (remoteSetting->usbOutput == USB_OUTPUT_STEREO_RECT) {
		// Rectification
		u8 *src = (app_data->bank == 0) ? (u8*)BUF_RECT_A : (u8*)BUF_RECT_B;
		u8 *dst = Buffer + app_data->header_size;
		for (int lr = 0; lr < 2; lr++) {
			for (int row = 0; row < 480; row++) {
				for (int col = 0; col < 640; col+=4) {
					// big endian -> little
					//unsigned int src_addr = lr * 524288 + row * 1024 + col; // aligned
					unsigned int src_addr = lr * 524288 + row * 640 + col; // continuous
					unsigned int dst_addr = (row * 1280 + col + lr * 640) * 2;
					dst[dst_addr++] = src[src_addr++];
					dst[dst_addr++] = 0x80;
					dst[dst_addr++] = src[src_addr++];
					dst[dst_addr++] = 0x80;
					dst[dst_addr++] = src[src_addr++];
					dst[dst_addr++] = 0x80;
					dst[dst_addr++] = src[src_addr];
					dst[dst_addr] = 0x80;
				}
			}
		}
		*data_ptr = Buffer;
	} else if (remoteSetting->usbOutput == USB_OUTPUT_STEREO_XSBL) {
		// Rectification + X-Sobel
		u8 *src = (app_data->bank == 0) ? (u8*)BUF_XSBL_A : (u8*)BUF_XSBL_B;
		u8 *dst = Buffer + app_data->header_size;
		for (int row = 0; row < 480; row++) {
			for (int col = 0; col < 640; col+=2) {
				unsigned int src_addr = row * 2048 + col * 2 + 3;
				unsigned int dst_addr_l = (row * 1280 + col) * 2;
				unsigned int dst_addr_r = (row * 1280 + col + 640) * 2;
				dst[dst_addr_l++] = src[src_addr--];
				dst[dst_addr_l++] = 0x80;
				dst[dst_addr_r++] = src[src_addr--];
				dst[dst_addr_r++] = 0x80;
				dst[dst_addr_l++] = src[src_addr--];
				dst[dst_addr_l] = 0x80;
				dst[dst_addr_r++] = src[src_addr--];
				dst[dst_addr_r] = 0x80;
			}
		}
		*data_ptr = Buffer;
	} else if (remoteSetting->usbOutput == USB_OUTPUT_STEREO_BM) {
		// Rectification + X-Sobel + Block Matching
		// continuous memory
		s16 *src = (app_data->bank == 0) ? (s16*)BUF_DISP_A : (s16*)BUF_DISP_B;
		u8 *dst = Buffer + app_data->header_size;
		for (int row = 0; row < IMAGE_HEIGHT; row++) {
			unsigned int dst_addr_l = (row * 1280) * 2;
			unsigned int dst_addr_r = (row * 1280 + 640) * 2;
			for (int col = 0; col < IMAGE_WIDTH; col++) {
				s16 tmps = *src;
				tmps = tmps >> 4;
				dst[dst_addr_l++] = (u8)(tmps);
				dst[dst_addr_l++] = 0x80;
				dst[dst_addr_r++] = 0x00;
				dst[dst_addr_r++] = 0x80;
				src++;
			}
		}
		*data_ptr = Buffer;
	}
}
