//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "core/Transform.h"
#include "core/Link.h"
#include "core/HyperGraph.h"
#include "core/GraphVertex.h"
#include "core/GraphEdge.h"

void addVertices(
	HyperGraph &graph,
	std::map<int, Transform> poses,
	std::list<Vertex*> &vertices);

void addEdges(
	HyperGraph &graph,
	std::multimap<int, Link> links,
	std::multimap<int, Edge*> &edges);

double runOptimize(
	std::map<int, Transform> poses,
	std::multimap<int, Link> links,
	int num,
	std::map<int, Transform> *optimized_poses);

double runOptimizeRobust(
	std::map<int, Transform> poses,
	std::multimap<int, Link> links,
	int num,
	std::map<int, Transform> *optimized_poses);
