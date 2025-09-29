#pragma once

// lock-free SPSC ring buffer
template <size_t SIZE>
class RingBuffer {
  std::array<Message, SIZE> buffer;
  std::atomic<size_t> head{0}, tail{0};

 public:
  bool push(Message&& msg) {
    auto h = head.load(std::memory_order_relaxed);
    if (h - tail.load(std::memory_order_acquire) >= SIZE) return false;  // Full
    buffer[h % SIZE] = std::move(msg);
    head.store(h + 1, std::memory_order_release);
    return true;
  }
  bool pop(Message& msg) {
    auto t = tail.load(std::memory_order_relaxed);
    if (t == head.load(std::memory_order_acquire)) return false;  // Empty
    msg = buffer[t % SIZE];
    tail.store(t + 1, std::memory_order_release);
    return true;
  }
};