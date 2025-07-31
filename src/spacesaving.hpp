#ifndef SS_H_
#define SS_H_

#include <assert.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <list>
#include <unordered_map>
#include <set>
#include "hash.hpp"
#include "sketch.hpp"

template<typename ID_TYPE>
class CocoSketch : public Sketch<ID_TYPE> {
public:
    CocoSketch(int memory, int depth): d(depth) {
        w = memory * 1024 / (sizeof(uint32_t) + sizeof(ID_TYPE)) / d;
        key_array = new ID_TYPE* [d];
        value_array = new uint32_t* [d];
        for (int i = 0; i < d; ++i) {
            key_array[i] = new ID_TYPE [w];
            value_array[i] = new uint32_t [w];
            memset(key_array[i], 0, w * sizeof(ID_TYPE));
            memset(value_array[i], 0, w * sizeof(uint32_t));
        }
    }

    void insert(ID_TYPE key, int32_t value) {
        int min_array = -1, min_bucket = -1;
        uint32_t min_value = -1;
        for (int i = 0; i < d; ++i) {
            uint32_t index = ::hash(key, 500 + i) % w;
            if (key_array[i][index] == key) {
                value_array[i][index] += value;
                return;
            }
            if (value_array[i][index] < min_value) {
                min_array = i;
                min_bucket = index;
                min_value = value_array[i][index];
            }
        }
        double r = static_cast<double>(std::rand()) / RAND_MAX;
        double prob_keep_old = static_cast<double>(min_value) / (min_value + value);
        value_array[min_array][min_bucket] += value;
        if (r >= prob_keep_old) {
            key_array[min_array][min_bucket] = key;
        }
    }

    int32_t query(ID_TYPE key) {
        for (int i = 0; i < d; ++i) {
            uint32_t index = ::hash(key, 500 + i) % w;
            if (key_array[i][index] == key) {
                return value_array[i][index];
            }
        }
        return 0;
    }

private:
    int d, w;
    ID_TYPE** key_array;
    uint32_t** value_array;
};

template<typename ID_TYPE>
class SpaceSaving : public Sketch<ID_TYPE> {
public:
    SpaceSaving(int memory) {
        // for each key in SS, assume that hash_map stores its key and value, and uses 2 pointers to form a list
        // sorted_set stores its key and value, and uses 3 pointers to form a red-black tree
        // so if max_size = N, then SS uses 2 * sizeof(ID_TYPE) + 2 * sizeof(int32_t) + 5 * sizeof(void*) bytes
        max_size = memory * 1024.0 / (2 * sizeof(ID_TYPE) + 2 * sizeof(int32_t) + 5 * sizeof(void*));
        hash_map.clear();
        sorted_set.clear();
    }

    void insert(ID_TYPE key, int32_t value) {
        if (hash_map.find(key) != hash_map.end()) {
            sorted_set.erase({hash_map[key], key});
            hash_map[key] += value;
            sorted_set.insert({hash_map[key], key});
        }
        else if (hash_map.size() < max_size) {
            hash_map[key] = value;
            sorted_set.insert({value, key});
        }
        else {
            auto min_it = *sorted_set.begin();
            ID_TYPE min_key = min_it.second;
            int32_t min_value = min_it.first;
            auto it = hash_map.find(min_key);
            sorted_set.erase({min_value, min_key});
            hash_map.erase(it);
            hash_map[key] = min_value + value;
            sorted_set.insert({min_value + value, key});
        }
    }

    int32_t query(ID_TYPE key) {
        if (hash_map.find(key) == hash_map.end()) {
            return 0;
        }
        return hash_map[key];
    }

    void print_info() {
        for (auto it = sorted_set.begin(), i = 0; i < 500; ++it, ++i) {
            std::cout << it->second << " " << it->first << "\n";
        }
    }


private:
    int max_size;
    std::unordered_map<ID_TYPE, int32_t> hash_map;
    std::multiset<std::pair<int32_t, ID_TYPE>> sorted_set;
};


template<typename ID_TYPE>
class UnbiasedSpaceSaving : public Sketch<ID_TYPE> {
public:
    UnbiasedSpaceSaving(int memory) {
        max_size = memory * 1024.0 / (2 * sizeof(ID_TYPE) + 2 * sizeof(int32_t) + 5 * sizeof(void*));
        hash_map.clear();
        sorted_set.clear();
    }


    void insert(ID_TYPE key, int32_t value) {
        if (hash_map.find(key) != hash_map.end()) {
            sorted_set.erase({hash_map[key], key});
            hash_map[key] += value;
            sorted_set.insert({hash_map[key], key});
        }
        else if (hash_map.size() < max_size) {
            hash_map[key] = value;
            sorted_set.insert({value, key});
        }
        else {
            auto min_it = *sorted_set.begin();
            ID_TYPE min_key = min_it.second;
            int32_t min_value = min_it.first;
            double r = static_cast<double>(std::rand()) / RAND_MAX;
            double prob_keep_old = static_cast<double>(min_value) / (min_value + value);
            auto it = hash_map.find(min_key);
            sorted_set.erase({min_value, min_key});
            hash_map.erase(it);
            if (r >= prob_keep_old) {
                hash_map[key] = min_value + value;
                sorted_set.insert({min_value + value, key});
            }
            else {
                hash_map[min_key] = min_value + value;
                sorted_set.insert({min_value + value, min_key});
            }   
        }
    }

    int32_t query(ID_TYPE key) {
        if (hash_map.find(key) == hash_map.end()) {
            return 0;
        }
        return hash_map[key];
    }

private:
    int max_size;
    std::unordered_map<ID_TYPE, int32_t> hash_map;
    std::multiset<std::pair<int32_t, ID_TYPE>> sorted_set;
};



#endif