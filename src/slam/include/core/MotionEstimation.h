//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "core/Transform.h"
#include "opencv2/opencv.hpp"
#include "core/StereoCameraModel.h"

Transform estimateMotion3DTo2D(
	const std::map<int, cv::Point3f> &words3A,
	const std::map<int, cv::KeyPoint> &words2B,
	const StereoCameraModel &cameraModel,
	int minInliers,
	int refineIterations,
	const Transform &guess,
	const std::map<int, cv::Point3f> &words3B,
	cv::Mat *covariance, // mean reproj error if words3B is not set
	std::vector<int> *matchesOut,
	std::vector<int> *inliersOut);

std::vector<float> computeReprojErrors(
	std::vector<cv::Point3f> opoints,
	std::vector<cv::Point2f> ipoints,
	const cv::Mat &cameraMatrix,
	const cv::Mat &distCoeffs,
	const cv::Mat &rvec,
	const cv::Mat &tvec,
	float threshold,
	std::vector<int> &inliers);

void solvePnPRansac(
	const std::vector<cv::Point3f> &objectPoints,
	const std::vector<cv::Point2f> &imagePoints,
	const cv::Mat &cameraMatrix,
	const cv::Mat &distCoeffs,
	cv::Mat &rvec,
	cv::Mat &tvec,
	int minInliersCount,
	int refineIterations,
	std::vector<int> &inliers);

