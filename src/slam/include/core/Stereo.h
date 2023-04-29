//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include <opencv2/core/core.hpp>
#include "core/StereoCameraModel.h"
#include "core/SensorData.h"

std::vector<cv::Point2f> computeCorrespondences(
	const cv::Mat &leftImage,
	const cv::Mat &rightImage,
	const std::vector<cv::Point2f> &leftCorners,
	std::vector<unsigned char> &status);

std::vector<cv::Point3f> generateKeypoints3DStereo(
	const std::vector<cv::Point2f> &leftCorners,
	const std::vector<cv::Point2f> &rightCorners,
	const StereoCameraModel &model,
	const std::vector<unsigned char> &mask,
	float minDepth,
	float maxDepth,
	cv::Mat disp,
	int depthMethod);

void generateKeypoints3D(
	SensorData data,
	StereoCameraModel StereoCameraModel,
	std::vector<cv::KeyPoint> kpts,
	std::vector<cv::Point3f> &kpts3d,
	cv::Mat disp,
	int depthMethod);

cv::Point3f projectDisparityTo3D(
	const cv::Point2f &pt2d,
	float disp,
	const StereoCameraModel &model);

bool isFinite(const cv::Point3f &pt);

cv::Point3f transformPoint(
	const cv::Point3f &point,
	const Transform &transform);

