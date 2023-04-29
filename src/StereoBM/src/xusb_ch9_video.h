//=============================================================================
// Copyright (C) 2020 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
//=============================================================================
//!
//! @file xusb_ch9_video.h
//!
//! Header file for xusb_ch9_video.c
//!
//! <pre>
//! MODIFICATION HISTORY:
//!
//! Ver   Who  Date       Changes
//! ----- ---- ---------- -----------------------------------------------------
//! 1.0   sd   2020.12.07 First release
//! </pre>
//=============================================================================
#ifndef XUSB_CH9_VIDEO_H
#define XUSB_CH9_VIDEO_H

#include "xil_types.h"
#include "xstatus.h"
#include "xusb_ch9.h"

//#include "xusb_main.h"
#include "xusbpsu_endpoint.h"
#include "xusbpsu_local.h"


//=============================================================================
// Standard USB Descriptors Defined in
//  Universal Serial Bus 3.0 Specification (Rev1.0)
//  9.6 Standard USB Descriptor Definitions
//=============================================================================
// this descriptor is defined in USB 3.0 standard but not included in "xusb_ch9.h".
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bFirstInterface;
	u8 bInterfaceCount;
	u8 bFunctionClass;
	u8 bFunctionSubClass;
	u8 bFunctionProtocol;
	u8 iFunction;
} attribute(USB_IF_ASC_DESC);


//=============================================================================
// Class-Specific Descriptors Defined in
//  Universal Serial Bus Device Class Definition for Video Devices (Rev 1.5)
//  3. Descriptors
//=============================================================================
// Class-Specific VC Interface Header Descriptor
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u16 bcdUVC;
	u16 wTotalLength;
	u32 dwClockFrequency;
	u8 bInCollection;
	u8 baInterfaceNr1;
} attribute(UVC_IF_DESC);

// Input Terminal Descriptor
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bTerminalID;
	u16 wTerminalType;
	u8 bAssocTerminal;
	u8 iTerminal;
} attribute(UVC_INPUT_TERM_DESC);

// Camera Terminal Descriptor
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bTerminalID;
	u16 wTerminalType;
	u8 bAssocTerminal;
	u8 iTerminal;
	u16 wObjectiveFocalLengthMin;
	u16 wObjectiveFocalLengthMax;
	u16 wOcularFocalLength;
	u8 bControlSize;
	u8 bmControls_L;
	u8 bmControls_M;
	u8 bmControls_H;
} attribute(UVC_CAMERA_TERM_DESC);

// Output Terminal Descriptor
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bTerminalID;
	u16 wTerminalType;
	u8 bAssocTerminal;
	u8 bSourceID;
	u8 iTerminal;
} attribute(UVC_OUTPUT_TERM_DESC);

// Selector Unit Descriptor (2-input type)
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bUnitID;
	u8 bNrInPins;
	u8 baSourceID_L;
	u8 baSourceID_H;
	u8 iSelector;
} attribute(UVC_SELECTOR_UNIT_DESC);

// Processing Unit Descriptor
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bUnitID;
	u8 bSourceID;
	u16 wMaxMultiplier;
	u8 bControlSize;
	u8 bmControls_L;
	u8 bmControls_M;
	u8 bmControls_H;
	u8 iProcessing;
} attribute(UVC_PROCESSING_UNIT_DESC);

typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bUnitID;
	u8 bSourceID;
	u16 wMaxMultiplier;
	u8 bControlSize;
	u8 bmControls_L;
	u8 bmControls_M;
	u8 bmControls_H;
	u8 iProcessing;
	u8 bmVideoStandards;
} attribute(UVC_PROCESSING_UNIT_DESC2);

// Extension Unit Descriptor
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bUnitID;
	u32 guidExtensionCode_LL;
	u32 guidExtensionCode_LH;
	u32 guidExtensionCode_HL;
	u32 guidExtensionCode_HH;
	u8 bNumControls;
	u8 bNrInPins;
	u8 baSourceID1;
	u8 bControlSize;
	u8 bmControls_L;
	u8 bmControls_M;
	u8 bmControls_H;
	u8 iExtension;
} attribute(UVC_EXTENSION_UNIT_DESC);

