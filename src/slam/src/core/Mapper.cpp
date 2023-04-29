//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/Mapper.h"
#include "core/Registration.h"
#include "core/Graph.h"
#include "core/Perf.h"
#include "core/Logger.h"

extern Perf perf;

Mapper::Mapper()
{
	_frameProcessed = 0;
	_intermediateCount = 0;
	_mapUpdate = 5;
	_maxStMemSize = 30;
	_idCount = 0;
	_idMapCount = 0;
	_lastNode = 0;
	_vwd = new VWDictionary();
	_th_param.state = 0;

	// change thread priority
	_xth.lowerProirity();

}

Mapper::~Mapper()
{
	clearNodes();
	delete _vwd;
}

void Mapper::cleanupThread() {
	if (_th_param.state != 0) {
		_xth.join();
		Link link = _th_param.link;
		if ((link.from() != 0) && (link.to() != 0)) {
			addLink(link);
		}
		_th_param.state = 0;
	}
}

void Mapper::init()
{
	_stMem.clear();
	_workingMem.clear();
	clearNodes();

	_lastNode = 0;
	_idCount = 0;
	_idMapCount = 0;

	if (_vwd) {
		_vwd->clear();
	}
}

void Mapper::clearNodes()
{
	for (auto itr = _nodes.begin(); itr != _nodes.end(); itr++) {
		delete itr->second;
	}

	_nodes.clear();
}

bool Mapper::process(SensorData &data, ODOM_INFO odomInfo, APP_SETTING appSetting)
{
	if (
		(_intermediateCount >= (_mapUpdate - 1)) &&
		!((appSetting.appType == APP_TYPE_SLAM_REALTIME) && (_th_param.state == 1)))
	{
		// not intermediate node
		_intermediateCount = 0;
	}
	else {
		// set as an intermediate node
		data.setIntermediate(true);
		data.clearFeatures();
		_intermediateCount++;
	}


	//==================================================================
	// Create a node and detect loop-closure
	//==================================================================
	if (data.isIntermediate()) {
		// intermediate node, no loop-closure detection
		Node *node = createNode(data, odomInfo);
		updateMemory(node, odomInfo.covariance);
	}
	else {
		// wait the thread from the previous cycle, then detect loop-closure
		if (_th_param.state != 0) {
			perf.startTime("join");
			_xth.join();
			perf.stopTime("join");
			Link link = _th_param.link;
			if ((link.from() != 0) && (link.to() != 0)) {
				addLink(link);
			}
		}

		// create a node
		Node *node = createNode(data, odomInfo);
		updateMemory(node, odomInfo.covariance);

		// create a thread and run
		_th_param.node = node;
		_th_param.vwd = _vwd;
		_th_param.nodes = &_nodes;
		_th_param.workingMem = &_workingMem;
		_th_param.link.setFrom(0); // mark as an invalid link
		_th_param.link.setTo(0);
		_th_param.state = 1; // thread is running
		TH_PARAM *pth_param = &_th_param;
		_xth.create(thread, (void*)pth_param);

		_key_id = node->id();
	}

	_frameProcessed++;

	return true;
}

