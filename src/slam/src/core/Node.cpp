//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/Node.h"
#include "core/Perf.h"
#include <core/Logger.h>

extern Perf perf;

Node::Node()
{
	_id = 0;
	_mapId = -1;
	_stamp = 0.0;
	_weight = 0;
}

Node::Node(
	int id,
	int mapId,
	int weight,
	const Transform & pose,
	const SensorData & sensorData)
{
	_id = id;
	_mapId = mapId;
	_stamp = sensorData.stamp();
	_weight = weight;
	_pose = pose;
	_sensorData = sensorData;

	_groundTruth = sensorData.groundTruth();
}

Node::~Node()
{
}

void Node::addLink(const Link & link)
{
	_links.insert(std::make_pair(link.to(), link));
}

bool Node::hasLink(int idTo, Link::Type type) const
{
	if (type == Link::Undefined) {
		return _links.find(idTo) != _links.end();
	}
	if (idTo == 0) {
		for (auto iter = _links.begin(); iter != _links.end(); ++iter) {
			if (type == iter->second.type()) {
				return true;
			}
		}
	}
	else {
		for (auto iter = _links.find(idTo); iter != _links.end() && iter->first == idTo; ++iter) {
			if (type == iter->second.type()) {
				return true;
			}
		}
	}

	return false;
}

void Node::getMemoryUsed()
{
	unsigned long memUsed = (unsigned long)(
		sizeof(Node) +
		_pose.getSize() * 3 +
		_words.size() * (12 + sizeof(int) + sizeof(int)));

	if (_links.size() > 0) {
		memUsed += (unsigned long)(_links.size() * (12 + sizeof(int) + _links.begin()->second.getSize()));
	}

	perf.registerMemoryUsed("Node", memUsed);

	// SensorData
	_sensorData.getMemoryUsed();
}
