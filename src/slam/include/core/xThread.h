//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#ifdef _WIN32
#include <thread>
#else
#include <pthread.h>
#endif

class xThread
{
public:
	xThread();
	~xThread();

	int create(void*(*func)(void*), void *arg);
	int join(void);
	int lowerProirity(void);

private:
#ifdef _WIN32
	std::thread _th;
#else
	pthread_t _th;
#endif
	
};
