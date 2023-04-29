//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/VWDictionary.h"
#include "core/Perf.h"

extern Perf perf;

VWDictionary::VWDictionary()
{
	_lastWordId = 0;
	_flannIndex = new FlannIndex();
}

VWDictionary::~VWDictionary()
{
	this->clear();
	delete _flannIndex;
}

void VWDictionary::clear()
{
	for (auto itr = _visualWords.begin(); itr != _visualWords.end(); itr++)
	{
		delete (*itr).second;
	}

	_visualWords.clear();
	_lastWordId = 0;
	_mapIndexId.clear();
	_flannIndex->release();
}

int VWDictionary::getNextId()
{
	return _lastWordId++;
}

std::list<int> VWDictionary::addNewWords(const cv::Mat descriptorsIn, int nodeId)
{
	// local parameter
	float nndrRatio = 0.8f;

	// convert packed binary to float
	cv::Mat descriptors;
	descriptorsIn.convertTo(descriptors, CV_32F);

	// KNN search against existing all VWs in the dictionary
	cv::Mat results;
	cv::Mat dists;
	if (_flannIndex->isBuilt()){
		int KNN = 2;
		int KNN_CHECKS = 32;
		_flannIndex->knnSearch(descriptors, results, dists, KNN, KNN_CHECKS);
	}

	// determine if new VW is unique
	std::list<int> wordIds;
	for (int i = 0; i < descriptors.rows; i++)
	{
		// store <dist, VW index> pair for each descriptor
		std::multimap<float, int> fullResults;
		for (int j = 0; j < dists.cols; j++)
		{
			float dist = dists.at<float>(i, j);
			int index = (int)results.at<size_t>(i, j);
			auto itr = _mapIndexId.find(index); // VW index associated with KD-tree index
			fullResults.insert(std::pair<float, int>(dist, itr->second));
		}

		// apply NNDR
		bool isUnique = false;
		if (fullResults.size() < 2) {
			isUnique = true;
		}
		else if (fullResults.begin()->first > nndrRatio * (++fullResults.begin())->first) {
			isUnique = true;
		}

		// add new VW to VW dictionary
		if (isUnique)
		{
			// new descriptor is not similar to any existing VWs
			// -> add this descriptor as new VW
			VisualWord * vw = new VisualWord(getNextId(), descriptorsIn.row(i), nodeId);
			_visualWords.insert(_visualWords.end(), std::pair<int, VisualWord *>(vw->id(), vw));

			// add to KDTree
			int index = 0;
			if (!_flannIndex->isBuilt()) {
				int KDTREE_SIZE = 4;
				_flannIndex->buildKDTreeIndex(descriptors.row(i), KDTREE_SIZE);
			}
			else {
				index = _flannIndex->addPoints(descriptors.row(i)).front();
			}
			_mapIndexId.insert(std::pair<int, int>(index, vw->id())); // <KDTree index, VW ID>

			wordIds.push_back(vw->id());
		}
		else
		{
			// new descriptor is similar to existing VW
			// -> increment reference counter of that VW
			int vwid = fullResults.begin()->second;
			VisualWord *vw = _visualWords.find(vwid)->second;
			vw->addRef(nodeId);

			wordIds.push_back(vwid);
		}
	}

	return wordIds;
}

const VisualWord* VWDictionary::getWord(int id) const
{
	auto itr = _visualWords.find(id);
	if (itr != _visualWords.end())
	{
		return itr->second;
	}
	else {
		return (VisualWord*)0;
	}
}

void VWDictionary::getMemoryUsed()
{
	unsigned long memUsed = (unsigned long)(
		sizeof(VWDictionary) +
		sizeof(std::map<int, int>) +
		_mapIndexId.size() * (12 + sizeof(int) + sizeof(int)));

	for (auto itr = _visualWords.begin(); itr != _visualWords.end(); itr++) {
		memUsed += itr->second->getSize();
	}

	perf.registerMemoryUsed("VWDictionary", memUsed);

	perf.registerMemoryUsed("_flannIndex", _flannIndex->getSize());
}
