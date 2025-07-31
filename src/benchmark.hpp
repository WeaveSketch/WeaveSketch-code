#ifndef BENCHMARK_H_
#define BENCHMARK_H_

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
#include "weavesketch.hpp"
#include "elastic.hpp"
#include "spacesaving.hpp"
#include "load_dataset.hpp"
using namespace std;


template<typename ID_TYPE, typename TS_TYPE>
map<ID_TYPE, int> get_ground_truth(vector<pair<ID_TYPE, TS_TYPE>> dataset) {
	map<ID_TYPE, int> ground_truth;
	int heavy_key = 0;
    for(auto &p : dataset){
        ID_TYPE key = p.first;
        ground_truth[key]++;
    }
	// std::cout << ground_truth.size() << "\n";
	return ground_truth;
}

template<typename ID_TYPE>
void get_error(Sketch<ID_TYPE>* sketch, map<ID_TYPE, int> ground_truth, int max_error, double insert_throughput) {
	double aae = 0, are = 0;
	double outliers = 0;
	auto start_time = std::chrono::high_resolution_clock::now();
	for (auto &p : ground_truth) {
        int result = sketch->query(p.first);
	}
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	double elapsed_time = duration.count() / 1000.0;
	double query_throughput = ground_truth.size() / elapsed_time / 1e6;
	for (auto &p : ground_truth) {
        int result = sketch->query(p.first);
        aae += 1.0 * fabs(p.second - result);
        are += 1.0 * fabs(p.second - result) / p.second;
        if (fabs(p.second - result) > max_error) {
            outliers++;
        }
    }
	aae /= ground_truth.size();
    are /= ground_truth.size();
	std::cout << aae << " " << are << " " << outliers << " " << insert_throughput << " " << query_throughput << "\n";;
}


template<typename ID_TYPE>
void get_heavy_error(Sketch<ID_TYPE>* sketch, map<ID_TYPE, int> ground_truth, int max_error) {
	double outliers = 0;
	int heavy_flow = 0;
	for (auto &p : ground_truth) {
		if (p.second < 1000) {
			continue;
		}
		heavy_flow++;
        int result = sketch->query(p.first);
        if (fabs(p.second - result) > max_error) {
            outliers++;
        }
    }
	std::cout << outliers << "\n";
}





template<typename ID_TYPE, typename TS_TYPE>
void run(vector<std::pair<ID_TYPE, TS_TYPE>> dataset, map<ID_TYPE, int> ground_truth) {
	int max_error = 14;
	for (int memory = 100; memory <= 2000; memory += 100) {
		int depth = 3;
		Sketch<ID_TYPE>* weavesketch = new WeaveSketch<ID_TYPE>(memory, 3, 3, max_error, 0.8);
		Sketch<ID_TYPE>* cmsketch = new CMSketch<ID_TYPE, int32_t>(memory, depth);
		Sketch<ID_TYPE>* cusketch = new CUSketch<ID_TYPE, int32_t>(memory, depth);
		Sketch<ID_TYPE>* countsketch = new CountSketch<ID_TYPE, int32_t>(memory, depth);
		Sketch<ID_TYPE>* elasticsketch = new ElasticSketch<ID_TYPE>(memory); 
		Sketch<ID_TYPE>* spacesaving = new SpaceSaving<ID_TYPE>(memory);
		Sketch<ID_TYPE>* uss = new UnbiasedSpaceSaving<ID_TYPE>(memory);
		Sketch<ID_TYPE>* coco = new CocoSketch<ID_TYPE>(memory, depth);
		auto start_time = std::chrono::high_resolution_clock::now();
		for (auto &p : dataset) {
			ID_TYPE key = p.first;
			weavesketch->insert(key, 1);
			// cmsketch->insert(key, 1);
			// cusketch->insert(key, 1);
			// countsketch->insert(key, 1);
			// elasticsketch->insert(key, 1);
			// spacesaving->insert(key, 1);
			// uss->insert(key, 1);
			// coco->insert(key, 1);
		}
		auto end_time = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
		double elapsed_time = duration.count() / 1000.0;
		double insert_throughput = dataset.size() / elapsed_time / 1e6;

		// std::cout << memory << " ";
		get_error(weavesketch, ground_truth, max_error, insert_throughput);
		// get_error(cmsketch, ground_truth, max_error, insert_throughput);
		// get_error(cusketch, ground_truth, max_error, insert_throughput);
		// get_error(countsketch, ground_truth, max_error, insert_throughput);
		// get_error(elasticsketch, ground_truth, max_error, insert_throughput);
		// get_error(spacesaving, ground_truth, max_error, insert_throughput);	
		// get_error(uss, ground_truth, max_error, insert_throughput);
		// get_error(coco, ground_truth, max_error, insert_throughput);
		// std::cout << "\n";
	}
}


#endif