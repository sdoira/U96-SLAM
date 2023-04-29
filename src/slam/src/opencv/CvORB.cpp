#include "opencv/CvORB.h"

// copied from OpenCV
void computeDescriptor(
	cv::InputArray _image,
	cv::InputArray _mask,
	std::vector<cv::KeyPoint>& keypoints,
	bool useProvidedKeypoints,
	cv::Mat &desc
) {
	//int nfeatures = 1500;
	float scaleFactor = 2.0f;
	int _nlevels = 3;
	int edgeThreshold = 19;
	int firstLevel = 0;
	int wta_k = 2;
	//int scoreType = 0;
	int patchSize = 31;
	//int fastThreshold = 20;

	//ROI handling
	const int HARRIS_BLOCK_SIZE = 9;
	int halfPatchSize = patchSize / 2;
	// sqrt(2.0) is for handling patch rotation
	int descPatchSize = cvCeil(halfPatchSize*sqrt(2.0));
	int border = std::max(edgeThreshold, std::max(descPatchSize, HARRIS_BLOCK_SIZE / 2)) + 1;

	cv::Mat image = _image.getMat(), mask = _mask.getMat();

	int i, level;
	int nLevels = _nlevels;
	int nkeypoints = (int)keypoints.size();
	//bool sortedByLevel = true;

	nLevels = 0;
	for (i = 0; i < nkeypoints; i++)
	{
		level = keypoints[i].octave;
		nLevels = std::max(nLevels, level);
	}
	nLevels++;

	std::vector<cv::Rect> layerInfo(nLevels);
	std::vector<int> layerOfs(nLevels);
	std::vector<float> layerScale(nLevels);
	cv::Mat imagePyramid, maskPyramid;
	cv::UMat uimagePyramid, ulayerInfo;

	int level_dy = image.rows + border * 2;
	cv::Point level_ofs(0, 0);
	cv::Size bufSize((image.cols + border * 2 + 15) & -16, 0);

	for (level = 0; level < nLevels; level++) // nLevels = 1
	{
		float scale = (float)std::pow(scaleFactor, (double)(level - firstLevel));
		layerScale[level] = scale;
		cv::Size sz(cvRound(image.cols / scale), cvRound(image.rows / scale));
		cv::Size wholeSize(sz.width + border * 2, sz.height + border * 2);

		cv::Rect linfo(level_ofs.x + border, level_ofs.y + border, sz.width, sz.height);
		layerInfo[level] = linfo;
		layerOfs[level] = linfo.y*bufSize.width + linfo.x;
		level_ofs.x += wholeSize.width;
	}
	bufSize.height = level_ofs.y + level_dy;

	imagePyramid.create(bufSize, CV_8U);

	// fills the extended area with zeros
	//for (int row = 0; row < imagePyramid.rows; row++) {
	//	for (int col = image.cols + border * 2; col < imagePyramid.cols; col++) {
	//		imagePyramid.at<unsigned char>(row, col) = 0;
	//	}
	//}

	cv::Mat prevImg = image, prevMask = mask;

	// Pre-compute the scale pyramids
	for (level = 0; level < nLevels; ++level) // nLevels = 1
	{
		cv::Rect linfo = layerInfo[level];
		cv::Size sz(linfo.width, linfo.height);
		cv::Size wholeSize(sz.width + border * 2, sz.height + border * 2);
		cv::Rect wholeLinfo = cv::Rect(linfo.x - border, linfo.y - border, wholeSize.width, wholeSize.height);
		cv::Mat extImg = imagePyramid(wholeLinfo), extMask;
		cv::Mat currImg = extImg(cv::Rect(border, border, sz.width, sz.height)), currMask;

		// Compute the resized image
		copyMakeBorder(image, extImg, border, border, border, border, cv::BORDER_REFLECT_101);

		prevImg = currImg;
		prevMask = currMask;
	}

	runByImageBorder(keypoints, image.size(), edgeThreshold);

	int dsize = 32; //descriptorSize();

	nkeypoints = (int)keypoints.size();

	std::vector<cv::Point> pattern;

	const int npoints = 512;
	//cv::Point patternbuf[npoints];
	const cv::Point* pattern0 = (const cv::Point*)bit_pattern_31_2;

	std::copy(pattern0, pattern0 + npoints, std::back_inserter(pattern));

	for (level = 0; level < nLevels; level++)
	{
		// preprocess the resized image
		cv::Mat workingMat = imagePyramid(layerInfo[level]);
		GaussianBlur(workingMat, workingMat, cv::Size(7, 7), 2, 2, cv::BORDER_REFLECT_101);
	}

	desc = cv::Mat(nkeypoints, dsize, CV_8U);
	computeOrbDescriptors(
		imagePyramid, layerInfo, layerScale,
		keypoints, desc, pattern, dsize, wta_k);

	return;
}

