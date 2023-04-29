//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include <core/Transform.h>
#include <opencv2/core/core.hpp>

class Link
{
public:
	enum Type {
		Neighbor,
		LoopClosure,
		Undefined
	};

	Link();
	Link(
		int from,
		int to,
		Type type,
		const Transform &transform,
		const cv::Mat &infMatrix = cv::Mat::eye(6, 6, CV_64FC1));

	void setFrom(int from) { from_ = from; }
	int from() const { return from_; }

	void setTo(int to) { to_ = to; }
	int to() const { return to_; }

	void setTransform(const Transform &transform) { transform_ = transform; }
	const Transform &transform() const { return transform_; }

	void setType(Type type) { type_ = type; }
	Type type() const { return type_; }

	void setInfMatrix(const cv::Mat &infMatrix) { infMatrix_ = infMatrix; }
	const cv::Mat &infMatrix() const { return infMatrix_; }

	bool isValid();
	Link inverse() const;

	unsigned long getSize();

private:
	int from_;
	int to_;
	Transform transform_;
	Type type_;
	cv::Mat infMatrix_; // Information matrix = covariance matrix ^ -1
};

