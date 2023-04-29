//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"

void generateKeypoints(cv::Mat &img, std::vector<cv::KeyPoint> &kpts2d);
void generateKeypoints2(cv::Mat &eig, unsigned short max, std::vector<cv::KeyPoint> &kpts2d);
