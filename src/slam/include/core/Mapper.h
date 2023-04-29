//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "core/Node.h"
#include "core/Transform.h"
#include "core/VWDictionary.h"
#include "core/Odometry.h"
#include "core/xThread.h"
#include "core/Parameters.h"

void getConnectedGraph(
	int fromId,
	std::map<int, Transform> & posesIn,
	std::multimap<int, Link> & linksIn,
	std::map<int, Transform> & posesOut,
	std::multimap<int, Link> & linksOut);

struct TH_PARAM {
	Node *node;
	VWDictionary *vwd;
	std::map<int, Node*> *nodes;
	std::map<int, double> *workingMem;
	Link link;
	int state;
};

void* thread(void* arg);
void* addWordIds(Node *node, VWDictionary *vwd);
void detectLoopClosure(std::map<int, Node*> &nodes, std::map<int, double> &workingMem, VWDictionary *_vwd, int id, Link *link);
std::map<int, float> computeLikelihood(Node *node, std::map<int, Node*> &_nodes, VWDictionary *_vwd, const std::list<int> & ids);

class Mapper
{
public:
	Mapper();
	virtual ~Mapper();

	bool process(SensorData &data, ODOM_INFO odomInfo, APP_SETTING appSetting);
	void init();
	void getGraph(std::map<int, Transform> &poses, std::multimap<int, Link> &links);
	cv::Mat getInformation(const cv::Mat & covariance) const;
	void cleanupThread();
	bool updateMemory(Node *node, cv::Mat &covariance);
	bool addLink(const Link &link);
	const std::map<int, double> &getWorkingMem() const { return _workingMem; }
	const std::set<int> &getStMem() const { return _stMem; }
	std::multimap<int, Link> getLinks(int nodeId) const;
	Node *getLastNode();
	const Node *getNode(int id) const;
	const VWDictionary *getVWDictionary() const;
	void getMemoryUsed();
	const std::map<int, Node*> &getNodes() const { return _nodes; }
	Node *_getNode(int id) const;
	void clearNodes();

private:
	void addNodeToStm(Node *node, const cv::Mat &covariance);
	void loadDataFromDb(bool postInitClosingEvents);
	void moveNodeToWMFromSTM();
	int getNextId();
	void initCountId();
	Node *createNode(SensorData &data, ODOM_INFO odomInfo);

	int _frameProcessed;
	int _intermediateCount;
	int _mapUpdate;
	std::map<int, float> _likelihood;
	int _key_id;
	int _maxStMemSize;
	int _idCount;
	int _idMapCount; // map id, reserved for multi-map session
	Node *_lastNode;
	std::map<int, Node*> _nodes;
	std::set<int> _stMem; // contains node IDs
	std::map<int, double> _workingMem; // <node ID, system time>
	VWDictionary *_vwd;
	TH_PARAM _th_param;
	xThread _xth;
};
