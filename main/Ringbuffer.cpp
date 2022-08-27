#include "Ringbuffer.hpp"


SPSCQueue::SPSCQueue(size_t capacity) : data_(capacity, 0) {}

bool SPSCQueue::push(int val) { 
  auto const writeIdx = writeIdx_.load(std::memory_order_relaxed);
  auto nextWriteIdx = writeIdx + 1;
  if (nextWriteIdx == data_.size()) {
    nextWriteIdx = 0;
  }
  if (nextWriteIdx == readIdxCached_) { // full
    readIdxCached_ = readIdx_.load(std::memory_order_acquire);
    if (nextWriteIdx == readIdxCached_) {
      return false;
    }
  }
  data_[writeIdx] = val;
  writeIdx_.store(nextWriteIdx, std::memory_order_release);
  return true;
}

bool SPSCQueue::pop(int &val) {
  auto const readIdx = readIdx_.load(std::memory_order_relaxed);
  if (readIdx == writeIdxCached_) {
    writeIdxCached_ = writeIdx_.load(std::memory_order_acquire);
    if (readIdx == writeIdxCached_) {
      return false;
    }
  }
  val = data_[readIdx];
  auto nextReadIdx = readIdx + 1;
  if (nextReadIdx == data_.size()) {
    nextReadIdx = 0;
  }
  readIdx_.store(nextReadIdx, std::memory_order_release); // unlock
  return true;
}