// Encoding Unit Descriptor
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bUnitID;
	u8 bSourceID;
	u8 iEncoding;
	u8 bControlSize;
	u8 bmControls[3];
	u8 bmControlsRuntime[3];
} attribute(UVC_ENCODING_UNIT_DESC);

// Class-Specific VC Interrupt Endpoint Descriptor
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u16 wMaxTransferSize;
} attribute(UVC_INTR_EP_DESC);

// Class-Specific VS Interface Input Header Descriptor
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bNumFormats;
	u16 wTotalLength;
	u8 bEndpointAddress;
	u8 bmInfo;
	u8 bTerminalLink;
	u8 bStillCaptureMethod;
	u8 bTriggerSupport;
	u8 bTriggerUsage;
	u8 bControlSize;
	u8 bmaControls;
} attribute(UVC_INPUT_HEADER_DESC);

// Class-Specific VS Interface Output Header Descriptor
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bNumFormats;
	u16 wTotalLength;
	u8 bEndpointAddress;
	u8 bTerminalLink;
	u8 bControlSize;
	u8 bmaControls;
} attribute(UVC_OUTPUT_HEADER_DESC);

// Still Image Frame Descriptor (1 pattern, no compression)
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bEndpointAddress;
	u8 bNumImageSizePatterns;
	u16 wWidth1;
	u16 wHeight1;
	u8 bNumCompressionPattern;
} attribute(UVC_STILL_IMAGE_FRAME_DESC);

// Color Matching
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bColorPrimaries;
	u8 bTransferCharacteristics;
	u8 bMatrixCoefficients;
} attribute(UVC_COLOR_MATCHING_DESC);

// Video Probe and Commit Controls
typedef struct {
	u16 bmHint;
	u8 bFormatIndex;
	u8 bFrameIndex;
	u32 dwFrameInterval;
	u16 wKeyFrameRate;
	u16 wPFrameRate;
	u16 wCompQuality;
	u16 wCompWindow;
	u16 wDelay;
	u32 dwMaxVideoFrameSize;
	u32 dwMaxPayloadTransferSize;
} attribute(UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC);


//=============================================================================
// Payload Header Defined in
//  Universal Serial Bus Device Class Definition for Video Devices (Rev 1.5)
//  2.4.3.3 Video and Still Image Payload Headers
//=============================================================================
typedef struct {
	u8 bHeaderLength;
	u8 bmHeaderInfo;
	u32 dwPresentationTime;
	u32 scrSourceClock;
	u16 SOFTokenCounter;
} attribute(UVC_PAYLOAD_HEADER);

// Bit Field Definition
#define UVC_PAYLOAD_HEADER_FID	0x01 // Frame Identifier
#define UVC_PAYLOAD_HEADER_EOF	0x02 // End of Frame
#define UVC_PAYLOAD_HEADER_PTS	0x04 // Presentation Time Stamp
#define UVC_PAYLOAD_HEADER_SCR	0x08 // Source Clock Reference
#define UVC_PAYLOAD_HEADER_STI	0x20 // Still Image
#define UVC_PAYLOAD_HEADER_ERR	0x40 // Error Bit
#define UVC_PAYLOAD_HEADER_EOH	0x80 // End of Header


//=============================================================================
// Payload-Specific Descriptors Defined in
//  Universal Serial Bus Device Class Definition for Video Devices: Uncompressed Payload (Rev 1.5)
//  3. Payload-Specific Information
//=============================================================================
// Uncompressed Video Format Descriptor
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bFormatIndex;
	u8 bNumFrameDescriptors;
	u32 guidFormat_LL;
	u32 guidFormat_LH;
	u32 guidFormat_HL;
	u32 guidFormat_HH;
	u8 bBitsPerPixel;
	u8 bDefaultFrameIndex;
	u8 bAspectRatioX;
	u8 bAspectRatioY;
	u8 bmInterlaceFlags;
	u8 bCopyProtect;
} attribute(UVC_UNCOMPRESSED_FORMAT_DESC);

