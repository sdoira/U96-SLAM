//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "Eigen/Sparse"
#include "EigenTypes.h"
#include "GraphVertex.h"

class Edge
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Edge();
    void setVertex(size_t i, Vertex* v);
    void setMeasurement(const Isometry3& m);
    void setInformation(const Matrix6D& information);
    std::vector<Vertex*>& vertices() { return _vertices; }
    void setInternalId(int id) { _internalId = id; }
	int internalId() const { return _internalId; }

    void computeError();
	double chi2();
    void computeJacobian();
	void constructQuadraticForm(double *b, std::vector<Eigen::Triplet<double>> &coef, int iteration, double *max_diag);

protected:
    std::vector<Vertex*> _vertices;
    Isometry3 _measurement;
    Isometry3 _inverseMeasurement;
    Matrix6D _information;
    Vector6D _error;
    Matrix6D _jacobianOplus[2];
    int _internalId;
	int _dimension;
};

