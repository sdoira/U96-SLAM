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

#include "g2o/SE3Gradient.h"

namespace SE3 {

	inline Isometry3::ConstLinearPart extractRotation(const Isometry3 &A)
	{
		return A.matrix().topLeftCorner<3, 3>();
	}

	void computeEdgeSE3Gradient(
		Matrix6D &JiConstRef,
		Matrix6D &JjConstRef,
		const Isometry3 &Z,
		const Isometry3 &Xi,
		const Isometry3 &Xj)
	{
		Matrix6D &Ji = JiConstRef;
		Matrix6D &Jj = JjConstRef;

		// compute the error at the linearization point
		const Isometry3 A = Z.inverse();
		const Isometry3 B = Xi.inverse() * Xj;

		Isometry3 E = A * B;

		Isometry3::ConstLinearPart Re = extractRotation(E);
		Isometry3::ConstLinearPart Ra = extractRotation(A);
		Isometry3::ConstLinearPart Rb = extractRotation(B);
		Isometry3::ConstTranslationPart tb = B.translation();

		Eigen::Matrix<number_t, 3, 9, Eigen::ColMajor>  dq_dR;
		compute_dq_dR(dq_dR,
			Re(0, 0), Re(1, 0), Re(2, 0),
			Re(0, 1), Re(1, 1), Re(2, 1),
			Re(0, 2), Re(1, 2), Re(2, 2));

		Ji.setZero();
		Jj.setZero();

		// dte/dti
		Ji.template block<3, 3>(0, 0) = -Ra;

		// dte/dtj
		Jj.template block<3, 3>(0, 0) = Re;

		// dte/dqi
		Matrix3 S;
		skew1(S, tb, 1);
		Ji.template block<3, 3>(0, 3) = Ra * S;

		// dte/dqj: this is zero

		number_t buf[27];
		Eigen::Map<Eigen::Matrix<number_t, 9, 3, Eigen::ColMajor> > M(buf);
		Matrix3 Sxt, Syt, Szt;
		// dre/dqi
		{
			skew2(Sxt, Syt, Szt, Rb, 1);
			Eigen::Map<Matrix3> Mx(buf);    Mx.noalias() = Ra * Sxt;
			Eigen::Map<Matrix3> My(buf + 9);  My.noalias() = Ra * Syt;
			Eigen::Map<Matrix3> Mz(buf + 18); Mz.noalias() = Ra * Szt;
			Ji.template block<3, 3>(3, 3) = dq_dR * M;
		}

		// dre/dqj
		{
			Matrix3& Sx = Sxt;
			Matrix3& Sy = Syt;
			Matrix3& Sz = Szt;
			skew2(Sx, Sy, Sz, Matrix3::Identity(), 0);
			Eigen::Map<Matrix3> Mx(buf);    Mx.noalias() = Re * Sx;
			Eigen::Map<Matrix3> My(buf + 9);  My.noalias() = Re * Sy;
			Eigen::Map<Matrix3> Mz(buf + 18); Mz.noalias() = Re * Sz;
			Jj.template block<3, 3>(3, 3) = dq_dR * M;
		}
	}

	void  compute_dq_dR(
		Eigen::Matrix<number_t, 3, 9, Eigen::ColMajor> &dq_dR,
		const number_t &r11, const number_t &r21, const number_t &r31,
		const number_t &r12, const number_t &r22, const number_t &r32,
		const number_t &r13, const number_t &r23, const number_t &r33) {
		number_t qw;
		number_t S;
		_q2m(S, qw, r11, r21, r31, r12, r22, r32, r13, r23, r33);
		S *= .25;
		compute_dq_dR_w(dq_dR, S, r11, r21, r31, r12, r22, r32, r13, r23, r33);
	}

	int _q2m(
		number_t &S, number_t &qw,
		const number_t &r00, const number_t &r10, const number_t &r20,
		const number_t &r01, const number_t &r11, const number_t &r21,
		const number_t &r02, const number_t &r12, const number_t &r22) {
		number_t tr = r00 + r11 + r22;
		S = sqrt(tr + 1.0) * 2; // S=4*qw 
		qw = 0.25 * S;
		return 0;
	}