// Uncompressed Frame Descriptor (discrete-2 type)
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bFrameIndex;
	u8 bmCapabilities;
	u16 wWidth;
	u16 wHeight;
	u32 dwMinBitRate;
	u32 dwMaxBitRate;
	u32 dwMaxVideoFrameBufferSize;
	u32 dwDefaultFrameInterval;
	u8 bFrameIntervalType;
	u32 dwFrameInterval;
} attribute(UVC_UNCOMPRESSED_FRAME_DESC);


//=============================================================================
// Payload-Specific Descriptors Defined in
//  Universal Serial Bus Device Class Definition for Video Devices: Motion-JPEG Payload (Rev 1.5)
//  3. Payload-Specific Information
//=============================================================================
// Motion-JPEG Video Format Descriptor
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bFormatIndex;
	u8 bNumFrameDescriptors;
	u8 bmFlags;
	u8 bDefaultFrameIndex;
	u8 bAspectRatioX;
	u8 bAspectRatioY;
	u8 bmInterlaceFlags;
	u8 bCopyProtect;
} attribute(UVC_MJPEG_FORMAT_DESC);

// Motion-JPEG Frame Descriptor (continuous-frame type)
typedef struct {
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bFrameIndex;
	u8 bmCapabilities;
	u16 wWidth;
	u16 wHeight;
	u32 dwMinBitRate;
	u32 dwMaxBitRate;
	u32 dwMaxVideoFrameBufferSize;
	u32 dwDefaultFrameInterval;
	u8 bFrameIntervalType;
	u32 dwFrameInterval;
} attribute(UVC_MJPEG_FRAME_DESC);


//=============================================================================
// Configuration Descriptor Chain Definitions for USB 2.0
//=============================================================================
// Class-Specific Video Control Interface Descriptor for USB 2.0
typedef struct {
	UVC_IF_DESC if0_vc;
	UVC_CAMERA_TERM_DESC itd_cam;
	UVC_PROCESSING_UNIT_DESC pud;
	UVC_OUTPUT_TERM_DESC otd;
} attribute(USB20_CS_VC);

// Class-Specific Video Stream Interface Descriptor for USB 2.0
typedef struct {
	UVC_INPUT_HEADER_DESC input_header;
	UVC_UNCOMPRESSED_FORMAT_DESC uncomp_format;
	UVC_UNCOMPRESSED_FRAME_DESC uncomp_frame1;
} attribute(USB20_CS_VS);

typedef struct {
	USB_STD_CFG_DESC cfg;
	USB_IF_ASC_DESC iad;
	USB_STD_IF_DESC if0;
	USB20_CS_VC cs_vc2;
	USB_STD_EP_DESC ep1_intr;
	UVC_INTR_EP_DESC ep1_intr_vc;
	USB_STD_IF_DESC if0_alt0;
	USB20_CS_VS cs_vs2;
	USB_STD_EP_DESC if0_alt0_ep;
} attribute(USB_CONFIG);


//=============================================================================
// Configuration Descriptor Chain Definitions for USB 3.0
//=============================================================================
// Class-Specific Video Control Interface Descriptor for USB 3.0
typedef struct {
	UVC_IF_DESC if0_vc;
	UVC_CAMERA_TERM_DESC itd_cam;
	UVC_PROCESSING_UNIT_DESC2 pud;
	UVC_OUTPUT_TERM_DESC otd;
} attribute(USB30_CS_VC);

// Class-Specific Video Stream Interface Descriptor for USB 3.0
typedef struct {
	UVC_INPUT_HEADER_DESC input_header;
	UVC_UNCOMPRESSED_FORMAT_DESC uncomp_format;
	UVC_UNCOMPRESSED_FRAME_DESC uncomp_frame1;
} attribute(USB30_CS_VS);

