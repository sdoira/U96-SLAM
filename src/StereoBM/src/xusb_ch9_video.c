//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file xusb_ch9_video.c
//!
//! C source for USB Video Class implementation.
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#include <string.h>
#include "xparameters.h"
#include "xusb_ch9_video.h"

u8 *usb_config;
struct UVC_APP_DATA app_data;


//=============================================================================
//! Device Descriptors
//=============================================================================
USB_STD_DEV_DESC __attribute__ ((aligned(16))) deviceDesc[] = {
	{	// USB 2.x
		sizeof(USB_STD_DEV_DESC),	// bLength
		USB_TYPE_DEVICE_DESC,		// bDescriptorType
		0x0210,						// bcdUSB
		0xEF,						// bDeviceClass
		0x02,						// bDeviceSubClass
		0x01,						// bDeviceProtocol
		0x40,						// bMaxPackedSize0
		0x1D50,						// idVendor
		0x60F9,						// idProduct
		0x0001,						// bcdDevice
		0x01,						// iManufacturer
		0x02,						// iProduct
		0x00,						// iSerialNumber
		0x01						// bNumConfigurations
	},
	{	// USB 3.x
		sizeof(USB_STD_DEV_DESC),	// bLength
		USB_TYPE_DEVICE_DESC,		// bDescriptorType
		0x0300,						// bcdUSB
		0xEF,						// bDeviceClass
		0x02,						// bDeviceSubClass
		0x01,						// bDeviceProtocol
		0x09,						// bMaxPackedSize0
		0x1D50,						// idVendor
		0x60FA,						// idProduct
		0x0002,						// bcdDevice
		0x01,						// iManufacturer
		0x02,						// iProduct
		0x00,						// iSerialNumber
		0x01						// bNumConfigurations
	}
};

//=============================================================================
// Configuration Descriptors for USB 2.x
//=============================================================================
USB_CONFIG __attribute__ ((aligned(16))) config2 = {
	{	// Configuration Descriptor
		sizeof(USB_STD_CFG_DESC),			// bLength
		USB_TYPE_CONFIG_DESC,				// bDescriptorType
		sizeof(USB_CONFIG),					// wTotalLength
		0x02,								// bNumInterfaces
		0x01,								// bConfigurationValue
		0x00,								// iConfiguration
		0xC0,								// bmAttribute
		0x00								// bMaxPower
	},
	{	// Interface Association Descriptor
		sizeof(USB_IF_ASC_DESC),			// bLength
		USB_TYPE_INTERFACE_ASSOCIATION,		// bDescriptorType
		0x00,								// bFirstInstance
		0x02,								// bInterfaceCount
		UVC_CC_VIDEO,						// bFunctionClass
		UVC_SC_VIDEO_INTERFACE_COLLECTION,	// bFunctionSubClass
		UVC_PC_PROTOCOL_UNDEFINED,			// bFunctionProtocol
		0x02								// iFunction
	},
	{	// Standard VC Interface #0 (VideoCtrol)
		sizeof(USB_STD_IF_DESC),			// bLength
		USB_TYPE_INTERFACE_DESC,			// bDescriptorType
		0x00,								// bInterfaceNumber
		0x00,								// bAlternateSetting
		0x01,								// bNumEndpoints
		UVC_CC_VIDEO,						// bInterfaceClass
		UVC_SC_VIDEOCONTROL,				// bInterfaceSubClass
		UVC_PC_PROTOCOL_UNDEFINED,			// bInterfaceProtocol
		0x02								// iInterface
	},
	{{	// Class-specific VC Interface
		sizeof(UVC_IF_DESC),				// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VC_HEADER,						// bDescriptorSubType
		0x0100,								// bcdUVC
		sizeof(USB20_CS_VC),				// wTotalLength
		100000000,							// dwClockFrequency
		0x01,								// bInCollection
		0x01								// baInterfaceNr(1)
	},
	{	// Input Terminal Descriptor (Camera)
		sizeof(UVC_CAMERA_TERM_DESC),		// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VC_INPUT_TERMINAL,				// bDescriptorSubType
		0x01,								// bTerminalID
		UVC_ITT_CAMERA,						// wTerminalType
		0x00,								// bAssocTerminal
		0x00,								// iTerminal
		0x0000,								// wObjectiveFocalLengthMin
		0x0000,								// wObjectiveFocalLengthMax
		0x0000,								// wOcularFocalLength
		0x03,								// bControlSize
		0x00,								// bmControls_L
		0x00,								// bmControls_M
		0x00								// bmControls_H
	},
	{	// Processing Unit Descriptor
		sizeof(UVC_PROCESSING_UNIT_DESC),	// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VC_PROCESSING_UNIT,				// bDescriptorSubType
		0x02,								// bUnitID
		0x01,								// bSourceID
		0x4000,								// wMaxMultiplier
		0x03,								// bControlSize
		0x00,								// bmControls_L
		0x00,								// bmControls_M
		0x00,								// bmControls_H
		0x00								// iProcessing
	},
	{	// Output Terminal Descriptor
		sizeof(UVC_OUTPUT_TERM_DESC),		// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VC_OUTPUT_TERMINAL,				// bDescriptorSubType
		0x03,								// bTerminalID
		UVC_TT_STREAMING,					// wTerminalType
		0x00,								// bAssocTerminal
		0x02,								// bSourceID
		0x00								// iTerminal
	},}, // end of CS VC descriptors
	{	// Standard Interrupt Endpoint Descriptor
		sizeof(USB_STD_EP_DESC),			// bLength
		USB_TYPE_ENDPOINT_CFG_DESC,			// bDescriptorType
		0x82,								// bEndpointAddress
		USB_EP_INTERRUPT,					// bmAttributes
		0x40,								// wMaxPacketSizeL
		0x00,								// wMaxPacketSizeH
		0x01								// bInterval
	},
	{	// Class-Specific Interrupt Endpoint Descriptor
		sizeof(UVC_INTR_EP_DESC),			// bLength
		UVC_CS_ENDPOINT,					// bDescriptorType
		USB_EP_INTERRUPT,					// bDescriptorSubType
		0x0040								// wMaxTransferSize
	},
	{	// Standard VS Interface #1 (Alt0)
		sizeof(USB_STD_IF_DESC),			// bLength
		USB_TYPE_INTERFACE_DESC,			// bDescriptorType
		0x01,								// bInterfaceNumber
		0x00,								// bAlternateSetting
		0x01,								// bNumEndpoints
		UVC_CC_VIDEO,						// bInterfaceClass
		UVC_SC_VIDEOSTREAMING,				// bInterfaceSubClass
		UVC_PC_PROTOCOL_UNDEFINED,			// bInterfaceProtocol
		0x00								// iInterface
	},
	{{	// Class-Specific Input Header
		sizeof(UVC_INPUT_HEADER_DESC),		// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VS_INPUT_HEADER,				// bDescriptorSubType
		0x01,								// bNumFormats
		sizeof(USB20_CS_VS),				// wTotalLength
		0x83,								// bEndpointAddress
		0x00,								// bmInfo
		0x03,								// bTerminalLink
		0x00,								// bStillCaptureMethod
		0x00,								// bTriggerSupport
		0x00,								// bTriggerUsage
		0x01,								// bControlSize
		0x00								// bmaControls
	},
	{	// Payload-Specific Format (#1, Uncompressed)
		sizeof(UVC_UNCOMPRESSED_FORMAT_DESC),
											// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VS_FORMAT_UNCOMPRESSED,			// bDescriptorSubType
		0x01,								// bFormatIndex
		0x01,								// bNumFrameDescriptors
		0x32595559,							// guidFormat_LL
		0x00100000,							// guidFormat_LH
		0xaa000080,							// guidFormat_HL
		0x719b3800,							// guidFormat_HH
		0x10,								// bBitsPerPixel
		0x01,								// bDefaultFrameIndex
		0x00,								// bAspectRatioX
		0x00,								// bAspectRatioY
		0x00,								// bmInterlaceFlags
		0x00								// bCopyProtect
	},
	{	// Payload-Specific Frame (#1, 1280x480 30fps)
		sizeof(UVC_UNCOMPRESSED_FRAME_DESC),
											// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VS_FRAME_UNCOMPRESSED,			// bDescriptorSubType
		0x01,								// bFrameIndex
		0x00,								// bmCapabilities
		1280,								// wWidth
		480,								// wHeight
		0x11940000,							// dwMinBitRate
		0x11940000,							// dwMaxBitRate
		1228800,							// dwMaxVideoFrameBufferSize
		333333,								// dwDefaultFrameInterval
		0x01,								// bFrameIntervalType
		333333								// dwFrameInterval
	},}, // end of CS VS descriptors
	{	// Standard VS Isochronous Video Data Endpoint Descriptor
		sizeof(USB_STD_EP_DESC),			// bLength
		USB_TYPE_ENDPOINT_CFG_DESC,			// bDescriptorType
		0x83,								// bEndpointAddress
		USB_EP_BULK,						// bmAttributes
		0x00,								// wMaxPacketSizeL
		0x02,								// wMaxPacketSizeH
		0x01								// bInterval
	}
};

