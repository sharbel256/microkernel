#pragma once

#include <dlfcn.h>
#include <pthread.h>
#include <sys/event.h>
#include <sys/types.h>

#include <atomic>
#include <filesystem>
#include <kernel/model.hpp>
#include <memory_resource>
#include <span>
#include <thread>
#include <unordered_map>
#include <vector>

using namespace trading;

class Kernel {
 public:
  void init() {
    auto cores = std::thread::hardware_concurrency();
    thread_pool.reserve(cores);
    for (size_t i = 0; i < cores; ++i) {
      thread_pool.emplace_back([this, i] {
        set_affinity(i);
        run_loop();
      });
    }

    for (auto& entry : std::filesystem::directory_iterator("plugins")) {
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
            for (auto& handler : it->second) {
              handler(msg);
            }
          }
        }
      }
      std::this_thread::yield();  // avoid busy-wait
    }
  }

  void load_plugin(const std::filesystem::path& path) {
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) return;
    using RegisterFn = void (*)(Kernel&);
    auto reg = (RegisterFn)dlsym(handle, "register_plugin");
    if (reg) reg(*this);
  }

  void register_plugin(std::unique_ptr<trading::Plugin> plugin,
                       std::vector<uint32_t> message_types) {
    auto id = next_plugin_id++;
    plugins[id] = std::move(plugin);
    in_buffers.emplace_back();
    for (auto type : message_types) {
      subscribers[type].push_back(
          [id, this](const trading::Message& msg) { plugins[id]->handle_message(msg); });
    }
    plugins[id]->init();
    plugins[id]->start();
  }

  bool post_message(uint64_t sender_id, Message&& msg) {
    return in_buffers[sender_id - 1].push(std::move(msg));
  }

 private:
  void set_affinity(size_t core) {
    pthread_t tid = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);
    pthread_setaffinity_np(tid, sizeof(cpu_set_t), &cpuset);
  }

  std::pmr::monotonic_buffer_resource arena{1024 * 1024 * 1024};  // 1GB
  std::vector<std::jthread> thread_pool;
  std::unordered_map<uint64_t, std::unique_ptr<trading::Plugin>> plugins;
  std::unordered_map<uint32_t, std::vector<trading::Plugin::Handler>> subscribers;
  std::vector<trading::RingBuffer<1'000'000>> in_buffers;  // one per plugin
  std::atomic<bool> running{true};
  uint64_t next_plugin_id{1};
};