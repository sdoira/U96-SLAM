//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/Parameters.h"
#include "core/Logger.h"

void parseArguments(
	int argc,
	char *argv[],
	ARG_PARAMS *args,
	APP_SETTING *appSetting,
	REMOTE_SETTING *remoteSetting)
{
	//==================================================================
	// Parse argument parameters
	//==================================================================
	// path to the base directory
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-dir") == 0) {
			args->baseDirectory = argv[i + 1];
			if (args->baseDirectory[args->baseDirectory.size() - 1] != '/') {
				args->baseDirectory += '/';
			}
			break;
		}
	}

	// set default values
	args->numImages = -1;
	args->quiet = 0;
	args->memory = 0;

	// parse parameters
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-app") == 0) {
			args->appType = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-l") == 0) {
			args->pathLeftImages = args->baseDirectory + argv[i + 1];
			if (args->pathLeftImages[args->pathLeftImages.size() - 1] != '/') {
				args->pathLeftImages += '/';
			}
			i++;
		}
		else if (strcmp(argv[i], "-r") == 0) {
			args->pathRightImages = args->baseDirectory + argv[i + 1];
			if (args->pathRightImages[args->pathRightImages.size() - 1] != '/') {
				args->pathRightImages += '/';
			}
			i++;
		}
		else if (strcmp(argv[i], "-t") == 0) {
			args->pathTimes = args->baseDirectory + argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-gt") == 0) {
			args->gtPath = args->baseDirectory + argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-n") == 0) {
			args->numImages = atoi(argv[i + 1]);
			i++;
		}
		else if (strcmp(argv[i], "-lc") == 0) {
			args->pathLeftCalib = args->baseDirectory + argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-rc") == 0) {
			args->pathRightCalib = args->baseDirectory + argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-quiet") == 0) {
			args->quiet = true;
		}
		else if (strcmp(argv[i], "-memory") == 0) {
			args->memory = true;
		}
	}

	LOG_INFO("\n");
	LOG_INFO("[Argument Parameters]\n");
	LOG_INFO("ApplicationType: %s\n", args->appType.c_str());
	LOG_INFO("baseDirectory  : %s\n", args->baseDirectory.c_str());
	LOG_INFO("pathLeftImages : %s\n", args->pathLeftImages.c_str());
	LOG_INFO("pathRightImages: %s\n", args->pathRightImages.c_str());
	LOG_INFO("pathTimes      : %s\n", args->pathTimes.c_str());
	LOG_INFO("gtPath         : %s\n", args->gtPath.c_str());
	LOG_INFO("numImages      : %d\n", args->numImages);
	LOG_INFO("pathLeftCalib  : %s\n", args->pathLeftCalib.c_str());
	LOG_INFO("pathRightCalib : %s\n", args->pathRightCalib.c_str());
	LOG_INFO("\n");


	//==================================================================
	// Set other parameters
	//==================================================================
	if (args->appType == "SLAM_BATCH") {
		appSetting->appType = APP_TYPE_SLAM_BATCH;
	}
	else if (args->appType == "SLAM_REALTIME") {
		appSetting->appType = APP_TYPE_SLAM_REALTIME;
	}
	else if (args->appType == "STEREO_CAPTURE") {
		appSetting->appType = APP_TYPE_STEREO_CAPTURE;
	}
	else if (args->appType == "FRAME_GRABBER") {
		appSetting->appType = APP_TYPE_FRAME_GRABBER;
	}
	else if (args->appType == "FPGA_TEST") {
		appSetting->appType = APP_TYPE_FPGA_TEST;
	}
	else {
		LOG_WARN("Undifned application type [%s]", args->appType.c_str());
	}

	if (args->quiet) {
		appSetting->quiet = 1;
	}

	if (args->memory) {
		appSetting->memory = 1;
	}

	setParameter(appSetting, remoteSetting);


	//==================================================================
	// Validity check
	//==================================================================
	if ((appSetting->inputType == INPUT_TYPE_FILE) && args->baseDirectory.empty()) {
		LOG_ERROR("base directory is not specified\n");
	}

	FILE *fp_calib_test;
	if (!args->pathLeftCalib.empty()) {
		fp_calib_test = fopen(args->pathLeftCalib.c_str(), "r");
		if (fp_calib_test == 0) {
			LOG_ERROR("failed to open calibration file %s\n", args->pathLeftCalib.c_str());
		}
		else {
			fclose(fp_calib_test);
		}
	}

	if (!args->pathRightCalib.empty()) {
		fp_calib_test = fopen(args->pathRightCalib.c_str(), "r");
		if (fp_calib_test == 0) {
			LOG_ERROR("failed to open calibration file %s\n", args->pathRightCalib.c_str());
		}
		else {
			fclose(fp_calib_test);
		}
	}
}

