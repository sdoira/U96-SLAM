//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include <core/Stereo.h>
#include "opencv/CvLKStereo.h"
#include "core/Parameters.h"

std::vector<cv::Point2f> computeCorrespondences(
	const cv::Mat &leftImage,
	const cv::Mat &rightImage,
	const std::vector<cv::Point2f> &leftCorners,
	std::vector<unsigned char> &status)
{
	// local parameters
	cv::Size winSize = cv::Size(15, 3);
	int maxLevel = 5;
	int iterations = 30;
	float epsilon = 0.01f;
	float minDisparity = 0.5f;
	float maxDisparity = 128.0f;

	// search correspondences in the right image
	std::vector<cv::Point2f> rightCorners;
	std::vector<float> err;
	calcOpticalFlowPyrLKStereo(
		leftImage,
		rightImage,
		leftCorners,
		rightCorners,
		status,
		err,
		winSize,
		maxLevel,
		cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, iterations, epsilon),
		cv::OPTFLOW_LK_GET_MIN_EIGENVALS,
		1e-4
	);

	// validity check
	for (int i = 0; i < (int)status.size(); i++){
		if (status[i] != 0){
			float disparity = leftCorners[i].x - rightCorners[i].x;
			if (disparity <= minDisparity || disparity > maxDisparity){
				status[i] = 0;
			}
		}
	}

	return rightCorners;
}

std::vector<cv::Point3f> generateKeypoints3DStereo(
	const std::vector<cv::Point2f> &leftCorners,
	const std::vector<cv::Point2f> &rightCorners,
	const StereoCameraModel &model,
	const std::vector<unsigned char> &mask,
	float minDepth,
	float maxDepth,
	cv::Mat disp,
	int depthMethod)
{
	std::vector<cv::Point3f> keypoints3d;
	keypoints3d.resize(leftCorners.size());
	float bad_point = std::numeric_limits<float>::quiet_NaN();
	for (unsigned int i = 0; i<leftCorners.size(); ++i)
	{
		cv::Point3f pt(bad_point, bad_point, bad_point);
		if (mask.empty() || mask[i])
		{
			// stereo disparity
			float disparity;
			if (
				(depthMethod == DEPTH_METHOD_CV_BM) ||
				(depthMethod == DEPTH_METHOD_CV_SGBM) ||
				(depthMethod == DEPTH_METHOD_FPGA_BM)
			){
				// from dense depth map
				short tmps = disp.at<short>((int)leftCorners[i].y, (int)leftCorners[i].x);
				disparity = (float)(tmps / 16.0f);
				if (disparity < 0) {
					disparity = 0;
				}
			}
			else {
				disparity = leftCorners[i].x - rightCorners[i].x;
			}

			// 3D coorindates of the key points
			if (disparity != 0.0f)
			{
				// [x,y,d] -> [X,Y,Z]
				cv::Point3f tmpPt = projectDisparityTo3D(
					leftCorners[i],
					disparity,
					model);

				// validity check
				if (
					isFinite(tmpPt) &&
					(minDepth < 0.0f || tmpPt.z > minDepth) &&
					(maxDepth <= 0.0f || tmpPt.z <= maxDepth))
				{
					pt = tmpPt;
					if (!model.localTransform().isNull())
					{
						pt = transformPoint(pt, model.localTransform());
					}
				}
			}
		}

		keypoints3d.at(i) = pt;
	}

	return keypoints3d;
}

void generateKeypoints3D(
	SensorData data,
	StereoCameraModel StereoCameraModel,
	std::vector<cv::KeyPoint> kpts,
	std::vector<cv::Point3f> &kpts3d,
	cv::Mat disp,
	int depthMethod)
{
	// convert cv::KeyPoint to cv::Point2f
	std::vector<cv::Point2f> leftCorners;
	cv::KeyPoint::convert(kpts, leftCorners);

	// search correspondences in the right image
	std::vector<unsigned char> status;
	std::vector<cv::Point2f> rightCorners;
	if (depthMethod == DEPTH_METHOD_CV_LK)
	{
		rightCorners = computeCorrespondences(
			data.imageLeft(),
			data.imageRight(),
			leftCorners,
			status);
	}

	// 3D coodinates of the key points
	float maxDepth = 0.000000;
	float minDepth = 0.000000;
	kpts3d = generateKeypoints3DStereo(
		leftCorners,
		rightCorners,
		StereoCameraModel,
		status,
		minDepth,
		maxDepth,
		disp,
		depthMethod);
}

cv::Point3f projectDisparityTo3D(
	const cv::Point2f &pt2d,
	float disp,
	const StereoCameraModel &model)
{
	cv::Point3f pt3d;
	if (disp > 0.0f)
	{
		// Z = baseline * f / (d + cx1-cx0);
		float c = (float)(model.cx_r() - model.cx_l());
		float Wx = (float)((model.Tx_l() / model.fx_l() - model.Tx_r() / model.fx_r()) / (disp + c));
		float Wy = (float)((model.Tx_l() / model.fy_l() - model.Tx_r() / model.fy_r()) / (disp + c));

		pt3d.x = (float)((pt2d.x - model.cx_l()) * Wx);
		pt3d.y = (float)((pt2d.y - model.cy_l()) * Wy);
		pt3d.z = (float)(model.fx_l() * Wx);
	}
	else {
		LOG_WARN("negative disparity");
		pt3d.x = std::numeric_limits<float>::quiet_NaN();
		pt3d.y = std::numeric_limits<float>::quiet_NaN();
		pt3d.z = std::numeric_limits<float>::quiet_NaN();
	}

	return pt3d;
}

bool isFinite(const cv::Point3f &pt)
{
	return std::isfinite(pt.x) && std::isfinite(pt.y) && std::isfinite(pt.z);
}

cv::Point3f transformPoint(
	const cv::Point3f &point,
	const Transform &transform)
{
	cv::Point3f ret = point;
	ret.x = transform.r11() * point.x + transform.r12() * point.y + transform.r13() * point.z + transform.o14();
	ret.y = transform.r21() * point.x + transform.r22() * point.y + transform.r23() * point.z + transform.o24();
	ret.z = transform.r31() * point.x + transform.r32() * point.y + transform.r33() * point.z + transform.o34();
	return ret;
}

