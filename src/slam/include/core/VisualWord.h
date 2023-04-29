//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include <opencv2/core/core.hpp>
#include <map>

class VisualWord
{
public:
	VisualWord(int id, const cv::Mat &descriptor, int nodeId = 0);
	~VisualWord();

	void addRef(int nodeId);
	int removeAllRef(int nodeId);

	int getTotalReferences() const { return _totalReferences; }
	int id() const { return _id; }
	const std::map<int, int> &getReferences() const { return _references; } // (node id , occurrence in the node)
	unsigned long getSize();

private:
	int _id;
	int _totalReferences;
	std::map<int, int> _references; // (node id , occurrence in the node)
};