//=============================================================================
// Configuration Descriptors for USB 3.x
//=============================================================================
USB30_CONFIG __attribute__ ((aligned(16))) config3 = {
	{	// Configuration Descriptor
		sizeof(USB_STD_CFG_DESC),			// bLength
		USB_TYPE_CONFIG_DESC,				// bDescriptorType
		sizeof(USB30_CONFIG),				// wTotalLength
		0x02,								// bNumInterfaces
		0x01,								// bConfigurationValue
		0x00,								// iConfiguration
		0xC0,								// bmAttribute
		0x00								// bMaxPower
	},
	{	// Interface Association Descriptor
		sizeof(USB_IF_ASC_DESC),			// bLength
		USB_TYPE_INTERFACE_ASSOCIATION,		// bDescriptorType
		0x00,								// bFirstInstance
		0x02,								// bInterfaceCount
		UVC_CC_VIDEO,						// bFunctionClass
		UVC_SC_VIDEO_INTERFACE_COLLECTION,	// bFunctionSubClass
		UVC_PC_PROTOCOL_UNDEFINED,			// bFunctionProtocol
		0x02								// iFunction
	},
	{	// Standard VC Interface #0 (VideoCtrol)
		sizeof(USB_STD_IF_DESC),			// bLength
		USB_TYPE_INTERFACE_DESC,			// bDescriptorType
		0x00,								// bInterfaceNumber
		0x00,								// bAlternateSetting
		0x01,								// bNumEndpoints
		UVC_CC_VIDEO,						// bInterfaceClass
		UVC_SC_VIDEOCONTROL,				// bInterfaceSubClass
		UVC_PC_PROTOCOL_UNDEFINED,			// bInterfaceProtocol
		0x02								// iInterface
	},
	{{	// Class-specific VC Interface
		sizeof(UVC_IF_DESC),				// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VC_HEADER,						// bDescriptorSubType
		0x0100,								// bcdUVC
		sizeof(USB30_CS_VC),				// wTotalLength
		100000000,							// dwClockFrequency
		0x01,								// bInCollection
		0x01								// baInterfaceNr(1)
	},
	{	// Input Terminal Descriptor (Camera)
		sizeof(UVC_CAMERA_TERM_DESC),		// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VC_INPUT_TERMINAL,				// bDescriptorSubType
		0x01,								// bTerminalID
		UVC_ITT_CAMERA,						// wTerminalType
		0x00,								// bAssocTerminal
		0x00,								// iTerminal
		0x0000,								// wObjectiveFocalLengthMin
		0x0000,								// wObjectiveFocalLengthMax
		0x0000,								// wOcularFocalLength
		0x03,								// bControlSize
		0x00,								// bmControls_L
		0x00,								// bmControls_M
		0x00								// bmControls_H
	},
	{	// Processing Unit Descriptor
		sizeof(UVC_PROCESSING_UNIT_DESC2),	// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VC_PROCESSING_UNIT,				// bDescriptorSubType
		0x02,								// bUnitID
		0x01,								// bSourceID
		0x4000,								// wMaxMultiplier
		0x03,								// bControlSize
		0x00,								// bmControls_L
		0x00,								// bmControls_M
		0x00,								// bmControls_H
		0x00,								// iProcessing
		0x00								// bmVideoStandards
	},
	{	// Output Terminal Descriptor
		sizeof(UVC_OUTPUT_TERM_DESC),		// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VC_OUTPUT_TERMINAL,				// bDescriptorSubType
		0x03,								// bTerminalID
		UVC_TT_STREAMING,					// wTerminalType
		0x00,								// bAssocTerminal
		0x02,								// bSourceID
		0x00								// iTerminal
	},}, // end of CS VC descriptors
	{	// Standard Interrupt Endpoint Descriptor
		sizeof(USB_STD_EP_DESC),			// bLength
		USB_TYPE_ENDPOINT_CFG_DESC,			// bDescriptorType
		0x82,								// bEndpointAddress
		USB_EP_INTERRUPT,					// bmAttributes
		0x40,								// wMaxPacketSizeL
		0x00,								// wMaxPacketSizeH
		0x01								// bInterval
	},
	{	// SuperSpeed Endpoint Companion Descriptor
		sizeof(USB_STD_EP_SS_COMP_DESC),	// bLength
		0x30,								// bDescriptorType
		0x00,								// bMaxBurst
		0x00,								// bmAttributes
		0x0040								// wBytesPerInterval
	},
	{	// Class-Specific Interrupt Endpoint Descriptor
		sizeof(UVC_INTR_EP_DESC),			// bLength
		UVC_CS_ENDPOINT,					// bDescriptorType
		USB_EP_INTERRUPT,					// bDescriptorSubType
		0x0040								// wMaxTransferSize
	},
	{	// Standard VS Interface #1 (Alt0)
		sizeof(USB_STD_IF_DESC),			// bLength
		USB_TYPE_INTERFACE_DESC,			// bDescriptorType
		0x01,								// bInterfaceNumber
		0x00,								// bAlternateSetting
		0x01,								// bNumEndpoints
		UVC_CC_VIDEO,						// bInterfaceClass
		UVC_SC_VIDEOSTREAMING,				// bInterfaceSubClass
		UVC_PC_PROTOCOL_UNDEFINED,			// bInterfaceProtocol
		0x00								// iInterface
	},
	{{	// Class-Specific VS Header (Input)
		sizeof(UVC_INPUT_HEADER_DESC),		// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VS_INPUT_HEADER,				// bDescriptorSubType
		0x01,								// bNumFormats
		sizeof(USB30_CS_VS),				// wTotalLength
		0x83,								// bEndpointAddress
		0x00,								// bmInfo
		0x03,								// bTerminalLink
		0x00,								// bStillCaptureMethod
		0x00,								// bTriggerSupport
		0x00,								// bTriggerUsage
		0x01,								// bControlSize
		0x00								// bmaControls
	},
	{	// Payload-Specific VS Format (#1, Uncompressed)
		sizeof(UVC_UNCOMPRESSED_FORMAT_DESC),
											// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VS_FORMAT_UNCOMPRESSED,			// bDescriptorSubType
		0x01,								// bFormatIndex
		0x01,								// bNumFrameDescriptors
		0x32595559,							// guidFormat_LL
		0x00100000,							// guidFormat_LH
		0xaa000080,							// guidFormat_HL
		0x719b3800,							// guidFormat_HH
		0x10,								// bBitsPerPixel
		0x01,								// bDefaultFrameIndex
		0x00,								// bAspectRatioX
		0x00,								// bAspectRatioY
		0x00,								// bmInterlaceFlags
		0x00								// bCopyProtect
	},
	{	// Payload-Specific Frame (#1, 1280x480 30fps)
		sizeof(UVC_UNCOMPRESSED_FRAME_DESC),// bLength
		UVC_CS_INTERFACE,					// bDescriptorType
		UVC_VS_FRAME_UNCOMPRESSED,			// bDescriptorSubType
		0x01,								// bFrameIndex
		0x00,								// bmCapabilities
		1280,								// wWidth
		480,								// wHeight
		294912000,							// dwMinBitRate
		294912000,							// dwMaxBitRate
		1228800,							// dwMaxVideoFrameBufferSize
		333333,								// dwDefaultFrameInterval
		0x01,								// bFrameIntervalType
		333333								// dwFrameInterval
	},}, // end of CS VS descriptors
	{	// Standard VS Isochronous Video Data Endpoint Descriptor
		sizeof(USB_STD_EP_DESC),			// bLength
		USB_TYPE_ENDPOINT_CFG_DESC,			// bDescriptorType
		0x83,								// bEndpointAddress
		USB_EP_BULK,						// bmAttributes
		0x00,								// wMaxPacketSizeL
		0x04,								// wMaxPacketSizeH
		0x00								// bInterval
	},
	{	// SuperSpeed Endpoint Companion Descriptor
		sizeof(USB_STD_EP_SS_COMP_DESC),	// bLength
		0x30,								// bDescriptorType
		0x0F,								// bMaxBurst
		0x00,								// bmAttributes
		0x0000								// wBytesPerInterval
	}
};

