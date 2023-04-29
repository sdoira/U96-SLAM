//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include <string>
#include <string.h>
#include <stdio.h>


//=============================================================================
// Operation Mode
//=============================================================================
// application type
enum APP_TYPE {
	APP_TYPE_SLAM_BATCH,		// SLAM with image files
	APP_TYPE_SLAM_REALTIME,		// SLAM with sensor input
	APP_TYPE_FRAME_GRABBER,		// capture raw images
	APP_TYPE_STEREO_CAPTURE,	// capture stereo rectified images
	APP_TYPE_FPGA_TEST			// Batch-process SLAM with FPGA accelaration
};

// how to generate depth map
enum DEPTH_METHOD {
	DEPTH_METHOD_NONE,
	DEPTH_METHOD_CV_LK,		// OpenCV LK Method (sparse)
	DEPTH_METHOD_CV_BM,		// OpenCV Block Matching
	DEPTH_METHOD_CV_SGBM,	// OpenCV Semi-GLobal Block Matching
	DEPTH_METHOD_FPGA_BM	// FPGA Block Matching
};

// how to detect keypoints
enum KPTS_METHOD {
	KPTS_METHOD_NONE,
	KPTS_METHOD_CV_GFTT,	// OpenCV GFTT
	KPTS_METHOD_FPGA_GFTT	// FPGA GFTT
};

enum INPUT_TYPE {
	INPUT_TYPE_FILE,	// file input, batch process
	INPUT_TYPE_SENSOR	// sensor input, real-time process
};

struct APP_SETTING {
	int appType;     // application type
	int inputType;   // file input or sensor input
	int depthMethod; // depth map generation method
	int kptsMethod;  // keypoint generation method
	int doResize;    // resize images to 640x480
	int useFpga;     // implicitly declare use of FPGA
	int quiet;       // no log message
	int memory;      // enable memory consumption report
};


//=============================================================================
// Remote Application Settings
//=============================================================================
// input data type
enum PATTERN_SELECT {
	PATTERN_SELECT_NORMAL,
	PATTERN_SELECT_HORIZ_INCR,
	PATTERN_SELECT_VERT_INCR,
	PATTERN_SELECT_FRAME_INCR,
	PATTERN_SELECT_COLOR_BAR,
	PATTERN_SELECT_GRID
};

// return data from remote app to linux app
// can be combined
enum RETURN_DATA {
	RETURN_DATA_NONE = 0,
	RETURN_DATA_RAW_FRAME = 1,
	RETURN_DATA_STEREO_RECT = 2,
	RETURN_DATA_STEREO_BM = 4,
	RETURN_DATA_GFTT = 8
};

// USB output data type
enum USB_OUTPUT {
	USB_OUTPUT_NONE,
	USB_OUTPUT_RAW_FRAME,
	USB_OUTPUT_STEREO_RECT,
	USB_OUTPUT_STEREO_XSBL,
	USB_OUTPUT_STEREO_BM
};

struct REMOTE_SETTING {
	int patternSelect;
	int returnData;
	int usbOutput;
};


//=============================================================================
// Argument Parameters
//=============================================================================
struct ARG_PARAMS {
	std::string appType;
	std::string baseDirectory;
	std::string pathLeftImages;
	std::string pathRightImages;
	std::string pathTimes;
	std::string gtPath;
	int numImages;
	std::string pathLeftCalib;
	std::string pathRightCalib;
	int quiet;
	int memory;
};


//=============================================================================
// Inter-Processor Communication
//=============================================================================
#define IPC_MSG_NONE			0x00000000

// message from linux system to remote app
#define IPC_MSG1_APP_START		0x00000001
#define IPC_MSG1_OP_START		0x00000002
#define IPC_MSG1_OP_STOP		0x00000003
#define IPC_MSG1_IDLE			0x00000004

// message from remote system to linux system
#define IPC_MSG2_DATA_READY		0x00000001

// return data type
#define IPC_MSG2_DATA_GRB		0x00000100
#define IPC_MSG2_DATA_XSBL		0x00000101
#define IPC_MSG2_DATA_RECT		0x00000102
#define IPC_MSG2_DATA_BM		0x00000103

#define IPC_PARM_NONE			0x00000000

// operation mode
#define IPC_PARM_FRAME_GRAB		0x00000000
#define IPC_PARM_STEREO_RECT	0x00000001
#define IPC_PARM_STEREO_XSBL	0x00000002
#define IPC_PARM_STEREO_BM		0x00000003
#define IPC_PARM_STEREO_GFTT	0x00000004

// input data type
#define IPC_PARM_SENSOR_INPUT	0x00000000
#define IPC_PARM_HORIZ_INCR		0x00000001
#define IPC_PARM_VERT_INCR		0x00000002
#define IPC_PARM_FRAME_INCR		0x00000003
#define IPC_PARM_COLOR_BAR		0x00000004
#define IPC_PARM_GRID			0x00000005
#define IPC_PARM_INPUT_NONE		0x00000006

//=============================================================================
// Function Prototypes
//=============================================================================
void parseArguments(
	int argc,
	char *argv[],
	ARG_PARAMS *args,
	APP_SETTING *appSetting,
	REMOTE_SETTING *remoteSetting);

void setParameter (
	APP_SETTING *appSetting,
	REMOTE_SETTING *remoteSetting
);
