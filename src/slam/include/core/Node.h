//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include <map>
#include <list>
#include <vector>
#include <set>
#include <core/Transform.h>
#include <core/SensorData.h>
#include <core/Link.h>

class Node
{

public:
	Node();
	Node(int id, int mapId, int weight, const Transform &pose, const SensorData &sensorData);
	Node(const SensorData & data);
	virtual ~Node();

	int id() const { return _id; }
	int mapId() const { return _mapId; }

	void setWeight(int weight) { _weight = weight; }
	int getWeight() const { return _weight; }

	double getStamp() const { return _stamp; }

	void addLink(const Link & link);
	bool hasLink(int idTo, Link::Type type = Link::Undefined) const;
	const std::multimap<int, Link> & getLinks() const { return _links; }

	void setWords(std::multimap<int, int> words) { _words = words; }
	const std::multimap<int, int> & getWords() const { return _words; }

	void setVelocity(Transform velocity) { _velocity = velocity; }
	const Transform getVelocity() const { return _velocity; }

	SensorData &sensorData() { return _sensorData; }
	const SensorData &sensorData() const { return _sensorData; }

	const Transform & getPose() const { return _pose; }
	const Transform &groundTruth() const { return _groundTruth; }

	void getMemoryUsed();

private:
	int _id;
	int _mapId; // reserved for multi-map session
	double _stamp;
	std::multimap<int, Link> _links; // <node ID, Link>
	int _weight;

	std::multimap<int, int> _words; // word <VW ID, keypoint index>

	Transform _pose;
	Transform _velocity;

	SensorData _sensorData;
	Transform _groundTruth;
};