//=============================================================================
// String Descriptors
//=============================================================================
static u8 StringList[2][6][128] = {
	{
		"Nu-Gate Technology",
		"U96-SVM Stereo Vision Front-end 2.0",
	},
	{
		"Nu-Gate Technology",
		"U96-SVM Stereo Vision Front-end 3.0",
	}
};

//=============================================================================
//! USB Chapter9 Device Descriptor Reply
//-----------------------------------------------------------------------------
//! @param *InstancePtr	Pointer to Usb_DevData instance
//! @param *BufPtr		Pointer to the buffer that is to be filled with
//! the descriptor
//! @param BufLen		Size of the provided buffer
//-----------------------------------------------------------------------------
//! @return Length of the descriptor in the buffer on success. 0 on error.
//-----------------------------------------------------------------------------
//! @brief This function returns the device descriptor for the device.
//=============================================================================
u32 Usb_Ch9SetupDevDescReply (
	struct Usb_DevData *InstancePtr,
	u8 *BufPtr,
	u32 BufLen
){
	u8 Index;
	s32 Status;

	Status = IsSuperSpeed(InstancePtr);
	if(Status != XST_SUCCESS) {
		// USB 2.0
		Index = 0;
	} else {
		// USB 3.0
		Index = 1;
	}

	// Check buffer pointer is there and buffer is big enough.
	if (!BufPtr) {
		return 0;
	}

	if (BufLen < sizeof(USB_STD_DEV_DESC)) {
		return 0;
	}

	memcpy(BufPtr, &deviceDesc[Index], sizeof(USB_STD_DEV_DESC));

	return sizeof(USB_STD_DEV_DESC);
}

//=============================================================================
//! USB Chapter9 Configuration Descriptor Reply
//-----------------------------------------------------------------------------
//! @param *InstancePtr	Pointer to Usb_DevData instance
//! @param *BufPtr		Pointer to the buffer that is to be filled with
//! the descriptor
//! @param BufLen		Size of the provided buffer
//-----------------------------------------------------------------------------
//! @return Length of the descriptor in the buffer on success. 0 on error.
//-----------------------------------------------------------------------------
//! @brief This function returns the configuration descriptor for the device.
//=============================================================================
u32 Usb_Ch9SetupCfgDescReply (
	struct Usb_DevData *InstancePtr,
	u8 *BufPtr,
	u32 BufLen
){
	s32 Status;
	u32 CfgDescLen;

	Status = IsSuperSpeed(InstancePtr);
	if(Status != XST_SUCCESS) {
		// USB 2.0
		usb_config = (u8*)&config2;
		CfgDescLen = sizeof(USB_CONFIG);
	} else {
		// USB 3.0
		usb_config = (u8*)&config3;
		CfgDescLen = sizeof(USB30_CONFIG);
	}

	// Check buffer pointer is OK and buffer is big enough.
	if (!BufPtr) {
		return 0;
	}

	if (BufLen < sizeof(USB_STD_CFG_DESC)) {
		return 0;
	}

	memcpy(BufPtr, usb_config, CfgDescLen);

	return CfgDescLen;
}

//=============================================================================
//! USB Chapter9 String Descriptor Reply
//-----------------------------------------------------------------------------
//! @param *InstancePtr	Pointer to Usb_DevData instance
//! @param *BufPtr		Pointer to the buffer that is to be filled with
//!       				the descriptor
//! @param BufLen		Size of the provided buffer
//! @param Index		Index of the string for which the descriptor is
//!        				requested.
//-----------------------------------------------------------------------------
//! @return Length of the descriptor in the buffer on success. 0 on error.
//-----------------------------------------------------------------------------
//! @brief This function returns a string descriptor for the given index.
//=============================================================================
u32 Usb_Ch9SetupStrDescReply (
	struct Usb_DevData *InstancePtr,
	u8 *BufPtr,
	u32 BufLen,
	u8 Index
){
	u32 i;
	char *String;
	u32 StringLen;
	u32 DescLen;
	u8 TmpBuf[128];
	s32 Status;

	USB_STD_STRING_DESC *StringDesc;
	u8 StrArray;

	Status = IsSuperSpeed(InstancePtr);
	if(Status != XST_SUCCESS) {
		// USB 2.0
		StrArray = 0;
	} else {
		// USB 3.0
		StrArray = 1;
	}

	if (!BufPtr) {
		return 0;
	}

	StringDesc = (USB_STD_STRING_DESC *) TmpBuf;

	// String descriptor 0 specifies languages supported by the device.
	if (0 == Index) {
		StringDesc->bLength = 4;
		StringDesc->bDescriptorType = 0x03;
		StringDesc->wLANGID[0] = 0x0409; // English (United States)
	}
	// All other strings can be pulled from the table above.
	else {
		//String = (char *)&StringList[StrArray][Index];
		String = (char *)&StringList[StrArray][Index - 1];
		StringLen = strlen(String);
		StringDesc->bLength = StringLen * 2 + 2;
		StringDesc->bDescriptorType = 0x03;

		for (i = 0; i < StringLen; i++) {
			StringDesc->wLANGID[i] = (u16) String[i];
		}
	}
	DescLen = StringDesc->bLength;

	// Check if the provided buffer is big enough to hold the descriptor.
	if (DescLen > BufLen) {
		return 0;
	}

	memcpy(BufPtr, StringDesc, DescLen);

	return DescLen;
}