void Mapper::getGraph(
	std::map<int, Transform> &poses,
	std::multimap<int, Link> &links)
{
	//============================================================
	// Retrieve all connected nodes
	//============================================================
	// starting from the latest node, returns all linked nodes.
	std::set<int> ids;
	std::list<int> curentSearchId;
	std::set<int> nextSearchId;

	// last node ID is the starting point
	nextSearchId.insert(getLastNode()->id());

	// recursively search all the connected nodes
	while (nextSearchId.size())
	{
		curentSearchId = std::list<int>(nextSearchId.rbegin(), nextSearchId.rend());
		nextSearchId.clear();

		for (std::list<int>::iterator jter = curentSearchId.begin(); jter != curentSearchId.end(); ++jter)
		{
			ids.insert(*jter);

			const Node *node = this->getNode(*jter);
			const std::multimap<int, Link> *links = &node->getLinks();
			for (std::multimap<int, Link>::const_iterator iter = links->begin(); iter != links->end(); ++iter) {
				if (ids.find(iter->first) == ids.end()) {
					nextSearchId.insert(iter->first);
				}
			}
		}
	}

	//============================================================
	// Retrieve poses and links
	//============================================================
	// Given node IDs, set their poses to "poses", all connected links to "constraints" 
	for (std::set<int>::const_iterator itr_ids = ids.begin(); itr_ids != ids.end(); ++itr_ids)
	{
		// retreive poses
		const Node *node = this->getNode(*itr_ids);
		poses.insert(std::make_pair(*itr_ids, node->getPose()));

		// retreive links
		std::multimap<int, Link> tmpLinks = getLinks(*itr_ids);
		for (std::multimap<int, Link>::iterator itr_links = tmpLinks.begin(); itr_links != tmpLinks.end(); ++itr_links)
		{
			std::multimap<int, Link>::iterator itr_findLink = findLink(links, *itr_ids, itr_links->first);
			if (itr_findLink == links.end()) {
				links.insert(std::make_pair(*itr_ids, itr_links->second));
			}
		}
	}
}

cv::Mat Mapper::getInformation(const cv::Mat & covariance) const
{
	cv::Mat information = covariance.inv();
	return information;
}

// reconnect the graph by selecting the shortest path, recompute the camera poses along the way.
void getConnectedGraph(
	int fromId,
	std::map<int, Transform> & posesIn,
	std::multimap<int, Link> & linksIn,
	std::map<int, Transform> & posesOut,
	std::multimap<int, Link> & linksOut)
{
	posesOut.clear();
	linksOut.clear();

	std::set<int> nextPoses;
	nextPoses.insert(fromId);

	std::multimap<int, int> biLinks;
	for (auto itr_linksIn = linksIn.begin(); itr_linksIn != linksIn.end(); ++itr_linksIn)
	{
		biLinks.insert(std::make_pair(itr_linksIn->second.from(), itr_linksIn->second.to()));
		biLinks.insert(std::make_pair(itr_linksIn->second.to(), itr_linksIn->second.from()));
	}

	while (nextPoses.size()) {
		int currentId = *nextPoses.rbegin();
		nextPoses.erase(*nextPoses.rbegin());

		if (posesOut.empty()) {
			// only for the 1st time
			posesOut.insert(std::make_pair(currentId, posesIn.find(currentId)->second));
		}

		for (
			auto itr_biLinks = biLinks.find(currentId);
			itr_biLinks != biLinks.end() && itr_biLinks->first == currentId;
			++itr_biLinks)
		{
			int toId = itr_biLinks->second;

			std::multimap<int, Link>::const_iterator itr_findLink = findLink(linksIn, currentId, toId);
			if (nextPoses.find(toId) == nextPoses.end()) {
				if (posesOut.find(toId) == posesOut.end()) {
					Transform t;
					if (itr_findLink->second.from() == currentId) {
						// forward link
						t = posesOut.at(currentId) * itr_findLink->second.transform();
					}
					else {
						// reverse link
						t = posesOut.at(currentId) * itr_findLink->second.transform().inverse();
					}
					posesOut.insert(std::make_pair(toId, t));

					nextPoses.insert(toId);
				}

				// only add unique links
				if (findLink(linksOut, currentId, toId) == linksOut.end()) {
					linksOut.insert(*itr_findLink);
				}
			}
		}
	}
}

bool Mapper::updateMemory(Node *node, cv::Mat &covariance)
{
	this->addNodeToStm(node, covariance);
	_lastNode = node;

	//============================================================
	// Transfer the oldest node of the short-term memory to
	// the working memory
	//============================================================
	int notIntermediateNodesCount = 0;
	for (std::set<int>::iterator iter = _stMem.begin(); iter != _stMem.end(); ++iter)
	{
		const Node *node = this->getNode(*iter);
		if (node->getWeight() >= 0)
		{
			++notIntermediateNodesCount;
		}
	}

	while (_stMem.size() && _maxStMemSize>0 && notIntermediateNodesCount > _maxStMemSize)
	{
		int id = *_stMem.begin();
		Node *node = this->_getNode(id);
		if (node->getWeight() >= 0)
		{
			--notIntermediateNodesCount;
		}

		moveNodeToWMFromSTM();
	}

	return true;
}

