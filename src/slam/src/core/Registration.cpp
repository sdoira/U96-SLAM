//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include <core/Registration.h>

//==================================================================
// Compute transformation between "from" and "to" images.
//==================================================================
Transform computeTransform(
	SensorData sensorFrom,
	SensorData sensorTo,
	Transform guess,
	struct REG_INFO *info)
{
	// search matched index pair <from:to>
	std::multimap<int, int> matchedIndex;
	if (guess.isNull()) {
		// no guessing information at 1st odometry and LC detection
		matchingNoGuess(sensorFrom, sensorTo, matchedIndex);
	}
	else {
		matchingGuess(sensorFrom, sensorTo, guess, matchedIndex);
	}

	Transform t;
	estimateMotion(sensorFrom, sensorTo, guess, t, info, matchedIndex);

	return t;
}

//==================================================================
// Projects 3D coordinates of the keypoints in "from" image 
// onto "to" image plane using guessing information.
//==================================================================
void matchingGuess_Projection(
	std::vector<cv::Point3f> kptsFrom3D,
	std::vector<cv::Point2f> &projectedPoint,
	std::vector<int> &projectedIndex,
	StereoCameraModel cameraModel,
	Transform guess)
{
	Transform guessCameraRef = (guess * cameraModel.localTransform()).inverse();

	// rotation matrix R, translation vector tvec, distortion coeffs K
	cv::Mat R = (cv::Mat_<double>(3, 3) <<
		(double)guessCameraRef.r11(), (double)guessCameraRef.r12(), (double)guessCameraRef.r13(),
		(double)guessCameraRef.r21(), (double)guessCameraRef.r22(), (double)guessCameraRef.r23(),
		(double)guessCameraRef.r31(), (double)guessCameraRef.r32(), (double)guessCameraRef.r33());
	cv::Mat rvec(1, 3, CV_64FC1);
	cv::Rodrigues(R, rvec);
	cv::Mat tvec = (cv::Mat_<double>(1, 3) << (double)guessCameraRef.x(), (double)guessCameraRef.y(), (double)guessCameraRef.z());
	cv::Mat K = cameraModel.K_l();

	// projects 3D coords of the key points in "from" image 
	// onto "to" image plane
	std::vector<cv::Point2f> projected;
	cv::projectPoints(kptsFrom3D, rvec, tvec, K, cv::Mat(), projected);

	// remove the points outside of the valid image area
	for (int i = 0; i < (int)projected.size(); i++)
	{
		if (
			(0.0f < projected[i].x) && (projected[i].x < cameraModel.imageSize().width - 1) &&
			(0.0f < projected[i].y) && (projected[i].y < cameraModel.imageSize().height - 1) &&
			(transformPoint(kptsFrom3D[i], guessCameraRef).z > 0.0))
		{
			projectedIndex.push_back(i);
			projectedPoint.push_back(projected[i]);
		}
	}

	return;
}

//--------------------------------------------------------------
// Apply radius search between keypoints in the image pair.
// The distance here is in unit of pixels.
//--------------------------------------------------------------
void matchingGuess_radiusSearch(
	std::vector<cv::KeyPoint> kptsTo,
	std::vector<cv::Point2f> projectedPoint,
	std::vector<std::vector<int>> &indices)
{
	// convert to cv::Mat format
	cv::Mat matKptsTo = cv::Mat((int)kptsTo.size(), 2, CV_32FC1);
	for (int i = 0; i < (int)kptsTo.size(); i++) {
		matKptsTo.at<float>(i, 0) = (float)kptsTo[i].pt.x;
		matKptsTo.at<float>(i, 1) = (float)kptsTo[i].pt.y;
	}

	cv::Mat matProjectedPoint = cv::Mat((int)projectedPoint.size(), 2, CV_32FC1);
	for (int i = 0; i < (int)projectedPoint.size(); i++) {
		matProjectedPoint.at<float>(i, 0) = (float)projectedPoint[i].x;
		matProjectedPoint.at<float>(i, 1) = (float)projectedPoint[i].y;
	}

	//---------------------------------------------------------
	// radius match
	//---------------------------------------------------------
	// std::vector<std::vector<cv::DMatch>> matchesRadius
	//   N x M, where N is the number of elements, M is points
	//   whose distance is shorter than guessWinSize.
	// cv::Dmatch
	//   float distance
	//   int imgIdx   // train image index
	//   int queryIdx // query descriptor index
	//   int trainIdx // train descriptor index
	//---------------------------------------------------------
	cv::BFMatcher cv_matcher(cv::NORM_L2);
	std::vector<std::vector<cv::DMatch>> matchesRadius;
	float guessWinSize = 40.0f;
	cv_matcher.radiusMatch(matProjectedPoint, matKptsTo, matchesRadius, guessWinSize);

	//---------------------------------------------------------
	// rearrange matching result in std::vector
	//---------------------------------------------------------
	// std::vector<std::vector<int>> indices
	//   N x M, index of matched descriptor
	//---------------------------------------------------------
	for (int i = 0; i < (int)matchesRadius.size(); i++) {
		std::vector<int> vec_trainIdx;
		for (int j = 0; j < (int)matchesRadius[i].size(); j++) {
			vec_trainIdx.push_back(matchesRadius[i][j].trainIdx);
		}
		indices.push_back(vec_trainIdx);
	}

	return;
}

