//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "opencv2/opencv.hpp"
#include "core/Transform.h"

class StereoCameraModel
{
public:
	StereoCameraModel();
	~StereoCameraModel();

	double baseline() const;
	bool load(
		const std::string &fileNameLeft,
		const std::string &fileNameRight,
		int doResize);

	const Transform & localTransform() const { return _localTransform; }
	const cv::Size & imageSize() const { return _imageSize; }

	double fx_l() const { return _P[0].at<double>(0, 0); }
	double fy_l() const { return _P[0].at<double>(1, 1); }
	double cx_l() const { return _P[0].at<double>(0, 2); }
	double cy_l() const { return _P[0].at<double>(1, 2); }
	double Tx_l() const { return _P[0].at<double>(0, 3); }
	double fx_r() const { return _P[1].at<double>(0, 0); }
	double fy_r() const { return _P[1].at<double>(1, 1); }
	double cx_r() const { return _P[1].at<double>(0, 2); }
	double cy_r() const { return _P[1].at<double>(1, 2); }
	double Tx_r() const { return _P[1].at<double>(0, 3); }
	cv::Mat K_l() const { return _P[0].colRange(0, 3); } // 3x3 camera matrix
	cv::Mat D_l() const { return cv::Mat::zeros(1, 5, CV_64FC1); }

	unsigned long getSize();

private:
	cv::Size _imageSize;
	cv::Mat _P[2];
	Transform _localTransform;
};

