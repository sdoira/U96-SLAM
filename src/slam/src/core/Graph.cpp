//=============================================================================
// Copyright (C) 2023 Nu-Gate Technology. All rights reserved.
// SPDX-License-Identifier: MIT
//=============================================================================
#include "core/Graph.h"
#include "core/Logger.h"

std::multimap<int, Link>::iterator findLink(std::multimap<int, Link> &links, int from, int to)
{
	// search a link connects "from" -> "to"
	std::multimap<int, Link>::iterator iter = links.find(from);
	while (iter != links.end() && iter->first == from) {
		if (iter->second.to() == to) {
			return iter;
		}
		++iter;
	}

	// search reverse direction
	iter = links.find(to);
	while (iter != links.end() && iter->first == to) {
		if (iter->second.to() == from) {
			return iter;
		}
		++iter;
	}

	// not found
	return links.end();
}

bool importPoses(const std::string &filePath, std::map<int, Transform> &poses)
{
	int id = 1;
	FILE *fp = fopen(filePath.c_str(), "r");
	if (fp != 0) {
		float r[3][3];
		float t[3];
		Transform p(
			0.0f, 0.0f, 1.0f, 0.0f,
			-1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f);
		while (1) {
			int ret = fscanf(
				fp,
				"%f %f %f %f %f %f %f %f %f %f %f %f\n",
				&r[0][0], &r[0][1], &r[0][2], &t[0],
				&r[1][0], &r[1][1], &r[1][2], &t[1],
				&r[2][0], &r[2][1], &r[2][2], &t[2]);

			if (ret != 12) break;

			Transform pose = Transform(
				r[0][0], r[0][1], r[0][2], t[0],
				r[1][0], r[1][1], r[1][2], t[1],
				r[2][0], r[2][1], r[2][2], t[2]);

			pose = p * pose * p.inverse();
			poses.insert(std::make_pair(id, pose));

			++id;
		};
		fclose(fp);
	}
	else {
		LOG_WARN("failed to open gt poses");
	}

	return true;
}

void savePoses(const char* filename, std::map<int, Transform> poses) {
	FILE *fp_map_poses = fopen(filename, "w");
	for (auto itr = poses.begin(); itr != poses.end(); ++itr) {
		fprintf(
			fp_map_poses,
			"%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,\n",
			itr->first,
			itr->second.r11(), itr->second.r12(), itr->second.r13(), itr->second.o14(),
			itr->second.r21(), itr->second.r22(), itr->second.r23(), itr->second.o24(),
			itr->second.r31(), itr->second.r32(), itr->second.r33(), itr->second.o34());
	}
	fclose(fp_map_poses);
}

void saveLinks(const char* filename, std::multimap<int, Link> links) {
	FILE *fp_map_links = fopen(filename, "w");
	for (auto itr = links.begin(); itr != links.end(); ++itr) {

		fprintf(fp_map_links, "%d,%d,%d,%d,", itr->second.from(), itr->second.from(), itr->second.to(), itr->second.type());

		Transform tr = itr->second.transform();
		fprintf(
			fp_map_links,
			"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,",
			tr.r11(), tr.r12(), tr.r13(), tr.o14(),
			tr.r21(), tr.r22(), tr.r23(), tr.o24(),
			tr.r31(), tr.r32(), tr.r33(), tr.o34());

		const cv::Mat *infMatrix = &itr->second.infMatrix();
		for (int row = 0; row < infMatrix->rows; row++) {
			for (int col = 0; col < infMatrix->cols; col++) {
				fprintf(fp_map_links, "%f,", infMatrix->at<double>(row, col));
			}
		}

		fprintf(fp_map_links, "\n");
	}
	fclose(fp_map_links);
}

void loadPoses(const char *filename, std::map<int, Transform> *poses)
{
	printf(" *** overwrite poses *** ");
	int id;
	float r11, r12, r13, o14;
	float r21, r22, r23, o24;
	float r31, r32, r33, o34;

	poses->clear();
	FILE *fp_poses = fopen(filename, "r");
	if (fp_poses == 0) {
		printf(" failed to open %s ", filename);
		return;
	}
	while (1) {
		int n = fscanf(fp_poses, "%d,", &id);
		if (n != 1) break;
		fscanf(fp_poses, "%f,%f,%f,%f,", &r11, &r12, &r13, &o14);
		fscanf(fp_poses, "%f,%f,%f,%f,", &r21, &r22, &r23, &o24);
		fscanf(fp_poses, "%f,%f,%f,%f,", &r31, &r32, &r33, &o34);

		Transform tr = Transform(r11, r12, r13, o14, r21, r22, r23, o24, r31, r32, r33, o34);

		poses->insert(std::make_pair(id, tr));
	}
	fclose(fp_poses);
}

void loadLinks(const char* filename, std::multimap<int, Link> *links)
{
	printf(" *** overwrite links *** ");
	int id[3];
	int type;
	float r11, r12, r13, o14;
	float r21, r22, r23, o24;
	float r31, r32, r33, o34;

	links->clear();
	FILE *fp_links = fopen(filename, "r");
	if (fp_links == 0) {
		printf(" failed to open %s ", filename);
		return;
	}
	while (1) {
		int n = fscanf(fp_links, "%d,", &id[0]);
		if (n != 1) break;
		fscanf(fp_links, "%d,", &id[1]);
		fscanf(fp_links, "%d,", &id[2]);
		fscanf(fp_links, "%d,", &type);
		fscanf(fp_links, "%f,%f,%f,%f,", &r11, &r12, &r13, &o14);
		fscanf(fp_links, "%f,%f,%f,%f,", &r21, &r22, &r23, &o24);
		fscanf(fp_links, "%f,%f,%f,%f,", &r31, &r32, &r33, &o34);
		Transform tr = Transform(r11, r12, r13, o14, r21, r22, r23, o24, r31, r32, r33, o34);
		cv::Mat infMatrix = cv::Mat(6, 6, CV_64FC1);
		for (int row = 0; row < infMatrix.rows; row++) {
			for (int col = 0; col < infMatrix.cols; col++) {
				fscanf(fp_links, "%lf,", &infMatrix.at<double>(row, col));
			}
		}
		Link link(id[1], id[2], (Link::Type)type, tr, infMatrix);
		links->insert(std::make_pair(id[0], link));
	}
	fclose(fp_links);
}
