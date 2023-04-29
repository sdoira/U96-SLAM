//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include <opencv2/core/core.hpp>
#include "core/StereoCameraModel.h"
#include "core/Logger.h"

class SensorData
{
public:
	SensorData();
	SensorData(const StereoCameraModel &cameraModel);
	~SensorData();

	int id() const { return _id; }
	void setId(int id) { _id = id; }
	double stamp() const { return _stamp; }
	void setStamp(double stamp) { _stamp = stamp; }
	void setIntermediate(bool val) { _intermediate = val; }
	bool isIntermediate() const { return _intermediate; }
	const cv::Size imageSize() { return _imageSize; }
	void setImageSize(cv::Size imageSize) { _imageSize = imageSize; }
	const int dispScale() const { return _dispScale; }
	void setGroundTruth(const Transform &pose) { groundTruth_ = pose; }
	const Transform &groundTruth() const { return groundTruth_; }

	cv::Mat &imageLeft() { return _imageLeft; }
	cv::Mat &imageRight() { return _imageRight; }
	cv::Mat &imageDepth() { return _imageDepth; }
	cv::Mat &imageEigen() { return _imageEigen; }
	void setImageLeft(cv::Mat &left) { _imageLeft = left; }
	void setImageRight(cv::Mat &right) { _imageRight = right; }
	void setImageDepth(cv::Mat &depth) { _imageDepth = depth; }
	void setImageEigen(cv::Mat &eig) { _imageEigen = eig; }

	unsigned short maxEigen() { return _maxEigen; }
	void setMaxEigen(unsigned short max) { _maxEigen = max; }

	void setStereoImage(const cv::Mat &left, const cv::Mat &right);

	const StereoCameraModel &stereoCameraModel() const { return _stereoCameraModel; }
	void setFeatures(
		const std::vector<cv::KeyPoint> &keypoints,
		const std::vector<cv::Point3f> &keypoints3D,
		const cv::Mat &descriptors,
		const cv::Mat &disparity);
	void clearFeatures();
	void clearDescriptors();
	const std::vector<cv::KeyPoint> &keypoints() const { return _keypoints; }
	const std::vector<cv::Point3f> &keypoints3D() const { return _keypoints3D; }
	const cv::Mat &descriptors() const { return _descriptors; }
	const cv::Mat &disparity() const { return _disparity; }
	void clearRawData();

	void limitKeypoints(
		std::vector<cv::KeyPoint> keypoints,
		std::vector<bool> &inliers,
		int maxKeypoints);

	void saveRectImageKpts();
	void saveRectImagePair();
	void saveDepthImage();
	void saveKpts2d();
	void saveKpts3d();
	void saveEigenvalue();
	void saveDescriptor();

	void getMemoryUsed();

private:
	int _id;
	double _stamp;
	cv::Size _imageSize;
	bool _intermediate;
	int _dispScale;
	Transform groundTruth_;

	StereoCameraModel _stereoCameraModel;

	cv::Mat _imageLeft;
	cv::Mat _imageRight;
	cv::Mat _imageDepth;
	cv::Mat _imageEigen;
	unsigned short _maxEigen;

	// features
	std::vector<cv::KeyPoint> _keypoints;
	std::vector<cv::Point3f> _keypoints3D;
	cv::Mat _descriptors;
	cv::Mat _disparity;
};

