//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include <map>
#include <vector>
#include <mutex>
#include <string>

float currentTimeMs();
float currentTimeSec();

class Perf
{
public:
	Perf();
	virtual ~Perf();

	void startTime(std::string functionName);
	void stopTime(std::string functionName);
	void addTimeLog(std::string functionName);
	void write(const char* filename);
	void setFrameId(int frameId) { _frameId = frameId; }
	float elapsedTimeMs();

	int retrieveMemoryId(std::string functionName);
	std::string retrieveMemoryName(int id);
	void registerMemoryUsed(std::string functionName, unsigned long memoryUsed);
	void writeMemoryUsed(const char* filename);

private:
	void registerTime(int functionId);
	int retrieveId(std::string functionName);
	std::string retrieveName(int id);

	std::mutex _mutex;

	int _frameId;
	float _initialTime;
	std::vector<float> _timer;
	std::map<std::string, int> _functionNameList; // <name, ID>
	std::map<int, std::map<int, float>> _procTime; // <frameID, funcID, time>

	std::map<std::string, int> _memoryNameList; // <name, ID>
	std::map<int, std::map<int, unsigned long>> _memoryUsed; // <frameID, funcID, used memory>
};
