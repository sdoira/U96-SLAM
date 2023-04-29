//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/StereoCameraModel.h"
#include "core/Logger.h"
#include "core/Parameters.h"

StereoCameraModel::StereoCameraModel() {
	_localTransform = Transform(
		 0.0f,  0.0f,  1.0f,  0.0f,
		-1.0f,  0.0f,  0.0f,  0.0f,
		 0.0f, -1.0f,  0.0f,  0.0f);
}

StereoCameraModel::~StereoCameraModel() {
}

bool StereoCameraModel::load(
	const std::string &fileNameLeft,
	const std::string &fileNameRight,
	int doResize)
{
	_P[0] = cv::Mat(3, 4, CV_64FC1);
	_P[1] = cv::Mat(3, 4, CV_64FC1);
	_imageSize = cv::Size();

	// read calibration files
	if (!fileNameRight.empty()) {
		// assume OpenCV style
		for (int lr = 0; lr < 2; lr++) {
			cv::FileStorage fs;
			cv::FileNode n;
			std::string filePath = (lr == 0) ? fileNameLeft : fileNameRight;
			if (fs.open(filePath, cv::FileStorage::READ))
			{
				if (lr == 0) {
					n = fs["image_width"];
					if (n.type() != cv::FileNode::NONE) {
						_imageSize.width = (int)fs["image_width"];
					}

					n = fs["image_height"];
					if (n.type() != cv::FileNode::NONE) {
						_imageSize.height = (int)fs["image_height"];
					}
				}

				n = fs["projection_matrix"];
				if (n.type() != cv::FileNode::NONE) {
					int rows = (int)n["rows"];
					int cols = (int)n["cols"];
					if ((rows != 3) || (cols != 4)) {
						LOG_WARN("Illegal projection matrix size (%d,%d)\n", rows, cols);
					}
					std::vector<double> data;
					n["data"] >> data;
					_P[lr] = cv::Mat(rows, cols, CV_64FC1, data.data()).clone();
				}

				fs.release();
			}
			else {
				LOG_WARN("Failed to open calibration file \"%s\".\n", filePath.c_str());
				return false;
			}
		}
	}
	else {
		// LR combined -> assume KITTI style
		_imageSize = cv::Size(1241, 376); // image size is not given for KITTI
		FILE *fp_calib = fopen(fileNameLeft.c_str(), "r");
		if (fp_calib != 0)
		{
			double d[12];
			int ret;
			ret = fscanf(
				fp_calib,
				"P0: %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
				&d[0], &d[1], &d[2], &d[3], &d[4], &d[5], &d[6], &d[7], &d[8], &d[9], &d[10], &d[11]);
			if (ret == 12) {
				memcpy(_P[0].data, d, 12 * sizeof(double));
			}
			else {
				LOG_WARN("Failed to load calibration data P0 \"%s\".\n", fileNameLeft.c_str());
				return false;
			}

			ret = fscanf(
				fp_calib,
				"P1: %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
				&d[0], &d[1], &d[2], &d[3], &d[4], &d[5], &d[6], &d[7], &d[8], &d[9], &d[10], &d[11]);
			if (ret == 12) {
				memcpy(_P[1].data, d, 12 * sizeof(double));
			}
			else {
				LOG_WARN("Failed to load calibration data P1 \"%s\".\n", fileNameLeft.c_str());
				return false;
			}
		}
		else {
			LOG_WARN("Failed to open calibration file \"%s\".\n", fileNameLeft.c_str());
			return false;
		}
	}

	// resize to 640x480 if necessary
	for (int lr = 0; lr < 2; lr++) {
		if (doResize) {
			double sx = 640.0 / _imageSize.width;
			double sy = 480.0 / _imageSize.height;
			_P[lr].at<double>(0, 0) *= sx; // fx
			_P[lr].at<double>(0, 2) *= sx; // cx
			_P[lr].at<double>(0, 3) *= sx; // Tx
			_P[lr].at<double>(1, 1) *= sy; // fy
			_P[lr].at<double>(1, 2) *= sy; // cy
			_P[lr].at<double>(1, 3) *= sy; // Ty
		}
	}

	return true;
}

double StereoCameraModel::baseline() const
{
	if (fx_r() != 0.0 && fx_l() != 0.0) {
		return Tx_l() / fx_l() - Tx_r() / fx_r();
	}
	else {
		return 0.0;
	}
}

unsigned long StereoCameraModel::getSize()
{
	unsigned long memUsed = (unsigned long)(
		sizeof(StereoCameraModel) +
		(sizeof(cv::Mat) + _P[0].total() * _P[0].elemSize()) * 2 +
		_localTransform.getSize());

	return memUsed;
}