// Configuration Descriptor Chain for USB 3.0
typedef struct {
	USB_STD_CFG_DESC cfg;
	USB_IF_ASC_DESC iad;
	USB_STD_IF_DESC if0;
	USB30_CS_VC cs_vc3;
	USB_STD_EP_DESC ep1_intr;
	USB_STD_EP_SS_COMP_DESC ep1_intr_ss;
	UVC_INTR_EP_DESC ep1_intr_vc;
	USB_STD_IF_DESC if0_alt0;
	USB30_CS_VS cs_vs3;
	USB_STD_EP_DESC ep3_bulk;
	USB_STD_EP_SS_COMP_DESC ep3_bulk_ss;
} attribute(USB30_CONFIG);


//=============================================================================
// USB Device Requests Defined in
//  Universal Serial Bus 3.0 Specification (Rev1.0)
//  9.3 USB Device Requests
//=============================================================================
#define USB_REQ_DIR_MASK	0x80
//#define USB_REQ_TYPE_MASK		0x60 // defined in xusb_ch9.h
#define USB_REQ_RCPT_MASK	0x1F

//#define USB_CMD_STDREQ			0x00 // defined in xusb_ch9.h
//#define USB_CMD_CLASSREQ		0x20 // defined in xusb_ch9.h
//#define USB_CMD_VENDREQ			0x40 // defined in xusb_ch9.h

#define USB_REQ_RCPT_DEV		0x00
#define USB_REQ_RCPT_IF			0x01
#define USB_REQ_RCPT_EP			0x02


//=============================================================================
// Class Codes Defined in
// Universal Serial Bus Device Class Definition for Video Devices (Rev1.5)
// Appendix A. Video Device Class Codes
//=============================================================================
// A.1. Video Interface Class Code
#define UVC_CC_VIDEO						0x0e

// A.2. Video Interface Subclass Codes
#define UVC_SC_UNDEFINED					0x00
#define UVC_SC_VIDEOCONTROL					0x01
#define UVC_SC_VIDEOSTREAMING				0x02
#define UVC_SC_VIDEO_INTERFACE_COLLECTION	0x03

// A.3. Video Interface Protocol Codes
#define UVC_PC_PROTOCOL_UNDEFINED			0x00
#define UVC_PC_PROTOCOL_15					0x01

// A.4. Video Class-Specific Descriptor Types
#define	UVC_CS_UNDEFINED					0x20
#define UVC_CS_DEVICE						0x21
#define UVC_CS_CONFIGURATION				0x22
#define UVC_CS_STRING						0x23
#define UVC_CS_INTERFACE					0x24
#define UVC_CS_ENDPOINT						0x25

// A.5. Video Class-Specific VC Interface Descriptor Subtypes
#define UVC_VC_DESCRIPTOR_UNDEFINED			0x00
#define UVC_VC_HEADER						0x01
#define UVC_VC_INPUT_TERMINAL				0x02
#define UVC_VC_OUTPUT_TERMINAL				0x03
#define	UVC_VC_SELECTOR_UNIT				0x04
#define UVC_VC_PROCESSING_UNIT				0x05
#define UVC_VC_EXTENSION_UNIT				0x06
#define UVC_VC_ENCODING_UNIT				0x07

// A.6. Video Class-Specific VS Interface Descriptor Subtypes
#define UVC_VS_UNDEFINED					0x00
#define UVC_VS_INPUT_HEADER					0x01
#define UVC_VS_OUTPUT_HEADER				0x02
#define UVC_VS_STILL_IMAGE_FRAME			0x03
#define UVC_VS_FORMAT_UNCOMPRESSED			0x04
#define UVC_VS_FRAME_UNCOMPRESSED			0x05
#define UVC_VS_FORMAT_MJPEG					0x06
#define UVC_VS_FRAME_MJPEG					0x07
#define UVC_VS_FORMAT_MPEG2TS				0x0a
#define UVC_VS_FORMAT_DV					0x0c
#define UVC_VS_COLORFORMAT					0x0d
#define UVC_VS_FORMAT_FRAME_BASED			0x10
#define UVC_VS_FRAME_FRAME_BASED			0x11
#define UVC_VS_FORMAT_STREAM_BASED			0x12
#define UVC_VS_FORMAT_H264					0x13
#define UVC_VS_FRAME_H264					0x14
#define UVC_VS_FORMAT_H264_SIMULCAST		0x15
#define UVC_VS_FORMAT_VP8					0x16
#define UVC_VS_FRAME_VP8					0x17
#define UVC_VS_FORMAT_VP8_SIMULCAST			0x18

