//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#define NOMINMAX

#include <stdio.h>
#include <signal.h>
#include <list>

#include "core/Odometry.h"
#include "core/Mapper.h"
#include "core/CameraStereoImages.h"
#include "core/Graph.h"
#include "rtabmap/KITTI.h"
#include "core/Logger.h"
#include "core/GFTT.h"
#include "opencv/CvORB.h"
#include "core/Stereo.h"
#include "core/EigenTypes.h"
#include "core/GraphVertex.h"
#include "core/GraphEdge.h"
#include "core/HyperGraph.h"
#include "core/FPGA.h"
#include "core/Parameters.h"
#include "core/Perf.h"
#include "core/Optimizer.h"
#include "octomap/octomap.h"
#include "octomap/OcTree.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h> // mkdir
#include <unistd.h> // sleep
#endif

APP_SETTING appSetting;
Perf perf;

int appStereoCapture (Fpga *fpga, ARG_PARAMS args);
int appFrameGrabber(Fpga *fpga, ARG_PARAMS args);
void buildOccupancyGridMap(
	Mapper &mapper,
	std::map<int, Transform> &optimized_poses
);


//=============================================================================
// U96-SLAM Main Function
//=============================================================================
int main(int argc, char * argv[])
{
	// working directory
#ifdef _WIN32
	_mkdir("work");
#else
	mkdir("work", 0700);
	mkdir("work/image_0", 0700);
	mkdir("work/image_1", 0700);
#endif

	// wait to avoid message overlapping on Petalinux
#ifndef _WIN32
	sleep(5); // seconds
#endif

	Fpga fpga;

	//==================================================================
	// Parameter Settings
	//==================================================================
	ARG_PARAMS args;
	REMOTE_SETTING remoteSetting;
	parseArguments(argc, argv, &args, &appSetting, &remoteSetting);


	//==================================================================
	// Initialize Hardware
	//==================================================================
	if (appSetting.useFpga) {
		// open FPGA register and memory space
		fpga.registerOpen();
		fpga.memoryOpen();

		// start remote application
		unsigned int parm = (
			((remoteSetting.usbOutput & 0xFF) << 16) +
			((remoteSetting.returnData & 0xFF) << 8) +
			(remoteSetting.patternSelect & 0xFF));
#ifndef _WIN32
		sleep(3); // seconds
#endif
		fpga.sendIpcMessage(IPC_MSG1_APP_START, parm);
	}

	//==================================================================
	// Application Select
	//==================================================================
	if (appSetting.appType == APP_TYPE_STEREO_CAPTURE)
	{
		appStereoCapture(&fpga, args);
		return 0;
	}
	else if (appSetting.appType == APP_TYPE_FRAME_GRABBER)
	{
		appFrameGrabber(&fpga, args);
		return 0;
	}

	if (appSetting.inputType == INPUT_TYPE_SENSOR)
	{
		fpga.ledBlink(3);
		while (fpga.isSwitchPressed() == 0) {}
		fpga.sendIpcMessage(IPC_MSG1_OP_START);
		fpga.ledOn();
	}


	//==================================================================
	// Initialize
	//==================================================================
	CameraStereoImages *camera = new CameraStereoImages(
		args.pathLeftImages,
		args.pathRightImages
	);
	camera->setTimestamps(args.pathTimes);
	camera->setGroundTruthPath(args.gtPath);
	camera->init(appSetting.inputType);

	StereoCameraModel stereoCameraModel;
	stereoCameraModel.load(args.pathLeftCalib, args.pathRightCalib, appSetting.doResize);

	int totalImages = (int)camera->filenames().size();
	LOG_INFO("Processing %d images...\n", totalImages);

	Odometry odom;
	ODOM_INFO odomInfo;
	Mapper mapper;
	mapper.init();

	//==================================================================
	// Main Loop
	//==================================================================
	std::vector<cv::KeyPoint> kpts2d;
	cv::Mat desc;
	std::vector<cv::Point3f> kpts3d;
	int iteration = 0;
	while (1)
	{
		perf.setFrameId(iteration);
		perf.addTimeLog("frame_start");

		//--------------------------------------------------------------
		// Capture sensor data
		//--------------------------------------------------------------
		SensorData data(stereoCameraModel);
		if (appSetting.inputType == INPUT_TYPE_FILE) {
			// batch process
			perf.startTime("captureImageLR");
			camera->captureFromFile(data, appSetting);
			perf.stopTime("captureImageLR");

			// FPGA test mode
			if (appSetting.useFpga)
			{
				// write rectified stereo images directly to FPGA work memory
				int bank = iteration % 2;
				fpga.setRectImage(bank, data.imageLeft(), data.imageRight());

				// forcibly run the process
				if (appSetting.depthMethod == DEPTH_METHOD_FPGA_BM){
					fpga.reg->xsbl.Control |= FPGA_XSBL_SW_START;
				}
				if (appSetting.kptsMethod == KPTS_METHOD_FPGA_GFTT){
					fpga.reg->gftt.Control |= FPGA_GFTT_SW_START;
				}

				// receive results from FPGA
				fpga.receiveData(data, appSetting);
			}
		}
		else if (appSetting.inputType == INPUT_TYPE_SENSOR)
		{
			// real-time process
			camera->captureFromFpga(&fpga, data, appSetting);
		}

		// end of the files
		if (data.imageLeft().empty()) {
			break;
		}

		//--------------------------------------------------------------
		// Generate features
		//--------------------------------------------------------------
		if (appSetting.depthMethod == DEPTH_METHOD_CV_BM) {
			int setNumDisparities = 64; // max search range
			int setUniquenessRatio = 10; // NNDR
			cv::Rect roi1, roi2;
			cv::Ptr<cv::StereoBM> bm = cv::StereoBM::create(16, 9);
			bm->setROI1(roi1);
			bm->setROI2(roi2);
			bm->setPreFilterCap(31);
			bm->setBlockSize(21);
			bm->setMinDisparity(0);
			bm->setNumDisparities(setNumDisparities);
			bm->setTextureThreshold(10);
			bm->setUniquenessRatio(setUniquenessRatio);
			bm->setSpeckleWindowSize(50);
			bm->setSpeckleRange(32);
			bm->setDisp12MaxDiff(1);

			cv::Mat disp;
			bm->compute(data.imageLeft(), data.imageRight(), disp);
			data.setImageDepth(disp);
		}
		else if (appSetting.depthMethod == DEPTH_METHOD_CV_SGBM) {
			cv::Ptr<cv::StereoSGBM> sgbm = cv::StereoSGBM::create(
				-64, // minDisparity
				128, // numDisparities
				11, // blockSize
				100, // P1
				1000, // P2
				32, // disp12MaxDiff
				0, // uniquenessRatio
				15, // speckleWindowSize
				1000, // speckleRange
				16, // mode
				cv::StereoSGBM::MODE_HH); // speckleRange

			cv::Mat disp;
			sgbm->compute(data.imageLeft(), data.imageRight(), disp);
			data.setImageDepth(disp);
		}

		perf.startTime("kpts");
		if (appSetting.kptsMethod == KPTS_METHOD_CV_GFTT) {
			generateKeypoints(data.imageLeft(), kpts2d);
		}
		else if (appSetting.kptsMethod == KPTS_METHOD_FPGA_GFTT) {
			generateKeypoints2(data.imageEigen(), data.maxEigen(), kpts2d);
		}
		perf.stopTime("kpts");

		perf.startTime("desc");
		computeDescriptor(data.imageLeft(), cv::noArray(), kpts2d, true, desc);
		perf.stopTime("desc");

		perf.startTime("kpts3d");
		generateKeypoints3D(data, stereoCameraModel, kpts2d, kpts3d, data.imageDepth(), appSetting.depthMethod);
		perf.stopTime("kpts3d");

		data.setFeatures(kpts2d, kpts3d, desc, data.imageDepth());

		// for debugging
		if (iteration == 0)
		{
			//data.saveRectImageKpts();
			//data.saveRectImagePair();
			//data.saveDepthImage();
			//data.saveKpts2d();
			//data.saveKpts3d();
			//data.saveEigenvalue();
			//data.saveDescriptor();
		}

		if (appSetting.inputType == INPUT_TYPE_FILE) {
			// for batch process
			if ((args.numImages != -1) && (iteration == args.numImages)) {
				LOG_INFO(" finish[%d,%d] ", iteration, args.numImages);
				break;
			}
		}
		else {
			// for real-time process, press switch to stop
			if (fpga.isSwitchPressed() == 1) {
				fpga.sendIpcMessage(IPC_MSG1_OP_STOP);
				fpga.ledBlink(0);
				break;
			}
		}

		//--------------------------------------------------------------
		// Visual Odometry
		//--------------------------------------------------------------
		perf.startTime("odom.process");
		odom.process(data, &odomInfo);
		perf.stopTime("odom.process");

		//--------------------------------------------------------------
		// Map Generator
		//--------------------------------------------------------------
		perf.startTime("mapper.process");
		mapper.process(data, odomInfo, appSetting);
		perf.stopTime("mapper.process");

		//--------------------------------------------------------------
		// Memory Usage
		//--------------------------------------------------------------
		if (appSetting.memory && (iteration % 10) == 9) {
			odom.getMemoryUsed();
			mapper.getMemoryUsed();
		}

		LOG_INFO("Iteration %d/%d\n", iteration, totalImages - 1);
		iteration++;
	}

	mapper.cleanupThread();

	LOG_INFO("Total time=%fs\n", perf.elapsedTimeMs() / 1000.0f);

	//==================================================================
	// Graph optimization
	//==================================================================
	std::map<int, Transform> poses;
	std::multimap<int, Link> links;
	mapper.getGraph(poses, links);

	// for debugging graph optimizer
	if (0) {
		savePoses("map_poses.csv", poses);
		saveLinks("map_links.csv", links);
	}
	
	std::map<int, Transform> optimized_poses;
	double err = runOptimizeRobust(poses, links, 20, &optimized_poses);
	LOG_INFO("graph optimizing end (error = %f)\n", err);

	LOG_INFO("Saving trajectory ...\n");
	savePoses("optimized_poses.csv", optimized_poses);

	//==================================================================
	// Ground truth comparison
	//==================================================================
	if (!args.gtPath.empty())
	{
		std::vector<Transform> groundTruth;
		for (std::map<int, Transform>::const_iterator iter = poses.begin(); iter != poses.end(); ++iter)
		{
			const Node *node = mapper.getNode(iter->first);
			Transform gtPose = node->groundTruth();
			if (!gtPose.isNull())
			{
				groundTruth.push_back(gtPose);
			}
		}

		std::vector<Transform> vecPoses;
		for (auto iter = optimized_poses.begin(); iter != optimized_poses.end(); ++iter)
		{
			vecPoses.push_back(iter->second);
		}

		// compute KITTI statistics
		float t_err = 0.0f;
		float r_err = 0.0f;
		graph::calcKittiSequenceErrors(groundTruth, vecPoses, t_err, r_err);
		LOG_INFO("Ground truth comparison:\n");
		LOG_INFO("   KITTI t_err = %f %%\n", t_err);
		LOG_INFO("   KITTI r_err = %f deg/m\n", r_err);
	}

	//==================================================================
	// Generate Occupancy Grid Map
	//==================================================================
	LOG_INFO("Building occupancy grid map\n");
	buildOccupancyGridMap(mapper, optimized_poses);

	// post process
	perf.write("perf_time.csv");
	perf.writeMemoryUsed("perf_memory.csv");

	if (appSetting.useFpga) {
		fpga.registerClose();
		fpga.memoryClose();
	}

	writeToLogFile("log.txt");


#ifdef _WIN32
	getchar();
#endif

	return 0;
}

