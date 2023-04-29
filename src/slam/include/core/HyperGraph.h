//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "Eigen/Sparse"
#include "EigenTypes.h"
#include "GraphVertex.h"
#include "GraphEdge.h"

class HyperGraph
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    HyperGraph();

    bool addVertex(Vertex *v);
    bool addEdge(Edge *e);
    Vertex *vertex(int id);

	void removeVertices();
	void removeEdges();

    int optimize(int iterations);
    void buildIndexMapping();
    double computeActiveErrors();
	void updateGraph(double *x);
	void buildSystem(
		int iteration,
		double *b,
		std::vector<Eigen::Triplet<double>> &coef,
		double *max_diag);
	void solveEigen(
		int iteration,
		double *b,
		std::vector<Eigen::Triplet<double>> &coef,
		double lambda,
		Eigen::VectorXd &eigen_x);
	double scaleLambda(double *x, double *b, double currentChi, double currentLambda);

protected:
    std::map<int, Vertex*> _vertices;
    std::vector<Edge*> _edges;
	int _dimension;

    int _numPoses;
	int _sizePoses;
	int _nextEdgeId;

    double _tau;
    double _lowerStep;
    double _upperStep;
};

