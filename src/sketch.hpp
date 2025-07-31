#ifndef SKETCH_H_
#define SKETCH_H_

#include <assert.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <stdint.h>
#include "hash.hpp"

const int count_sketch_sign[2] = {-1, 1};


template<typename ID_TYPE>
class Sketch {
public:
    virtual ~Sketch() {}
	virtual void insert(ID_TYPE key, int32_t value) = 0;
	virtual int32_t query(ID_TYPE key) = 0;
};

template<typename ID_TYPE, typename DATA_TYPE>
class CMSketch : public Sketch<ID_TYPE> {
public:
	CMSketch(int memory, int _d): d(_d) {
		w = memory * 1024 / sizeof(DATA_TYPE) / d;
		counter = new DATA_TYPE* [d];
		for (int i = 0; i < d; ++i) {
			counter[i] = new DATA_TYPE [w];
			memset(counter[i], 0, w * sizeof(DATA_TYPE));
		}
	}

	~CMSketch() {
		for (int i = 0; i < d; ++i) {
			delete[] counter[i];
		}
		delete[] counter;
	}

	void insert(ID_TYPE key, int32_t value) {
		for (int i = 0; i < d; ++i) {
			uint32_t index = ::hash(key, 33 + i) % w;
			counter[i][index] += value;
		}
	}

	int32_t query(ID_TYPE key) {
		int32_t min_value = 1e9;
		for (int i = 0; i < d; ++i) {
			uint32_t index = ::hash(key, 33 + i) % w;
			min_value = MIN(counter[i][index], min_value);
		}
		return min_value;
	}

	int32_t query_max(ID_TYPE key) {
		int32_t max_value = 0;
		for (int i = 0; i < d; ++i) {
			uint32_t index = ::hash(key, 33 + i) % w;
			max_value = MAX(counter[i][index], max_value);
		}
		return max_value;
	}
	double calculate_memory() {
		return d * w * sizeof(DATA_TYPE) / 1024.0;
	}
	void print_info() {
		for(int i = 0; i < d; ++i) {
			for (int j = 0; j < w; ++j) {
				std::cout << (int)(counter[i][j]) << " ";
			}
			std::cout << "\n";
		}
	}
private:
	int d, w;
	DATA_TYPE** counter;
};

template<typename ID_TYPE, typename DATA_TYPE>
class CUSketch : public Sketch<ID_TYPE> {
public:
	CUSketch(int memory, int _d): d(_d) {
		w = memory * 1024 / sizeof(DATA_TYPE) / d;
		counter = new DATA_TYPE* [d];
		for (int i = 0; i < d; ++i) {
			counter[i] = new DATA_TYPE [w];
			memset(counter[i], 0, w * sizeof(DATA_TYPE));
		}
	}

	~CUSketch() {
		for (int i = 0; i < d; ++i) {
			delete[] counter[i];
		}
		delete[] counter;
	}

	void insert(ID_TYPE key, int32_t value) {
		int32_t min_value = 1e9;
		for (int i = 0; i < d; ++i) {
			uint32_t index = ::hash(key, 33 + i) % w;
			min_value = MIN(min_value, counter[i][index]);
		}
		for (int i = 0; i < d; ++i) {
			uint32_t index = ::hash(key, 33 + i) % w;
			if (counter[i][index] <= min_value + value) {
				counter[i][index] = min_value + value;
			}
		}
	}

	int32_t query(ID_TYPE key) {
		int32_t min_value = 1e9;
		for (int i = 0; i < d; ++i) {
			uint32_t index = ::hash(key, 33 + i) % w;
			min_value = MIN(counter[i][index], min_value);
		}
		return min_value;
	}

	int32_t query_max(ID_TYPE key) {
		int32_t max_value = 0;
		for (int i = 0; i < d; ++i) {
			uint32_t index = ::hash(key, 33 + i) % w;
			max_value = MAX(counter[i][index], max_value);
		}
		return max_value;
	}
	double calculate_memory() {
		return d * w * sizeof(DATA_TYPE) / 1024.0;
	}
	void print_info() {
		for(int i = 0; i < d; ++i) {
			for (int j = 0; j < w; ++j) {
				std::cout << counter[i][j] << " ";
			}
			std::cout << "\n";
		}
	}
private:
	int d, w;
	DATA_TYPE** counter;
};

template<typename ID_TYPE, typename DATA_TYPE>
class CountSketch : public Sketch<ID_TYPE> {
public:
	CountSketch(int memory, int _d): d(_d) {
		w = memory * 1024 / sizeof(DATA_TYPE) / d;
		counter = new DATA_TYPE* [d];
		for (int i = 0; i < d; ++i) {
			counter[i] = new DATA_TYPE [w];
			memset(counter[i], 0, w * sizeof(DATA_TYPE));
		}
	}

	~CountSketch() {
		for (int i = 0; i < d; ++i) {
			delete[] counter[i];
		}
		delete[] counter;
	}

	void insert(ID_TYPE key, int32_t value) {
		for (int i = 0; i < d; ++i) {
			uint32_t index = ::hash(key, 33 + i) % w;
			uint32_t sign_index = ::hash(key, 99 + i) % 2;
			counter[i][index] += count_sketch_sign[sign_index] * value;
		}
	}

	int32_t query(ID_TYPE key) {
		std::vector<int32_t> vec;
		for (int i = 0; i < d; ++i) {
			uint32_t index = ::hash(key, 33 + i) % w;
			uint32_t sign_index = ::hash(key, 99 + i) % 2;
			vec.push_back(count_sketch_sign[sign_index] * counter[i][index]);
		}
		std::sort(vec.begin(), vec.end());
		return vec[(d - 1) / 2];
	}
	double calculate_memory() {
		return d * w * sizeof(DATA_TYPE) / 1024.0;
	}
	void print_info() {
		for(int i = 0; i < d; ++i) {
			for (int j = 0; j < w; ++j) {
				std::cout << counter[i][j] << " ";
			}
			std::cout << "\n";
		}
	}
private:
	int d, w;
	DATA_TYPE** counter;
};

#endif