#ifndef WEAVESKETCH_H_
#define WEAVESKETCH_H_

#include <assert.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <tuple>
#include "hash.hpp"
#include "sketch.hpp"


using namespace std;

#define BUCKET_SIZE 4

template <typename ID_TYPE>
class Bucket {
public:
	Bucket() {
		memset(key, 0, sizeof(key));
		memset(value, 0, sizeof(value));
		memset(error, 0, sizeof(error));
	}
	ID_TYPE key[BUCKET_SIZE];
	uint32_t value[BUCKET_SIZE];
	int32_t error[BUCKET_SIZE];
};

template <typename ID_TYPE>
class HeavyPart {
public:
	HeavyPart(uint32_t memory) {
		array_num = 2;
		array_size = memory * 1024 / sizeof(Bucket<ID_TYPE>) / array_num;
		array = new Bucket<ID_TYPE>* [array_num];
		for (int i = 0; i < array_num; i++) {
			array[i] = new Bucket<ID_TYPE> [array_size];
			memset(array[i], 0, array_size * sizeof(Bucket<ID_TYPE>));
		}
	}

	int insert(ID_TYPE key, int32_t value) {
		// return -1 if insertion success
		// else, return the minimum value in all related buckets
		int min_value = 1e9;
		for (int i = 0; i < array_num; ++i) {
			uint32_t index = ::hash(key, i) % array_size;
			for (int j = 0; j < BUCKET_SIZE; ++j) {
				if (array[i][index].key[j] == key) {
					array[i][index].value[j] += value;
					return -1;
				}
				else {
					min_value = MIN(min_value, array[i][index].value[j]);
				}
			}
		}
		return min_value;
	}

	tuple<ID_TYPE, uint32_t> insert_with_replace(ID_TYPE key, uint32_t value, int32_t error) {
		uint32_t min_array_index, min_bucket_index, min_cell_index;
		ID_TYPE min_key;
		uint32_t min_value = -1;
		for (int i = 0; i < array_num; ++i) {
			uint32_t index = ::hash(key, i) % array_size;
			for (int j = 0; j < BUCKET_SIZE; ++j) {
				if (array[i][index].value[j] < min_value) {
					min_key = array[i][index].key[j];
					min_value = array[i][index].value[j];
					min_array_index = i;
					min_bucket_index = index;
					min_cell_index = j;
				}
			}
		}
		array[min_array_index][min_bucket_index].key[min_cell_index] = key;
		array[min_array_index][min_bucket_index].value[min_cell_index] = value;
		array[min_array_index][min_bucket_index].error[min_cell_index] = error;
		return make_pair(min_key, min_value);
	}

	tuple<bool, uint32_t, uint32_t> query(ID_TYPE key) {
		for (int i = 0; i < array_num; ++i) {
			uint32_t index = ::hash(key, i) % array_size;
			for (int j = 0; j < BUCKET_SIZE; ++j) {
				if (array[i][index].key[j] == key) {
					return make_tuple(true, array[i][index].value[j], array[i][j].error[j]);
				}
			}
		}
		return make_tuple(false, 0, 0);
	}

	void expansion() {
		// std::cout << "heavy expansion\n";
		Bucket<ID_TYPE> array_old[array_num][array_size];
		uint32_t array_size_old = array_size;
		for (int i = 0; i < array_num; ++i) {
			memcpy(array_old[i], array[i], sizeof(Bucket<ID_TYPE>) * array_size_old);
			delete[] array[i];
		}
		delete[] array;

		array_size *= 2;
		array = new Bucket<ID_TYPE>* [array_num];
		for (int i = 0; i < array_num; i++) {
			array[i] = new Bucket<ID_TYPE> [array_size];
			memcpy(array[i], array_old[i], sizeof(Bucket<ID_TYPE>) * array_size_old);
			memcpy(array[i] + array_size / 2, array_old[i], sizeof(Bucket<ID_TYPE>) * array_size_old);
		}
		for (int i = 0; i < array_num; ++i) {
			for (int k = 0; k < array_size; ++k) {
				for (int j = 0; j < BUCKET_SIZE; ++j) {
					if (!array[i][k].key[j]) {
						continue;
					}
					int32_t correct_index = ::hash(array[i][k].key[j], i) % array_size;
					if (correct_index != k) {
						array[i][k].key[j] = 0;
						array[i][k].value[j] = 0;
						array[i][k].error[j] = 0;
					}
				}
			}
		}
	}
	double calculate_memory() {
		int used_cells = 0, num_cells = array_num * array_size * BUCKET_SIZE;
		for (int i = 0; i < array_num; ++i) {
			for (int j = 0; j < array_size; ++j) {
				for (int k = 0; k < BUCKET_SIZE; ++k) {
					if (array[i][j].key[k]) {
						used_cells++;
					}
				}	
			}
		}
		return 2 * array_size * sizeof(Bucket<ID_TYPE>) / 1024.0;
	}
private:
	Bucket<ID_TYPE>** array;
	uint32_t array_num;
	uint32_t array_size;
};

