#ifndef ELASTIC_H_
#define ELASTIC_H_

#include <assert.h>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include "hash.hpp"
#include "sketch.hpp"



template<typename ID_TYPE>
class Bucket_Elastic {
public:
    ID_TYPE key = 0;
    int32_t pos_vote = 0;
    int32_t neg_vote = 0;
    bool flag = false;
};




template<typename ID_TYPE>
class ElasticSketch : public Sketch<ID_TYPE> {
public:
    ElasticSketch(int memory) {
        array_size = 0.5 * memory * 1024 / sizeof(Bucket_Elastic<ID_TYPE>);
        bucket = new Bucket_Elastic<ID_TYPE>[array_size];
        cmsketch = new CMSketch<ID_TYPE, uint16_t>(0.5 * memory, 3);
    }

    ~ElasticSketch() {
        delete[] bucket;
        delete cmsketch;
    }
    void insert(ID_TYPE key, int32_t value) {
        uint32_t index = ::hash(key, 50) % array_size;
        if (bucket[index].key == key) {
            bucket[index].pos_vote++;
        }
        else if (!bucket[index].key){
            bucket[index].key = key;
            bucket[index].pos_vote = value;
        }
        else {
            bucket[index].neg_vote++;
            if (bucket[index].neg_vote >= 8 * bucket[index].pos_vote) {
                cmsketch->insert(bucket[index].key, bucket[index].pos_vote);
                bucket[index].key = key;
                bucket[index].pos_vote = value;
                bucket[index].neg_vote = 0;
                bucket[index].flag = true;
            }
            else {
                cmsketch->insert(key, value);
            }
        }
    }
    int32_t query(ID_TYPE key) {
        int32_t result = 0;
        uint32_t index = ::hash(key, 50) % array_size;
        if (bucket[index].key == key) {
            result += bucket[index].pos_vote;
            if (!bucket[index].flag) {
                return result;
            }
        }
        result += cmsketch->query(key);
        return result;
    }
private:
    Bucket_Elastic<ID_TYPE>* bucket;
    int array_size;
    CMSketch<ID_TYPE, uint16_t>* cmsketch;
};














#endif