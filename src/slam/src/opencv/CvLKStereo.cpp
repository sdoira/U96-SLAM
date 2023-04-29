
#include "opencv/CvLKStereo.h"
#include "core/Logger.h"

typedef float acctype;
typedef float itemtype;
#define  CV_DESCALE(x,n)     (((x) + (1 << ((n)-1))) >> (n))

//
// Adapted from OpenCV cv::calcOpticalFlowPyrLK() to force
// only optical flow on x-axis (assuming that prevImg is the left
// image and nextImg is the right image):
// https://github.com/Itseez/opencv/blob/ddf82d0b154873510802ef75c53e628cd7b2cb13/modules/video/src/lkpyramid.cpp#L1088
//
// The difference is on this line:
// https://github.com/Itseez/opencv/blob/ddf82d0b154873510802ef75c53e628cd7b2cb13/modules/video/src/lkpyramid.cpp#L683-L684
// - cv::Point2f delta( (float)((A12*b2 - A22*b1) * D), (float)((A12*b1 - A11*b2) * D));
// + cv::Point2f delta( (float)((A12*b2 - A22*b1) * D), 0); //<--- note the 0 for y
//
void calcOpticalFlowPyrLKStereo(
	cv::InputArray _prevImg, cv::InputArray _nextImg,
	cv::InputArray _prevPts, cv::InputOutputArray _nextPts,
	cv::OutputArray _status, cv::OutputArray _err,
	cv::Size winSize, int maxLevel,
	cv::TermCriteria criteria,
	int flags, double minEigThreshold)
{
	cv::Mat prevPtsMat = _prevPts.getMat();
	const int derivDepth = cv::DataType<short>::depth;

	CV_Assert(maxLevel >= 0 && winSize.width > 2 && winSize.height > 2);

	int level = 0, i, npoints;
	CV_Assert((npoints = prevPtsMat.checkVector(2, CV_32F, true)) >= 0);

	if (npoints == 0)
	{
		_nextPts.release();
		_status.release();
		_err.release();
		return;
	}

	if (!(flags & cv::OPTFLOW_USE_INITIAL_FLOW)) {
		_nextPts.create(prevPtsMat.size(), prevPtsMat.type(), -1, true);
	}

	cv::Mat nextPtsMat = _nextPts.getMat();
	CV_Assert(nextPtsMat.checkVector(2, CV_32F, true) == npoints);

	const cv::Point2f* prevPts = prevPtsMat.ptr<cv::Point2f>();
	cv::Point2f* nextPts = nextPtsMat.ptr<cv::Point2f>();

	_status.create((int)npoints, 1, CV_8U, -1, true);
	cv::Mat statusMat = _status.getMat(), errMat;
	CV_Assert(statusMat.isContinuous());
	uchar* status = statusMat.ptr();
	float* err = 0;

	for (i = 0; i < npoints; i++) {
		status[i] = true;
	}

	if (_err.needed())
	{
		_err.create((int)npoints, 1, CV_32F, -1, true);
		errMat = _err.getMat();
		CV_Assert(errMat.isContinuous());
		err = errMat.ptr<float>();
	}

	std::vector<cv::Mat> prevPyr, nextPyr;
	int levels1 = -1;
	int lvlStep1 = 1;
	int levels2 = -1;
	int lvlStep2 = 1;


	if (_prevImg.kind() != cv::_InputArray::STD_VECTOR_MAT)
	{
		//create pyramid
		maxLevel = cv::buildOpticalFlowPyramid(_prevImg, prevPyr, winSize, maxLevel, true);
	}
	else if (_prevImg.kind() == cv::_InputArray::STD_VECTOR_MAT)
	{
		_prevImg.getMatVector(prevPyr);
	}

	levels1 = int(prevPyr.size()) - 1;
	CV_Assert(levels1 >= 0);

	if (levels1 % 2 == 1 && prevPyr[0].channels() * 2 == prevPyr[1].channels() && prevPyr[1].depth() == derivDepth)
	{
		lvlStep1 = 2;
		levels1 /= 2;
	}

	// ensure that pyramid has required padding
	if (levels1 > 0)
	{
		cv::Size fullSize;
		cv::Point ofs;
		prevPyr[lvlStep1].locateROI(fullSize, ofs);
		CV_Assert(ofs.x >= winSize.width && ofs.y >= winSize.height
			&& ofs.x + prevPyr[lvlStep1].cols + winSize.width <= fullSize.width
			&& ofs.y + prevPyr[lvlStep1].rows + winSize.height <= fullSize.height);
	}

	if (levels1 < maxLevel) {
		maxLevel = levels1;
	}

	if (_nextImg.kind() != cv::_InputArray::STD_VECTOR_MAT)
	{
		//create pyramid
		maxLevel = cv::buildOpticalFlowPyramid(_nextImg, nextPyr, winSize, maxLevel, false);
	}
	else if (_nextImg.kind() == cv::_InputArray::STD_VECTOR_MAT)
	{
		_nextImg.getMatVector(nextPyr);
	}

	levels2 = int(nextPyr.size()) - 1;
	CV_Assert(levels2 >= 0);

	if (levels2 % 2 == 1 && nextPyr[0].channels() * 2 == nextPyr[1].channels() && nextPyr[1].depth() == derivDepth)
	{
		lvlStep2 = 2;
		levels2 /= 2;
	}

	// ensure that pyramid has required padding
	if (levels2 > 0)
	{
		cv::Size fullSize;
		cv::Point ofs;
		nextPyr[lvlStep2].locateROI(fullSize, ofs);
		CV_Assert(ofs.x >= winSize.width && ofs.y >= winSize.height
			&& ofs.x + nextPyr[lvlStep2].cols + winSize.width <= fullSize.width
			&& ofs.y + nextPyr[lvlStep2].rows + winSize.height <= fullSize.height);
	}

	if (levels2 < maxLevel) {
		maxLevel = levels2;
	}

	if ((criteria.type & cv::TermCriteria::COUNT) == 0) {
		criteria.maxCount = 30;
	}
	else {
		criteria.maxCount = std::min(std::max(criteria.maxCount, 0), 100);
	}
	if ((criteria.type & cv::TermCriteria::EPS) == 0) {
		criteria.epsilon = 0.01;
	}
	else {
		criteria.epsilon = std::min(std::max(criteria.epsilon, 0.), 10.);
	}
	criteria.epsilon *= criteria.epsilon;

	// for all pyramids
	for (level = maxLevel; level >= 0; level--)
	{
		cv::Mat derivI = prevPyr[level * lvlStep1 + 1];

		CV_Assert(prevPyr[level * lvlStep1].size() == nextPyr[level * lvlStep2].size());
		CV_Assert(prevPyr[level * lvlStep1].type() == nextPyr[level * lvlStep2].type());

		const cv::Mat & prevImg = prevPyr[level * lvlStep1];
		const cv::Mat & prevDeriv = derivI;
		const cv::Mat & nextImg = nextPyr[level * lvlStep2];

		// for all corners
		{
			cv::Point2f halfWin((winSize.width - 1)*0.5f, (winSize.height - 1)*0.5f);
			const cv::Mat& I = prevImg;
			const cv::Mat& J = nextImg;
			const cv::Mat& derivI = prevDeriv;

			int j, cn = I.channels(), cn2 = cn * 2;
			cv::AutoBuffer<short> _buf(winSize.area()*(cn + cn2));
			int derivDepth = cv::DataType<short>::depth;

			cv::Mat IWinBuf(winSize, CV_MAKETYPE(derivDepth, cn), (short*)_buf);
			cv::Mat derivIWinBuf(winSize, CV_MAKETYPE(derivDepth, cn2), (short*)_buf + winSize.area()*cn);

			for (int ptidx = 0; ptidx < npoints; ptidx++)
			{
				cv::Point2f prevPt = prevPts[ptidx] * (float)(1. / (1 << level));
				cv::Point2f nextPt;
				if (level == maxLevel)
				{
					if (flags & cv::OPTFLOW_USE_INITIAL_FLOW) {
						nextPt = nextPts[ptidx] * (float)(1. / (1 << level));
					}
					else {
						nextPt = prevPt;
					}
				}
				else {
					nextPt = nextPts[ptidx] * 2.f;
				}
				nextPts[ptidx] = nextPt;

				cv::Point2i iprevPt, inextPt;
				prevPt -= halfWin;
				iprevPt.x = cvFloor(prevPt.x);
				iprevPt.y = cvFloor(prevPt.y);

				if (iprevPt.x < -winSize.width || iprevPt.x >= derivI.cols ||
					iprevPt.y < -winSize.height || iprevPt.y >= derivI.rows)
				{
					if (level == 0)
					{
						if (status) {
							status[ptidx] = false;
						}
						if (err) {
							err[ptidx] = 0;
						}
					}
					continue;
				}

				float a = prevPt.x - iprevPt.x;
				float b = prevPt.y - iprevPt.y;
				const int W_BITS = 14, W_BITS1 = 14;
				const float FLT_SCALE = 1.f / (1 << 20);
				int iw00 = cvRound((1.f - a)*(1.f - b)*(1 << W_BITS));
				int iw01 = cvRound(a*(1.f - b)*(1 << W_BITS));
				int iw10 = cvRound((1.f - a)*b*(1 << W_BITS));
				int iw11 = (1 << W_BITS) - iw00 - iw01 - iw10;

				if ((derivI.total() == 0) || (I.total() == 0) || J.total() == 0) {
					LOG_WARN("Mat size is 0");
				}
				int dstep = (int)(derivI.step / derivI.elemSize1());
				int stepI = (int)(I.step / I.elemSize1());
				int stepJ = (int)(J.step / J.elemSize1());
				acctype iA11 = 0, iA12 = 0, iA22 = 0;
				float A11, A12, A22;

				// extract the patch from the first image, compute covariation cv::Matrix of derivatives
				int x, y;
				for (y = 0; y < winSize.height; y++)
				{
					const uchar* src = I.ptr() + (y + iprevPt.y)*stepI + iprevPt.x*cn;
					const short* dsrc = derivI.ptr<short>() + (y + iprevPt.y)*dstep + iprevPt.x*cn2;

					short* Iptr = IWinBuf.ptr<short>(y);
					short* dIptr = derivIWinBuf.ptr<short>(y);

					x = 0;

					for (; x < winSize.width*cn; x++, dsrc += 2, dIptr += 2)
					{
						int ival = CV_DESCALE(src[x] * iw00 + src[x + cn] * iw01 +
							src[x + stepI] * iw10 + src[x + stepI + cn] * iw11, W_BITS1 - 5);
						int ixval = CV_DESCALE(dsrc[0] * iw00 + dsrc[cn2] * iw01 +
							dsrc[dstep] * iw10 + dsrc[dstep + cn2] * iw11, W_BITS1);
						int iyval = CV_DESCALE(dsrc[1] * iw00 + dsrc[cn2 + 1] * iw01 + dsrc[dstep + 1] * iw10 +
							dsrc[dstep + cn2 + 1] * iw11, W_BITS1);

						Iptr[x] = (short)ival;
						dIptr[0] = (short)ixval;
						dIptr[1] = (short)iyval;

						iA11 += (itemtype)(ixval*ixval);
						iA12 += (itemtype)(ixval*iyval);
						iA22 += (itemtype)(iyval*iyval);
					}
				}

				A11 = iA11*FLT_SCALE;
				A12 = iA12*FLT_SCALE;
				A22 = iA22*FLT_SCALE;

				float D = A11*A22 - A12*A12;
				float minEig = (A22 + A11 - std::sqrt((A11 - A22)*(A11 - A22) +
					4.f*A12*A12)) / (2 * winSize.width*winSize.height);

				if (err && (flags & cv::OPTFLOW_LK_GET_MIN_EIGENVALS) != 0) {
					err[ptidx] = (float)minEig;
				}

				if (minEig < minEigThreshold || D < FLT_EPSILON)
				{
					if (level == 0 && status) {
						status[ptidx] = false;
					}
					continue;
				}

				D = 1.f / D;

				nextPt -= halfWin;
				cv::Point2f prevDelta;

				for (j = 0; j < criteria.maxCount; j++)
				{
					inextPt.x = cvFloor(nextPt.x);
					inextPt.y = cvFloor(nextPt.y);

					if (inextPt.x < -winSize.width || inextPt.x >= J.cols ||
						inextPt.y < -winSize.height || inextPt.y >= J.rows)
					{
						if (level == 0 && status) {
							status[ptidx] = false;
						}
						break;
					}

					a = nextPt.x - inextPt.x;
					b = nextPt.y - inextPt.y;
					iw00 = cvRound((1.f - a)*(1.f - b)*(1 << W_BITS));
					iw01 = cvRound(a*(1.f - b)*(1 << W_BITS));
					iw10 = cvRound((1.f - a)*b*(1 << W_BITS));
					iw11 = (1 << W_BITS) - iw00 - iw01 - iw10;
					acctype ib1 = 0, ib2 = 0;
					float b1, b2;

					for (y = 0; y < winSize.height; y++)
					{
						const uchar* Jptr = J.ptr() + (y + inextPt.y)*stepJ + inextPt.x*cn;
						const short* Iptr = IWinBuf.ptr<short>(y);
						const short* dIptr = derivIWinBuf.ptr<short>(y);

						x = 0;

						for (; x < winSize.width*cn; x++, dIptr += 2)
						{
							int diff = CV_DESCALE(Jptr[x] * iw00 + Jptr[x + cn] * iw01 +
								Jptr[x + stepJ] * iw10 + Jptr[x + stepJ + cn] * iw11,
								W_BITS1 - 5) - Iptr[x];
							ib1 += (itemtype)(diff*dIptr[0]);
							ib2 += (itemtype)(diff*dIptr[1]);
						}
					}

					b1 = ib1*FLT_SCALE;
					b2 = ib2*FLT_SCALE;

					cv::Point2f delta((float)((A12*b2 - A22*b1) * D),
						0);//(float)((A12*b1 - A11*b2) * D)); // MODIFICATION
						   //delta = -delta;

					nextPt += delta;
					nextPts[ptidx] = nextPt + halfWin;

					if (delta.ddot(delta) <= criteria.epsilon) {
						break;
					}

					if (j > 0 && std::abs(delta.x + prevDelta.x) < 0.01 &&
						std::abs(delta.y + prevDelta.y) < 0.01)
					{
						nextPts[ptidx] -= delta*0.5f;
						break;
					}
					prevDelta = delta;
				}

				if (status[ptidx] && err && level == 0 && (flags & cv::OPTFLOW_LK_GET_MIN_EIGENVALS) == 0)
				{
					cv::Point2f nextPoint = nextPts[ptidx] - halfWin;
					cv::Point inextPoint;

					inextPoint.x = cvFloor(nextPoint.x);
					inextPoint.y = cvFloor(nextPoint.y);

					if (inextPoint.x < -winSize.width || inextPoint.x >= J.cols ||
						inextPoint.y < -winSize.height || inextPoint.y >= J.rows)
					{
						if (status) {
							status[ptidx] = false;
						}
						continue;
					}

					float aa = nextPoint.x - inextPoint.x;
					float bb = nextPoint.y - inextPoint.y;
					iw00 = cvRound((1.f - aa)*(1.f - bb)*(1 << W_BITS));
					iw01 = cvRound(aa*(1.f - bb)*(1 << W_BITS));
					iw10 = cvRound((1.f - aa)*bb*(1 << W_BITS));
					iw11 = (1 << W_BITS) - iw00 - iw01 - iw10;
					float errval = 0.f;

					for (y = 0; y < winSize.height; y++)
					{
						const uchar* Jptr = J.ptr() + (y + inextPoint.y)*stepJ + inextPoint.x*cn;
						const short* Iptr = IWinBuf.ptr<short>(y);

						for (x = 0; x < winSize.width*cn; x++)
						{
							int diff = CV_DESCALE(Jptr[x] * iw00 + Jptr[x + cn] * iw01 +
								Jptr[x + stepJ] * iw10 + Jptr[x + stepJ + cn] * iw11,
								W_BITS1 - 5) - Iptr[x];
							errval += std::abs((float)diff);
						}
					}
					err[ptidx] = errval * 1.f / (32 * winSize.width*cn*winSize.height);
				}
			}
		}

	}
}