void matchingGuess_Nndr(
	cv::Mat descriptorsFrom,
	cv::Mat descriptorsTo,
	std::vector<int> projectedIndex,
	std::vector<cv::Point2f> projectedPoint,
	std::vector<std::vector<int>> indices,
	std::multimap<int, int> &matchedIndex)
{
	//--------------------------------------------------------------
	// Apply NNDR
	//--------------------------------------------------------------
	std::set<int> addedIndex;
	cv::Mat descriptors(10, descriptorsTo.cols, descriptorsTo.type());
	for (int i = 0; i < (int)projectedPoint.size(); i++)
	{
		// index of matched keypoints in "from" image
		int matchedIndexFrom = projectedIndex[i];

		//--------------------------------------------------------------
		// Find the most matched descriptor in "to" image.
		//--------------------------------------------------------------
		int matchedIndexTo = -1;
		if (indices[i].size() >= 2)
		{
			// copy matched "to" descriptor
			std::vector<int> descriptorsIndices(indices[i].size());
			int num = 0;
			if ((int)indices[i].size() > descriptors.rows) {
				descriptors.resize(indices[i].size());
			}

			for (int j = 0; j < (int)indices[i].size(); j++)
			{
				descriptorsTo.row(indices[i].at(j)).copyTo(descriptors.row(num));
				descriptorsIndices[num] = indices[i].at(j);
				num++;
			}

			// bruteforce knn
			cv::BFMatcher matcher(cv::NORM_HAMMING, 0);
			std::vector<std::vector<cv::DMatch>> matchesKnn;
			matcher.knnMatch(descriptorsFrom.row(matchedIndexFrom), cv::Mat(descriptors, cv::Range(0, num)), matchesKnn, 2);
			float nndr = 0.8f;
			if (matchesKnn[0].at(0).distance < nndr * matchesKnn[0].at(1).distance) {
				matchedIndexTo = descriptorsIndices.at(matchesKnn[0].at(0).trainIdx);
			}
		}
		else if (indices[i].size() == 1)
		{
			matchedIndexTo = indices[i].at(0);
		}

		//--------------------------------------------------------------
		// index of matching pair
		//--------------------------------------------------------------
		if (matchedIndexTo >= 0 &&
			addedIndex.find(matchedIndexTo) == addedIndex.end())
		{
			addedIndex.insert(matchedIndexTo);
			matchedIndex.insert(matchedIndex.end(), std::make_pair(matchedIndexFrom, matchedIndexTo));
		}
	}

	return;
}

//--------------------------------------------------------------
// Find the most similar "to" descriptor in "from" image.
//---------------------------------------------------------------
int matchingGuess_search (
	cv::Mat descriptorFrom,
	cv::Mat descriptorsTo,
	std::vector<int> indices)
{
	int matchedIndexTo;
	if (indices.size() == 1)
	{
		matchedIndexTo = indices.at(0);
	}
	else if (indices.size() >= 2)
	{
		// copy descriptors that were hit during radius search
		cv::Mat train((int)indices.size(), descriptorsTo.cols, descriptorsTo.type());
		std::vector<int> descriptorsIndex(indices.size());
		for (int i = 0; i < (int)indices.size(); i++)
		{
			descriptorsTo.row(indices[i]).copyTo(train.row(i));
			descriptorsIndex[i] = indices[i];
		}

		// bruteforce KNN where K = 2
		cv::BFMatcher matcher(cv::NORM_HAMMING, 0);
		std::vector<std::vector<cv::DMatch>> matchesKnn;
		matcher.knnMatch(descriptorFrom, train, matchesKnn, 2);

		// apply NNDR
		float nndr = 0.8f;
		if (matchesKnn[0][0].distance < nndr * matchesKnn[0][1].distance)
		{
			// index of the most similar "to" descriptor
			matchedIndexTo = descriptorsIndex[matchesKnn[0][0].trainIdx];
		}
		else {
			matchedIndexTo = -1;
		}
	}
	else {
		matchedIndexTo = -1;
	}

	return matchedIndexTo;
}

