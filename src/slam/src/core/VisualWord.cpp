//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/VisualWord.h"
#include "core/Logger.h"

VisualWord::VisualWord(int id, const cv::Mat &descriptor, int nodeId)
{
	_id = id;
	_totalReferences = 0;
	_references.clear();

	if (nodeId)
	{
		addRef(nodeId);
	}
}

VisualWord::~VisualWord()
{
}

void VisualWord::addRef(int nodeId)
{
	auto iter = _references.find(nodeId);
	if (iter != _references.end())
	{
		(*iter).second += 1;
	}
	else
	{
		_references.insert(std::pair<int, int>(nodeId, 1));
	}
	++_totalReferences;
}

int VisualWord::removeAllRef(int nodeId)
{
	int removed;
	auto itr = _references.find(nodeId);
	if (itr != _references.end()) {
		removed = itr->second;
		_references.erase(itr);
	}
	else {
		removed = 0;
	}

	_totalReferences -= removed;
	return removed;
}

unsigned long VisualWord::getSize()
{
	unsigned long memUsed = (unsigned long)(
		sizeof(VisualWord) +
		sizeof(std::map<int, int>) +
		_references.size() * (12 + sizeof(int) + sizeof(int)));

	return memUsed;
}