//=============================================================================
//! USB Chapter9 BOS Descriptor Reply
//-----------------------------------------------------------------------------
//! @param *BufPtr		Pointer to the buffer that is to be filled with
//!       				the descriptor
//! @param BufLen		Size of the provided buffer
//-----------------------------------------------------------------------------
//! @return Length of the descriptor in the buffer on success. 0 on error.
//-----------------------------------------------------------------------------
//! @brief This function returns the BOS descriptor for the device.
//=============================================================================
u32 Usb_Ch9SetupBosDescReply(u8 *BufPtr, u32 BufLen)
{
	static USB_BOS_DESC __attribute__ ((aligned(16))) bosDesc = {
		// BOS descriptor
		{
			sizeof(USB_STD_BOS_DESC),	// bLength
			USB_TYPE_BOS_DESC,			// DescriptorType
			sizeof(USB_BOS_DESC),		// wTotalLength
			0x02						// bNumDeviceCaps
		},
		{
			sizeof(USB_STD_DEVICE_CAP_7BYTE),	// bLength
			0x10,		// bDescriptorType
			0x02,		// bDevCapabiltyType
			0x06		// bmAttributes
		},
		{
			sizeof(USB_STD_DEVICE_CAP_10BYTE),	// bLength
			0x10,		// bDescriptorType
			0x03,		// bDevCapabiltyType
			0x00,		// bmAttributes
			(0x000F),	// wSpeedsSupported
			0x01,		// bFunctionalitySupport
			0x01,		// bU1DevExitLat
			(0x01F4)	// wU2DevExitLat
		}
	};

	// Check buffer pointer is OK and buffer is big enough.
	if (!BufPtr) {
		return 0;
	}

	if (BufLen < sizeof(USB_STD_BOS_DESC)) {
		return 0;
	}

	memcpy(BufPtr, &bosDesc, sizeof(USB_BOS_DESC));

	return sizeof(USB_BOS_DESC);
}

//=============================================================================
//! USB Set Configuration
//-----------------------------------------------------------------------------
//! @param *InstancePtr	Pointer to the Usb_DevData instance
//! @param *Ctrl		Pointer to the Setup packet data
//-----------------------------------------------------------------------------
//! @return XST_SUCCESS else XST_FAILURE
//-----------------------------------------------------------------------------
//! @brief Changes State of Core to USB configured State.
//=============================================================================
s32 Usb_SetConfiguration(struct Usb_DevData *InstancePtr, SetupPacket *Ctrl)
{
	u8 State;
	s32 Ret = XST_SUCCESS;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Ctrl != NULL);

	State = InstancePtr->State;
	SetConfigDone(InstancePtr->PrivateData, 0U);

	switch (State) {
		case USB_STATE_DEFAULT:
			Ret = XST_FAILURE;
			break;

		case USB_STATE_ADDRESS:
			InstancePtr->State = USB_STATE_CONFIGURED;
			break;

		case USB_STATE_CONFIGURED:
			break;

		default:
			Ret = XST_FAILURE;
			break;
	}

	return Ret;
}

//=============================================================================
//! USB Set Configuration App
//-----------------------------------------------------------------------------
//! @param *InstancePtr	Pointer to the Usb_DevData instance
//! @param *SetupData	Setup packet received from Host
//-----------------------------------------------------------------------------
//! @return XST_SUCCESS else XST_FAILURE
//-----------------------------------------------------------------------------
//! @brief This function is called by Chapter9 handler when SET_CONFIGURATION
//! command is received from Host.
//-----------------------------------------------------------------------------
//! @note Non control endpoints must be enabled after SET_CONFIGURATION
//! command since hardware clears all previously enabled endpoints
//! except control endpoints when this command is received.
//=============================================================================
s32 Usb_SetConfigurationApp (
	struct Usb_DevData *InstancePtr,
	SetupPacket *SetupData
){
	s32 Status;
	u16 MaxPktSize;

	u32 RegVal = XUsbPsu_ReadReg((struct XUsbPsu*)InstancePtr->PrivateData, XUSBPSU_DCFG);
	int addr = (RegVal >> 3) & 0x7F;
	xil_printf("USB address assigned %d\r\n", addr);
	xil_printf("USB configuration %d\r\n", SetupData->wValue && 0xff);

	if(InstancePtr->Speed == USB_SPEED_SUPER) {
		MaxPktSize = 1024;
	} else {
		MaxPktSize = 512;
	}

	Status = Uvc_InitState(InstancePtr, usb_config);
	if (Status != XST_SUCCESS) {
		xil_printf("Uvc_InitState failed\r\n");
		return XST_FAILURE;
	}

	// When we run CV test suite application in Windows, need to
	// add SET_CONFIGURATION command with value 0/1 to pass test suite
	if ((SetupData->wValue && 0xff) ==  1) {
		// SET_CONFIGURATION with value 1
		// Endpoint enables - not needed for Control EP
		Status = EpEnable (
			InstancePtr->PrivateData, 2, USB_EP_DIR_IN, MaxPktSize, USB_EP_TYPE_INTERRUPT
		);
		if (Status != XST_SUCCESS) {
			xil_printf("failed to enable INTERRUPT IN Ep\r\n");
			return XST_FAILURE;
		}
		Status = EpEnable (
			InstancePtr->PrivateData, 3, USB_EP_DIR_IN, MaxPktSize, USB_EP_TYPE_BULK
		);
		if (Status != XST_SUCCESS) {
			xil_printf("failed to enable BULK IN Ep\r\n");
			return XST_FAILURE;
		}

		SetConfigDone(InstancePtr->PrivateData, 1U);
	} else {
		// SET_CONFIGURATION with value 0

		// Endpoint disables - not needed for Control EP
		Status = EpDisable(InstancePtr->PrivateData, 2, USB_EP_DIR_IN);
		if (Status != XST_SUCCESS) {
			xil_printf("failed to disable INTERRUPT IN Ep\r\n");
			return XST_FAILURE;
		}
		Status = EpDisable(InstancePtr->PrivateData, 3, USB_EP_DIR_IN);
		if (Status != XST_SUCCESS) {
			xil_printf("failed to disable BULK IN Ep\r\n");
			return XST_FAILURE;
		}

		SetConfigDone(InstancePtr->PrivateData, 0U);
	}

	return XST_SUCCESS;
}

//=============================================================================
//! USB Set Interface Handler
//-----------------------------------------------------------------------------
//! @param *InstancePtr	Pointer to the Usb_DevData instance
//! @param *SetupData	Setup packet received from Host
//-----------------------------------------------------------------------------
//! @brief This function is called by Chapter9 handler when SET_INTERFACE
//! command is received from Host.
//=============================================================================
void Usb_SetInterfaceHandler(
	struct Usb_DevData *InstancePtr,
	SetupPacket *SetupData
) {
	struct UVC_APP_DATA *app_data = Uvc_GetAppData(InstancePtr);

	app_data->cur_if = SetupData->wIndex;
	app_data->alt_if = SetupData->wValue;

	xil_printf("USB Set Interface %d, Alt %d\r\n", app_data->cur_if, app_data->alt_if);

	return;
}

