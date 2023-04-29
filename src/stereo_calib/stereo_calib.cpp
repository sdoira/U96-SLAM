
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <direct.h>


//******************************************************************************
// Function Prototype
//******************************************************************************
static void StereoCalib(
	const std::vector<std::string> &imagelist,
	cv::Size boardSize,
	float squareSize);
static bool readStringList(const std::string& filename, std::vector<std::string>& l);
void dir_file(const char *path, char *dir, char *file);


//******************************************************************************
// Stereo calibration main function
//******************************************************************************
int main(int argc, char** argv)
{
	//================================================================
	// Command Parse
	//================================================================
	cv::CommandLineParser parser(argc, argv, "{w|9|}{h|6|}{s|1.0|}{@input|../data/stereo_calib.xml|}");

	std::string imagelistfn = parser.get<std::string>("@input");
	cv::Size boardSize = cv::Size(parser.get<int>("w"), parser.get<int>("h"));
	float squareSize = parser.get<float>("s");
	if (!parser.check())
	{
		parser.printErrors();
		getchar();
		return 1;
	}
	std::vector<std::string> imagelist;
	bool ok = readStringList(imagelistfn, imagelist);
	if (!ok || imagelist.empty())
	{
		std::cout << "can not open " << imagelistfn << " or the string list is empty" << std::endl;
		getchar();
		return 1;
	}

	//================================================================
	// Stereo Calibration
	//================================================================
	StereoCalib(imagelist, boardSize, squareSize);
	return 0;
}