void Mapper::addNodeToStm(Node *node, const cv::Mat &covariance)
{
	if (_stMem.size())
	{
		// motion
		Transform motionEstimate = _nodes.at(*_stMem.rbegin())->getPose().inverse() * node->getPose();

		// information matrix
		cv::Mat infMatrix = cv::Mat::zeros(6, 6, CV_64FC1);
		infMatrix.at<double>(0, 0) = 1.0 / covariance.at<double>(0, 0);
		infMatrix.at<double>(1, 1) = 1.0 / covariance.at<double>(1, 1);
		infMatrix.at<double>(2, 2) = 1.0 / covariance.at<double>(2, 2);
		infMatrix.at<double>(3, 3) = 1.0 / covariance.at<double>(3, 3);
		infMatrix.at<double>(4, 4) = 1.0 / covariance.at<double>(4, 4);
		infMatrix.at<double>(5, 5) = 1.0 / covariance.at<double>(5, 5);

		// add link
		Link link_forward = Link(*_stMem.rbegin(), node->id(), Link::Neighbor, motionEstimate, infMatrix);
		_nodes.at(*_stMem.rbegin())->addLink(link_forward);

		Link link_reverse = Link(node->id(), *_stMem.rbegin(), Link::Neighbor, motionEstimate.inverse(), infMatrix);
		node->addLink(link_reverse);
	}

	_nodes.insert(_nodes.end(), std::pair<int, Node *>(node->id(), node));
	_stMem.insert(_stMem.end(), node->id());
}

void Mapper::moveNodeToWMFromSTM()
{
	_workingMem.insert(_workingMem.end(), std::make_pair(*_stMem.begin(), currentTimeSec()));
	_stMem.erase(*_stMem.begin());
}

const Node * Mapper::getNode(int id) const
{
	return _getNode(id);
}

Node * Mapper::_getNode(int id) const
{
	auto itr = _nodes.find(id);
	if (itr != _nodes.end()) {
		return itr->second;
	}
	else {
		return (Node*)0;
	}
}

const VWDictionary * Mapper::getVWDictionary() const
{
	return _vwd;
}

std::multimap<int, Link> Mapper::getLinks(int nodeId) const
{
	std::multimap<int, Link> links;
	if (nodeId > 0)
	{
		auto itr = _nodes.find(nodeId);
		if (itr != _nodes.end()) {
			links = itr->second->getLinks();
		}
		else {
			LOG_WARN("Cannot find node %d in memory\n", nodeId);
		}
	}

	return links;
}

int Mapper::getNextId()
{
	return ++_idCount;
}

Node* Mapper::getLastNode()
{
	return _lastNode;
}

bool Mapper::addLink(const Link &link)
{
	Node *to = _getNode(link.to());
	Node *from = _getNode(link.from());
	if (to && from) {
		if (to->hasLink(link.from())) {
			return true;
		}

		to->addLink(link.inverse());
		from->addLink(link);

		from->setWeight(from->getWeight() + to->getWeight());
		to->setWeight(0);
	}

	return true;
}

Node * Mapper::createNode(SensorData &data, ODOM_INFO odomInfo)
{
	data.clearRawData();

	Node *node = new Node(
		this->getNextId(),	// get unique ID for new node
		_idMapCount,		// mapId
		0,					// weight
		odomInfo.pose,		// pose, camera pose in this frame (robot coords)
		data);				// sensorData

	if (data.isIntermediate()) {
		// set as intermediate node
		node->setWeight(-1);
	}

	node->setVelocity(odomInfo.velocity);

	return node;
}

