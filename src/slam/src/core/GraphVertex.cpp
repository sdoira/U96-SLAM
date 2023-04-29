//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/GraphVertex.h"

Vertex::Vertex()
{
	_estimate = Isometry3::Identity();
	_id = 0;
	_fixed = false;
	_hessianIndex = 0;
}

// update pose in the original parameter space
void Vertex::oplus(const double *update)
{
	// convert to original parameter space
	Eigen::Map<const Vector6D> v(update);

	Isometry3 increment;
	increment = fromCompactQuaternion(v.block<3, 1>(3, 0));
	increment.translation() = v.block<3, 1>(0, 0);

	// update pose
	_estimate = _estimate * increment;
}

// compact quaternion -> rotation matrix
Matrix3D Vertex::fromCompactQuaternion(const Vector3D& v) {
	double w = 1 - v.squaredNorm();
	if (w < 0) {
		return Matrix3D::Identity();
	}
	else {
		w = sqrt(w);
	}
	return Quaternion(w, v[0], v[1], v[2]).toRotationMatrix();
}
