//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/FlannIndex.h"
#include "flann/flann.hpp"

FlannIndex::FlannIndex()
{
	index_ = 0;
	nextIndex_ = 0;
	featuresType_ = 0;
	featuresDim_ = 0;
	featureSize_ = 0;
}

FlannIndex::~FlannIndex()
{
	this->release();
}

void FlannIndex::release()
{
	if (index_) {
		delete (flann::Index<flann::L1<float>>*)index_;
		index_ = 0;
	}

	nextIndex_ = 0;
	addedDescriptors_.clear();
}

unsigned int FlannIndex::indexedFeatures()
{
	if (!index_) {
		return 0;
	}
	return (unsigned int)((const flann::Index<flann::L1<float>>*)index_)->size();
}

void FlannIndex::buildKDTreeIndex(const cv::Mat &features, int trees)
{
	this->release();
	featuresType_ = features.type();
	featuresDim_ = features.cols;
	featureSize_ = features.rows * features.cols * sizeof(float);

	flann::KDTreeIndexParams params(trees);

	flann::Matrix<float> dataset((float*)features.data, features.rows, features.cols);
	index_ = new flann::Index<flann::L1<float> >(dataset, params);
	((flann::Index<flann::L1<float> >*)index_)->buildIndex();

	for (int i = 0; i<features.rows; ++i) {
		addedDescriptors_.insert(std::make_pair(nextIndex_, features.row(i)));
		nextIndex_++;
	}
}

bool FlannIndex::isBuilt()
{
	return (index_ != 0);
}

std::vector<unsigned int> FlannIndex::addPoints(const cv::Mat & features)
{
	flann::Matrix<float> points((float*)features.data, features.rows, features.cols);
	flann::Index<flann::L2<float>> *index = (flann::Index<flann::L2<float>>*)index_;
	index->addPoints(points, 0);

	std::vector<unsigned int> indexes;
	for (int i = 0; i<features.rows; ++i)
	{
		indexes.push_back(nextIndex_);
		addedDescriptors_.insert(std::make_pair(nextIndex_, features.row(i)));
		nextIndex_++;
	}

	return indexes;
}

void FlannIndex::knnSearch(const cv::Mat &query, cv::Mat &indices, cv::Mat &dists, int knn, int checks)
{
	indices.create(query.rows, knn, sizeof(size_t) == 8 ? CV_64F : CV_32S);
	dists.create(query.rows, knn, featuresType_ == CV_8UC1 ? CV_32S : CV_32F);

	flann::Matrix<size_t> indicesF((size_t*)indices.data, indices.rows, indices.cols);
	flann::SearchParams params = flann::SearchParams(checks);
	flann::Matrix<float> distsF((float*)dists.data, dists.rows, dists.cols);
	flann::Matrix<float> queryF((float*)query.data, query.rows, query.cols);

	((flann::Index<flann::L2<float> >*)index_)->knnSearch(queryF, indicesF, distsF, knn, params);
}

unsigned long FlannIndex::getSize()
{
	unsigned long sizeofDescriptor;
	if (addedDescriptors_.size() > 0) {
		sizeofDescriptor = (unsigned long)(
			sizeof(cv::Mat) +
			addedDescriptors_.begin()->second.total() * addedDescriptors_.begin()->second.elemSize());
	}
	else {
		sizeofDescriptor = 0;
	}

	unsigned long memUsed = (unsigned long)(
		sizeof(FlannIndex) +
		nextIndex_ * featureSize_ +
		sizeof(std::map<int, cv::Mat>) +
		addedDescriptors_.size() * (12 + sizeof(int) + sizeofDescriptor));

	return memUsed;
}
