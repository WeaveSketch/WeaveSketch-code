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

#include "weavesketch.hpp"
#include "benchmark.hpp"

using namespace std;


int main() {
	vector<pair<uint64_t, uint64_t>> dataset = loadCAIDA("/share/datasets/CAIDA2018/dataset/130100.dat", 20000000);
	// vector<pair<uint64_t, uint64_t>> dataset = loadMAWI("/share/pcap_zhangyd/time07.dat", 20000000);
	// vector<pair<uint32_t, uint32_t>> dataset = loadWeb("/share/datasets/webpage/webdocs_form00.dat", 20000000);
	map<uint64_t, int> ground_truth = get_ground_truth(dataset);
	run(dataset, ground_truth);
	return 0;
}