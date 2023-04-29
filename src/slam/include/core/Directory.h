//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include <string>
#include <vector>
#include <list>

class Directory
{
public:
	Directory(const std::string & path = "");
	~Directory();

	void update(const std::string & path);
	std::string getNextFileName();
	const std::list<std::string> & getFileNames() const {return fileNames_;}

private:
	std::string path_;
	std::list<std::string> fileNames_;
	std::list<std::string>::iterator iFileName_;
};

