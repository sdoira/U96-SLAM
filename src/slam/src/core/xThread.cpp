//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/xThread.h"
#include "core/Logger.h"

#ifdef _WIN32
#include <windows.h>
#endif

xThread::xThread() {}
xThread::~xThread() {}

int xThread::create(void*(*func)(void*), void *arg)
{
#ifdef _WIN32
	_th = std::thread(func, arg);
	return 0;
#else
	int ret = pthread_create(&_th, NULL, func, arg);
	if (ret != 0) {
		LOG_WARN("pthread_create failed[%d]", ret);
		return -1;
	}
	return 0;
#endif
}

int xThread::join()
{
#ifdef _WIN32
	_th.join();
	return 0;
#else
	void *status;
	int ret = pthread_join(_th, &status);
	if (ret != 0) {
		LOG_WARN("pthread_join failed[%d,%d]", ret, status);
		return -1;
	}
	return 0;
#endif
}

int xThread::lowerProirity(void)
{
#if defined(_WIN32)
	int result = SetThreadPriority(
		reinterpret_cast<HANDLE>(_th.native_handle()),
		THREAD_PRIORITY_BELOW_NORMAL);
	if (result != 0) {
		LOG_WARN("Setting priority number failed with %d", GetLastError());
		return -1;
	}
	return 0;
#else
	// NOTE:default priority is 0. below code has no effect.
	// get default thread attribute
	pthread_attr_t attr;
	int ret = pthread_attr_init(&attr);
	if (ret != 0) {
		LOG_WARN("pthread_attr_init failed[%d]", ret);
		return -1;
	}

	// get scheduling parameter
	sched_param param;
	ret = pthread_attr_getschedparam(&attr, &param);
	if (ret != 0) {
		LOG_WARN("pthread_attr_getschedparam failed[%d]", ret);
		return -1;
	}

	// lower the priority
	if (param.sched_priority > 0) {
		param.sched_priority -= 1;

		// set new scheduling parameter
		ret = pthread_attr_setschedparam(&attr, &param);
		if (ret != 0) {
			LOG_WARN("pthread_attr_setschedparam failed[%d]", ret);
			return -1;
		}
	}

	return 0;
#endif
}
