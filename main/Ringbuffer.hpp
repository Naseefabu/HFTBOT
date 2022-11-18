#pragma once

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include "types.hpp"


static constexpr int CACHELINE_SIZE = 64;

// Single Producer Single Consumer Ringbuffer Queue

template <typename T> struct SPSCQueue {
    std::vector<T> data_{};
    alignas(CACHELINE_SIZE) std::atomic<size_t> readIdx_{0};
    alignas(CACHELINE_SIZE) size_t writeIdxCached_{0};
    alignas(CACHELINE_SIZE) std::atomic<size_t> writeIdx_{0};
    alignas(CACHELINE_SIZE) size_t readIdxCached_{0};

    SPSCQueue(size_t capacity) { data_.resize(capacity); }

    bool push(T&& val) {
        auto const writeIdx = writeIdx_.load(std::memory_order_relaxed); // free to order anywhere
        auto       nextWriteIdx = writeIdx + 1;
        if (nextWriteIdx == data_.size()) {
            nextWriteIdx = 0;
        }
        if (nextWriteIdx == readIdxCached_) {                          // full
            readIdxCached_ = readIdx_.load(std::memory_order_acquire); // lock
            if (nextWriteIdx == readIdxCached_) {
                return false;
            }
        }
        data_[writeIdx] = val;
        writeIdx_.store(nextWriteIdx, std::memory_order_release);
        return true;
    }

    T pop() {
        auto const readIdx = readIdx_.load(std::memory_order_relaxed);
        if (readIdx == writeIdxCached_) {
            writeIdxCached_ = writeIdx_.load(std::memory_order_acquire);
            if (readIdx == writeIdxCached_) {
                return false;
            }
        }
        double val         = data_[readIdx];
        auto   nextReadIdx = readIdx + 1;
        if (nextReadIdx == data_.size()) {
            nextReadIdx = 0;
        }
        readIdx_.store(nextReadIdx, std::memory_order_release); // unlock
        return val;
    }
};



