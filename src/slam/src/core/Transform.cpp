//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/Transform.h"
#include <iomanip>

Transform::Transform()
{
	data_ = cv::Mat::zeros(3, 4, CV_32FC1);
}

Transform::~Transform()
{
}

Transform::Transform(
	float r11, float r12, float r13, float o14,
	float r21, float r22, float r23, float o24,
	float r31, float r32, float r33, float o34)
{
	data_ = cv::Mat(3, 4, CV_32FC1);
	data_.at<float>(0, 0) = r11;
	data_.at<float>(0, 1) = r12;
	data_.at<float>(0, 2) = r13;
	data_.at<float>(0, 3) = o14;
	data_.at<float>(1, 0) = r21;
	data_.at<float>(1, 1) = r22;
	data_.at<float>(1, 2) = r23;
	data_.at<float>(1, 3) = o24;
	data_.at<float>(2, 0) = r31;
	data_.at<float>(2, 1) = r32;
	data_.at<float>(2, 2) = r33;
	data_.at<float>(2, 3) = o34;
}

Transform::Transform(
	double r11, double r12, double r13, double o14,
	double r21, double r22, double r23, double o24,
	double r31, double r32, double r33, double o34)
{
	data_ = cv::Mat(3, 4, CV_32FC1);
	data_.at<float>(0, 0) = (float)r11;
	data_.at<float>(0, 1) = (float)r12;
	data_.at<float>(0, 2) = (float)r13;
	data_.at<float>(0, 3) = (float)o14;
	data_.at<float>(1, 0) = (float)r21;
	data_.at<float>(1, 1) = (float)r22;
	data_.at<float>(1, 2) = (float)r23;
	data_.at<float>(1, 3) = (float)o24;
	data_.at<float>(2, 0) = (float)r31;
	data_.at<float>(2, 1) = (float)r32;
	data_.at<float>(2, 2) = (float)r33;
	data_.at<float>(2, 3) = (float)o34;
}

Transform::Transform(const cv::Mat & mat)
{
	if (mat.type() == CV_32FC1) {
		data_ = mat;
	}
	else {
		mat.convertTo(data_, CV_32F);
	}
}

Transform::Transform(float x, float y, float z, float roll, float pitch, float yaw)
{
	// roll/pitch/yaw to rotation matrix
	float A = cos(yaw);
	float B = sin(yaw);
	float C = cos(pitch);
	float D = sin(pitch);
	float E = cos(roll);
	float F = sin(roll);
	float DE = D * E;
	float DF = D * F;

	Eigen::Affine3f t;
	t(0, 0) = A*C;  t(0, 1) = A*DF - B*E;  t(0, 2) = B*F + A*DE;  t(0, 3) = x;
	t(1, 0) = B*C;  t(1, 1) = A*E + B*DF;  t(1, 2) = B*DE - A*F;  t(1, 3) = y;
	t(2, 0) = -D;   t(2, 1) = C*F;         t(2, 2) = C*E;         t(2, 3) = z;
	t(3, 0) = 0;    t(3, 1) = 0;           t(3, 2) = 0;           t(3, 3) = 1;

	*this = fromEigen3f(t);
}

bool Transform::isNull() const
{
	if (data_.empty() || (cv::countNonZero(data_) == 0)) {
		return true;
	}

	return false;
}

void Transform::setNull()
{
	*this = Transform();
}

Transform Transform::getIdentity()
{
	Transform tr = Transform(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f);

	return tr;
}

Transform Transform::inverse() const
{
	Eigen::Matrix4f e4f = toEigen4f();

	// Computation of matrix inverse and determinant, with invertibility check.
	bool invertible = false;
	Eigen::Matrix4f inverse;
	Eigen::Matrix4f::RealScalar det;
	e4f.computeInverseAndDetWithCheck(inverse, det, invertible);
	
	Transform tr = fromEigen4f(inverse);

	return tr;
}

void Transform::getAngles(float & roll, float & pitch, float & yaw) const
{
	// rotation matrix to roll/pitch/yaw
	roll = atan2(this->r32(), this->r33());
	pitch = asin(-1.0f * this->r31());
	yaw = atan2(this->r21(), this->r11());
}

float Transform::getNorm() const
{
	float ss = (
		this->x()*this->x() +
		this->y()*this->y() +
		this->z()*this->z());

	float srss = std::sqrt(ss);

	return srss;
}

Transform Transform::fromEigen4f(const Eigen::Matrix4f & matrix)
{
	Transform tr = Transform(
		matrix(0, 0), matrix(0, 1), matrix(0, 2), matrix(0, 3),
		matrix(1, 0), matrix(1, 1), matrix(1, 2), matrix(1, 3),
		matrix(2, 0), matrix(2, 1), matrix(2, 2), matrix(2, 3));

	return tr;
}

Transform Transform::fromEigen3f(const Eigen::Affine3f & matrix)
{
	Transform tr = Transform(
		matrix(0, 0), matrix(0, 1), matrix(0, 2), matrix(0, 3),
		matrix(1, 0), matrix(1, 1), matrix(1, 2), matrix(1, 3),
		matrix(2, 0), matrix(2, 1), matrix(2, 2), matrix(2, 3));

	return tr;
}

Eigen::Matrix4f Transform::toEigen4f() const
{
	Eigen::Matrix4f e4f;
	e4f(0, 0) = this->r11();
	e4f(0, 1) = this->r12();
	e4f(0, 2) = this->r13();
	e4f(0, 3) = this->o14();
	e4f(1, 0) = this->r21();
	e4f(1, 1) = this->r22();
	e4f(1, 2) = this->r23();
	e4f(1, 3) = this->o24();
	e4f(2, 0) = this->r31();
	e4f(2, 1) = this->r32();
	e4f(2, 2) = this->r33();
	e4f(2, 3) = this->o34();
	e4f(3, 0) = 0.0f;
	e4f(3, 1) = 0.0f;
	e4f(3, 2) = 0.0f;
	e4f(3, 3) = 1.0f;

	return e4f;
}

Transform Transform::operator*(const Transform & tr) const
{
	// 4 x 4 matrix multiplication
	Eigen::Matrix4f e4f = toEigen4f() * tr.toEigen4f();

	// make sure rotation is always normalized!
	Eigen::Affine3f af = Eigen::Affine3f(e4f);
	af.linear() = Eigen::Quaternionf(af.linear()).normalized().toRotationMatrix();

	return fromEigen3f(af);
}

Transform & Transform::operator*=(const Transform & tr)
{
	*this = *this * tr;
	return *this;
}

unsigned long Transform::getSize() {
	unsigned long memUsed = (unsigned long)(data_.total() * data_.elemSize());
	return memUsed;
}