// A.7. Video Class-Specific Endpoint Descriptor Subtypes
#define UVC_EP_UNDEFINED					0x00
#define UVC_EP_GENERAL						0x01
#define UVC_EP_ENDPOINT						0x02
#define UVC_EP_INTERRUPT					0x03

// A.8. Video Class-Specific Request Codes
#define UVC_RC_UNDEFINED					0x00
#define UVC_SET_CUR							0x01
#define UVC_SET_CUR_ALL						0x11
#define UVC_GET_CUR							0x81
#define UVC_GET_MIN							0x82
#define UVC_GET_MAX							0x83
#define UVC_GET_RES							0x84
#define UVC_GET_LEN							0x85
#define UVC_GET_INFO						0x86
#define UVC_GET_DEF							0x87
#define UVC_GET_CUR_ALL						0x91
#define UVC_GET_MIN_ALL						0x92
#define UVC_GET_MAX_ALL						0x93
#define UVC_GET_RES_ALL						0x94
#define UVC_GET_DEF_ALL						0x97

// A.9.1. VideoControl Interface Control Selectors
#define UVC_VC_CONTROL_UNDEFINED				0x00
#define UVC_VC_VIDEO_POWER_MODE_CONTROL			0x01
#define UVC_VC_REQUEST_ERROR_CODE_CONTROL		0x02

// A.9.2. Terminal Control Selectors
#define UVC_TE_CONTROL_UNDEFINED				0x00

// A.9.3. Selector Unit Control Selectors
#define UVC_SU_CONTROL_UNDEFINED				0x00
#define UVC_SU_INPUT_SELECT_CONTROL				0x01

// A.9.4. Camera Terminal Control Selectors
#define UVC_CT_CONTROL_UNDEFINED				0x00
#define UVC_CT_SCANNING_MODE_CONTROL			0x01
#define UVC_CT_AE_MODE_CONTROL					0x02
#define UVC_CT_AE_PRIORITY_CONTROL				0x03
#define UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL	0x04
#define UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL	0x05
#define UVC_CT_FOCUS_ABSOLUTE_CONTROL			0x06
#define UVC_CT_FOCUS_RELATIVE_CONTROL			0x07
#define UVC_CT_FOCUS_AUTO_CONTROL				0x08
#define UVC_CT_IRIS_ABSOLUTE_CONTROL			0x09
#define UVC_CT_IRIS_RELATIVE_CONTROL			0x0a
#define UVC_CT_ZOOM_ABSOLUTE_CONTROL			0x0b
#define UVC_CT_ZOOM_RELATIVE_CONTROL			0x0c
#define UVC_CT_PANTILT_ABSOLUTE_CONTROL			0x0d
#define UVC_CT_PANTILT_RELATIVE_CONTROL			0x0e
#define UVC_CT_ROLL_ABSOLUTE_CONTROL			0x0f
#define UVC_CT_ROLL_RELATIVE_CONTROL			0x10
#define UVC_CT_PRIVACY_CONTROL					0x11
#define UVC_CT_FOCUS_SIMPLE_CONTROL				0x12
#define UVC_CT_WINDOW_CONTROL					0x13
#define UVC_CT_REGION_OF_INTEREST_CONTROL		0x14