void* addWordIds(Node *node, VWDictionary *vwd)
{
	std::vector<cv::KeyPoint> keypoints = node->sensorData().keypoints();
	cv::Mat descriptors = node->sensorData().descriptors().clone();

	//==================================================================
	// Limit the number of keypoints to be stored in node
	//==================================================================
	std::vector<bool> inliers;
	cv::Mat descriptorsForVwd = descriptors;
	std::vector<int> indexForVwd;
	int maxFeatures = 750;
	bool kptsLimited = false;

	if (descriptors.rows > maxFeatures)
	{
		kptsLimited = true;

		// limit the number of keypoints based on "response" field.
		// corresponding bits in "inliers" will be set.
		node->sensorData().limitKeypoints(keypoints, inliers, maxFeatures);

		// copy survived descriptors to "descriptorsForVwd".
		// "indexForVwd" contains indices of that descriptors.
		descriptorsForVwd = cv::Mat(maxFeatures, descriptors.cols, descriptors.type());
		indexForVwd.resize(maxFeatures);
		unsigned int oi = 0;
		for (int k = 0; k < descriptors.rows; ++k) {
			if (inliers[k]) {
				memcpy(descriptorsForVwd.ptr<char>(oi), descriptors.ptr<char>(k), descriptors.cols*sizeof(char));
				indexForVwd[oi] = k;
				++oi;
			}
		}
	}

	//==================================================================
	// Update VW dictionary with new words
	//------------------------------------------------------------------
	// "addedWordIds" contains VW IDs given to new descriptors.
	//==================================================================
	std::list<int> addedWordIds;
	addedWordIds = vwd->addNewWords(descriptorsForVwd, node->id());

	// "wordsIds" is a list of VW IDs associated with input keypoints.
	// Keypoints whose descriptors are not added to VW dictionary will
	// have unique negative IDs.
	std::list<int> wordIds;
	if (kptsLimited) {
		int negIndex = -1;
		auto itr = addedWordIds.begin();
		for (int i = 0; i < (int)keypoints.size(); i++) {
			if (inliers[i]) {
				wordIds.push_back(*itr++);
			}
			else {
				wordIds.push_back(negIndex--);
			}
		}
	}
	else {
		wordIds = addedWordIds;
	}

	std::multimap<int, int> words;
	for (auto iter = wordIds.begin(); iter != wordIds.end(); iter++) {
		words.insert(std::make_pair(*iter, words.size())); // <VW.ID, keypoint index>
	}
	node->setWords(words);

	return 0;
}

void Mapper::getMemoryUsed()
{
	unsigned long memUsed = (unsigned long)(
		sizeof(Mapper) +
		sizeof(std::map<int, float>) +
		_likelihood.size() * (12 + sizeof(int) + sizeof(float)) +
		sizeof(std::set<int>) +
		_stMem.size() * (8 + sizeof(int)) +
		sizeof(std::map<int, double>) +
		_workingMem.size() * (12 + sizeof(int) + sizeof(double)) +
		sizeof(std::map<int, Node*>));

	perf.registerMemoryUsed("Mapper", memUsed);

	for (auto itr = _nodes.begin(); itr != _nodes.end(); itr++) {
		itr->second->getMemoryUsed();
	}

	_vwd->getMemoryUsed();
}

void* thread(void *param)
{
	TH_PARAM *th_param = (TH_PARAM*)param;

	// VW dictionary update
	perf.startTime("addWordIds");
	addWordIds(th_param->node, th_param->vwd);
	perf.stopTime("addWordIds");

	// detect loop closure
	perf.startTime("detectLoopClosure");
	detectLoopClosure(*th_param->nodes, *th_param->workingMem, th_param->vwd, th_param->node->id(), &th_param->link);
	perf.stopTime("detectLoopClosure");

	th_param->state = 2; // thread is complete
	return 0;
}

Node *findNode(std::map<int, Node*> &nodes, int id)
{
	auto itr = nodes.find(id);
	if (itr != nodes.end()) {
		return itr->second;
	}
	else {
		return (Node*)0;
	}
}

