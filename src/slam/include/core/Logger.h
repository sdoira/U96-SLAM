//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include <stdio.h>
#include <stdarg.h>

enum LOG_LEVEL { LOG_LEVEL_DEBUG, LOG_LEVEL_INFO, LOG_LEVEL_WARN, LOG_LEVEL_ERROR };

void log_write(
	LOG_LEVEL level,
	const char *file,
	int line,
	const char *function,
	const char *msg,
	...);

#define LOG_DEBUG(...)	log_write(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...)	log_write(LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_WARN(...)	log_write(LOG_LEVEL_WARN, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOG_ERROR(...)	log_write(LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

void clearLogFile();
void writeToLogFile(const char* filename);
