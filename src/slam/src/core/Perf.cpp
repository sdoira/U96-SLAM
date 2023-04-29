//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/Perf.h"

#include <time.h>
#include <chrono>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <set>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

float currentTimeMs()
{
#ifdef _WIN32
	SYSTEMTIME st;
	GetLocalTime(&st);
	float time = (float)(((st.wHour * 60 + st.wMinute) * 60 + st.wSecond) * 1000 + st.wMilliseconds);
#else
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	float time = (float)(ts.tv_sec * 1000 + (ts.tv_nsec / 1000000.0));
#endif

	return time;
}

float currentTimeSec()
{
	return currentTimeMs() / 1000.0f;
}

Perf::Perf()
{
	_frameId = 0;
	_initialTime = currentTimeMs();

	_timer.clear();
	_functionNameList.clear();
	_procTime.clear();
	_memoryNameList.clear();
	_memoryUsed.clear();
}

Perf::~Perf()
{
}

float Perf::elapsedTimeMs()
{
	return currentTimeMs() - _initialTime;
}

void Perf::startTime(std::string functionName)
{
	_mutex.lock();

	int functionId = retrieveId(functionName);
	_timer[functionId] = currentTimeMs();

	_mutex.unlock();
}

// retrieve ID, create new one if not registered
int Perf::retrieveId(std::string functionName)
{
	int functionId;

	auto itr = _functionNameList.find(functionName);
	if (itr == _functionNameList.end()) {
		// not found, add new one
		functionId = (int)_functionNameList.size();
		_functionNameList[functionName] = functionId;
		_timer.push_back(0);
	}
	else {
		functionId = itr->second;
	}

	return functionId;
}

std::string Perf::retrieveName(int id)
{
	std::string functionName;

	for (auto itr = _functionNameList.begin(); itr != _functionNameList.end(); itr++) {
		if (itr->second == id) {
			functionName = itr->first;
		}
	}

	return functionName;
}

void Perf::stopTime(std::string functionName)
{
	_mutex.lock();

	int functionId = retrieveId(functionName);
	_timer[functionId] = currentTimeMs() - _timer[functionId];
	registerTime(functionId);

	_mutex.unlock();
}

void Perf::registerTime(int functionId)
{
	// add observed value to _procTime table.
	auto itr_frame = _procTime.find(_frameId);
	if (itr_frame != _procTime.end()) {
		auto itr_func = _procTime[_frameId].find(functionId);
		if (itr_func != _procTime[_frameId].end()) {
			// already exits -> accumulate
			_procTime[_frameId][functionId] += _timer[functionId];
		}
		else {
			_procTime[_frameId][functionId] = _timer[functionId];
		}
	}
	else {
		// first item for the current frame
		_procTime[_frameId][functionId] = _timer[functionId];
	}
}

void Perf::addTimeLog(std::string functionName)
{
	_mutex.lock();

	int functionId = retrieveId(functionName);
	_procTime[_frameId][functionId] = currentTimeMs();

	_mutex.unlock();
}

void Perf::write(const char* filename)
{
	FILE *fp = fopen(filename, "w");
	if (fp != 0) {
		// function name
		for (int i = 0; i < (int)_functionNameList.size(); i++) {
			fprintf(fp, "%s,", retrieveName(i).c_str());
		}
		fprintf(fp, "\n");

		// processing time for each function
		for (auto itr_frame = _procTime.begin(); itr_frame != _procTime.end(); itr_frame++) {
			for (int functionId = 0; functionId < (int)_functionNameList.size(); functionId++) {
				auto itr_find = itr_frame->second.find(functionId);
				if (itr_find != itr_frame->second.end()) {
					fprintf(fp, "%f,", itr_find->second);
				}
				else {
					// fills zeros for missing elements
					fprintf(fp, "0,");
				}
			}
			fprintf(fp, "\n");
		}

		fclose(fp);
	}
}

// retrieve ID, create new one if not registered
int Perf::retrieveMemoryId(std::string functionName)
{
	int functionId;

	auto itr = _memoryNameList.find(functionName);
	if (itr == _memoryNameList.end()) {
		// not found, add new one
		functionId = (int)_memoryNameList.size();
		_memoryNameList[functionName] = functionId;
	}
	else {
		functionId = itr->second;
	}

	return functionId;
}

std::string Perf::retrieveMemoryName(int id)
{
	std::string functionName;

	for (auto itr = _memoryNameList.begin(); itr != _memoryNameList.end(); itr++) {
		if (itr->second == id) {
			functionName = itr->first;
		}
	}

	return functionName;
}

void Perf::registerMemoryUsed(std::string functionName, unsigned long memoryUsed)
{
	_mutex.lock();

	int functionId = retrieveMemoryId(functionName);

	// add observed value to _procTime table.
	auto itr_frame = _memoryUsed.find(_frameId);
	if (itr_frame != _memoryUsed.end()) {
		auto itr_func = _memoryUsed[_frameId].find(functionId);
		if (itr_func != _memoryUsed[_frameId].end()) {
			// already exits -> accumulate
			_memoryUsed[_frameId][functionId] += memoryUsed;
		}
		else {
			_memoryUsed[_frameId][functionId] = memoryUsed;
		}
	}
	else {
		// first item for the current frame
		_memoryUsed[_frameId][functionId] = memoryUsed;
	}

	_mutex.unlock();
}

void Perf::writeMemoryUsed(const char* filename)
{
	FILE *fp = fopen(filename, "w");
	if (fp != 0) {
		// function name
		for (int i = 0; i < (int)_memoryNameList.size(); i++) {
			fprintf(fp, "%s,", retrieveMemoryName(i).c_str());
		}
		fprintf(fp, "\n");

		// used memory for each item
		for (auto itr_frame = _memoryUsed.begin(); itr_frame != _memoryUsed.end(); itr_frame++) {
			for (int functionId = 0; functionId < (int)_memoryNameList.size(); functionId++) {
				auto itr_find = itr_frame->second.find(functionId);
				if (itr_find != itr_frame->second.end()) {
					fprintf(fp, "%ld,", itr_find->second);
				}
				else {
					// fills zeros for missing elements
					fprintf(fp, "0,");
				}
			}
			fprintf(fp, "\n");
		}

		fclose(fp);
	}
}
