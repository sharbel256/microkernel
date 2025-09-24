#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

class ConfigManager {
 public:
  using ConfigChangedCallback = std::function<void(const nlohmann::json&)>;

  ConfigManager();
  ~ConfigManager();

  bool loadConfig(const std::string& configPath);

  template <typename T>
  T get(const std::string& key, const T& defaultValue) const {
    std::cout << "key: " << key << std::endl;
    if (config_.contains(key)) {
      std::cout << "key exists" << std::endl;
      try {
        return config_.at(key).get<T>();
      } catch (const std::exception& e) {
        std::cerr << "Type mismatch for key '" << key << "': " << e.what() << "\n";
        return defaultValue;
      }
    }
    return defaultValue;
  }

  void watchForChanges(const std::string& key, ConfigChangedCallback callback);
  void checkForUpdates();
  void updateConfig(const nlohmann::json& newConfig);
  std::vector<std::filesystem::path> getPluginFilesFromDirectory(const std::string& directoryPath);
  nlohmann::json getJson();

 private:
  void setupFileWatcher();
  void notifySubscribers(const std::string& key, const nlohmann::json& value);

  std::filesystem::path configPath_;
  nlohmann::json config_;
  std::unordered_map<std::string, std::vector<ConfigChangedCallback>> subscribers_;
  std::filesystem::file_time_type lastModified_;
  bool watching_ = false;
};