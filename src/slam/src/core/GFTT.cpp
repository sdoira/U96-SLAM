//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/GFTT.h"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

void generateKeypoints(cv::Mat &img, std::vector<cv::KeyPoint> &kpts2d) {

	int maxFeatures = 1500;
	double qualityLevel = 0.010000;
	double minDistance = 7.000000;
	int blockSize = 3;
	bool useHarrisDetector = 0;
	double k = 0.040000;
	cv::Ptr<cv::GFTTDetector> gftt = cv::GFTTDetector::create(maxFeatures, qualityLevel, minDistance, blockSize, useHarrisDetector, k);

	cv::Mat maskRoi;
	gftt->detect(img, kpts2d, maskRoi);

	return;
}


//=================================================================
// GFTT detector with FPGA acceleration
//=================================================================
struct greaterThanPtr :
	public std::binary_function<const unsigned short *, const unsigned short *, bool>
{
	bool operator () (const unsigned short * a, const unsigned short * b) const
		// Ensure a fully deterministic result of the sort
	{
		return (*a > *b) ? true : (*a < *b) ? false : (a > b);
	}
};

void generateKeypoints2(
	cv::Mat &eig,
	unsigned short max,
	std::vector<cv::KeyPoint> &kpts2d)
{
	//=================================================================
	// Parameters
	//=================================================================
	int nfeatures = 1500;
	double qualityLevel = 0.01;
	double minDistance = 7.0;
	int blockSize = 3;

	//=================================================================
	// Thresholding
	//=================================================================
	double thr = max * qualityLevel;

	std::vector<unsigned short*> tmpCorners;
	for (int y = 1; y < eig.rows - 1; y++) {
		unsigned short* eig_data = (unsigned short*)eig.ptr(y);
		for (int x = 1; x < eig.cols - 1; x++) {
			float val = eig_data[x];
			if (val >= thr) {
				tmpCorners.push_back(eig_data + x);
			}
		}
	}

	size_t total = tmpCorners.size();

	// sort in descending order
	std::sort(tmpCorners.begin(), tmpCorners.end(), greaterThanPtr());

	//=================================================================
	// Trim Neighbor
	//=================================================================
	size_t ncorners = 0;
	std::vector<cv::Point2f> corners;
	if (minDistance >= 1) {
		// Partition the image into larger grids
		int w = eig.cols;
		int h = eig.rows;

		const int cell_size = cvRound(minDistance);
		const int grid_width = (w + cell_size - 1) / cell_size;
		const int grid_height = (h + cell_size - 1) / cell_size;

		std::vector<std::vector<cv::Point2f> > grid(grid_width*grid_height);

		minDistance *= minDistance;

		for (size_t i = 0; i < total; i++) {
			int ofs = (int)((const uchar*)tmpCorners[i] - eig.ptr());
			int y = (int)(ofs / eig.step);
			int x = (int)((ofs - y*eig.step) / sizeof(unsigned short));

			bool good = true;

			int x_cell = x / cell_size;
			int y_cell = y / cell_size;

			int x1 = x_cell - 1;
			int y1 = y_cell - 1;
			int x2 = x_cell + 1;
			int y2 = y_cell + 1;

			// boundary check
			x1 = std::max(0, x1);
			y1 = std::max(0, y1);
			x2 = std::min(grid_width - 1, x2);
			y2 = std::min(grid_height - 1, y2);

			for (int yy = y1; yy <= y2; yy++) {
				for (int xx = x1; xx <= x2; xx++) {
					std::vector <cv::Point2f> &m = grid[yy*grid_width + xx];
					if (m.size()) {
						for (size_t j = 0; j < m.size(); j++) {
							float dx = x - m[j].x;
							float dy = y - m[j].y;
							if (dx*dx + dy*dy < minDistance) {
								good = false;
								goto break_out;
							}
						}
					}
				}
			}

			break_out:

			if (good) {
				grid[y_cell*grid_width + x_cell].push_back(cv::Point2f((float)x, (float)y));

				corners.push_back(cv::Point2f((float)x, (float)y));
				++ncorners;

				if (nfeatures > 0 && (int)ncorners == nfeatures) {
					break;
				}
			}
		}
	}
	else {
		// no trimming
		for (size_t i = 0; i < total; i++) {
			int ofs = (int)((const uchar*)tmpCorners[i] - eig.ptr());
			int y = (int)(ofs / eig.step);
			int x = (int)((ofs - y*eig.step) / sizeof(unsigned short));

			corners.push_back(cv::Point2f((float)x, (float)y));
			++ncorners;
			if (nfeatures > 0 && (int)ncorners == nfeatures) {
				break;
			}
		}
	}

	//=================================================================
	// Keypoint Conversion
	//=================================================================
	kpts2d.resize(corners.size());
	std::vector<cv::Point2f>::const_iterator corner_it = corners.begin();
	std::vector<cv::KeyPoint>::iterator keypoint_it = kpts2d.begin();
	for (; corner_it != corners.end(); ++corner_it, ++keypoint_it) {
		*keypoint_it = cv::KeyPoint(*corner_it, (float)blockSize);
	}

	return;
}