//=============================================================================
//! USB Class Specific Request
//-----------------------------------------------------------------------------
//! @param *InstancePtr	Pointer to the Usb_DevData instance
//! @param *SetupData	Setup packet received from Host
//-----------------------------------------------------------------------------
//! @brief This function is called when UVC class specific request is received.
//=============================================================================
void Usb_ClassReq (
	struct Usb_DevData *InstancePtr,
	SetupPacket *SetupData
) {
	s32 Status;
	u8 *config;

	u8 term_id = (SetupData->wIndex & 0xFF00) >> 8; // Terminal ID
	u8 intf_num  =  SetupData->wIndex & 0x00FF; // Interface number
	u16 term_type;
	u8 ctrl_sel = (u8)((SetupData->wValue & 0xFF00) >> 8);

	// Meaning of wValue depends on what entity is addressed.
	// TermID|   IF#| Request Type
	//      0|     0| Video Control
	//      0|  else| Video Streaming
	//   else|     X| Unit and Terminal Control
	if (term_id == 0x0000) {
		if (intf_num == 0x0000) {
			// VideoControl Requests
			switch (ctrl_sel) {
			case UVC_VC_VIDEO_POWER_MODE_CONTROL:
			case UVC_VC_REQUEST_ERROR_CODE_CONTROL:
				// Unhandled VideoControl request
				Usb_UnhandledRequest(SetupData);
				EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
				break;
			default:
				// Unknown VideoControl request
				Usb_UnknownRequest(SetupData);
				EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
				break;
			}
		} else {
			// VideoStreaming Requests
			switch (ctrl_sel) {
			case UVC_VS_PROBE_CONTROL:
				Uvc_ProbeControl(InstancePtr, SetupData);
				break;
			case UVC_VS_COMMIT_CONTROL:
				Uvc_CommitControl(InstancePtr, SetupData);
				break;
			case UVC_VS_STILL_PROBE_CONTROL:
			case UVC_VS_STILL_COMMIT_CONTROL:
			case UVC_VS_STILL_IMAGE_TRIGGER_CONTROL:
			case UVC_VS_STREAM_ERROR_CODE_CONTROL:
			case UVC_VS_GENERATE_KEY_FRAME_CONTROL:
			case UVC_VS_UPDATE_FRAME_SEGMENT_CONTROL:
			case UVC_VS_SYNCH_DELAY_CONTROL:
				// Unhandled VideoStreaming request
				Usb_UnhandledRequest(SetupData);
				EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
				break;
			default:
				// Unknown VideoStreaming request
				Usb_UnknownRequest(SetupData);
				EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
				break;
			}
		}
	} else {
		// configuration descriptors
		Status = IsSuperSpeed(InstancePtr);
		if(Status != XST_SUCCESS) {
			config = (u8*)&config2;
		} else {
			config = (u8*)&config3;
		}

		// Unit and Terminal Control Requests
		u8 subtype = Uvc_GetTermtype(config, term_id, &term_type);
		switch (subtype) {
		case UVC_VC_INPUT_TERMINAL:
			if (term_type == UVC_ITT_CAMERA) {
				//Camera Terminal Control Requests
				switch (ctrl_sel) {
				case UVC_CT_SCANNING_MODE_CONTROL:
				case UVC_CT_AE_MODE_CONTROL:
				case UVC_CT_AE_PRIORITY_CONTROL:
				case UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
				case UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL:
				case UVC_CT_FOCUS_ABSOLUTE_CONTROL:
				case UVC_CT_FOCUS_RELATIVE_CONTROL:
				case UVC_CT_FOCUS_AUTO_CONTROL:
				case UVC_CT_IRIS_ABSOLUTE_CONTROL:
				case UVC_CT_IRIS_RELATIVE_CONTROL:
				case UVC_CT_ZOOM_ABSOLUTE_CONTROL:
				case UVC_CT_ZOOM_RELATIVE_CONTROL:
				case UVC_CT_PANTILT_ABSOLUTE_CONTROL:
				case UVC_CT_PANTILT_RELATIVE_CONTROL:
				case UVC_CT_ROLL_ABSOLUTE_CONTROL:
				case UVC_CT_ROLL_RELATIVE_CONTROL:
				case UVC_CT_PRIVACY_CONTROL:
				case UVC_CT_FOCUS_SIMPLE_CONTROL:
				case UVC_CT_WINDOW_CONTROL:
				case UVC_CT_REGION_OF_INTEREST_CONTROL:
					// Unhandled Camera Terminal Control Request
					Usb_UnhandledRequest(SetupData);
					EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
					break;
				default:
					// Unknown Camera Terminal Control Request
					Usb_UnknownRequest(SetupData);
					EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
					break;
				}
			}
			break;
		case UVC_VC_OUTPUT_TERMINAL:
			// Unknown Output Terminal Control Request
			Usb_UnknownRequest(SetupData);
			EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
			break;
		case UVC_VC_SELECTOR_UNIT:
			// Selector Unit Control Requests
			switch (ctrl_sel) {
			case UVC_SU_INPUT_SELECT_CONTROL:
				// Unhandled Selector Unit Control Request
				Usb_UnhandledRequest(SetupData);
				EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
				break;
			default:
				// Unknown Selector Unit Control Request
				Usb_UnknownRequest(SetupData);
				EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
				break;
			}
			break;
		case UVC_VC_PROCESSING_UNIT:
			// Processing Unit Control Requests
			switch (ctrl_sel) {
			case UVC_PU_BRIGHTNESS_CONTROL:
			case UVC_PU_BACKLIGHTCOMPENSATION_CONTROL:
			case UVC_PU_CONTRAST_CONTROL:
			case UVC_PU_GAIN_CONTROL:
			case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
			case UVC_PU_HUE_CONTROL:
			case UVC_PU_SATURATION_CONTROL:
			case UVC_PU_SHARPNESS_CONTROL:
			case UVC_PU_GAMMA_CONTROL:
			case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
			case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
			case UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL:
			case UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL:
			case UVC_PU_DIGITAL_MULTIPLIER_CONTROL:
			case UVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL:
			case UVC_PU_HUE_AUTO_CONTROL:
			case UVC_PU_ANALOG_VIDEO_STANDARD_CONTROL:
			case UVC_PU_ANALOG_LOCK_STATUS_CONTROL:
			case UVC_PU_CONTRAST_AUTO_CONTROL:
				// Unhandled Processing Unit Control Request
				Usb_UnhandledRequest(SetupData);
				EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
				break;
			default:
				// Unknown Processing Unit Control Request
				Usb_UnknownRequest(SetupData);
				EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
				break;
			}
			break;
		case UVC_VC_EXTENSION_UNIT:
			// Unknown Extension Unit Request
			Usb_UnknownRequest(SetupData);
			EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
			break;
		case UVC_VC_ENCODING_UNIT:
			// Encoding Unit Requests
			switch (ctrl_sel) {
			case UVC_EU_SELECT_LAYER_CONTROL:
			case UVC_EU_PROFILE_TOOLSET_CONTROL:
			case UVC_EU_VIDEO_RESOLUTION_CONTROL:
			case UVC_EU_MIN_FRAME_INTERVAL_CONTROL:
			case UVC_EU_SLICE_MODE_CONTROL:
			case UVC_EU_RATE_CONTROL_MODE_CONTROL:
			case UVC_EU_AVERAGE_BITRATE_CONTROL:
			case UVC_EU_CPB_SIZE_CONTROL:
			case UVC_EU_PEAK_BIT_RATE_CONTROL:
			case UVC_EU_QUANTIZATION_PARAMS_CONTROL:
			case UVC_EU_SYNC_REF_FRAME_CONTROL:
			case UVC_EU_LTR_BUFFER_CONTROL:
			case UVC_EU_LTR_PICTURE_CONTROL:
			case UVC_EU_LTR_VALIDATION_CONTROL:
			case UVC_EU_LEVEL_IDC_LIMIT_CONTROL:
			case UVC_EU_SEI_PAYLOADTYPE_CONTROL:
			case UVC_EU_QP_RANGE_CONTROL:
			case UVC_EU_PRIORITY_CONTROL:
			case UVC_EU_START_OR_STOP_LAYER_CONTROL:
			case UVC_EU_ERROR_RESILIENCY_CONTROL:
				// Unhandled Encoding Unit Request
				Usb_UnhandledRequest(SetupData);
				EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
				break;
			default:
				// Unknown Encoding Unit Request
				Usb_UnknownRequest(SetupData);
				EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
				break;
			}
			break;
		default:
			// no terminal or unit with specified ID
			Usb_UnknownRequest(SetupData);
			EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
			break;
		}
	}

	return;
}