void detectLoopClosure(
	std::map<int, Node*> &nodes,
	std::map<int, double> &workingMem,
	VWDictionary *_vwd,
	int id,
	Link *link)
{
	Node *node = findNode(nodes, id);

	if ((node->getWeight() >= 0) && workingMem.size())
	{
		//============================================================
		// Search all nodes, set their IDs to nodesToCompare 
		// unless they are intermediate nodes.
		//------------------------------------------------------------
		// std::map<int, double> _workingMem
		//   <Node ID, System time when moved from StM to WM>
		//============================================================
		std::list<int> nodesToCompare;
		for (auto iter = workingMem.begin(); iter != workingMem.end(); iter++)
		{
			const Node *n = findNode(nodes, iter->first);
			if (n->getWeight() != -1) {
				// ignore intermediate nodes
				nodesToCompare.push_back(iter->first);
			}
		}

		// For a given node, calcualtes likelihood against all other nodes in WM
		std::map<int, float> likelihood = computeLikelihood(node, nodes, _vwd, nodesToCompare);

		//============================================================
		// Select the highest hypothesis
		//============================================================
		std::pair<int, float> highestHypothesis = std::make_pair(0, 0.0f);
		for (auto iter = likelihood.begin(); iter != likelihood.end(); ++iter) {
			if (iter->first > 0 && iter->second > highestHypothesis.second) {
				highestHypothesis = *iter;
			}
		}

		//============================================================
		// compute LC transform
		//============================================================
		float loopThr = 0.2f;
		if (highestHypothesis.second >= loopThr)
		{
			int fromId = highestHypothesis.first;
			int toId = node->id();
			SensorData sensorFrom = findNode(nodes, fromId)->sensorData();
			SensorData sensorTo = findNode(nodes, toId)->sensorData();
			struct REG_INFO reg_info;
			reg_info.covariance = cv::Mat::eye(6, 6, CV_64FC1);
			Transform transform = computeTransform(sensorFrom, sensorTo, Transform(), &reg_info);

			if (transform.isNull()) {
				LOG_INFO(" LC rejected[%d,%d,%f] ", toId, fromId, highestHypothesis.second);
			}
			else {
				LOG_INFO(" LC accepted[%d,%d,%f] ", toId, fromId, highestHypothesis.second);

				// adds a link between the nodes
				transform = transform.inverse();
				cv::Mat information = reg_info.covariance.inv();
				*link = Link(node->id(), highestHypothesis.first, Link::LoopClosure, transform, information);
			}
		}
	}
}

std::map<int, float> computeLikelihood(
	Node *node,
	std::map<int, Node*> &nodes,
	VWDictionary *vwd,
	const std::list<int> & ids)
{
	std::map<int, float> likelihood;
	std::map<int, float> calculatedWordsRatio;

	for (auto iter = ids.begin(); iter != ids.end(); ++iter)
	{
		likelihood.insert(likelihood.end(), std::pair<int, float>(*iter, 0.0f));
	}

	std::multimap<int, int> words = node->getWords();
	std::list<int> wordIds;
	std::list<int>::reverse_iterator lastValue;
	for (auto iter = words.begin(); iter != words.end(); ++iter)
	{
		if (iter == words.begin() || (iter != words.begin() && *lastValue != iter->first))
		{
			wordIds.push_back(iter->first);
			lastValue = wordIds.rbegin();
		}
	}

	// tf-idf = (nwi / ni) log (N / nw)
	// where nwi: the number of occurences of word w in image i
	//       ni : the total number of words in image i
	//       nw : the number of images containig word w
	//       N  : the total number of images
	// [reference]
	// Fast and Incremental Method for Loop-Closure Detection Using Bags of Visual Words
	// (Angeli 2008)

	float N = (float)nodes.size();
	if (N)
	{
		for (auto i = wordIds.begin(); i != wordIds.end(); ++i)
		{
			if (*i > 0)
			{
				const VisualWord *vw = vwd->getWord(*i);
				const std::map<int, int> & refs = vw->getReferences();
				float nw = (float)refs.size();

				float logNnw = log10(float(N) / nw);
				if (logNnw)
				{
					for (auto j = refs.begin(); j != refs.end(); ++j)
					{
						auto iter = likelihood.find(j->first);
						if (iter != likelihood.end())
						{
							float nwi = (float)j->second;

							int ni = 0;
							Node *node = findNode(nodes, j->first);
							if (node)
							{
								ni = (int)((Node*)node)->getWords().size();
								iter->second += (nwi  * logNnw) / ni;
							}
						}
					}
				}
			}
		}
	}

	return likelihood;
}
