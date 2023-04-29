//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/MotionEstimation.h"
#include "core/Stereo.h"
#include "opencv/CvSolvePnP.h"
#include "core/Logger.h"


//=============================================================================
// Utility functions
//=============================================================================
inline float calcVariance(const float *v, unsigned int size)
{
	// Mean
	float mean = 0;
	if (v && size) {
		for (unsigned int i = 0; i < size; i++) {
			mean += v[i];
		}
		mean /= size;
	}

	// Variance
	float var = 0;
	if (v && size > 1) {
		float sum = 0;
		for (unsigned int i = 0; i < size; i++) {
			sum += (v[i] - mean) * (v[i] - mean);
		}
		var = sum / (size - 1);
	}

	return var;
}

inline float calcNormSquared(const float &x1, const float &x2, const float &x3)
{
	return (x1*x1 + x2*x2 + x3*x3);
}

inline float calcNormSquared(const float &x1, const float &x2)
{
	return (x1*x1 + x2*x2);
}


//=============================================================================
// estimateMotion3DTo2D
//-----------------------------------------------------------------------------
// std::map<int, cv::Point3f> words3A
//  A pair of [VW's ID in "from" image] and it's [3D coords].
// std::map<int, cv::KeyPoint> words2B
//  A pair of [VW's ID in "to" image] and it's [2D coords].
// std::map<int, cv::Point3f> words3B
//  A pair of [VW's ID in "to" image] and it's [3D coords].
//=============================================================================
Transform estimateMotion3DTo2D(
	const std::map<int, cv::Point3f> &words3A,
	const std::map<int, cv::KeyPoint> &words2B,
	const StereoCameraModel &cameraModel,
	int minInliers,
	int refineIterations,
	const Transform &guess,
	const std::map<int, cv::Point3f> &words3B,
	cv::Mat *covariance,
	std::vector<int> *matchesOut,
	std::vector<int> *inliersOut)
{
	Transform transform;
	std::vector<int> matches, inliers;
	*covariance = cv::Mat::eye(6, 6, CV_64FC1);

	//==================================================================
	// Find keypoints that are common to the image pairs, store 
	// their coordinates.
	//------------------------------------------------------------------
	// std::vector<int> ids
	//   VW IDs in "to" image.
	// std::vector<cv::Point3f> objectPoints
	//   3D coords of the keypoints in "from" image.
	// std::vector<cv::Point2f> imagePoints
	//   2D coords of the keypoints in "to" image.
	// std::vector<int> matches
	//   VW ID of the keypoints that are stored in "objectPoints"
	//   and "imagePoints".
	//==================================================================
	std::vector<int> ids;
	for (auto iter = words2B.begin(); iter != words2B.end(); ++iter)
	{
		ids.push_back(iter->first);
	}
	std::vector<cv::Point3f> objectPoints(ids.size());
	std::vector<cv::Point2f> imagePoints(ids.size());
	int num_matches = 0;
	matches.resize(ids.size());
	// scan VW ID in "to" image
	for (int i = 0; i < (int)ids.size(); i++) {
		// if VW ID exists in "from" image and it's 3D coords is valid
		//  -> store the 3D coords in "objectPoints",
		//     store the 2D coords in "imagePoints",
		//     store VW ID in "matches".
		std::map<int, cv::Point3f>::const_iterator iter = words3A.find(ids[i]);
		if (iter != words3A.end() && isFinite(iter->second)) {
			const cv::Point3f & pt = iter->second;
			objectPoints[num_matches].x = pt.x;
			objectPoints[num_matches].y = pt.y;
			objectPoints[num_matches].z = pt.z;
			imagePoints[num_matches] = words2B.find(ids[i])->second.pt;
			matches[num_matches] = ids[i];
			num_matches++;
		}
	}

	objectPoints.resize(num_matches);
	imagePoints.resize(num_matches);
	matches.resize(num_matches);

	if ((int)matches.size() >= minInliers)
	{
		cv::Mat K = cameraModel.K_l();
		cv::Mat D = cameraModel.D_l();
		Transform guessCameraFrame = (guess * cameraModel.localTransform()).inverse();
		cv::Mat R = (cv::Mat_<double>(3, 3) <<
			(double)guessCameraFrame.r11(), (double)guessCameraFrame.r12(), (double)guessCameraFrame.r13(),
			(double)guessCameraFrame.r21(), (double)guessCameraFrame.r22(), (double)guessCameraFrame.r23(),
			(double)guessCameraFrame.r31(), (double)guessCameraFrame.r32(), (double)guessCameraFrame.r33());

		cv::Mat rvec(1, 3, CV_64FC1);
		cv::Rodrigues(R, rvec);
		cv::Mat tvec = (cv::Mat_<double>(1, 3) <<
			(double)guessCameraFrame.x(), (double)guessCameraFrame.y(), (double)guessCameraFrame.z());

		// objectPoints: 3D coords in "from"
		// imagePoints: 2D coords in "to"
		// solvePnPRansac calculates a camera pose that projects objectPoints (from) to imagePoints (to).
		solvePnPRansac(
			objectPoints,
			imagePoints,
			K,
			D,
			rvec,
			tvec,
			minInliers,
			refineIterations,
			inliers);

		if ((int)inliers.size() >= minInliers)
		{
			cv::Rodrigues(rvec, R);
			Transform pnp(
				R.at<double>(0, 0), R.at<double>(0, 1), R.at<double>(0, 2), tvec.at<double>(0),
				R.at<double>(1, 0), R.at<double>(1, 1), R.at<double>(1, 2), tvec.at<double>(1),
				R.at<double>(2, 0), R.at<double>(2, 1), R.at<double>(2, 2), tvec.at<double>(2));

			transform = (cameraModel.localTransform() * pnp).inverse();

			// compute variance (like in PCL computeVariance() method of sac_model.h)
			//if (words3B.size())
			//{
				std::vector<float> errorSqrdDists(inliers.size());
				std::vector<float> errorSqrdAngles(inliers.size());
				int num_valid = 0;
				for (int i = 0; i < (int)inliers.size(); i++)
				{
					std::map<int, cv::Point3f>::const_iterator iter = words3B.find(matches[inliers[i]]);
					if (iter != words3B.end() && isFinite(iter->second))
					{
						const cv::Point3f & objPt = objectPoints[inliers[i]];
						cv::Point3f newPt = transformPoint(iter->second, transform);
						errorSqrdDists[num_valid] = calcNormSquared(objPt.x - newPt.x, objPt.y - newPt.y, objPt.z - newPt.z);

						Eigen::Vector4f v1(objPt.x - transform.x(), objPt.y - transform.y(), objPt.z - transform.z(), 0);
						Eigen::Vector4f v2(newPt.x - transform.x(), newPt.y - transform.y(), newPt.z - transform.z(), 0);
						double rad = v1.normalized().dot(v2.normalized());
						if (rad < -1.0) {
							rad = -1.0;
						}
						else if (rad >  1.0) {
							rad = 1.0;
						}
						errorSqrdAngles[num_valid] = (float)acos(rad);

						num_valid++;
					}
				}

				errorSqrdDists.resize(num_valid);
				errorSqrdAngles.resize(num_valid);
				if (errorSqrdDists.size())
				{
					// median of squared errors
					std::sort(errorSqrdDists.begin(), errorSqrdDists.end());
					double median_dist = (double)errorSqrdDists[errorSqrdDists.size() >> 1];

					std::sort(errorSqrdAngles.begin(), errorSqrdAngles.end());
					double median_angle = (double)errorSqrdAngles[errorSqrdAngles.size() >> 1];

					// avoid zero in stationary environment
					double epsilon = 0.0001;
					if (median_dist < epsilon) median_dist = epsilon;
					if (median_angle < epsilon) median_angle = epsilon;

					(*covariance)(cv::Range(0, 3), cv::Range(0, 3)) *= median_dist;
					(*covariance)(cv::Range(3, 6), cv::Range(3, 6)) *= median_angle;
				}
				else {
					LOG_WARN("Not enough close points to compute covariance!\n");
				}

				if (float(num_valid) / float(inliers.size()) < 0.2f) {
					LOG_WARN("A very low number of inliers have valid depth (%d/%d), the transform returned may be wrong!\n", num_valid, (int)inliers.size());
				}
			//}
			/*
			else
			{
				// compute variance, which is the rms of reprojection errors
				std::vector<cv::Point2f> imagePointsReproj;
				cv::projectPoints(objectPoints, rvec, tvec, K, cv::Mat(), imagePointsReproj);
				float err = 0.0f;
				for (int i = 0; i < (int)inliers.size(); i++) {
					err += calcNormSquared(imagePoints.at(inliers[i]).x - imagePointsReproj.at(inliers[i]).x, imagePoints.at(inliers[i]).y - imagePointsReproj.at(inliers[i]).y);
				}
				*covariance *= std::sqrt(err / float(inliers.size()));
			}
			*/
		}
	}

	// matched key points and inliers
	*matchesOut = matches;

	inliersOut->resize(inliers.size());
	for (int i = 0; i < (int)inliers.size(); i++) {
		inliersOut->at(i) = matches[inliers[i]];
	}

	return transform;
}

