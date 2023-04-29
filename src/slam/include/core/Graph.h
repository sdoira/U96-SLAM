//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#pragma once

#include <map>
#include <core/Link.h>

std::multimap<int, Link>::iterator findLink(std::multimap<int, Link> &links, int from, int to);

bool importPoses(const std::string &filePath, std::map<int, Transform> &poses);

void savePoses(const char* filename, std::map<int, Transform> poses);
void saveLinks(const char* filename, std::multimap<int, Link> links);
void loadPoses(const char *filename, std::map<int, Transform> *poses);
void loadLinks(const char* filename, std::multimap<int, Link> *links);
