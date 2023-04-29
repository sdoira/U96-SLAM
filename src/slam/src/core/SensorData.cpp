//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/SensorData.h"
#include "core/Perf.h"

extern Perf perf;

// empty constructor
SensorData::SensorData() {
	_id = 0;
	_stamp = 0.0;
	_imageSize = cv::Size(0, 0);
	_intermediate = 0;
}

SensorData::SensorData(const StereoCameraModel &cameraModel)
{
	_id = 0;
	_stamp = 0.0;
	_intermediate = 0;
	_stereoCameraModel = cameraModel;
	_imageSize = cv::Size(0, 0);
}

SensorData::~SensorData()
{
}

void SensorData::setStereoImage(
	const cv::Mat &left,
	const cv::Mat &right)
{
	_imageLeft = left;
	_imageRight = right;
	_imageSize = left.size();
}

void SensorData::setFeatures(
	const std::vector<cv::KeyPoint> &keypoints,
	const std::vector<cv::Point3f> &keypoints3D,
	const cv::Mat &descriptors,
	const cv::Mat &disparity)
{
	_keypoints = keypoints;
	_keypoints3D = keypoints3D;
	_descriptors = descriptors;

	// decimate depth map to save memory
	_dispScale = 4;
	cv::Mat tmp = cv::Mat(disparity.rows / _dispScale, disparity.cols / _dispScale, CV_16SC1);
	for (int row = 0; row < tmp.rows; row++) {
		for (int col = 0; col < tmp.cols; col++) {
			tmp.at<short>(row, col) = disparity.at<short>(row * _dispScale, col * _dispScale);
		}
	}
	_disparity = tmp;
}

void SensorData::clearFeatures()
{
	_keypoints = std::vector<cv::KeyPoint>();
	_keypoints3D = std::vector<cv::Point3f>();
	_descriptors = cv::Mat();
	_disparity = cv::Mat();
}

void SensorData::clearDescriptors()
{
	_descriptors = cv::Mat();
}

void SensorData::clearRawData()
{
	_imageLeft = cv::Mat();
	_imageRight = cv::Mat();
	_imageDepth = cv::Mat();
	_imageEigen = cv::Mat();
}

void SensorData::getMemoryUsed()
{
	perf.registerMemoryUsed("SensorData", (unsigned long)(sizeof(SensorData) + groundTruth_.getSize() + _stereoCameraModel.getSize()));

	if (_imageLeft.total() > 0) {
		perf.registerMemoryUsed("_imageLeft", (unsigned long)(sizeof(cv::Mat) + _imageLeft.total() * _imageLeft.elemSize()));
	}
	if (_imageRight.total() > 0) {
		perf.registerMemoryUsed("_imageRight", (unsigned long)(sizeof(cv::Mat) + _imageRight.total() * _imageRight.elemSize()));
	}
	if (_imageDepth.total() > 0) {
		perf.registerMemoryUsed("_imageDepth", (unsigned long)(sizeof(cv::Mat) + _imageDepth.total() * _imageDepth.elemSize()));
	}
	if (_imageEigen.total() > 0) {
		perf.registerMemoryUsed("_imageEigen", (unsigned long)(sizeof(cv::Mat) + _imageEigen.total() * _imageEigen.elemSize()));
	}
	if (_descriptors.total() > 0) {
		perf.registerMemoryUsed("_descriptors", (unsigned long)(sizeof(cv::Mat) + _descriptors.total() * _descriptors.elemSize()));
	}
	if (_disparity.total() > 0) {
		perf.registerMemoryUsed("_disparity", (unsigned long)(sizeof(cv::Mat) + _disparity.total() * _disparity.elemSize()));
	}

	perf.registerMemoryUsed("_keypoints", (unsigned long)(sizeof(std::vector<cv::KeyPoint>) + _keypoints.size() * sizeof(cv::KeyPoint)));
	perf.registerMemoryUsed("_keypoints3D", (unsigned long)(sizeof(std::vector<cv::Point3f>) + _keypoints3D.size() * sizeof(cv::Point3f)));
}

void SensorData::limitKeypoints(
	std::vector<cv::KeyPoint> keypoints,
	std::vector<bool> &inliers,
	int maxKeypoints)
{
	// Remove words under the new hessian threshold
	if (maxKeypoints > 0 && (int)keypoints.size() > maxKeypoints) {
		// Sort words by hessian
		std::multimap<float, int> hessianMap; // <hessian,id>
		for (unsigned int i = 0; i <keypoints.size(); ++i) {
			//Keep track of the data, to be easier to manage the data in the next step
			hessianMap.insert(std::pair<float, int>(fabs(keypoints[i].response), i));
		}

		// Keep keypoints with highest response
		auto iter = hessianMap.rbegin();
		inliers.resize(keypoints.size(), false);
		for (int k = 0; k < maxKeypoints && iter != hessianMap.rend(); ++k, ++iter) {
			inliers[iter->second] = true;
		}
	}
	else {
		inliers.resize(keypoints.size(), true);
	}
}