std::vector<float> computeReprojErrors(
	std::vector<cv::Point3f> opoints,
	std::vector<cv::Point2f> ipoints,
	const cv::Mat &cameraMatrix,
	const cv::Mat &distCoeffs,
	const cv::Mat &rvec,
	const cv::Mat &tvec,
	float threshold,
	std::vector<int> &inliers)
{
	int count = (int)opoints.size();

	// Reprojection
	std::vector<cv::Point2f> projpoints;
	projectPoints(opoints, rvec, tvec, cameraMatrix, distCoeffs, projpoints);

	// Compute Reprojection Errors
	inliers.resize(count, 0);
	std::vector<float> err(count);
	int num = 0;
	for (int i = 0; i < count; i++)
	{
		float e = (float)cv::norm(ipoints[i] - projpoints[i]);
		if (e <= threshold)
		{
			inliers[num] = i;
			err[num] = e;
			num++;
		}
	}
	inliers.resize(num);
	err.resize(num);

	return err;
}

void solvePnPRansac(
	const std::vector<cv::Point3f> &objectPoints,
	const std::vector<cv::Point2f> &imagePoints,
	const cv::Mat &cameraMatrix,
	const cv::Mat &distCoeffs,
	cv::Mat &rvec,
	cv::Mat &tvec,
	int minInliersCount,
	int refineIterations,
	std::vector<int> &inliers
) {
	// Local parameters
	float reprojectionError = 2.0;
	float refineSigma = 3.0;
	bool useExtrinsicGuess = true;
	int iterationsCount = 300;
	double confidence = 0.99;

	cv3::solvePnPRansac(
		objectPoints,
		imagePoints,
		cameraMatrix,
		distCoeffs,
		rvec,
		tvec,
		useExtrinsicGuess,
		iterationsCount,
		reprojectionError,
		confidence,
		inliers);

	if (((int)inliers.size() >= minInliersCount) && (refineIterations > 0))
	{
		float error_threshold = reprojectionError;
		std::vector<int> new_inliers;
		std::vector<int> prev_inliers = inliers;
		cv::Mat new_rvec = rvec;
		cv::Mat new_tvec = tvec;

		int refine_count = 0;
		while (refine_count < refineIterations)
		{
			// store the current inliers
			std::vector<cv::Point3f> opoints_inliers(prev_inliers.size());
			std::vector<cv::Point2f> ipoints_inliers(prev_inliers.size());
			for (unsigned int i = 0; i<prev_inliers.size(); ++i)
			{
				opoints_inliers[i] = objectPoints[prev_inliers[i]];
				ipoints_inliers[i] = imagePoints[prev_inliers[i]];
			}

			// solve PnP
			cv::solvePnP(
				opoints_inliers,
				ipoints_inliers,
				cameraMatrix,
				distCoeffs,
				new_rvec,
				new_tvec,
				true); // useExtrinsicGuess

			// store the points to "new_inliers" only when whose reprojection errors are below the threshold
			std::vector<float> err = computeReprojErrors(
				objectPoints,
				imagePoints,
				cameraMatrix,
				distCoeffs,
				new_rvec,
				new_tvec,
				error_threshold,
				new_inliers);

			// calculate new projection error threshold based on the variance
			float variance = calcVariance(err.data(), (unsigned int)err.size());
			error_threshold = std::min(reprojectionError, refineSigma * float(sqrt(variance)));

			// exit the loop if
			//  1. number of inliers is below the threshold, or
			//  2. inliers doesn't change.
			if (
				((int)new_inliers.size() < minInliersCount) ||
				(std::equal(new_inliers.begin(), new_inliers.end(), prev_inliers.begin(), prev_inliers.end())))
			{
				break;
			}

			// to the next loop
			std::swap(new_inliers, prev_inliers); // put the new inliers in "prev_inliers"
			refine_count++;
		}

		std::swap(new_inliers, inliers);
		rvec = new_rvec;
		tvec = new_tvec;
	}
}
