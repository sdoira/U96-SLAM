//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/GraphEdge.h"
#include "g2o/SE3Gradient.h"

Edge::Edge() {
	_dimension = 6;
    _vertices.resize(2, 0);
    _internalId = 0;
}

void Edge::setVertex(size_t i, Vertex* v) {
	_vertices[i] = v;
}

void Edge::setMeasurement(const Isometry3& m) {
    _measurement = m;
    _inverseMeasurement = m.inverse();
}

void Edge::setInformation(const Matrix6D& information){
    _information = information;
}

void Edge::computeError()
{
	// compute delta
    Vertex *from = _vertices[0];
    Vertex *to = _vertices[1];
    Isometry3 delta = _inverseMeasurement * from->estimate().inverse() * to->estimate();

	// axis of a normalized quaternion for rotation part
	Matrix3D R = delta.matrix().topLeftCorner<3, 3>();
	Quaternion q(R);
	q.normalize();
	if (q.w() < 0) q.coeffs() *= -1;
	_error.block<3, 1>(3, 0) = q.coeffs().head<3>();

	// translation part
	_error.block<3, 1>(0, 0) = delta.translation();
}

double Edge::chi2()
{
	double err = _error.dot(_information * _error);

	return err;
}

void Edge::computeJacobian()
{
	const Isometry3 &Xi = _vertices[0]->estimate(); // from
	const Isometry3 &Xj = _vertices[1]->estimate(); // to
    const Isometry3 &Z = _measurement;
    SE3::computeEdgeSE3Gradient(_jacobianOplus[0], _jacobianOplus[1], Z, Xi, Xj);
}

void Edge::constructQuadraticForm(
	double *b,
	std::vector<Eigen::Triplet<double>> &coef,
	int iteration,
	double *max_diag)
{
    const Matrix6D &omega = _information;
    const Vector6D &weightedError = -_information * _error;
	
	// right-hand side vector B
	for (int n = 0; n < 2; n++) {
		Vertex *v = _vertices[n];
		if (!v->fixed()) {
			Vector6D JtE = _jacobianOplus[n].transpose() * weightedError;
			for (int i = 0; i < _dimension; i++) {
				b[v->hessianIndex() * _dimension + i] += JtE[i];
			}
		}
	}

	// diagonal elements of matrix A
	*max_diag = 0;
	for (int n = 0; n < 2; n++) {
		Vertex *v = _vertices[n];
		if (!v->fixed()) {
			Matrix6D J = _jacobianOplus[n];
			Matrix6D JtO = J.transpose() * omega;
			Matrix6D m = JtO * J;
			for (int i = 0; i < _dimension; i++) {
				for (int j = 0; j < _dimension; j++) {
					int ind1 = v->hessianIndex() * _dimension + j;
					int ind2 = v->hessianIndex() * _dimension + i;
					coef.push_back(Eigen::Triplet<double>(ind1, ind2, m(i, j)));

					// max diagonal for initial lambda
					if ((iteration == 0) && (i == j)) {
						if (fabs(m(i, j)) > *max_diag) {
							*max_diag = fabs(m(i, j));
						}
					}
				}
			}
		}
	}

	// other elements of matrix A
	Vertex *from = _vertices[0];
	Vertex *to = _vertices[1];
	if (!from->fixed()) {
		Matrix6D JtO = _jacobianOplus[0].transpose() * omega;
		Matrix6D m = JtO * _jacobianOplus[1];
		for (int i = 0; i < _dimension; i++) {
			for (int j = 0; j < _dimension; j++) {
				int ind1 = to->hessianIndex() * _dimension + j;
				int ind2 = from->hessianIndex() * _dimension + i;
				coef.push_back(Eigen::Triplet<double>(ind1, ind2, m(i, j)));
			}
		}
	}
}