// A.9.5. Processing Unit Control Selectors
#define UVC_PU_CONTROL_UNDEFINED						0x00
#define UVC_PU_BACKLIGHTCOMPENSATION_CONTROL			0x01
#define UVC_PU_BRIGHTNESS_CONTROL						0x02
#define UVC_PU_CONTRAST_CONTROL							0x03
#define UVC_PU_GAIN_CONTROL								0x04
#define UVC_PU_POWER_LINE_FREQUENCY_CONTROL				0x05
#define UVC_PU_HUE_CONTROL								0x06
#define UVC_PU_SATURATION_CONTROL						0x07
#define UVC_PU_SHARPNESS_CONTROL						0x08
#define UVC_PU_GAMMA_CONTROL							0x09
#define UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL		0x0a
#define UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL	0x0b
#define UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL			0x0c
#define UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL		0x0d
#define UVC_PU_DIGITAL_MULTIPLIER_CONTROL				0x0e
#define UVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL			0x0f
#define UVC_PU_HUE_AUTO_CONTROL							0x10
#define UVC_PU_ANALOG_VIDEO_STANDARD_CONTROL			0x11
#define UVC_PU_ANALOG_LOCK_STATUS_CONTROL				0x12
#define UVC_PU_CONTRAST_AUTO_CONTROL					0x13

// A.9.6. Encoding Unit Control Selectors
#define UVC_EU_CONTROL_UNDEFINED				0x00
#define UVC_EU_SELECT_LAYER_CONTROL				0x01
#define UVC_EU_PROFILE_TOOLSET_CONTROL			0x02
#define UVC_EU_VIDEO_RESOLUTION_CONTROL			0x03
#define UVC_EU_MIN_FRAME_INTERVAL_CONTROL		0x04
#define UVC_EU_SLICE_MODE_CONTROL				0x05
#define UVC_EU_RATE_CONTROL_MODE_CONTROL		0x06
#define UVC_EU_AVERAGE_BITRATE_CONTROL			0x07
#define UVC_EU_CPB_SIZE_CONTROL					0x08
#define UVC_EU_PEAK_BIT_RATE_CONTROL			0x09
#define UVC_EU_QUANTIZATION_PARAMS_CONTROL		0x0a
#define UVC_EU_SYNC_REF_FRAME_CONTROL			0x0b
#define UVC_EU_LTR_BUFFER_CONTROL				0x0c
#define UVC_EU_LTR_PICTURE_CONTROL				0x0d
#define UVC_EU_LTR_VALIDATION_CONTROL			0x0e
#define UVC_EU_LEVEL_IDC_LIMIT_CONTROL			0x0f
#define UVC_EU_SEI_PAYLOADTYPE_CONTROL			0x10
#define UVC_EU_QP_RANGE_CONTROL					0x11
#define UVC_EU_PRIORITY_CONTROL					0x12
#define UVC_EU_START_OR_STOP_LAYER_CONTROL		0x13
#define UVC_EU_ERROR_RESILIENCY_CONTROL			0x14

// A.9.7. Extension Unit Control Selectors
#define UVC_XU_CONTROL_UNDEFINED				0x00

// A.9.8. VideoStreaming Interface Control Selectors
#define UVC_VS_CONTROL_UNDEFINED				0x00
#define UVC_VS_PROBE_CONTROL					0x01
#define UVC_VS_COMMIT_CONTROL					0x02
#define UVC_VS_STILL_PROBE_CONTROL				0x03
#define UVC_VS_STILL_COMMIT_CONTROL				0x04
#define UVC_VS_STILL_IMAGE_TRIGGER_CONTROL		0x05
#define UVC_VS_STREAM_ERROR_CODE_CONTROL		0x06
#define UVC_VS_GENERATE_KEY_FRAME_CONTROL		0x07
#define UVC_VS_UPDATE_FRAME_SEGMENT_CONTROL		0x08
#define UVC_VS_SYNCH_DELAY_CONTROL				0x09

//=============================================================================
// Terminal Types Defined in
// Universal Serial Bus Device Class Definition for Video Devices (Rev1.5)
// Appendix B. Terminal Types
//=============================================================================
// B.1. USB Terminal Types
#define UVC_TT_VENDOR_SPECIFIC				0x0100
#define UVC_TT_STREAMING					0X0101

