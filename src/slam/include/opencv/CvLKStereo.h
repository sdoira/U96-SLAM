
#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/video/tracking.hpp>

// exactly as cv::calcOpticalFlowPyrLK but it should be called with pyramid (from cv::buildOpticalFlowPyramid()) and delta drops the y error.
void calcOpticalFlowPyrLKStereo(
	cv::InputArray _prevImg,
	cv::InputArray _nextImg,
	cv::InputArray _prevPts,
	cv::InputOutputArray _nextPts,
	cv::OutputArray _status,
	cv::OutputArray _err,
	cv::Size winSize = cv::Size(15, 3),
	int maxLevel = 3,
	cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, 0.01),
	int flags = 0,
	double minEigThreshold = 1e-4);
