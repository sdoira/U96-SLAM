//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include <list>
#include <opencv2/opencv.hpp>

class FlannIndex
{
public:
	FlannIndex();
	virtual ~FlannIndex();

	void release();
	unsigned int indexedFeatures();
	void buildKDTreeIndex(const cv::Mat &features, int trees);
	bool isBuilt();
	std::vector<unsigned int> addPoints(const cv::Mat & features);
	void knnSearch(const cv::Mat &query, cv::Mat &indices, cv::Mat &dists, int knn, int checks);

	unsigned long getSize();

private:
	void *index_;
	unsigned int nextIndex_;
	int featuresType_;
	int featuresDim_;
	unsigned long featureSize_;
	std::map<int, cv::Mat> addedDescriptors_;
};

