//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include <vector>
#include <string>
#include <map>
#include "Eigen/Core"
#include "Eigen/Geometry"
#include "opencv2/core/core.hpp"

class Transform
{
public:
	// Zero by default
	Transform();

	// rotation matrix r## and origin o##
	Transform(
		float r11, float r12, float r13, float o14,
		float r21, float r22, float r23, float o24,
		float r31, float r32, float r33, float o34);
	Transform(
		double r11, double r12, double r13, double o14,
		double r21, double r22, double r23, double o24,
		double r31, double r32, double r33, double o34);

	// should have 3 rows, 4 cols and type CV_32FC1
	Transform(const cv::Mat & mat);

	// x,y,z, roll,pitch,yaw
	Transform(float x, float y, float z, float roll, float pitch, float yaw);

	~Transform();

	float r11() const { return data_.at<float>(0, 0); }
	float r12() const { return data_.at<float>(0, 1); }
	float r13() const { return data_.at<float>(0, 2); }
	float o14() const { return data_.at<float>(0, 3); }
	float r21() const { return data_.at<float>(1, 0); }
	float r22() const { return data_.at<float>(1, 1); }
	float r23() const { return data_.at<float>(1, 2); }
	float o24() const { return data_.at<float>(1, 3); }
	float r31() const { return data_.at<float>(2, 0); }
	float r32() const { return data_.at<float>(2, 1); }
	float r33() const { return data_.at<float>(2, 2); }
	float o34() const { return data_.at<float>(2, 3); }
	float & x() { return data_.at<float>(0, 3); }
	float & y() { return data_.at<float>(1, 3); }
	float & z() { return data_.at<float>(2, 3); }
	const float & x() const { return data_.at<float>(0, 3); }
	const float & y() const { return data_.at<float>(1, 3); }
	const float & z() const { return data_.at<float>(2, 3); }

	bool isNull() const;
	void setNull();
	static Transform getIdentity();
	Transform inverse() const;
	void getAngles(float & roll, float & pitch, float & yaw) const;
	float getNorm() const;

	static Transform fromEigen4f(const Eigen::Matrix4f & matrix);
	static Transform fromEigen3f(const Eigen::Affine3f & matrix);
	Eigen::Matrix4f toEigen4f() const;

	Transform operator*(const Transform & t) const;
	Transform & operator*=(const Transform & t);

	unsigned long getSize();

private:
	cv::Mat data_;
};