int appStereoCapture (Fpga *fpga, ARG_PARAMS args)
{
	char filename[100];

	int time = (int)currentTimeMs();

#ifdef _WIN32
	LOG_ERROR("Capture mode doesn't work on Windows");
#else
	mkdir("capture", 0777);
	sprintf(filename, "capture/%06d", time);
	mkdir(filename, 0777);
	sprintf(filename, "capture/%06d/image_0", time);
	mkdir(filename, 0777);
	sprintf(filename, "capture/%06d/image_1", time);
	mkdir(filename, 0777);
#endif

	// initialize
	CameraStereoImages *camera = new CameraStereoImages();

	StereoCameraModel stereoCameraModel;
	stereoCameraModel.load(args.pathLeftCalib, args.pathRightCalib, appSetting.doResize);

	sprintf(filename, "capture/%06d/timestamp.txt", time);
	FILE *fp_time = fopen(filename, "w");
	if (fp_time == 0) {
		LOG_WARN("failed to open timestamp.\n");
		return 0;
	}

	// wait switch input to start
	fpga->ledBlink(3);
	LOG_INFO("\n");
	LOG_INFO("Press switch to start capturing, press again to finish.\n");
	while (fpga->isSwitchPressed() == 0) {}
	fpga->sendIpcMessage(IPC_MSG1_OP_START);
	fpga->ledOn();
	LOG_INFO("Capturing...\n");

	// drop the first few frames, time stamp not stable
	for (int i = 0; i < 10; i++) {
		SensorData data(stereoCameraModel);
		camera->captureFromFpga(fpga, data, appSetting);
	}

	// file capture loop, press switch to leave
	int num_images = 0;
	double start_time = 0;
	while (fpga->isSwitchPressed() == 0)
	{
		// capture sensor data
		SensorData data(stereoCameraModel);
		camera->captureFromFpga(fpga, data, appSetting);

		// left image
		cv::Mat imgDebug;
		imgDebug = data.imageLeft().clone();
		cv::cvtColor(imgDebug, imgDebug, CV_GRAY2RGB);
		sprintf(filename, "capture/%06d/image_0/%06d.jpg", time, num_images);
		imwrite(filename, imgDebug);

		// right image
		imgDebug = data.imageRight().clone();
		cv::cvtColor(imgDebug, imgDebug, CV_GRAY2RGB);
		sprintf(filename, "capture/%06d/image_1/%06d.jpg", time, num_images);
		imwrite(filename, imgDebug);

		// time-stamp
		if (num_images == 0) {
			start_time = data.stamp();
			fprintf(fp_time, "%f,\n", 0.0);
		}
		else {
			fprintf(fp_time, "%f,\n", data.stamp() - start_time);
		}

		num_images++;
	}

	// end of application, system shutdown
	fpga->ledBlink(3);
	LOG_INFO("Captured %d images.\n", num_images);
	fclose(fp_time);
	return 0;
}

