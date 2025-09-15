#include "kernel/ConfigManager.hpp"
#include <fstream>
#include <iostream>
#include <thread>

ConfigManager::ConfigManager() {
    // set up file watching thread
}

ConfigManager::~ConfigManager() {
    watching_ = false;
}

bool ConfigManager::loadConfig(const std::string& configPath) {
    configPath_ = configPath;
    
    try {
        std::ifstream file(configPath);
        if (!file.is_open()) {
            std::cerr << "Config file not found: " << configPath << std::endl;
            return false;
        }
        
        config_ = nlohmann::json::parse(file);
        for (auto& [key, value] : config_.items()) {
            std::cout << key << " " << value << std::endl; 
        }

        lastModified_ = std::filesystem::last_write_time(configPath_);
        
        std::cout << "Configuration loaded from: " << configPath << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
        return false;
    }
}

void ConfigManager::watchForChanges(const std::string& key, ConfigChangedCallback callback) {
    subscribers_[key].push_back(callback);
}

void ConfigManager::checkForUpdates() {
    if (!std::filesystem::exists(configPath_)) {
        return;
    }
    
    auto currentModified = std::filesystem::last_write_time(configPath_);
    if (currentModified > lastModified_) {
        try {
            std::ifstream file(configPath_);
            nlohmann::json newConfig = nlohmann::json::parse(file);
            
            for (auto& [key, value] : newConfig.items()) {
                if (config_[key] != value) {
                    notifySubscribers(key, value);
                }
            }
            
            config_ = std::move(newConfig);
            lastModified_ = currentModified;
            std::cout << "Configuration updated" << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "Error updating config: " << e.what() << std::endl;
        }
    }
}

void ConfigManager::updateConfig(const nlohmann::json& newConfig) {
    config_ = newConfig;

    for (auto& [key, value] : newConfig.items()) {
        notifySubscribers(key, value);
    }
}

void ConfigManager::notifySubscribers(const std::string& key, const nlohmann::json& value) {
    if (subscribers_.find(key) != subscribers_.end()) {
        for (auto& callback : subscribers_[key]) {
            try {
                callback(value);
            } catch (const std::exception& e) {
                std::cerr << "Error in config callback: " << e.what() << std::endl;
            }
        }
    }
}

std::vector<std::filesystem::path> ConfigManager::getPluginFilesFromDirectory(const std::string& directoryPath) {
    try {
        std::filesystem::path pluginsDir(directoryPath);
        
        if (!std::filesystem::exists(pluginsDir) || !std::filesystem::is_directory(pluginsDir)) {
            std::cerr << "Plugin directory not found or not a directory: " << directoryPath << std::endl;
            return {};
        }

        std::cout << "Scanning for plugins in: " << directoryPath << std::endl;
        std::vector<std::filesystem::path> pluginFiles;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(pluginsDir)) {
            if (std::filesystem::is_regular_file(entry) && entry.path().extension() == ".dylib") {
                pluginFiles.push_back(entry.path());
            }
        }

        return pluginFiles;

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error while loading plugins: " << e.what() << std::endl;
        return {};
    }
}

nlohmann::json ConfigManager::getJson() {
    return config_;
}