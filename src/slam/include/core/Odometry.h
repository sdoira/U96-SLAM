//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include "core/Transform.h"
#include "core/SensorData.h"
#include "core/Registration.h"

struct ODOM_INFO {
	Transform pose;
	bool lost;
	double stamp;
	double interval;
	Transform velocity;
	Transform transform;
	float distanceTravelled;
	cv::Mat covariance;
};

class Odometry
{
public:
	enum State {
		Initialized,
		Running,
		Lost
	};

	Odometry();
	~Odometry();
	void process(SensorData data, ODOM_INFO *odomInfo);

	const Transform &getPose() const { return _pose; }
	unsigned int framesProcessed() const { return framesProcessed_; }

	Transform updateMotion(SensorData data, const Transform guess);

	void setNumObjects(int numObjects) { _numObjects = numObjects; }
	int getNumObjects() { return _numObjects; }

	REG_INFO regInfo() { return _regInfo; }

	void getMemoryUsed();

private:
	Transform _pose; // current pose
	double previousStamp_;
	Transform velocityGuess_;
	unsigned int framesProcessed_;
	Transform lastKeyFramePose_;
	SensorData refFrame_;
	int _numObjects;
	float _guessRatio;
	int _numFeatures;
	bool _keyFrameAdded;
	float _distanceTravelled;
	REG_INFO _regInfo;
	int _state;
};
