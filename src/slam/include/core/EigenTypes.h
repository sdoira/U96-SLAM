//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "Eigen/Core"
#include "Eigen/Geometry"
#include <stack>
#include <vector>

typedef Eigen::Matrix<double, 3, 1, Eigen::ColMajor>                   Vector3D;
typedef Eigen::Matrix<double, 6, 1, Eigen::ColMajor>                   Vector6D;
typedef Eigen::Matrix<double, 3, 3, Eigen::ColMajor>                   Matrix3D;
typedef Eigen::Matrix<double, 6, 6, Eigen::ColMajor>                   Matrix6D;
typedef Eigen::Transform<double, 3, Eigen::Isometry, Eigen::ColMajor>  Isometry3;
typedef Eigen::Quaternion<double>                                      Quaternion;

// sparse linear system
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>	PoseMatrixType;
typedef Eigen::Map<Matrix6D, Matrix6D::Flags& Eigen::Aligned> HessianBlockType;
typedef std::tuple<HessianBlockType> HessianTuple;
typedef std::stack<Isometry3, std::vector<Isometry3, Eigen::aligned_allocator<Isometry3>>> BackupStackType;
