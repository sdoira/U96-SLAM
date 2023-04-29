//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/Logger.h"
#include "core/Parameters.h"
#include <mutex>
#include <vector>

extern APP_SETTING appSetting;
std::mutex loggerMutex_;
std::vector<std::string> messages_;

void log_write(
	LOG_LEVEL level,
	const char *file,
	int line,
	const char *function,
	const char *msg,
	...)
{
	loggerMutex_.lock();

	va_list args;
	va_start(args, msg);

	// show messages
	if (!appSetting.quiet) {
		if (level == LOG_LEVEL_WARN) {
			printf(" [WARNING %s line%d %s] ", file, line, function);
		}
		else if (level == LOG_LEVEL_ERROR) {
			printf(" [ERROR %s line%d %s] ", file, line, function);
		}

		if (level != LOG_LEVEL_DEBUG) {
			vprintf(msg, args);
		}
	}

	// save log messages, they will be written to a file later
	int len = vsnprintf(NULL, 0, msg, args);
	std::vector<char> buf(len + 1);
	vsnprintf(&buf[0], len + 1, msg, args);
	std::string str(&buf[0], &buf[0] + len);
	messages_.push_back(str);

	va_end(args);

	loggerMutex_.unlock();

	// halt on error
	if (level == LOG_LEVEL_ERROR) {
		while (1) {}
	}
}

void writeToLogFile(const char* filename) {
	FILE *fp_log = fopen(filename, "w");
	if (fp_log != 0) {
		for (int i = 0; i < (int)messages_.size(); i++) {
			fprintf(fp_log, "%s", messages_[i].c_str());
		}
		messages_.clear();
		messages_.shrink_to_fit();
		fclose(fp_log);
	}
}
