#pragma once

#include <iostream>
#include <kernel/model.hpp>

template <size_t Size>
class RingBuffer {
   public:
    RingBuffer(size_t size, std::pmr::memory_resource* resource)
        : buffer(resource), head(0), tail(0) {
        buffer.resize(size);
    }

  explicit RingBuffer(std::pmr::memory_resource* resource = std::pmr::get_default_resource())
      : RingBuffer(Size, resource) {}

  RingBuffer(const RingBuffer&) = delete;
  RingBuffer& operator=(const RingBuffer&) = delete;

  RingBuffer(RingBuffer&& other) noexcept
      : buffer(std::move(other.buffer)),
        head(other.head.load(std::memory_order_relaxed)),
        tail(other.tail.load(std::memory_order_relaxed)) {
    other.head.store(0, std::memory_order_relaxed);
    other.tail.store(0, std::memory_order_relaxed);
  }

  RingBuffer& operator=(RingBuffer&& other) noexcept {
    if (this != &other) {
      buffer = std::move(other.buffer);
      head.store(other.head.load(std::memory_order_relaxed), std::memory_order_relaxed);
      tail.store(other.tail.load(std::memory_order_relaxed), std::memory_order_relaxed);
      other.head.store(0, std::memory_order_relaxed);
      other.tail.store(0, std::memory_order_relaxed);
    }
    return *this;
  }

  bool push(trading::Message&& msg) {
    size_t current_tail = tail.load(std::memory_order_relaxed);
    size_t next_tail = (current_tail + 1) % buffer.size();
    if (next_tail == head.load(std::memory_order_acquire)) {
      return false;
    }
    buffer[current_tail] = std::move(msg);
    tail.store(next_tail, std::memory_order_release);
    return true;
  }

  bool pop(trading::Message& msg) {
    size_t current_head = head.load(std::memory_order_relaxed);
    if (current_head == tail.load(std::memory_order_acquire)) {
      return false;
    }
    msg = std::move(buffer[current_head]);
    head.store((current_head + 1) % buffer.size(), std::memory_order_release);
    return true;
  }

 private:
  std::pmr::vector<trading::Message> buffer;
  std::atomic<size_t> head;
  std::atomic<size_t> tail;
};