//=============================================================================
//! UVC Get Terminal Type
//-----------------------------------------------------------------------------
//! @param *config		Pointer to the configuration descriptor tree
//! @param id			ID of terminal or unit to be searched
//! @param *term_type	wTerminalType of found terminal or unit
//-----------------------------------------------------------------------------
//! @return Subtype of the terminal or unit with given ID. Returns 0 when not
//! found.
//-----------------------------------------------------------------------------
//! @brief This function searches for terminal or unit with the given ID,
//! returns it's subtype when found.
//=============================================================================
u8 Uvc_GetTermtype (u8* config, u8 id, u16* term_type) {

	u16 tot_len;
	tot_len  = (u16)(*(config + 2)); // lower byte of wTotalLength
	tot_len += (u16)(*(config + 3) << 8); // higher byte of wTotalLength
	u8 *ptr = config;

	while (ptr < (u8*)(config + tot_len)) {
		u8  len     = ( u8)(*ptr);			// bLength
		u8  type    = ( u8)(*(ptr + 1));	// bDescriptorType
		u8  subtype = ( u8)(*(ptr + 2));	// bDescriptorSubType
		u8  term_id = ( u8)(*(ptr + 3));	// bTerminalID
		*term_type  = (u16)(*(ptr + 4));	// wTerminalType
		*term_type += (u16)(*(ptr + 5) << 8);

		// check if this descriptor belongs to terminal or unit,
		// and terminal ID is matched with the specified ID.
		if ((type == UVC_CS_INTERFACE) && (term_id == id)) {
			switch (subtype) {
			case UVC_VC_INPUT_TERMINAL:
			case UVC_VC_OUTPUT_TERMINAL:
			case UVC_VC_SELECTOR_UNIT:
			case UVC_VC_PROCESSING_UNIT:
			case UVC_VC_EXTENSION_UNIT:
			case UVC_VC_ENCODING_UNIT:
				return subtype;
			default:
				continue;
			}
		}

		// next descriptor
		ptr += len;
	}

	// specified terminal not found
	return 0;
}

//=============================================================================
//! UVC Get Frame Descriptor
//-----------------------------------------------------------------------------
//! @param *config		Pointer to the configuration descriptor tree
//! @param format_id	Format ID of format descriptor to be searched
//! @param frame_id		Frame ID of frame descriptor to be searched
//-----------------------------------------------------------------------------
//! @return Pointer to the frame descriptor with given format ID and frame ID.
//! 0 if not found.
//-----------------------------------------------------------------------------
//! @brief This function searches descriptor tree for frame descriptor with
//! specified frame/format index.
//=============================================================================
UVC_UNCOMPRESSED_FRAME_DESC* Uvc_GetFrameDesc (u8* config, u8 format_id, u8 frame_id) {

	u16 tot_len;
	tot_len  = (u16)(*(config + 2)); // lower byte of wTotalLength
	tot_len += (u16)(*(config + 3) << 8); // higher byte of wTotalLength
	u8 *ptr = config;
	int format_found = 0;

	while (ptr < (u8*)(config + tot_len)) {
		u8 len          = (u8)(*ptr);		// bLength
		u8 type         = (u8)(*(ptr + 1));	// bDescriptorType
		u8 subtype      = (u8)(*(ptr + 2));	// bDescriptorSubType
		u8 format_index = (u8)(*(ptr + 3));	// bFormatIndex/bFrameIndex

		if ((type == UVC_CS_INTERFACE) && (subtype == UVC_VS_FORMAT_UNCOMPRESSED)) {
			if (format_index == format_id) {
				// format descriptor with given format index is found
				format_found = 1;
			}
			else {
				format_found = 0;
			}
		}
		else if ((type == UVC_CS_INTERFACE) && (subtype == UVC_VS_FRAME_UNCOMPRESSED)) {
			if (format_found && (format_index == frame_id)) {
				// frame descriptor with given format/frame index is found
				return (UVC_UNCOMPRESSED_FRAME_DESC*)ptr;
			}
		}

		// next descriptor
		ptr += len;
	}

	// specified frame descriptor not found
	return 0;
}

//=============================================================================
//! UVC Probe Control
//-----------------------------------------------------------------------------
//! @param *InstancePtr	Pointer to the Usb_DevData instance
//! @param *SetupData	Setup packet received from Host
//-----------------------------------------------------------------------------
//! @brief Probe control request handler.
//=============================================================================
void Uvc_ProbeControl(struct Usb_DevData *InstancePtr, SetupPacket *SetupData) {
	u8 buf[sizeof(UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC)] ALIGNMENT_CACHELINE;

	struct UVC_APP_DATA *app_data = Uvc_GetAppData(InstancePtr);

	switch (SetupData->bRequest) {
	case UVC_GET_INFO:
		buf[0] = UVC_GET_INFO_SUPPORT_SET + UVC_GET_INFO_SUPPORT_GET;
		EpBufferSend(InstancePtr->PrivateData, 0, buf, 1);
		break;
	case UVC_GET_CUR:
		EpBufferSend(InstancePtr->PrivateData, 0, (u8*)&app_data->probe, SetupData->wLength);
		break;
	case UVC_SET_CUR:
		xil_printf("UVC video probe\r\n");
		s32 Status = XUsbPsu_Ep0Recv((struct XUsbPsu*)InstancePtr->PrivateData, buf, SetupData->wLength);
		Xil_AssertVoid(Status == XST_SUCCESS);

		// necessary when DCache is enabled.
		Xil_DCacheInvalidateRange((UINTPTR)buf, SetupData->wLength);
		usleep(100);

		//  Update video stream parameters
		if (Uvc_UpdateParam (
			InstancePtr,
			(UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC*)buf
		) != 0) {
			xil_printf("Error in Updated parameters\r\n");
		}

		UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC *probe = &app_data->probe;
		xil_printf("  bmHint         : %X\r\n", probe->bmHint);
		xil_printf("  bFormatIndex   : %d\r\n", probe->bFormatIndex);
		xil_printf("  bFrameIndex    : %d\r\n", probe->bFrameIndex);
		xil_printf("  dwFrameInterval: %d\r\n", probe->dwFrameInterval);
		xil_printf("  wKeyFrameRate  : %d\r\n", probe->wKeyFrameRate);
		xil_printf("  wPFrameRate    : %d\r\n", probe->wPFrameRate);
		xil_printf("  wCompQuality   : %d\r\n", probe->wCompQuality);
		xil_printf("  wCompWindow    : %d\r\n", probe->wCompWindow);
		xil_printf("  wDelay         : %d\r\n", probe->wDelay);
		xil_printf("  dwMaxVideoFrameSize     : %d\r\n", probe->dwMaxVideoFrameSize);
		xil_printf("  dwMaxPayloadTransferSize: %d\r\n", probe->dwMaxPayloadTransferSize);

		EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);

		break;
	case UVC_GET_MAX:
		EpBufferSend(InstancePtr->PrivateData, 0, (u8*)&app_data->max, SetupData->wLength);
		break;
	case UVC_GET_MIN:
		EpBufferSend(InstancePtr->PrivateData, 0, (u8*)&app_data->min, SetupData->wLength);
		break;
	case UVC_GET_RES:
	case UVC_GET_DEF:
		// unhandled request
		Usb_UnhandledRequest(SetupData);
		EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
		break;
	default:
		// unsupported request
		Usb_UnknownRequest(SetupData);
		EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
		break;
	}
}