//=============================================================================
//! Matching with guessing information
//-----------------------------------------------------------------------------
//! @brief Matching with guessing information
//=============================================================================
void matchingGuess(
	SensorData sensorFrom,
	SensorData sensorTo,
	Transform guess,
	std::multimap<int, int> &matchedIndex)
{
	const std::vector<cv::KeyPoint> &kptsTo = sensorTo.keypoints();
	const std::vector<cv::Point3f> &kptsFrom3D = sensorFrom.keypoints3D();
	cv::Mat descriptorsFrom = sensorFrom.descriptors();
	cv::Mat descriptorsTo = sensorTo.descriptors();

	// project 3D coordinates of the keypoints in "from" image 
	// onto "to" image plane using guessing information
	std::vector<int> projectedIndex;
	std::vector<cv::Point2f> projectedPoint;
	matchingGuess_Projection(kptsFrom3D, projectedPoint, projectedIndex, sensorTo.stereoCameraModel(), guess);

	// match keypoints between pair of images
	if (projectedPoint.size())
	{
		// apply radius search to reduce the number of candidates
		std::vector<std::vector<int>> indices;
		matchingGuess_radiusSearch(kptsTo, projectedPoint, indices);

		// find matched keypoints
		std::set<int> addedIndex;
		for (int i = 0; i < (int)projectedPoint.size(); i++)
		{
			// index of matched keypoints in "from" image
			int matchedIndexFrom = projectedIndex[i];

			// paired index in "to" image
			int matchedIndexTo = matchingGuess_search(
				descriptorsFrom.row(matchedIndexFrom),
				descriptorsTo,
				indices[i]);

			// store the pair of matched indices
			if (matchedIndexTo >= 0 &&
				addedIndex.find(matchedIndexTo) == addedIndex.end())
			{
				addedIndex.insert(matchedIndexTo);
				matchedIndex.insert(matchedIndex.end(), std::make_pair(matchedIndexFrom, matchedIndexTo));
			}
			else {
				// no match or multiple matches found
			}
		}
	}
	else
	{
		LOG_WARN("All projected points are outside the camera");
	}
}


//=============================================================================
//! Matching without guessing information
//-----------------------------------------------------------------------------
//! @brief Matching without guessing information
//=============================================================================
void matchingNoGuess(SensorData sensorFrom, SensorData sensorTo, std::multimap<int, int> &matchedIndex)
{
	cv::Mat descriptorsFrom = sensorFrom.descriptors();
	cv::Mat descriptorsTo = sensorTo.descriptors();

	// search the most similar "to" descriptors in "from" image by brute force
	// KNN where K = 2
	cv::BFMatcher matcher(cv::NORM_HAMMING, 0);
	std::vector<std::vector<cv::DMatch>> matches;
	matcher.knnMatch(descriptorsFrom, descriptorsTo, matches, 2);

	std::set<int> alreadyAdded;
	for (int i = 0; i < (int)matches.size(); ++i)
	{
		// apply NNDR, remove multiples
		float nndrRatio = 0.8f;
		if (matches[i][0].distance < nndrRatio * matches[i][1].distance)
		{
			if (alreadyAdded.find(matches[i][0].trainIdx) == alreadyAdded.end()) {
				alreadyAdded.insert(matches[i][0].trainIdx);
				matchedIndex.insert(matchedIndex.end(), std::make_pair(i, matches[i][0].trainIdx));
			}
		}
	}
}

void estimateMotion(
	SensorData sensorFrom,
	SensorData sensorTo,
	Transform guess,
	Transform &transform,
	REG_INFO *reg_info,
	std::multimap<int, int> matchedIndex)
{
	//==================================================================
	// Set 2D and 3D coords for VWs that are mathced in image pairs.
	//------------------------------------------------------------------
	// std::map<int, cv::Point3f> words3A
	//   VW IDs and 3D coords of keypoints in "from" image.
	// std::map<int, cv::KeyPoint> wordsB
	//   VW IDs and 2D coords of keypoints in "to" image.
	// std::map<int, cv::Point3f> words3B
	//   VW IDs and 3D coords of keypoints in "to" image.
	//==================================================================
	std::map<int, cv::Point3f> words3A;
	std::map<int, cv::Point3f> words3B;
	std::map<int, cv::KeyPoint> wordsB;
	for (std::map<int, int>::iterator iter = matchedIndex.begin(); iter != matchedIndex.end(); ++iter)
	{
		if (iter->first < (int)sensorFrom.keypoints3D().size()) {
			words3A.insert(std::make_pair(iter->first, sensorFrom.keypoints3D()[iter->first]));
		}
		words3B.insert(std::make_pair(iter->first, sensorTo.keypoints3D()[iter->second]));
		wordsB.insert(std::make_pair(iter->first, sensorTo.keypoints()[iter->second]));
	}

	//==================================================================
	// Estimate camera motion between "from" -> "to"
	//==================================================================
	// local parameters
	int minInliers = 20; // minimum inliers threshold
	int refineIterations = 1; // # of runs to refine

	std::vector<int> matches;
	std::vector<int> inliers;
	transform = estimateMotion3DTo2D(
		words3A,
		wordsB,
		sensorTo.stereoCameraModel(),
		minInliers,
		refineIterations,
		guess,
		words3B,
		&reg_info->covariance,
		&matches,
		&inliers);

	if (transform.isNull())
	{
		LOG_DEBUG("Not enough inliers %d/%d (matches=%d)", (int)inliers.size(), minInliers, (int)matches.size());
	}

	LOG_INFO(" inliers: %d/%d/%d ", (int)sensorTo.keypoints().size(), (int)matches.size(), (int)inliers.size());

	reg_info->num_inliers = (int)inliers.size();
	reg_info->num_matches = (int)matches.size();
}
