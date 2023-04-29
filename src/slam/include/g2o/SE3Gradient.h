// g2o - General Graph Optimization
// Copyright (C) 2011 R. Kuemmerle, G. Grisetti, W. Burgard
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Copied from g2o, and modified by Nu-Gate Technology in accordance 
// with the above license.

#pragma once

#include "core/EigenTypes.h"

namespace SE3
{
	using number_t = double;
	using Matrix3 = Eigen::Matrix<number_t, 3, 3, Eigen::ColMajor>;

    void computeEdgeSE3Gradient(
        Matrix6D& JiConstRef,
        Matrix6D& JjConstRef,
        const Isometry3& Z,
        const Isometry3& Xi,
        const Isometry3& Xj
    );
    void compute_dq_dR(
    	Eigen::Matrix<double, 3, 9, Eigen::ColMajor>& dq_dR,
    	const double& r11, const double& r21, const double& r31,
    	const double& r12, const double& r22, const double& r32,
    	const double& r13, const double& r23, const double& r33);
    int _q2m(
    	double& S, double& qw,
    	const double& r00, const double& r10, const double& r20,
    	const double& r01, const double& r11, const double& r21,
    	const double& r02, const double& r12, const double& r22);
    void compute_dq_dR_w(
    	Eigen::Matrix<double, 3, 9 >& dq_dR_w, const double& qw,
    	const double& r00, const double& r10, const double& r20,
    	const double& r01, const double& r11, const double& r21,
    	const double& r02, const double& r12, const double& r22);
    void skew1(Matrix3& s, const Isometry3::ConstTranslationPart& v, bool transposed);
    void skew2(Matrix3& Sx, Matrix3& Sy, Matrix3& Sz, const Matrix3& R, bool transposed);
};