//=============================================================================
//! UVC Update Parameters
//-----------------------------------------------------------------------------
//! @param *InstancePtr	Pointer to the Usb_DevData instance
//! @param *new			Pointer to the video probe data structure that contains
//!        				video streaming parameters requested by the host.
//-----------------------------------------------------------------------------
//! @return "1" if data has erroneous values, "0" no error.
//-----------------------------------------------------------------------------
//! @brief This function updates video streaming parameters with host requested
//! values.
//=============================================================================
int Uvc_UpdateParam (
	struct Usb_DevData *InstancePtr,
	UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC *new
) {
	struct UVC_APP_DATA *app_data = Uvc_GetAppData(InstancePtr);

	UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC *max =  &app_data->max;
	UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC *min = &app_data->min;
	UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC *probe = &app_data->probe;

	// update only non-zero values
	int error = 0;
	if (new->bmHint != 0) {
		probe->bmHint = new->bmHint;
	}
	if (new->bFormatIndex != 0) {
		if ((new->bFormatIndex < min->bFormatIndex) || (max->bFormatIndex < new->bFormatIndex)) {
			error = 1;
		} else {
			probe->bFormatIndex = new->bFormatIndex;
		}
	}
	if (new->bFrameIndex != 0) {
		if ((new->bFrameIndex < min->bFrameIndex) || (max->bFrameIndex < new->bFrameIndex)) {
			error = 1;
		} else {
			probe->bFrameIndex = new->bFrameIndex;
		}
	}
	if (new->dwFrameInterval != 0) {
		if ((new->dwFrameInterval < min->dwFrameInterval) || (max->dwFrameInterval < new->dwFrameInterval)) {
			error = 1;
		} else {
			probe->dwFrameInterval = new->dwFrameInterval;
		}
	}
	if (new->wKeyFrameRate != 0) {
		if ((new->wKeyFrameRate < min->wKeyFrameRate) || (max->wKeyFrameRate < new->wKeyFrameRate)) {
			error = 1;
		} else {
			probe->wKeyFrameRate = new->wKeyFrameRate;
		}
	}
	if (new->wPFrameRate != 0) {
		if ((new->wPFrameRate < min->wPFrameRate) || (max->wPFrameRate < new->wPFrameRate)) {
			error = 1;
		} else {
			probe->wPFrameRate = new->wPFrameRate;
		}
	}
	if (new->wCompQuality != 0) {
		if ((new->wCompQuality < min->wCompQuality) || (max->wCompQuality < new->wCompQuality)) {
			error = 1;
		} else {
			probe->wCompQuality = new->wCompQuality;
		}
	}
	if (new->wCompWindow != 0) {
		if ((new->wCompWindow < min->wCompWindow) || (max->wCompWindow < new->wCompWindow)) {
			error = 1;
		} else {
			probe->wCompWindow = new->wCompWindow;
		}
	}
	if (new->wDelay != 0) {
		if ((new->wDelay < min->wDelay) || (max->wDelay < new->wDelay)) {
			error = 1;
		} else {
			probe->wDelay = new->wDelay;
		}
	}
	if (new->dwMaxVideoFrameSize != 0) {
		if ((new->dwMaxVideoFrameSize < min->dwMaxVideoFrameSize) || (max->dwMaxVideoFrameSize < new->dwMaxVideoFrameSize)) {
			error = 1;
		} else {
			probe->dwMaxVideoFrameSize = new->dwMaxVideoFrameSize;
		}
	}
	if (new->dwMaxPayloadTransferSize != 0) {
		if ((new->dwMaxPayloadTransferSize < min->dwMaxPayloadTransferSize) || (max->dwMaxPayloadTransferSize < new->dwMaxPayloadTransferSize)) {
			error = 1;
		} else {
			probe->dwMaxPayloadTransferSize = new->dwMaxPayloadTransferSize;
		}
	}

	return error;
}

//=============================================================================
//! UVC Commit Control
//-----------------------------------------------------------------------------
//! @param *InstancePtr	Pointer to the Usb_DevData instance
//! @param *SetupData	Setup packet received from Host
//-----------------------------------------------------------------------------
//! @brief Commit control request handler.
//=============================================================================
void Uvc_CommitControl(struct Usb_DevData *InstancePtr, SetupPacket *SetupData) {
	s32 Status;
	struct UVC_APP_DATA *app_data = Uvc_GetAppData(InstancePtr);

	u8 buf[sizeof(UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC)] ALIGNMENT_CACHELINE;

	switch (SetupData->bRequest) {
	case UVC_GET_INFO:
		buf[0] = UVC_GET_INFO_SUPPORT_SET + UVC_GET_INFO_SUPPORT_GET;
		EpBufferSend(InstancePtr->PrivateData, 0, buf, 1);
		break;
	case UVC_GET_CUR:
		EpBufferSend(InstancePtr->PrivateData, 0, (u8*)&app_data->commit, SetupData->wLength);
		break;
	case UVC_SET_CUR:
		xil_printf("UVC video commit\r\n");
		s32 ret = XUsbPsu_Ep0Recv((struct XUsbPsu *)InstancePtr->PrivateData, buf, SetupData->wLength);
		if (ret != XST_SUCCESS) {
			xil_printf("EpBufferRecv failed\r\n");
		}

		// necessary when DCache is enabled.
		Xil_DCacheInvalidateRange((UINTPTR)buf, SetupData->wLength);
		usleep(100);

		memcpy(&app_data->commit, &buf, sizeof(UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC));

		UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC *commit = &app_data->probe;
		xil_printf("  bmHint         : %X\r\n", commit->bmHint);
		xil_printf("  bFormatIndex   : %d\r\n", commit->bFormatIndex);
		xil_printf("  bFrameIndex    : %d\r\n", commit->bFrameIndex);
		xil_printf("  dwFrameInterval: %d\r\n", commit->dwFrameInterval);
		xil_printf("  wKeyFrameRate  : %d\r\n", commit->wKeyFrameRate);
		xil_printf("  wPFrameRate    : %d\r\n", commit->wPFrameRate);
		xil_printf("  wCompQuality   : %d\r\n", commit->wCompQuality);
		xil_printf("  wCompWindow    : %d\r\n", commit->wCompWindow);
		xil_printf("  wDelay         : %d\r\n", commit->wDelay);
		xil_printf("  dwMaxVideoFrameSize     : %d\r\n", commit->dwMaxVideoFrameSize);
		xil_printf("  dwMaxPayloadTransferSize: %d\r\n", commit->dwMaxPayloadTransferSize);

		EpBufferSend(InstancePtr->PrivateData, 0, NULL, 0);

		// video control committed. start UVC operation.
		xil_printf("UVC video streaming parameters committed\r\n");
		app_data->state = UVC_STATE_COMMIT;

		Status = Uvc_InitTxInfo(InstancePtr, usb_config);
		if (Status != XST_SUCCESS) {
			xil_printf("Uvc_InitTxInfo failed\r\n");
		}

		break;
	case UVC_GET_MAX:
	case UVC_GET_MIN:
	case UVC_GET_RES:
	case UVC_GET_DEF:
		// unhandled request
		EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
		break;
	default:
		// unsupported request
		EpSetStall(InstancePtr->PrivateData, 0, USB_EP_DIR_OUT);
		break;
	}
}

