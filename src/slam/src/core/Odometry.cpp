//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/Odometry.h"
#include "opencv/CvLKStereo.h"
#include "core/Logger.h"
#include "core/Registration.h"
#include "core/Perf.h"

extern Perf perf;

Odometry::Odometry()
{
	_pose = Transform::getIdentity();
	previousStamp_ = 0.000000;
	framesProcessed_ = 0;
	lastKeyFramePose_.setNull();
	refFrame_ = SensorData();
	_numObjects = 0;
	_guessRatio = 0.25;
	_numFeatures = 0;
	_keyFrameAdded = false;
	_distanceTravelled = 0.0f;
	_state = Odometry::Initialized;
}

Odometry::~Odometry()
{}

void Odometry::process(SensorData data, ODOM_INFO *odomInfo)
{
	// delta t
	double dt;
	if (framesProcessed() == 0) {
		dt = 0.0;
	}
	else {
		dt = data.stamp() - previousStamp_;
	}

	// Guess information from previous odometry
	Transform guess;
	if (velocityGuess_.isNull()) {
		guess = Transform();
	}
	else {
		float vx, vy, vz, vroll, vpitch, vyaw;
		vx = velocityGuess_.x();
		vy = velocityGuess_.y();
		vz = velocityGuess_.z();
		velocityGuess_.getAngles(vroll, vpitch, vyaw);
		guess = Transform(
			vx*(float)dt, vy*(float)dt, vz*(float)dt,
			vroll*(float)dt, vpitch*(float)dt, vyaw*(float)dt);
	}

	// Estimate camera transform
	Transform t = this->updateMotion(data, guess);
	
	if (dt)
	{
		// rotation matrix to roll/pitch/yaw
		float vx, vy, vz, vroll, vpitch, vyaw;
		vx = t.x();
		vy = t.y();
		vz = t.z();
		t.getAngles(vroll, vpitch, vyaw);

		// velocity
		vx /= (float)dt;
		vy /= (float)dt;
		vz /= (float)dt;
		vroll /= (float)dt;
		vpitch /= (float)dt;
		vyaw /= (float)dt;

		// convert back to [R,t] format
		velocityGuess_ = Transform(vx, vy, vz, vroll, vpitch, vyaw);
	}
	else
	{
		velocityGuess_.setNull();
	}

	_distanceTravelled += t.getNorm();
	previousStamp_ = data.stamp();
	++framesProcessed_;

	// Update current position
	_pose *= t;

	// Output info
	odomInfo->pose = _pose;
	odomInfo->lost = t.isNull();
	odomInfo->stamp = data.stamp();
	odomInfo->interval = dt;
	odomInfo->transform = t;
	odomInfo->distanceTravelled = _distanceTravelled;
	odomInfo->velocity = velocityGuess_;
	odomInfo->covariance = _regInfo.covariance;
}

Transform Odometry::updateMotion(
	SensorData data,
	const Transform guess)
{
	// Register key frame
	// becomes null if the key frame has been updated
	if (lastKeyFramePose_.isNull())
	{
		lastKeyFramePose_ = this->getPose(); // set current pose
	}

	Transform motionSinceLastKeyFrame = lastKeyFramePose_.inverse() * this->getPose();

	//==================================================================
	// Calculate camera pose between frames
	//==================================================================
	Transform t;
	struct REG_INFO regInfo;
	if (framesProcessed_ == 0)
	{
		t = Transform::getIdentity();
		regInfo.covariance = cv::Mat::eye(6, 6, CV_64FC1) * 9999.0;
	}
	else
	{
		Transform guessUpdate;
		if (guess.isNull()) {
			guessUpdate = Transform(); // set null
		}
		else {
			guessUpdate = motionSinceLastKeyFrame * guess;
		}

		t = computeTransform(refFrame_, data, guessUpdate, &regInfo);

		if (!guessUpdate.isNull() && (regInfo.num_matches < this->getNumObjects() * _guessRatio)) {
			LOG_INFO(" Wrong Guess ");
			t = computeTransform(refFrame_, data, Transform(), &regInfo);
		}

		// store the number of matched keypoints
		this->setNumObjects(regInfo.num_matches);
	}

	if (t.isNull()) {
		if (_state == Odometry::Running) {
			LOG_INFO(" Odometry Lost ");
		}
		_state = Odometry::Lost;
	}
	else {
		_state = Odometry::Running;
	}

	Transform output = motionSinceLastKeyFrame.inverse() * t;

	//==================================================================
	// Key frame update
	//==================================================================
	// the key frame will be updated when the number of inliers is 
	// below the threshold.
	float keyFrameThr = 0.3f;
	int visKeyFrameThr = 150;
	bool addKeyFrame = false;
	if (
		(framesProcessed_ == 0) ||
		float(regInfo.num_inliers) <= keyFrameThr * float(refFrame_.keypoints().size()) ||
		regInfo.num_inliers <= visKeyFrameThr)
	{
		refFrame_ = data;
		lastKeyFramePose_.setNull();
		addKeyFrame = true;
	}

	_numFeatures = (int)data.keypoints().size();
	_keyFrameAdded = addKeyFrame;
	_regInfo = regInfo;

	return output;
}

void Odometry::getMemoryUsed()
{
	unsigned long memUsed =
		sizeof(Odometry) +
		_pose.getSize() +
		velocityGuess_.getSize() +
		lastKeyFramePose_.getSize();

	perf.registerMemoryUsed("Odometry", memUsed);

	refFrame_.getMemoryUsed();
}
