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
        auto const writeIdx = writeIdx_.load(std::memory_order_relaxed); // memory order here is relaxed because no other threads are modifying this variable except the current thread its on, so latest value will always be in push() thread core cache or memory so that its always update to date like in normal single threaded application will be.
        auto nextWriteIdx = writeIdx + 1;
        if (nextWriteIdx == data_.size()) {
            nextWriteIdx = 0;
        }
        if (nextWriteIdx == readIdxCached_) {                          // full
            readIdxCached_ = readIdx_.load(std::memory_order_acquire); // lock this is acquire because it ensure that it sees latest value of readIdx written by pop thread
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
            writeIdxCached_ = writeIdx_.load(std::memory_order_acquire); // this is acquire because it ensures that it sees the latest value of writeIdx_ written by push thread. because in multicore system it can be possible that latest value of writeidx_ is not flushed to memory from pop() thread cache so that means push() thread cannot see latest value, but std::memory_order_acquire ensures it is flushed to memory and updated in other thread as well
            if (readIdx == writeIdxCached_) { // latest value for writeIdxCached_ should be seen otherwise it is possible that the compiler and hardware could reorder memory accesses in a way that makes the latest value of writeIdx_ written by the other thread not visible to the current thread. This could result in the code not being able to correctly determine whether the queue is full or empty, which could lead to incorrect behavior.
                return false;
            }
        }
        double val = data_[readIdx];
        auto nextReadIdx = readIdx + 1;
        if (nextReadIdx == data_.size()) {
            nextReadIdx = 0;
        }
        readIdx_.store(nextReadIdx, std::memory_order_release); // unlock
        return val;
    }
};