void setParameter (
	APP_SETTING *appSetting,
	REMOTE_SETTING *remoteSetting)
{
	// default parameters
	if (appSetting->appType == APP_TYPE_SLAM_BATCH)
	{
		// linux application
		appSetting->inputType = INPUT_TYPE_FILE;
		appSetting->depthMethod = DEPTH_METHOD_CV_BM;
		appSetting->kptsMethod = KPTS_METHOD_CV_GFTT;

		// remote application
		remoteSetting->patternSelect = PATTERN_SELECT_NORMAL;
		remoteSetting->returnData = RETURN_DATA_NONE;
		remoteSetting->usbOutput = USB_OUTPUT_NONE;
	}
	else if (appSetting->appType == APP_TYPE_SLAM_REALTIME)
	{
		// linux application
		appSetting->inputType = INPUT_TYPE_SENSOR;
		appSetting->depthMethod = DEPTH_METHOD_FPGA_BM;
		appSetting->kptsMethod = KPTS_METHOD_FPGA_GFTT;

		// remote application
		remoteSetting->patternSelect = PATTERN_SELECT_NORMAL;
		remoteSetting->returnData = RETURN_DATA_STEREO_RECT + RETURN_DATA_STEREO_BM + RETURN_DATA_GFTT;
		remoteSetting->usbOutput = USB_OUTPUT_STEREO_RECT;
	}
	else if (appSetting->appType == APP_TYPE_STEREO_CAPTURE)
	{
		// linux application
		appSetting->inputType = INPUT_TYPE_SENSOR;
		appSetting->depthMethod = DEPTH_METHOD_NONE;
		appSetting->kptsMethod = KPTS_METHOD_NONE;

		// remote application
		remoteSetting->patternSelect = PATTERN_SELECT_NORMAL;
		remoteSetting->returnData = RETURN_DATA_STEREO_RECT;
		remoteSetting->usbOutput = USB_OUTPUT_STEREO_RECT;
	}
	else if (appSetting->appType == APP_TYPE_FRAME_GRABBER)
	{
		// linux application
		appSetting->inputType = INPUT_TYPE_SENSOR;
		appSetting->depthMethod = DEPTH_METHOD_NONE;
		appSetting->kptsMethod = KPTS_METHOD_NONE;

		// remote application
		remoteSetting->patternSelect = PATTERN_SELECT_NORMAL;
		remoteSetting->returnData = RETURN_DATA_NONE;
		remoteSetting->usbOutput = USB_OUTPUT_RAW_FRAME;
	}
	else if (appSetting->appType == APP_TYPE_FPGA_TEST)
	{
		// linux application
		appSetting->inputType = INPUT_TYPE_FILE;
		appSetting->depthMethod = DEPTH_METHOD_FPGA_BM;
		appSetting->kptsMethod = KPTS_METHOD_FPGA_GFTT;

		// remote application
		remoteSetting->patternSelect = PATTERN_SELECT_NORMAL;
		remoteSetting->returnData = RETURN_DATA_STEREO_BM + RETURN_DATA_GFTT;
		remoteSetting->usbOutput = USB_OUTPUT_NONE;
	}

	appSetting->doResize = 0;
	appSetting->useFpga = (
		(remoteSetting->returnData != RETURN_DATA_NONE) ||
		(remoteSetting->usbOutput != USB_OUTPUT_NONE)
	);

#ifdef _WIN32
	if (appSetting->useFpga) {
		LOG_ERROR("FPGA not allowed on Windows");
	}
#endif

}