int appFrameGrabber(Fpga *fpga, ARG_PARAMS args)
{
#ifdef _WIN32
	LOG_ERROR("Frame Grabber mode doesn't work on Windows");
#endif

	// start streaming
	fpga->sendIpcMessage(IPC_MSG1_OP_START);
	fpga->ledOn();
	LOG_INFO("Streaming...\n");

	// streaming, press switch to leave
	while (fpga->isSwitchPressed() == 0) {}
	fpga->ledBlink(3);

	return 0;
}

void buildOccupancyGridMap(
	Mapper &mapper,
	std::map<int, Transform> &optimized_poses
){
	octomap::OcTree tree(0.1);  // create empty tree with resolution 0.1
	float rangeMax_ = 5.0;
	float rangeMaxSqrd = rangeMax_* rangeMax_;
	int i = 1;
	for (std::map<int, Transform>::iterator iter = optimized_poses.begin(); iter != optimized_poses.end(); ++iter) {
		const Node *node = mapper.getNode(iter->first);
		if (node->getWeight() != -1) {
			Eigen::Isometry3f pose;
			pose(0, 0) = optimized_poses[i].r11();
			pose(0, 1) = optimized_poses[i].r12();
			pose(0, 2) = optimized_poses[i].r13();
			pose(0, 3) = optimized_poses[i].o14();
			pose(1, 0) = optimized_poses[i].r21();
			pose(1, 1) = optimized_poses[i].r22();
			pose(1, 2) = optimized_poses[i].r23();
			pose(1, 3) = optimized_poses[i].o24();
			pose(2, 0) = optimized_poses[i].r31();
			pose(2, 1) = optimized_poses[i].r32();
			pose(2, 2) = optimized_poses[i].r33();
			pose(2, 3) = optimized_poses[i].o34();

			octomap::point3d sensorOrigin(pose(0, 3), pose(1, 3), pose(2, 3));

			cv::Mat depth = node->sensorData().disparity();
			int scale = node->sensorData().dispScale();

			for (int row = 0; row < depth.rows; row++)
			{
				for (int col = 0; col < depth.cols; col++)
				{
					float disparity = (float)(depth.at<short>(row, col) / 16.0f);
					if (disparity > 0)
					{
						StereoCameraModel stereoCameraModel = node->sensorData().stereoCameraModel();
						cv::Point2f pt2d = cv::Point2f((float)(col * scale), (float)(row * scale));
						cv::Point3f pt3d = projectDisparityTo3D(pt2d, disparity, stereoCameraModel);

						if (isFinite(pt3d))
						{
							pt3d = transformPoint(pt3d, stereoCameraModel.localTransform());
							pt3d = transformPoint(pt3d, optimized_poses[i]);

							octomap::point3d pt = octomap::point3d(pt3d.x, pt3d.y, pt3d.z);

							octomap::point3d v(pt.x() - sensorOrigin.x(), pt.y() - sensorOrigin.y(), pt.z() - sensorOrigin.z());
							if (v.norm() <= rangeMaxSqrd)
							{
								octomap::OcTreeKey key;
								tree.coordToKeyChecked(pt, key);
								tree.updateNode(key, true);
							}
						}
					}
				}
			}

		}

		i++;
	}

	tree.writeBinary("slam.bt");
}