template<typename ID_TYPE, typename DATA_TYPE>
class LightPart {
public:
	LightPart(int _memory, int _d): d(_d), memory(_memory){
		count_sketch = new CountSketch<ID_TYPE, DATA_TYPE>(memory / 2, d);
		cm_sketch = new CMSketch<ID_TYPE, DATA_TYPE>(memory / 2, d);
	}
	~LightPart() {
		delete count_sketch;
		delete cm_sketch;
	}

	void insert(ID_TYPE key, int32_t value) {
		cm_sketch->insert(key, value);
		count_sketch->insert(key, value);
	}

	uint32_t query_upper_bound(ID_TYPE key) {
		return cm_sketch->query_max(key);
	}

	int32_t query_error(ID_TYPE key) {
		return count_sketch->query(key);
	}

	void expansion() {
		// std::cout << "light expansion\n";
		delete cm_sketch;
		delete count_sketch;
		memory *= 2;
		count_sketch = new CountSketch<ID_TYPE, DATA_TYPE>(memory / 2, d);
		cm_sketch = new CMSketch<ID_TYPE, DATA_TYPE>(memory / 2, d);
	}

	double calculate_memory() {
		return count_sketch->calculate_memory() + cm_sketch->calculate_memory();
	}

private:
	int d;
	int memory;
	CountSketch<ID_TYPE, DATA_TYPE>* count_sketch;
	CMSketch<ID_TYPE, DATA_TYPE>* cm_sketch;
};



template<typename ID_TYPE>
class WeaveSketch: public Sketch<ID_TYPE> {
public:
	WeaveSketch() {}
	WeaveSketch(uint32_t memory, int d, int _max_expansion_time, int _max_error, double memory_ratio): max_expansion_time(_max_expansion_time), max_error(_max_error) {
        current_error = max_error / (pow(2, max_expansion_time + 1) - 1);
		total_error = max_error / (pow(2, max_expansion_time + 1) - 1);
		int heavy_memory = memory_ratio * memory, light_memory = (1 - memory_ratio) * memory;
        int initial_heavy_memory = heavy_memory / pow(2, max_expansion_time), initial_light_memory = light_memory / pow(2, max_expansion_time);
		stage1 = new HeavyPart<ID_TYPE>(initial_heavy_memory);
		stage2 = new LightPart<ID_TYPE, int8_t>(initial_light_memory, d);
	}

	void insert(ID_TYPE key, int32_t value) {
		int min_value = stage1->insert(key, value);
		if (min_value < 0) {
			return;
		}
		if (min_value * 4 > current_error) {
			stage1_insertion_failure++;
			if(stage1_insertion_failure >= pow(4, stage1_expansion_time + 1) && stage1_expansion_time < max_expansion_time) {
                // stage1 expansion
				stage1->expansion();
				stage1_expansion_time++;
				stage1_insertion_failure = 0;
			}	
		}
		uint32_t error = stage2->query_error(key);
		auto replaced_item = stage1->insert_with_replace(key, value, error);
		ID_TYPE replaced_key = get<0>(replaced_item);
		uint32_t replaced_value = get<1>(replaced_item);

		uint32_t cm_upper_bound = stage2->query_upper_bound(key);
		if (cm_upper_bound + replaced_value > current_error && stage2_expansion_time < max_expansion_time) {
			// stage2 expansion
			stage2->expansion();
			current_error *= 2;
			total_error += current_error;
			stage2_expansion_time++;
		}
		
		stage2->insert(replaced_key, replaced_value);
	}

	int32_t query(ID_TYPE key) {
		auto heavy_result = stage1->query(key);
		bool flag = get<0>(heavy_result);
		int32_t value = get<1>(heavy_result), error = get<2>(heavy_result);
		if (flag) {
			return value + error;
		}
		else {
			return stage2->query_error(key);
		}
	}
	int32_t calculate_memory() {
        int stage1_memory = stage1->calculate_memory(), stage2_memory = stage2->calculate_memory();
		std::cout << "Stage1: " << stage1_memory << ", Stage2: " << stage2_memory << "\n";
		return stage1_memory + stage2_memory;
	}
private:
	HeavyPart<ID_TYPE>* stage1;
	LightPart<ID_TYPE, int8_t>* stage2;
    int stage1_expansion_time = 0, stage2_expansion_time = 0;
	int stage1_insertion_failure = 0;
	int max_expansion_time;
	int current_error, total_error, max_error;
};



#endif