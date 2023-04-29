/*
Copyright (c) 2010-2016, Mathieu Labbe - IntRoLab - Universite de Sherbrooke
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of the Universite de Sherbrooke nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Copied from RTAB-Map, and modified by Nu-Gate Technology
// in accordance with the above license.

#include "rtabmap/KITTI.h"

namespace graph
{
	// KITTI evaluation
	float lengths[] = { 100,200,300,400,500,600,700,800 };
	int32_t num_lengths = 8;

	struct errors {
		int32_t first_frame;
		float   r_err;
		float   t_err;
		float   len;
		float   speed;
		errors(int32_t first_frame, float r_err, float t_err, float len, float speed) :
			first_frame(first_frame), r_err(r_err), t_err(t_err), len(len), speed(speed) {}
	};

	std::vector<float> trajectoryDistances(const std::vector<Transform> &poses)
	{
		std::vector<float> dist;
		dist.push_back(0);
		for (unsigned int i = 1; i<poses.size(); i++) {
			Transform P1 = poses[i - 1];
			Transform P2 = poses[i];
			float dx = P1.x() - P2.x();
			float dy = P1.y() - P2.y();
			float dz = P1.z() - P2.z();
			dist.push_back(dist[i - 1] + sqrt(dx*dx + dy*dy + dz*dz));
		}
		return dist;
	}
	int32_t lastFrameFromSegmentLength(std::vector<float> &dist, int32_t first_frame, float len)
	{
		for (unsigned int i = first_frame; i < dist.size(); i++) {
			if (dist[i] > dist[first_frame] + len) {
				return i;
			}
		}
		return -1;
	}

	inline float rotationError(const Transform &pose_error)
	{
		float a = pose_error.r11();
		float b = pose_error.r22();
		float c = pose_error.r33();
		float d = 0.5f * (a + b + c - 1.0f);
		return std::acos(std::max(std::min(d, 1.0f), -1.0f));
	}

	inline float translationError(const Transform &pose_error)
	{
		float dx = pose_error.x();
		float dy = pose_error.y();
		float dz = pose_error.z();
		return sqrt(dx*dx + dy*dy + dz*dz);
	}

	void calcKittiSequenceErrors(
		const std::vector<Transform> &poses_gt,
		const std::vector<Transform> &poses_result,
		float & t_err,
		float & r_err)
	{
		// error vector
		std::vector<errors> err;

		// parameters
		int32_t step_size = 10; // every second

		// pre-compute distances (from ground truth as reference)
		std::vector<float> dist = trajectoryDistances(poses_gt);

		// for all start positions do
		for (unsigned int first_frame = 0; first_frame<poses_gt.size(); first_frame += step_size) {

			// for all segment lengths do
			for (int32_t i = 0; i<num_lengths; i++) {

				// current length
				float len = lengths[i];

				// compute last frame
				int32_t last_frame = lastFrameFromSegmentLength(dist, first_frame, len);

				// continue, if sequence not long enough
				if (last_frame == -1) {
					continue;
				}

				// compute rotational and translational errors
				Transform pose_delta_gt = poses_gt[first_frame].inverse()*poses_gt[last_frame];
				Transform pose_delta_result = poses_result[first_frame].inverse()*poses_result[last_frame];
				Transform pose_error = pose_delta_result.inverse()*pose_delta_gt;
				float r_err = rotationError(pose_error);
				float t_err = translationError(pose_error);

				// compute speed
				float num_frames = (float)(last_frame - first_frame + 1);
				float speed = len / (0.1f * num_frames);

				// write to file
				err.push_back(errors(first_frame, r_err / len, t_err / len, len, speed));
			}
		}

		t_err = 0;
		r_err = 0;

		// for all errors do => compute sum of t_err, r_err
		for (std::vector<errors>::iterator it = err.begin(); it != err.end(); it++)
		{
			t_err += it->t_err;
			r_err += it->r_err;
		}

		// save errors
		float num = (float)err.size();
		t_err /= num;
		r_err /= num;
		t_err *= 100.0f;    // Translation error (%)
		r_err *= (float)(180.0 / CV_PI); // Rotation error (deg/m)
	}
}
