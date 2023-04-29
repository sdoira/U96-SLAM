
#ifndef PARAMETERS_H
#define PARAMETERS_H

//=============================================================================
// Operation Mode
//=============================================================================
// application type
enum APP_TYPE {
	APP_TYPE_SLAM_BATCH,
	APP_TYPE_SLAM_REALTIME,
	APP_TYPE_FRAME_GRABBER,
	APP_TYPE_STEREO_RECTIFIER,
	APP_TYPE_CUSTOM
};

// how to generate depth map
enum DEPTH_MODE {
	DEPTH_MODE_NONE,
	DEPTH_MODE_CV_LK,	// OpenCV LK Method (sparse)
	DEPTH_MODE_CV_BM,	// OpenCV Block Matching
	DEPTH_MODE_FPGA_BM	// RTL Block Matching
};

// how to detect keypoints
enum KPTS_MODE {
	KPTS_MODE_NONE,
	KPTS_MODE_CV_GFTT,	// OpenCV GFTT
	KPTS_MODE_FPGA_GFTT	// RTL GFTT
};

struct APP_SETTING {
	int appType;
	int depthMode;
	int kptsMode;
};


//=============================================================================
// Remote Application Settings
//=============================================================================
// input data type
enum INPUT_DATA {
	INPUT_DATA_SENSOR_INPUT,
	INPUT_DATA_HORIZ_INCR,
	INPUT_DATA_VERT_INCR,
	INPUT_DATA_FRAME_INCR,
	INPUT_DATA_COLOR_BAR,
	INPUT_DATA_GRID,
	INPUT_DATA_NONE
};

// return data type
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

// operation mode
enum REMOTE_OP_MODE {
	REMOTE_OP_MODE_AUTO,
	REMOTE_OP_MODE_REMOTE
};

struct REMOTE_SETTING {
	int inputData;
	int returnData;
	int usbOutput;
	int opMode;
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

#endif