void SensorData::saveRectImageKpts()
{
	// keypoints
	std::vector<cv::KeyPoint> kpts = this->keypoints();

	// rectified left image
	cv::Mat imgDebug = this->imageLeft().clone();
	cv::cvtColor(imgDebug, imgDebug, CV_GRAY2RGB);

	// draw circles at keypoint locations
	for (int i = 0; i < (int)kpts.size(); i++) {
		cv::circle(imgDebug, kpts[i].pt, 3, cv::Scalar(0, 255, 0));
	}

	// print the number of keypoints at the left bottom corner of the image.
	char number[100];
	sprintf(number, "%d", (int)kpts.size());
	cv::putText(imgDebug, number, cv::Point(10, imgDebug.rows - 10), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0));

	// save the file in jpeg format
	char filename[100];
	sprintf(filename, "rect_kpts_%06d.jpg", this->id());
	imwrite(filename, imgDebug);
}

void SensorData::saveRectImagePair()
{
	char filename[100];

	// rectified stereo image pair are concatenated horizontally
	sprintf(filename, "rect_%04d.png", this->id());
	cv::Mat left = this->imageLeft();
	cv::Mat right = this->imageRight();
	cv::Mat mat = cv::Mat(this->imageLeft().rows, this->imageLeft().cols + this->imageRight().cols, CV_8UC1);
	for (int row = 0; row < left.rows; row++) {
		for (int col = 0; col < left.cols; col++) {
			mat.at<unsigned char>(row, col) = left.at<unsigned char>(row, col);
		}
		for (int col = 0; col < right.cols; col++) {
			mat.at<unsigned char>(row, left.cols + col) = right.at<unsigned char>(row, col);
		}
	}

	imwrite(filename, mat);
}

void SensorData::saveDepthImage()
{
	char filename[100];

	cv::Mat matSrc = this->imageDepth();
	sprintf(filename, "disp_%04d.png", this->id());
	cv::Mat matDst = cv::Mat(matSrc.rows, matSrc.cols, CV_8UC1);
	for (int row = 0; row < matSrc.rows; row++) {
		for (int col = 0; col < matSrc.cols; col++) {
			short tmps = matSrc.at<short>(row, col);
			if (tmps < 0) {
				tmps = 0;
			}
			else {
				tmps /= 16; // drop fraction part
			}
			matDst.at<unsigned char>(row, col) = (unsigned char)floor(tmps);
		}
	}

	imwrite(filename, matDst);
}

void SensorData::saveKpts2d()
{
	char filename[100];

	std::vector<cv::KeyPoint> kpts2d = this->keypoints();

	sprintf(filename, "kpts_%04d.csv", this->id());
	FILE *fp_kpts = fopen(filename, "w");
	for (int i = 0; i < (int)kpts2d.size(); i++) {
		fprintf(fp_kpts, "%f,%f,\n", kpts2d[i].pt.x, kpts2d[i].pt.y);
	}
	fclose(fp_kpts);
}

void SensorData::saveKpts3d()
{
	char filename[100];

	std::vector<cv::Point3f> kpts3d = this->keypoints3D();

	sprintf(filename, "kpts3d_%04d.csv", this->id());
	FILE *fp_kpts3d = fopen(filename, "w");
	for (int i = 0; i < (int)kpts3d.size(); i++) {
		fprintf(fp_kpts3d, "%f,%f,%f,\n", kpts3d[i].x, kpts3d[i].y, kpts3d[i].z);
	}
	fclose(fp_kpts3d);
}

void SensorData::saveEigenvalue()
{
	char filename[100];

	sprintf(filename, "eig_%04d.csv", this->id());
	FILE *fp_eig = fopen(filename, "w");
	cv::Mat eig = this->imageEigen();
	for (int row = 0; row < eig.rows; row++) {
		for (int col = 0; col < eig.cols; col++) {
			fprintf(fp_eig, "%d,", eig.at<unsigned short>(row, col));
		}
		fprintf(fp_eig, "\n");
	}
	fclose(fp_eig);
}

void SensorData::saveDescriptor()
{
	char filename[100];

	cv::Mat desc = this->descriptors();

	sprintf(filename, "desc_%04d.txt", this->id());
	FILE *fp_desc = fopen(filename, "w");
	for (int row = 0; row < desc.rows; row++) {
		for (int col = 0; col < desc.cols; col++) {
			fprintf(fp_desc, "%02x", desc.at<unsigned char>(row, col));
		}
		fprintf(fp_desc, "\n");
	}
	fclose(fp_desc);
}
