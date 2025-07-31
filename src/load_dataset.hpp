#ifndef DATASET_H_
#define DATASET_H_

#include <cinttypes>
#include <iostream>
#include <fstream>
#include <random>
#include <stdexcept>
#include <assert.h>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cmath>
#include <chrono>
#include <map>
using namespace std;

vector<pair<uint64_t, uint64_t>> loadCAIDA(const char *filename, int length) {
	FILE *pf = fopen(filename, "rb");
	if (!pf) {
		printf("%s not found!\n", filename);
		exit(-1);
	}
	map<uint64_t, uint64_t> last_come;
	last_come.clear();

	vector<pair<uint64_t, uint64_t>> dataset;
	dataset.clear();
  
	char trace[30];
	while (fread(trace, 1, 21, pf)) {
		uint64_t tkey = *(uint64_t *)(trace);
    	uint64_t ttime = *(uint64_t *)(trace + 13);
		if (last_come.count(tkey))
    		dataset.push_back(pair<uint64_t, uint64_t>(tkey, ttime - last_come[tkey]));
		last_come[tkey] = ttime;
		if (dataset.size() == length)
    		break;
	}
	fclose(pf);
	return dataset;
}

vector<pair<uint64_t, uint64_t>> loadMAWI(const char *filename, int length) {
	FILE *pf = fopen(filename, "rb");
	if (!pf) {
		printf("%s not found!\n", filename);
		exit(-1);
	}
	map<uint64_t, uint64_t> last_come;
	last_come.clear();

	vector<pair<uint64_t, uint64_t>> dataset;
	dataset.clear();
  
	char trace[30];
	while (fread(trace, 1, 21, pf)) {
		uint64_t tkey = *(uint64_t *)(trace);
    	uint64_t ttime = *(uint64_t *)(trace + 13);
		if (last_come.count(tkey))
    		dataset.push_back(pair<uint64_t, uint64_t>(tkey, ttime - last_come[tkey]));
		last_come[tkey] = ttime;
		if (dataset.size() == length)
    		break;
	}
	fclose(pf);
	return dataset;
}

vector<pair<uint32_t, uint32_t>> loadWeb(const char *filename, int length) {
	FILE *pf = fopen(filename, "rb");
	if (!pf) {
		printf("%s not found!\n", filename);
		exit(-1);
	}
	map<uint32_t, uint32_t> last_come;
	last_come.clear();

	vector<pair<uint32_t, uint32_t>> dataset;
	dataset.clear();
  
	char trace[30];
	while (fread(trace, 1, 8, pf)) {
		uint32_t tkey = *(uint32_t *)(trace);
    	uint32_t ttime = *(uint32_t *)(trace + 4);
		if (last_come.count(tkey))
    		dataset.push_back(pair<uint32_t, uint32_t>(tkey, ttime - last_come[tkey]));
		last_come[tkey] = ttime;
		if (dataset.size() == length)
    		break;
	}
	fclose(pf);
	return dataset;
}

#endif