struct RoiPredicate
{
	RoiPredicate(const cv::Rect& _r) : r(_r)
	{}

	bool operator()(const cv::KeyPoint& keyPt) const
	{
		return !r.contains(keyPt.pt);
	}

	cv::Rect r;
};

void runByImageBorder(std::vector<cv::KeyPoint>& keypoints, cv::Size imageSize, int borderSize)
{
	if (borderSize > 0)
	{
		if (imageSize.height <= borderSize * 2 || imageSize.width <= borderSize * 2)
		{
			keypoints.clear();
		}
		else
		{
			cv::Rect roi = cv::Rect(
				cv::Point(borderSize, borderSize),
				cv::Point(imageSize.width - borderSize, imageSize.height - borderSize));
			keypoints.erase(
				std::remove_if(keypoints.begin(), keypoints.end(), RoiPredicate(roi)),
				keypoints.end());
		}
	}
}

#define GET_VALUE(idx) \
               (x = pattern[idx].x*a - pattern[idx].y*b, \
                y = pattern[idx].x*b + pattern[idx].y*a, \
                ix = cvRound(x), \
                iy = cvRound(y), \
                *(center + iy*step + ix) )

void computeOrbDescriptors(
	const cv::Mat& imagePyramid,
	const std::vector<cv::Rect>& layerInfo,
	const std::vector<float>& layerScale,
	std::vector<cv::KeyPoint>& keypoints,
	cv::Mat& descriptors,
	const std::vector<cv::Point>& _pattern,
	int dsize,
	int wta_k)
{
	int step = (int)imagePyramid.step;
	int j, i, nkeypoints = (int)keypoints.size();

	for (j = 0; j < nkeypoints; j++)
	{
		const cv::KeyPoint& kpt = keypoints[j];
		const cv::Rect& layer = layerInfo[kpt.octave];
		float scale = 1.f / layerScale[kpt.octave];
		float angle = kpt.angle;

		angle *= (float)(CV_PI / 180.f);
		float a = (float)cos(angle), b = (float)sin(angle);

		const uchar* center = &imagePyramid.at<uchar>(
			cvRound(kpt.pt.y*scale) + layer.y,
			cvRound(kpt.pt.x*scale) + layer.x);
		float x, y;
		int ix, iy;
		const cv::Point* pattern = &_pattern[0];
		uchar* desc = descriptors.ptr<uchar>(j);

		for (i = 0; i < dsize; ++i, pattern += 16)
		{
			int t0, t1, val;
			t0 = GET_VALUE(0); t1 = GET_VALUE(1);
			val = t0 < t1;
			t0 = GET_VALUE(2); t1 = GET_VALUE(3);
			val |= (t0 < t1) << 1;
			t0 = GET_VALUE(4); t1 = GET_VALUE(5);
			val |= (t0 < t1) << 2;
			t0 = GET_VALUE(6); t1 = GET_VALUE(7);
			val |= (t0 < t1) << 3;
			t0 = GET_VALUE(8); t1 = GET_VALUE(9);
			val |= (t0 < t1) << 4;
			t0 = GET_VALUE(10); t1 = GET_VALUE(11);
			val |= (t0 < t1) << 5;
			t0 = GET_VALUE(12); t1 = GET_VALUE(13);
			val |= (t0 < t1) << 6;
			t0 = GET_VALUE(14); t1 = GET_VALUE(15);
			val |= (t0 < t1) << 7;

			desc[i] = (uchar)val;
		}
	}
}
