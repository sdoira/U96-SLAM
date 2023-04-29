//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/Optimizer.h"
#include "core/Mapper.h"

void addVertices(
	HyperGraph &graph,
	std::map<int, Transform> poses,
	std::list<Vertex*> &vertices)
{
	vertices.clear();

	for (auto itr = poses.begin(); itr != poses.end(); itr++)
	{
		int id = itr->first;

		Eigen::Isometry3d pose;
		pose(0, 0) = itr->second.r11();
		pose(0, 1) = itr->second.r12();
		pose(0, 2) = itr->second.r13();
		pose(0, 3) = itr->second.o14();
		pose(1, 0) = itr->second.r21();
		pose(1, 1) = itr->second.r22();
		pose(1, 2) = itr->second.r23();
		pose(1, 3) = itr->second.o24();
		pose(2, 0) = itr->second.r31();
		pose(2, 1) = itr->second.r32();
		pose(2, 2) = itr->second.r33();
		pose(2, 3) = itr->second.o34();

		Vertex *v = new Vertex();
		v->setEstimate(pose);
		v->setId(id);
		if (id == 1) {
			v->setFixed(true);
		}

		graph.addVertex(v);
		vertices.push_back(v);
	}
}

void addEdges(
	HyperGraph &graph,
	std::multimap<int, Link> links,
	std::multimap<int, Edge*> &edges)
{
	edges.clear();

	for (auto itr = links.begin(); itr != links.end(); itr++)
	{
		int id1 = itr->second.from();
		int id2 = itr->second.to();

		Transform tr = itr->second.transform();
		Eigen::Isometry3d pose;
		pose(0, 0) = tr.r11();
		pose(0, 1) = tr.r12();
		pose(0, 2) = tr.r13();
		pose(0, 3) = tr.o14();
		pose(1, 0) = tr.r21();
		pose(1, 1) = tr.r22();
		pose(1, 2) = tr.r23();
		pose(1, 3) = tr.o24();
		pose(2, 0) = tr.r31();
		pose(2, 1) = tr.r32();
		pose(2, 2) = tr.r33();
		pose(2, 3) = tr.o34();

		const cv::Mat *infMatrix = &itr->second.infMatrix();
		Eigen::Matrix<double, 6, 6> inf;
		for (int row = 0; row < infMatrix->rows; row++) {
			for (int col = 0; col < infMatrix->cols; col++) {
				inf(row, col) = infMatrix->at<double>(row, col);
			}
		}

		Edge *e = new Edge();
		Vertex *v1 = graph.vertex(id1);
		Vertex *v2 = graph.vertex(id2);
		e->setVertex(0, v1);
		e->setVertex(1, v2);
		e->setMeasurement(pose);
		e->setInformation(inf);
		e->setInternalId((int)edges.size());

		graph.addEdge(e);
		edges.insert(std::make_pair(itr->first, e));
	}
}

double runOptimize(
	std::map<int, Transform> poses,
	std::multimap<int, Link> links,
	int num,
	std::map<int, Transform> *optimized_poses)
{
	HyperGraph graph;
	std::list<Vertex*> vertices;
	std::multimap<int, Edge*> edges;

	//--------------------------------------------------------------
	// Add vertices/edges to the graph
	//--------------------------------------------------------------
	addVertices(graph, poses, vertices);
	addEdges(graph, links, edges);

	//--------------------------------------------------------------
	// Optimize
	//--------------------------------------------------------------
	graph.optimize(num);
	double err = graph.computeActiveErrors();

	//--------------------------------------------------------------
	// Save optimized poses
	//--------------------------------------------------------------
	for (auto itr = vertices.begin(); itr != vertices.end(); itr++)
	{
		Vertex *v = *itr;
		int id = v->id();

		Eigen::Isometry3d pose;
		if (v->fixed()) {
			// graph optimizer computes only non-fixed nodes
			pose = v->estimate();
		}
		else {
			pose = graph.vertex(id)->estimate();
		}

		Transform tr = Transform(
			pose(0, 0), pose(0, 1), pose(0, 2), pose(0, 3),
			pose(1, 0), pose(1, 1), pose(1, 2), pose(1, 3),
			pose(2, 0), pose(2, 1), pose(2, 2), pose(2, 3));

		optimized_poses->insert(std::make_pair(id, tr));
	}

	graph.removeVertices();
	graph.removeEdges();

	return err;
}

double runOptimizeRobust(
	std::map<int, Transform> poses,
	std::multimap<int, Link> links,
	int num,
	std::map<int, Transform> *optimized_poses)
{
	std::multimap<int, Link> inliers(links);

	double err;
	double thr = 10.0;
	while (1)
	{
		//--------------------------------------------------------------
		// Rebuild graph
		//--------------------------------------------------------------
		std::map<int, Transform> posesOut;
		std::multimap<int, Link> linksOut;
		getConnectedGraph(1, poses, inliers, posesOut, linksOut);

		//--------------------------------------------------------------
		//	Graph Optimization
		//--------------------------------------------------------------
		HyperGraph graph;

		std::list<Vertex*> vertices;
		addVertices(graph, posesOut, vertices);

		std::multimap<int, Edge*> edges;
		addEdges(graph, linksOut, edges);

		graph.optimize(5);
		graph.computeActiveErrors();

		//--------------------------------------------------------------
		//	Outlier removal
		//--------------------------------------------------------------
		int outlier_id1 = -1;
		int outlier_id2 = -1;
		double outlier_err = 0;
		for (auto itr = edges.begin(); itr != edges.end(); ++itr)
		{
			std::vector<Vertex*> vertices = itr->second->vertices();
			int id1 = vertices[0]->id();
			int id2 = vertices[1]->id();
			double err = itr->second->chi2();

			// apply thresholding to the LC link with the biggest error
			if ((id1 != id2 + 1) && (id2 != id1 + 1) && (err >= thr)) {
				if (err > outlier_err) {
					outlier_id1 = id1;
					outlier_id2 = id2;
					outlier_err = err;
				}
			}
		}

		if (outlier_id1 == -1) {
			// all errors are within the threshold
			err = runOptimize(posesOut, linksOut, num, optimized_poses);
			break;
		}
		else {
			// remove LC link with the biggest error
			for (auto itr = linksOut.begin(); itr != linksOut.end(); ++itr)
			{
				if ((itr->second.from() == outlier_id1) && (itr->second.to() == outlier_id2)) {
					linksOut.erase(itr);
				}
			}

			inliers = linksOut;
		}

		graph.removeVertices();
		graph.removeEdges();
	}

	return err;
}