	void compute_dq_dR_w(
		Eigen::Matrix<number_t, 3, 9 > &dq_dR_w, const number_t &qw,
		const number_t &r00, const number_t &r10, const number_t &r20,
		const number_t &r01, const number_t &r11, const number_t &r21,
		const number_t &r02, const number_t &r12, const number_t &r22) {
		(void)r00;
		(void)r11;
		(void)r22;
		number_t _aux1 = 1 / pow(qw, 3);
		number_t _aux2 = -0.03125 * (r21 - r12) * _aux1;
		number_t _aux3 = 1 / qw;
		number_t _aux4 = 0.25 * _aux3;
		number_t _aux5 = -0.25 * _aux3;
		number_t _aux6 = 0.03125 * (r20 - r02) * _aux1;
		number_t _aux7 = -0.03125 * (r10 - r01) * _aux1;
		dq_dR_w(0, 0) = _aux2;
		dq_dR_w(0, 1) = 0;
		dq_dR_w(0, 2) = 0;
		dq_dR_w(0, 3) = 0;
		dq_dR_w(0, 4) = _aux2;
		dq_dR_w(0, 5) = _aux4;
		dq_dR_w(0, 6) = 0;
		dq_dR_w(0, 7) = _aux5;
		dq_dR_w(0, 8) = _aux2;
		dq_dR_w(1, 0) = _aux6;
		dq_dR_w(1, 1) = 0;
		dq_dR_w(1, 2) = _aux5;
		dq_dR_w(1, 3) = 0;
		dq_dR_w(1, 4) = _aux6;
		dq_dR_w(1, 5) = 0;
		dq_dR_w(1, 6) = _aux4;
		dq_dR_w(1, 7) = 0;
		dq_dR_w(1, 8) = _aux6;
		dq_dR_w(2, 0) = _aux7;
		dq_dR_w(2, 1) = _aux4;
		dq_dR_w(2, 2) = 0;
		dq_dR_w(2, 3) = _aux5;
		dq_dR_w(2, 4) = _aux7;
		dq_dR_w(2, 5) = 0;
		dq_dR_w(2, 6) = 0;
		dq_dR_w(2, 7) = 0;
		dq_dR_w(2, 8) = _aux7;
	}

	void skew1(Matrix3 &s, const Isometry3::ConstTranslationPart &v, bool transposed) {
		const double x = 2 * v(0);
		const double y = 2 * v(1);
		const double z = 2 * v(2);
		if (transposed) {
			s << 0., -z, y, z, 0, -x, -y, x, 0;
		}
		else {
			s << 0., z, -y, -z, 0, x, y, -x, 0;
		}
	}

	void skew2(Matrix3 &Sx, Matrix3 &Sy, Matrix3 &Sz, const Matrix3 &R, bool transposed) {
		const double
			r11 = 2 * R(0, 0), r12 = 2 * R(0, 1), r13 = 2 * R(0, 2),
			r21 = 2 * R(1, 0), r22 = 2 * R(1, 1), r23 = 2 * R(1, 2),
			r31 = 2 * R(2, 0), r32 = 2 * R(2, 1), r33 = 2 * R(2, 2);

		if (transposed) {
			Sx << 0, 0, 0, r31, r32, r33, -r21, -r22, -r23;
			Sy << -r31, -r32, -r33, 0, 0, 0, r11, r12, r13;
			Sz << r21, r22, r23, -r11, -r12, -r13, 0, 0, 0;
		}
		else {
			Sx << 0, 0, 0, -r31, -r32, -r33, r21, r22, r23;
			Sy << r31, r32, r33, 0, 0, 0, -r11, -r12, -r13;
			Sz << -r21, -r22, -r23, r11, r12, r13, 0, 0, 0;
		}
	}
};
