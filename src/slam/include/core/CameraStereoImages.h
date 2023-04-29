//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "core/StereoCameraModel.h"
#include "core/SensorData.h"
#include "core/Directory.h"
#include "core/FPGA.h"
#include "core/Parameters.h"

class CameraStereoImages
{
public:
	CameraStereoImages(
		const std::string pathLeftImages = "",
		const std::string pathRightImages = ""
	);
	~CameraStereoImages();

	bool init(int inputType);

	std::vector<std::string> filenames() const;
	void captureFromFile(SensorData &data, APP_SETTING appSetting);
	void captureFile(const char *path, cv::Mat &image, int doResize);
	void captureFromFpga(Fpga *fpga, SensorData &data, APP_SETTING appSetting);

	void setTimestamps(const std::string & filePath) { _timestampsPath = filePath; }

	void setGroundTruthPath(const std::string & filePath) { _groundTruthPath = filePath; }


protected:
	int getNextSeqID() { int tmp = _seq; _seq++; return tmp; }

private:
	int _seq;
	std::string _timestampsPath;
	std::string _groundTruthPath;
	std::list<Transform> groundTruth_;
	std::vector<double> _stamps;
	int _frameNum;
	float _captureTime;

	// left imager
	std::string _path_l;
	Directory *_dir_l;

	// right imager
	std::string _path_r;
	Directory *_dir_r;
};
