//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "EigenTypes.h"

class Vertex
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Vertex();

    void setEstimate(const Isometry3 &et) { _estimate = et; }
    const Isometry3 &estimate() const { return _estimate; }
    void setFixed(bool fixed) { _fixed = fixed; }
    void setId(int newId) { _id = newId; }
    int id() const { return _id; }
    bool fixed() const { return _fixed; }
    void setHessianIndex(int ti) { _hessianIndex = ti; }
    int hessianIndex() const { return _hessianIndex; }

    void oplus(const double *update);

protected:
	Isometry3 fromVectorMQT(const Vector6D& v);
	Matrix3D fromCompactQuaternion(const Vector3D& v);

    Isometry3 _estimate;
    int _id;
    bool _fixed;
    int _hessianIndex;
};

