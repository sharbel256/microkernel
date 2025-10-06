#pragma once

#include <dlfcn.h>
#include <sys/event.h>
#include <sys/types.h>
#include <atomic>
#include <filesystem>
#include <functional>
#include <memory_resource>
#include <span>
#include <thread>
#include <unordered_map>
#include <vector>
#include <kernel/model.hpp>
#include <kernel/ring_buffer.hpp>

namespace trading {

class Kernel {
 public:
  Kernel() : plugins(&arena), subscribers(&arena), in_buffers(&arena) {}

  void init() {
    auto cores = std::thread::hardware_concurrency();
    thread_pool.reserve(cores);
    for (size_t i = 0; i < cores; ++i) {
      thread_pool.emplace_back([this] { run_loop(); });
    }

    for (const auto& entry : std::filesystem::directory_iterator("plugins")) {
      if (entry.path().extension() == ".dylib") {
        load_plugin(entry.path());
      }
    }
  }

  void run_loop() {
    trading::Message msg;
    while (running.load(std::memory_order_relaxed)) {
      for (auto& buf : in_buffers) {
        while (buf.pop(msg)) {
          auto it = subscribers.find(msg.type);
          if (it != subscribers.end()) {
            for (const auto& handler : it->second) {
              handler(msg);
            }
          }
        }
      }
      std::this_thread::yield(); // Avoid busy-wait
    }
  }

  void load_plugin(const std::filesystem::path& path) {
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
      throw std::runtime_error("Failed to load plugin: " + std::string(dlerror()));
    }
    using RegisterFn = void (*)(Kernel&);
    auto reg = reinterpret_cast<RegisterFn>(dlsym(handle, "register_plugin"));
    if (!reg) {
      dlclose(handle);
      throw std::runtime_error("Failed to find register_plugin in: " + path.string());
    }
    reg(*this);
    // Note: handle is not closed to keep plugin loaded
  }

  void register_plugin(std::unique_ptr<trading::Plugin> plugin,
                       std::vector<uint32_t> message_types) {
    auto id = next_plugin_id++;
    plugins[id] = std::move(plugin);
    in_buffers.emplace_back(1'000'000, &arena); // Pass size and allocator
    for (auto type : message_types) {
      subscribers[type].push_back(
          [id, this](const trading::Message& msg) { plugins[id]->handle_message(msg); });
    }
    plugins[id]->init();
    plugins[id]->start();
  }

  bool post_message(uint64_t sender_id, trading::Message&& msg) {
    if (sender_id == 0 || sender_id > in_buffers.size()) {
      return false;
    }
    return in_buffers[sender_id - 1].push(std::move(msg));
  }

 private:
  std::pmr::monotonic_buffer_resource arena{1024 * 1024 * 1024}; // 1GB
  std::vector<std::jthread> thread_pool;
  std::pmr::unordered_map<uint64_t, std::unique_ptr<trading::Plugin>> plugins;
  std::pmr::unordered_map<uint32_t, std::pmr::vector<std::function<void(const trading::Message&)>>> subscribers;
  std::pmr::vector<RingBuffer<1'000'000>> in_buffers;
  std::atomic<bool> running{true};
  uint64_t next_plugin_id{1};
};

} // namespace trading