//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "flann/flann.hpp"
#include "opencv/CvLKStereo.h"
#include "core/Node.h"
#include "core/VisualWord.h"
#include "core/MotionEstimation.h"
#include "core/Logger.h"
#include "core/Stereo.h"

struct REG_INFO {
	cv::Mat covariance;
	int num_inliers;
	int num_matches;
};

Transform computeTransform(
	SensorData sensorFrom,
	SensorData sensorTo,
	Transform guess,
	struct REG_INFO *info);

void matchingGuess_Projection(
	std::vector<cv::Point3f> kptsFrom3D,
	std::vector<cv::Point2f> &projectedPoint,
	std::vector<int> &projectedIndex,
	StereoCameraModel cameraModel,
	Transform guess);

void matchingGuess_radiusSearch(
	std::vector<cv::KeyPoint> kptsTo,
	std::vector<cv::Point2f> projectedPoint,
	std::vector<std::vector<int>> &indices);

void matchingGuess_Nndr(
	cv::Mat descriptorsFrom,
	cv::Mat descriptorsTo,
	std::vector<int> projectedIndex,
	std::vector<cv::Point2f> projectedPoint,
	std::vector<std::vector<int>> indices,
	std::multimap<int, int> &matchedIndex);

int matchingGuess_search(
	cv::Mat descriptorFrom,
	cv::Mat descriptorsTo,
	std::vector<int> indices);

void matchingGuess(
	SensorData sensorFrom,
	SensorData sensorTo,
	Transform guess,
	std::multimap<int, int> &matchedIndex);

void matchingNoGuess(
	SensorData sensorFrom,
	SensorData sensorTo,
	std::multimap<int, int> &matchedIndex);

void estimateMotion(
	SensorData sensorFrom,
	SensorData sensorTo,
	Transform guess,
	Transform &transform,
	REG_INFO *reg_info,
	std::multimap<int, int> matchedIndex);
