//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/Link.h"
#include "core/Logger.h"

Link::Link()
{
	from_ = 0;
	to_ = 0;
	type_ = Undefined;
	infMatrix_ = cv::Mat::eye(6, 6, CV_64FC1);
}

Link::Link(
	int from,
	int to,
	Type type,
	const Transform &transform,
	const cv::Mat &infMatrix)
{
	from_ = from;
	to_ = to;
	transform_ = transform;
	type_ = type;
	setInfMatrix(infMatrix);
}

bool Link::isValid()
{
	bool valid = (from_ != 0) && (to_ != 0) && !transform_.isNull() && (type_ != Undefined);

	return valid;
}

Link Link::inverse() const
{
	Link link(
		to_,
		from_,
		type_,
		transform_.isNull() ? Transform() : transform_.inverse(),
		transform_.isNull() ? cv::Mat::eye(6, 6, CV_64FC1) : infMatrix_);

	return link;
}

unsigned long Link::getSize() {
	unsigned long memUsed = (unsigned long)(
		sizeof(Link) +
		transform_.getSize() +
		infMatrix_.total() * infMatrix_.elemSize());

	return memUsed;
}