// B.2. Input Terminal Types
#define UVC_ITT_VENDOR_SPECIFIC				0x0200
#define UVC_ITT_CAMERA						0x0201
#define UVC_ITT_MEDIA_TRANSPORT_INPUT		0x0202

// B.3. Output Terminal Types
#define UVC_OTT_VENDOR_SPECIFIC				0x0300
#define UVC_OTT_DISPLAY						0x0301
#define UVC_OTT_MEDIA_TRANSPORT_OUTPUT		0x0302

// B.4. External Terminal Types
#define UVC_EXTERNAL_VENDOR_SPECIFIC		0x0400
#define UVC_COMPOSITE_CONNECTOR				0x0401
#define UVC_SVIDEO_CONNECTOR				0x0402
#define UVC_COMPONENT_CONNECTOR				0x0403

// GET_INFO Defined Bits
#define UVC_GET_INFO_SUPPORT_GET				0x01
#define UVC_GET_INFO_SUPPORT_SET				0x02
#define UVC_GET_INFO_DISABLED_AUTOMATIC			0x04
#define UVC_GET_INFO_AUTOUPDATE					0x08
#define UVC_GET_INFO_ASYNCHRONOUS				0x10
#define UVC_GET_INFO_DISABLED_INCOMPATIBILITY	0x20

//=============================================================================
// User-Defined Application Data
//=============================================================================
struct UVC_APP_DATA {
	// UVC State
	u8 cur_if;
	u8 alt_if;
	u16 brightness; // signed number
	u8 bSelector; // selector input
	UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC probe;
	UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC commit;
	UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC max;
	UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC min;
	int state;

	// TX Buffer and Control
	volatile int busy; // volatile is necessary for release build
	int max_buf_size;
	int header_size;
	int max_payload_size;
	UVC_PAYLOAD_HEADER header;
	int bank;
	int img_received;
	volatile int grb_received;
	volatile int rect_received;
	volatile int xsbl_received;
	volatile int bm_received;
	volatile int gftt_received;
	volatile int intr_received;
};

enum UVC_STATE	{UVC_STATE_IDLE, UVC_STATE_COMMIT};


//=============================================================================
// Function Prototypes
//=============================================================================
u32 Usb_Ch9SetupDevDescReply(struct Usb_DevData *InstancePtr, u8 *BufPtr, u32 BufLen);
u32 Usb_Ch9SetupCfgDescReply(struct Usb_DevData *InstancePtr, u8 *BufPtr, u32 BufLen);
u32 Usb_Ch9SetupBosDescReply(u8 *BufPtr, u32 BufLen);
u32 Usb_Ch9SetupStrDescReply(struct Usb_DevData *InstancePtr, u8 *BufPtr, u32 BufLen, u8 Index);
s32 Usb_SetConfiguration(struct Usb_DevData *InstancePtr, SetupPacket *Ctrl);
s32 Usb_SetConfigurationApp(struct Usb_DevData *InstancePtr, SetupPacket *Ctrl);
void Usb_SetInterfaceHandler(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
void Usb_ClassReq(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
u8 Uvc_GetTermtype (u8* config, u8 id, u16* term_type);
UVC_UNCOMPRESSED_FRAME_DESC* Uvc_GetFrameDesc (u8* config, u8 format_id, u8 frame_id);
void Uvc_ProbeControl(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
int Uvc_UpdateParam (
	struct Usb_DevData *InstancePtr,
	UVC_VIDEO_PROBE_AND_COMMIT_CONTROLS_DESC *new
);
void Uvc_CommitControl(struct Usb_DevData *InstancePtr, SetupPacket *SetupData);
s32 Uvc_InitState (struct Usb_DevData *InstancePtr, u8* config);
s32 Uvc_InitTxInfo (struct Usb_DevData *InstancePtr, u8* config);
struct UVC_APP_DATA* Uvc_GetAppData(struct Usb_DevData *InstancePtr);
void Usb_UnhandledRequest (SetupPacket *SetupData);
void Usb_UnknownRequest (SetupPacket *SetupData);


#endif // XUSB_CH9_VIDEO_H
