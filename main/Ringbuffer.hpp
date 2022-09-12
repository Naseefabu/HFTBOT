#pragma once

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

static constexpr int CACHELINE_SIZE = 64;

// Single Producer Single Consumer Ringbuffer Queue

struct SPSCQueue {
  std::vector<int> data_{};
  alignas(CACHELINE_SIZE) std::atomic<size_t> readIdx_{0};
  alignas(CACHELINE_SIZE) size_t writeIdxCached_{0};
  alignas(CACHELINE_SIZE) std::atomic<size_t> writeIdx_{0};
  alignas(CACHELINE_SIZE) size_t readIdxCached_{0};

  SPSCQueue(size_t capacity);

  bool push(int val);

  bool pop(int &val);
};


