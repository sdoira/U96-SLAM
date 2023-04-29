//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/Directory.h"
#include "core/Logger.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

Directory::Directory(const std::string & path)
{
	path_ = path;
	iFileName_ = fileNames_.begin();
	this->update(path);
}

Directory::~Directory()
{
}

bool comp(std::string s1, std::string s2)
{
	// length check
	const char* c1 = s1.c_str();
	const char* c2 = s2.c_str();
	int len1 = 0;
	while (*c1 != '.') {len1++; c1++;}
	int len2 = 0;
	while (*c2 != '.') {len2++; c2++;}
	if (len1 < len2) {
		return true;
	}
	else if (len2 < len1) {
		return false;
	}

	// ASCII code check
	c1 = s1.c_str();
	c2 = s2.c_str();
	while(1) {
		if (*c1 < *c2) {
			return true;
		}
		else if (*c2 < *c1) {
			return false;
		}
		c1++;
		c2++;
	}
}

void Directory::update(const std::string & path)
{
	// search all the image files contained in the designaged directory
	// and save their names.
	fileNames_.clear();

#ifdef _WIN32
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	std::string search_path = path + "*.*"; // search all files
	if ((hFind = FindFirstFile(search_path.c_str(), &FindFileData)) != INVALID_HANDLE_VALUE) {
		do {
			// file extension
			char *ext;
			ext = strrchr(FindFileData.cFileName, '.');

			// store only image files
			if ((strcmp(ext, ".png") == 0) || (strcmp(ext, ".jpg") == 0) || (strcmp(ext, ".bmp") == 0)) {
				fileNames_.push_back(FindFileData.cFileName);
			}
		} while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
#else
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_type == DT_REG) {
				// regular file 
				std::string fname = ent->d_name;
				if (
					(fname.find("png") != std::string::npos) ||
					(fname.find("jpg") != std::string::npos) ||
					(fname.find("bmp") != std::string::npos)
					) {
					fileNames_.push_back(ent->d_name);
				}
			}
		}
		closedir(dir);
	}
	else {
		// could not open directory
		LOG_WARN("failed to open the directory %s\n", path.c_str());
}
#endif

	// sort image files in alphabetical order
	fileNames_.sort(comp);

	// the first image file name
	iFileName_ = fileNames_.begin();

	// check to see the file exists
	std::string firstfile = path + *iFileName_;
	FILE *fp = fopen(firstfile.c_str(), "r");
	if (fp == 0) {
		LOG_WARN("failed to open the image file (%s)\n", firstfile.c_str());
	}
	fclose(fp);

}

std::string Directory::getNextFileName()
{
	std::string fileName;
	if(iFileName_ != fileNames_.end())
	{
		fileName = *iFileName_;
		++iFileName_;
	}
	return fileName;
}
