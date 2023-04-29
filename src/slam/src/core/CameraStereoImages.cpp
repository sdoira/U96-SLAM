//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/CameraStereoImages.h"
#include "core/Perf.h"
#include "core/Graph.h"

#ifndef _WIN32
#include <unistd.h>
#endif

CameraStereoImages::CameraStereoImages(
	const std::string pathLeftImages,
	const std::string pathRightImages
){
	_frameNum = 0;
	_captureTime = 0;
	_seq = 0;

	// left
	_path_l = pathLeftImages;
	_dir_l = 0;

	// right
	_path_r = pathRightImages;
	_dir_r = 0;
}

CameraStereoImages::~CameraStereoImages()
{}

bool CameraStereoImages::init(int inputType)
{
	if (inputType == INPUT_TYPE_FILE) {
		// set paths to the image files
		_dir_l = new Directory(_path_l);
		_dir_r = new Directory(_path_r);

		// read time stamps
		_stamps.clear();
		if (!_timestampsPath.empty()) {
			FILE *fp = fopen(_timestampsPath.c_str(), "r");
			if (fp != 0) {
				double tmpd;
				while (fscanf(fp, "%lf\n", &tmpd) == 1) {
					_stamps.push_back(tmpd);
				}
				fclose(fp);
			}
			else {
				LOG_WARN(" failed to open timestamp file ");
			}
		}

		// read ground-truth
		groundTruth_.clear();
		if (!_groundTruthPath.empty()) {
			std::map<int, Transform> poses;
			importPoses(_groundTruthPath, poses);
			for (auto iter = poses.begin(); iter != poses.end(); ++iter)
			{
				groundTruth_.push_back(iter->second);
			}
		}
	}

	return true;
}

void CameraStereoImages::captureFromFile(SensorData &data, APP_SETTING appSetting)
{
	// capture time
	_captureTime = currentTimeSec();

	// time stamp
	double stamp;
	if (_stamps.size())
	{
		stamp = _stamps[_frameNum];
	}
	else {
		stamp = currentTimeSec();
	}

	// left image
	cv::Mat imageLeft;
	std::string next = _dir_l->getNextFileName();
	if (!next.empty()) {
		std::string imageFilePath = _path_l + next;
		captureFile(imageFilePath.c_str(), imageLeft, appSetting.doResize);
	}

	// right image
	cv::Mat imageRight;
	std::string next2 = _dir_r->getNextFileName();
	if (!next2.empty()) {
		std::string imageFilePath = _path_r + next2;
		captureFile(imageFilePath.c_str(), imageRight, appSetting.doResize);
	}

	// build sensor data
	data.setStereoImage(imageLeft, imageRight);
	data.setId(this->getNextSeqID());
	data.setStamp(stamp);

	Transform groundTruthPose;
	if (groundTruth_.size())
	{
		groundTruthPose = groundTruth_.front();
		groundTruth_.pop_front();
	}
	data.setGroundTruth(groundTruthPose);

	_frameNum++;

	return;
}

void CameraStereoImages::captureFile(const char *path, cv::Mat &image, int doResize)
{
	image = cv::imread(path, 0);
	if (image.empty()) {
		LOG_WARN(" Failed to read %s ", path);
	}

	if (doResize) {
		cv::Mat imageResize;
		cv::resize(image, imageResize, cv::Size(640, 480));
		image = imageResize;
	}
}

void CameraStereoImages::captureFromFpga(Fpga *fpga, SensorData &data, APP_SETTING appSetting)
{
	// capture time
	_captureTime = currentTimeSec();

	// capture data
	fpga->receiveData(data, appSetting);

	// build sensor data
	data.setStamp(_captureTime);
	data.setId(this->getNextSeqID());

	_frameNum++;

	return;
}

std::vector<std::string> CameraStereoImages::filenames() const
{
	std::vector<std::string> fileNames;
	if (_dir_r)
	{
		fileNames = std::vector<std::string>(
			_dir_r->getFileNames().begin(),
			_dir_r->getFileNames().end());
	}
	return fileNames;
}