//=============================================================================
//! UVC Initialize State
//-----------------------------------------------------------------------------
//! @param *InstancePtr	Pointer to the Usb_DevData instance
//! @param *config		Pointer to the configuration descriptor tree
//-----------------------------------------------------------------------------
//! @return XST_SUCCESS else XST_FAILURE
//-----------------------------------------------------------------------------
//! @brief This function initializes UVC related data structure and it's state.
//=============================================================================
s32 Uvc_InitState (struct Usb_DevData *InstancePtr, u8* config) {

	struct UVC_APP_DATA *app_data = Uvc_GetAppData(InstancePtr);

	app_data->cur_if = 0;
	app_data->alt_if = 0;
	app_data->brightness = 0;
	app_data->bSelector = 0;
	app_data->state = 0;

	//------------------------------------------------------------------
	// Initialize Video Probe Controls
	//------------------------------------------------------------------
	// set initial negotiation values based on the values of default
	// frame descriptor
	u8 default_format_id = 1;
	u8 default_frame_id = 1;

	// retrieve a frame descriptor with given format/frame ID
	UVC_UNCOMPRESSED_FRAME_DESC* frame_desc = Uvc_GetFrameDesc (
		config,
		default_format_id,
		default_frame_id
	);
	if (frame_desc == 0) {
		// failed to find default frame descriptor
		return XST_FAILURE;
	}

	// Starting values of video streaming parameters, will be updated during negotiation
	app_data->probe.bmHint = 0x0000;
	app_data->probe.bFormatIndex = default_format_id;
	app_data->probe.bFrameIndex = default_frame_id;
	app_data->probe.dwFrameInterval = frame_desc->dwDefaultFrameInterval;
	app_data->probe.wKeyFrameRate = 0;
	app_data->probe.wPFrameRate = 0;
	app_data->probe.wCompQuality = 0;
	app_data->probe.wCompWindow = 0;
	app_data->probe.wDelay = 0;
	app_data->probe.dwMaxVideoFrameSize = frame_desc->wHeight * frame_desc->wWidth * 2;

	// max payload size is determined experimentally
	s32 Status = IsSuperSpeed(InstancePtr);
	if(Status != XST_SUCCESS) {
		// USB 2.0
		app_data->probe.dwMaxPayloadTransferSize = 3068;
	} else {
		// USB 3.0
		app_data->probe.dwMaxPayloadTransferSize = 16396;
	}

	//------------------------------------------------------------------
	// Initialize Min/Max Negotiation Values
	//------------------------------------------------------------------
	// set default values to allowable min/max (no negotiation)
	memcpy(&app_data->min, &app_data->probe, sizeof(UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC));
	memcpy(&app_data->max, &app_data->probe, sizeof(UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC));

	return XST_SUCCESS;
}

//=============================================================================
//! UVC Initialize TX Info
//-----------------------------------------------------------------------------
//! @param *InstancePtr	Pointer to the Usb_DevData instance
//! @param *config		Pointer to the configuration descriptor tree
//-----------------------------------------------------------------------------
//! @return XST_SUCCESS else XST_FAILURE
//-----------------------------------------------------------------------------
//! @brief This function initializes information regarding to USB data
//! transfer.
//=============================================================================
s32 Uvc_InitTxInfo (struct Usb_DevData *InstancePtr, u8* config) {

	struct UVC_APP_DATA *app_data = Uvc_GetAppData(InstancePtr);

	// retrieve a frame descriptor with committed format and frame ID
	UVC_UNCOMPRESSED_FRAME_DESC* frame_desc = Uvc_GetFrameDesc (
		config,
		app_data->commit.bFormatIndex,
		app_data->commit.bFrameIndex
	);
	if (frame_desc == 0) {
		// failed to find default frame descriptor
		return XST_FAILURE;
	}

	app_data->max_buf_size = app_data->commit.dwMaxPayloadTransferSize;
	app_data->header_size = sizeof(UVC_PAYLOAD_HEADER);
	app_data->max_payload_size = app_data->max_buf_size - sizeof(UVC_PAYLOAD_HEADER);
	app_data->img_received = 0;
	app_data->grb_received = 0;
	app_data->rect_received = 0;
	app_data->xsbl_received = 0;
	app_data->bm_received = 0;
	app_data->intr_received = 0;


	app_data->header.bHeaderLength = sizeof(UVC_PAYLOAD_HEADER);
	app_data->header.bmHeaderInfo = (
		UVC_PAYLOAD_HEADER_EOH |
		UVC_PAYLOAD_HEADER_SCR |
		UVC_PAYLOAD_HEADER_PTS |
		UVC_PAYLOAD_HEADER_FID
	);
	app_data->header.dwPresentationTime = 0;
	app_data->header.scrSourceClock = 0;
	app_data->header.SOFTokenCounter = 0;

	return XST_SUCCESS;
}

//=============================================================================
//! UVC Get Application Data
//-----------------------------------------------------------------------------
//! @param *InstancePtr	Pointer to the Usb_DevData instance
//-----------------------------------------------------------------------------
//! @return Pointer to the user-defined application data structure
//-----------------------------------------------------------------------------
//! @brief This function initializes information regarding to USB data
//! transfer.
//=============================================================================
struct UVC_APP_DATA* Uvc_GetAppData(struct Usb_DevData *InstancePtr) {
	// successively trace reference pointers to retrieve application
	// data structure
	struct XUsbPsu *PrivateData = (struct XUsbPsu*)InstancePtr->PrivateData;

	USBCH9_DATA *storage_data = (USBCH9_DATA*)PrivateData->data_ptr;

	struct UVC_APP_DATA *app_data = (struct UVC_APP_DATA*)storage_data->data_ptr;

	return app_data;
}

//=============================================================================
//! USB Unhandled Request
//-----------------------------------------------------------------------------
//! @param *SetupData	Setup packet received from Host
//-----------------------------------------------------------------------------
//! @brief This function is called on receipt of a request which doesn't have
//! a handler function.
//=============================================================================
void Usb_UnhandledRequest (SetupPacket *SetupData) {
	xil_printf("Unhandled request received\r\n");
	xil_printf("  bmRequestType 0x%02x\r\n", SetupData->bRequestType);
	xil_printf("  bRequest      0x%02x\r\n", SetupData->bRequest);
	xil_printf("  wValue        0x%04x\r\n", SetupData->wValue);
	xil_printf("  wIndex        0x%04x\r\n", SetupData->wIndex);
	xil_printf("  wLength       0x%04x\r\n", SetupData->wLength);
}

//=============================================================================
//! USB Unknown Request
//-----------------------------------------------------------------------------
//! @param *SetupData	Setup packet received from Host
//-----------------------------------------------------------------------------
//! @brief This function is called on receipt of an unknown request.
//=============================================================================
void Usb_UnknownRequest (SetupPacket *SetupData) {
	xil_printf("Unknown request received\r\n");
	xil_printf("  bmRequestType 0x%02x\r\n", SetupData->bRequestType);
	xil_printf("  bRequest      0x%02x\r\n", SetupData->bRequest);
	xil_printf("  wValue        0x%04x\r\n", SetupData->wValue);
	xil_printf("  wIndex        0x%04x\r\n", SetupData->wIndex);
	xil_printf("  wLength       0x%04x\r\n", SetupData->wLength);
}
