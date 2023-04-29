//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/HyperGraph.h"
#include "core/Logger.h"

HyperGraph::HyperGraph()
{
	_dimension = 6;
	_lowerStep = 1. / 3.; // from g2o impl
	_upperStep = 2. / 3.;
    _nextEdgeId = 0;
    _numPoses = 0;
	_sizePoses = 0;
    _tau = 1e-5;
	_vertices.clear();
	_edges.clear();
}

bool HyperGraph::addVertex(Vertex *v)
{
    auto result = _vertices.insert(std::make_pair(v->id(), v));
    return result.second;
}

bool HyperGraph::addEdge(Edge *e)
{
    _edges.push_back(e);
    e->setInternalId(_nextEdgeId++);

    return true;
}

Vertex* HyperGraph::vertex(int id)
{
    auto it = _vertices.find(id);
    if (it == _vertices.end()) {
        return nullptr;
    }

    return it->second;
}

void HyperGraph::removeVertices() {
	for (auto itr = _vertices.begin(); itr != _vertices.end(); itr++) {
		delete itr->second;
	}
}

void HyperGraph::removeEdges() {
	for (auto itr = _edges.begin(); itr != _edges.end(); itr++) {
		delete *itr;
	}
}

int HyperGraph::optimize(int iterations)
{
    buildIndexMapping();

	double *b = (double*)Eigen::internal::aligned_malloc(_numPoses * _dimension * sizeof(double));
	std::vector<Eigen::Triplet<double>> coef;

	double currentLambda;
    for (int iteration = 0; iteration < iterations; iteration++)
	{
		// clear
		memset(b, 0, _sizePoses * sizeof(double));
		coef.clear();

		// build system
		double currentChi = computeActiveErrors();
		double max_diag;
		buildSystem(iteration, b, coef, &max_diag);

		if (iteration == 0) {
			currentLambda = _tau * max_diag;
		}

		LOG_DEBUG(" [%d, %f,%f] \n", iteration, currentChi, currentLambda);

		// solve
		Eigen::VectorXd eigen_x;
		solveEigen(iteration, b, coef, currentLambda, eigen_x);
		double *x = eigen_x.data();

		// update
		updateGraph(x);

		double scaleFactor = scaleLambda(x, b, currentChi, currentLambda);
		currentLambda *= scaleFactor;
    }

	free(b);

    return iterations;
}

// assign unique IDs to vertices except fixed nodes.
void HyperGraph::buildIndexMapping()
{
	_numPoses = 0;
    for (std::map<int, Vertex*>::iterator it = _vertices.begin(); it != _vertices.end(); it++) {
        Vertex *v = it->second;
        if (!v->fixed()) {
            v->setHessianIndex(_numPoses);
			_numPoses++;
        }
        else {
            v->setHessianIndex(-1);
        }
    }

	_sizePoses = _numPoses * _dimension;
}

// scale factor for lambda update
double HyperGraph::scaleLambda(double *x, double *b, double currentChi, double currentLambda)
{
	double scale = 0;
	for (int i = 0; i < _sizePoses; i++) {
		scale += x[i] * (currentLambda * x[i] + b[i]);
	}

	double rho = currentChi - computeActiveErrors();
	scale += 1e-3; // make sure it's non-zero :)
	rho /= scale;

	double alpha = 1. - pow((2 * rho - 1), 3);
	// crop lambda between minimum and maximum factors
	alpha = (std::min)(alpha, _upperStep);
	double scaleFactor = (std::max)(_lowerStep, alpha);

	return scaleFactor;
}

double HyperGraph::computeActiveErrors() {
    double chi = 0.0;
    for (int k = 0; k < (int)_edges.size(); ++k) {
        Edge *e = _edges[k];
        e->computeError();
        chi += e->chi2();
    }

    return chi;
}

void HyperGraph::buildSystem(
	int iteration,
	double *b,
	std::vector<Eigen::Triplet<double>> &coef,
	double *max_diag)
{
    for (int i = 0; i < (int)_edges.size(); i++) {
        Edge *e = _edges[i];
        e->computeJacobian();
		e->constructQuadraticForm(b, coef, iteration, max_diag);
    }
}

// update pose in the original parameter space
void HyperGraph::updateGraph(double *x)
{
	for (auto it = _vertices.begin(); it != _vertices.end(); ++it) {
		Vertex *v = it->second;
		if (!v->fixed()) {
			v->oplus(x);
			x += _dimension;
		}
	}
}

// eigen sparce solver
void HyperGraph::solveEigen(
		int iteration,
		double *b,
		std::vector<Eigen::Triplet<double>> &coef,
		double lambda,
		Eigen::VectorXd &eigen_x)
{
	// right-hand side vector B
	Eigen::Map<Eigen::VectorXd> eigen_b(b, _sizePoses);

	// LM method
	for (int i = 0; i < _numPoses; i++) {
		for (int j = 0; j < _dimension; j++) {
			int ind = i * _dimension + j;
			coef.push_back(Eigen::Triplet<double>(ind, ind, lambda));
		}
	}

	// matrix A
	Eigen::SparseMatrix<double> A(_sizePoses, _sizePoses);
	A.setFromTriplets(coef.begin(), coef.end());

	// solver
	Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver;
	solver.compute(A);
	if (solver.info() != Eigen::Success) {
		LOG_WARN("decomposition failed");
	}

	// solve
	eigen_x = solver.solve(eigen_b);
	if (solver.info() != Eigen::Success) {
		LOG_WARN("solve failed");
	}
}
