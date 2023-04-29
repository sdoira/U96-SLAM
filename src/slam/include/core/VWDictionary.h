//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "core/VisualWord.h"
#include "core/FlannIndex.h"

class VWDictionary
{
public:
	VWDictionary();
	~VWDictionary();
	std::list<int> addNewWords(const cv::Mat descriptors, int nodeId);
	const VisualWord* getWord(int id) const;
	void clear();
	void getMemoryUsed();

protected:
	int getNextId();
	std::map<int, VisualWord*> _visualWords; // <id, VisualWord*>

private:
	int _lastWordId;
	FlannIndex *_flannIndex;
	std::map<int, int> _mapIndexId; // <KDTree index, VW ID> of all VWs in the tree
};