//******************************************************************************
// Stereo Calibration
//******************************************************************************
static void StereoCalib(
	const std::vector<std::string> &imagelist,
	cv::Size boardSize,
	float squareSize)
{
	//================================================================
	// Variables, pre-process
	//================================================================
	cv::Size imageSize;
	int i, j, k;
	int nimages = (int)imagelist.size() / 2;
	cv::FileStorage fs;

	_mkdir("work");


	//================================================================
	// Chessboard pattern detection loop
	//================================================================
	std::vector<std::vector<cv::Point2f>> imagePoints[2];
	std::vector<std::vector<cv::Point3f>> objectPoints;
	imagePoints[0].resize(nimages);
	imagePoints[1].resize(nimages);
	std::vector<std::string> goodImageList;

	for (i = j = 0; i < nimages; i++) {
		for (k = 0; k < 2; k++) {

			//================================================================
			// File read
			//================================================================
			const std::string &filename = imagelist[i * 2 + k];
			cv::Mat img = cv::imread(filename, cv::IMREAD_GRAYSCALE);
			if (img.empty()) {
				break;
			}

			if (imageSize == cv::Size()) {
				imageSize = img.size();
			}
			else if (img.size() != imageSize) {
				std::cout << "The image " << filename << " has the size different from the first image size. Skipping the pair\n";
				break;
			}

			//================================================================
			// Find chessboard pattern
			//================================================================
			bool found = false;
			const int maxScale = 2;
			std::vector<cv::Point2f>& corners = imagePoints[k][j];
			for (int scale = 1; scale <= maxScale; scale++) {
				cv::Mat timg;
				if (scale == 1) {
					timg = img;
				}
				else {
					resize(img, timg, cv::Size(), scale, scale);
				}
				found = findChessboardCorners(
					timg,
					boardSize,
					corners,
					cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE
				);
				if (found) {
					std::cout << "chessboard found " << filename << "\n";
					if (scale > 1) {
						cv::Mat cornersMat(corners);
						cornersMat *= 1. / scale;
					}
					break;
				}
				else {
					std::cout << "chessboard not found " << filename << "\n";
				}
			}

			//================================================================
			// Draw chessboard pattern
			//================================================================
			cv::Mat cimg;
			cvtColor(img, cimg, cv::COLOR_GRAY2BGR);
			drawChessboardCorners(
				cimg,
				boardSize,
				corners,
				found
			);

			char dir[100], file[100], filename2[200];
			dir_file(filename.c_str(), dir, file);
			sprintf(filename2, "work\\chessboard_%s", file);
			imwrite(filename2, cimg);

			if (!found) {
				break;
			}

			//================================================================
			// Calculate sub-pixel resolution of corner coordinates
			//================================================================
			cornerSubPix(
				img,
				corners,
				cv::Size(11, 11),
				cv::Size(-1, -1),
				cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, 0.01)
			);
		}
		if (k == 2) {
			// image pair with successful detecion of chessboard pattern.
			goodImageList.push_back(imagelist[i * 2]);
			goodImageList.push_back(imagelist[i * 2 + 1]);
			j++;
		}
	}
	std::cout << j << " pairs have been successfully detected.\n";
	nimages = j;
	if (nimages < 2) {
		std::cout << "Error: too little pairs to run the calibration\n";
		getchar();
		return;
	}

	imagePoints[0].resize(nimages);
	imagePoints[1].resize(nimages);
	objectPoints.resize(nimages);


	//================================================================
	// Object coorinates
	//----------------------------------------------------------------
	// will be a simple incremental pattern since we assume pattern
	// size is 1. actual length should be set via argument parameters.
	//================================================================
	for (i = 0; i < nimages; i++) {
		for (j = 0; j < boardSize.height; j++) {
			for (k = 0; k < boardSize.width; k++) {
				objectPoints[i].push_back(cv::Point3f(k*squareSize, j*squareSize, 0));
			}
		}
	}

	//================================================================
	// Calibration
	//================================================================
	std::cout << "Running stereo calibration ...\n";

	// estimate initial value of camara matrix, distortion will be 0.
	cv::Mat M[2], D[2];
	M[0] = initCameraMatrix2D(objectPoints, imagePoints[0], imageSize, 0);
	M[1] = initCameraMatrix2D(objectPoints, imagePoints[1], imageSize, 0);

	cv::Mat R, T, E, F;
	int flag_calibrate = cv::CALIB_FIX_INTRINSIC; // no camera matrix update
	double rms = stereoCalibrate(
		objectPoints,
		imagePoints[0],
		imagePoints[1],
		M[0], D[0],
		M[1], D[1],
		imageSize,
		R, T, E, F,
		flag_calibrate,
		cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, 1e-5)
	);
	std::cout << "done with RMS error=" << rms << std::endl;

	// CALIBRATION QUALITY CHECK
	// because the output fundamental matrix implicitly
	// includes all the output information,
	// we can check the quality of calibration using the
	// epipolar geometry constraint: m2^t*F*m1=0
	std::cout << "epipolar err" << std::endl;
	int npoints = 0;
	std::vector<cv::Vec3f> lines[2];
	for (i = 0; i < nimages; i++)
	{
		int npt = (int)imagePoints[0][i].size();
		cv::Mat imgpt[2];
		for (k = 0; k < 2; k++)
		{
			imgpt[k] = cv::Mat(imagePoints[k][i]);
			undistortPoints(imgpt[k], imgpt[k], M[k], D[k], cv::Mat(), M[k]);
			computeCorrespondEpilines(imgpt[k], k + 1, F, lines[k]);
		}
		double erri = 0;
		for (j = 0; j < npt; j++)
		{
			double errij = (
				fabs(
					imagePoints[0][i][j].x*lines[1][j][0] +
					imagePoints[0][i][j].y*lines[1][j][1] +
					lines[1][j][2]
				) +
				fabs(
					imagePoints[1][i][j].x*lines[0][j][0] +
					imagePoints[1][i][j].y*lines[0][j][1] +
					lines[0][j][2]
				)
			);
			erri += errij;
		}
		erri /= (double)npt;
		std::cout << "image " << i << " error " << erri;
		if (erri > 10.0) {
			std::cout << " large error found!" << std::endl;
		}
		else {
			std::cout << std::endl;
		}
		npoints += npt;
	}


	//================================================================
	// Calculate stereo rectification parameters
	//================================================================
	cv::Mat R1, R2, P1, P2, Q;
	cv::Rect validRoi[2];
	stereoRectify(
		M[0], D[0],
		M[1], D[1],
		imageSize,
		R, T,
		R1, R2,
		P1, P2,
		Q,
		cv::CALIB_ZERO_DISPARITY,
		1, // alpha, 0=crop, 1=no-crop, -1=default scaling
		imageSize,
		&validRoi[0], &validRoi[1]
	);

	//================================================================
	// Parameter output
	//================================================================
	// camera intrinsic parameters
	fs.open("intrinsics.yml", cv::FileStorage::WRITE);
	if (fs.isOpened()) {
		fs << "M1" << M[0] << "D1" << D[0];
		fs << "M2" << M[1] << "D2" << D[1];
		fs.release();
		std::cout << "Output camera intrinsic parameters as \"intrinsics.yml\". \n";
	}
	else {
		std::cout << "Error: can not save the intrinsic parameters\n";
	}

	// camera extrinsic parameters
	fs.open("extrinsics.yml", cv::FileStorage::WRITE);
	if (fs.isOpened()) {
		fs << "R"  << R  << "T"  << T;
		fs << "R1" << R1 << "R2" << R2;
		fs << "P1" << P1 << "P2" << P2;
		fs << "Q"  << Q;
		fs.release();
		std::cout << "Output camera extrinsic parameters as \"extrinsics.yml\". \n";
	}
	else {
		std::cout << "Error: can not save the extrinsic parameters\n";
	}

	// calibration parameters for left and right cameras
	fs.open("calib_left.yml", cv::FileStorage::WRITE);
	if (fs.isOpened()) {
		fs << "image_width" << imageSize.width;
		fs << "image_height" << imageSize.height;
		fs << "camera_matrix" << M[0];
		fs << "distortion_coefficients" << D[0];
		fs << "rectification_matrix" << R1;
		fs << "projection_matrix" << P1;
		fs.release();
		std::cout << "Output calibration parameters for left camera as \"calib_left.yml\". \n";
	}
	else {
		std::cout << "Error: can not save the calibration left parameters\n";
	}

	fs.open("calib_right.yml", cv::FileStorage::WRITE);
	if (fs.isOpened()) {
		fs << "image_width" << imageSize.width;
		fs << "image_height" << imageSize.height;
		fs << "camera_matrix" << M[1];
		fs << "distortion_coefficients" << D[1];
		fs << "rectification_matrix" << R2;
		fs << "projection_matrix" << P2;
		fs.release();
		std::cout << "Output calibration parameters for right camera as \"calib_right.yml\". \n";
	}
	else {
		std::cout << "Error: can not save the calibration left parameters\n";
	}


	//================================================================
	// Computes transformation map
	//================================================================
	cv::Mat rmap[2][2];
	// Precompute maps for cv::remap()
	initUndistortRectifyMap(
		M[0],
		D[0],
		R1, P1,
		imageSize,
		CV_16SC2,
		rmap[0][0], rmap[0][1]
	);
	initUndistortRectifyMap(
		M[1],
		D[1],
		R2, P2,
		imageSize,
		CV_16SC2,
		rmap[1][0], rmap[1][1]
	);


	//================================================================
	// Stereo Rectification
	//================================================================
	cv::Mat canvas; // result image, [2h x w]
	int done = 0;

	double sf = 600. / MAX(imageSize.width, imageSize.height);
	int w = cvRound(imageSize.width*sf);
	int h = cvRound(imageSize.height*sf);
	canvas.create(h, w * 2, CV_8UC3);

	for (i = 0; i < nimages; i++)
	{
		for (k = 0; k < 2; k++)
		{
			// Frontal Parallel Configuration
			cv::Mat img = cv::imread(goodImageList[i * 2 + k], 0);
			cv::Mat rimg, cimg;
			remap(img, rimg, rmap[k][0], rmap[k][1], cv::INTER_LINEAR);
			cvtColor(rimg, cimg, cv::COLOR_GRAY2BGR);

			// horizontaly concatenate L and R images
			cv::Mat canvasPart = canvas(cv::Rect(w*k, 0, w, h));
			resize(cimg, canvasPart, canvasPart.size(), 0, 0, cv::INTER_AREA);
			cv::Rect vroi(
				cvRound(validRoi[k].x*sf),
				cvRound(validRoi[k].y*sf),
				cvRound(validRoi[k].width*sf),
				cvRound(validRoi[k].height*sf)
			);
			rectangle(canvasPart, vroi, cv::Scalar(0, 0, 255), 3, 8);
		}

		// draw parallel lines
		for (j = 0; j < canvas.rows; j += 16) {
			line(canvas, cv::Point(0, j), cv::Point(canvas.cols, j), cv::Scalar(0, 255, 0), 1, 8);
		}

		// save concatenated images
		char dir[100], file[100], filename2[200];
		const std::string& filename = imagelist[i * 2];
		dir_file(filename.c_str(), dir, file);
		sprintf(filename2, "work\\canvas_%s", file);
		imwrite(filename2, canvas);
	}

	printf("done.\n");
	printf("press any key to close.\n");
	getchar();
}


//******************************************************************************
// readStringList
//******************************************************************************
static bool readStringList(const std::string& filename, std::vector<std::string>& l)
{
	l.resize(0);
	cv::FileStorage fs(filename, cv::FileStorage::READ);
	if (!fs.isOpened())
		return false;
	cv::FileNode n = fs.getFirstTopLevelNode();
	if (n.type() != cv::FileNode::SEQ)
		return false;
	cv::FileNodeIterator it = n.begin(), it_end = n.end();
	for (; it != it_end; ++it)
		l.push_back((std::string)*it);
	return true;
}

//******************************************************************************
// dir_file
//------------------------------------------------------------------------------
// separate full file path into directory path and file name.
//******************************************************************************
void dir_file(const char *path, char *dir, char *file)
{
	// search '/' character from the end of the sentence.
	const char *p = strrchr(path, '/');

	if (p == NULL) {        // '/' not found
		*dir = '\0';
		strcpy(file, path);
	}
	else if (p == path) {   // '/' at head
		strcpy(dir, "/");
		strcpy(file, p + 1);
	}
	else {                  // '/' is found
		memcpy(dir, path, p - path); dir[p - path] = '\0';
		strcpy(file, p + 1);
	}
}
