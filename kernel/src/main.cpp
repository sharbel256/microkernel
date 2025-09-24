#include <dlfcn.h>

#include <functional>
#include <iostream>
#include <kernel/ConfigManager.hpp>
#include <kernel/PluginInterface.hpp>
#include <memory>
#include <vector>

class PluginManager {
 public:
  ~PluginManager() {
    for (auto& pluginInfo : plugins) {
      if (pluginInfo.destroy && pluginInfo.plugin) {
        pluginInfo.destroy(pluginInfo.plugin);
      }
      if (pluginInfo.handle) {
        dlclose(pluginInfo.handle);
      }
    }
  }

  void loadPlugin(const std::string& path) {
    void* handle = dlopen(path.c_str(), RTLD_NOW);
    if (!handle) {
      std::cerr << "Cannot load plugin: " << dlerror() << std::endl;
      return;
    }

    dlerror();

    auto create = reinterpret_cast<PluginCreateFunc>(dlsym(handle, "createPlugin"));
    const char* dlsym_error = dlerror();
    if (dlsym_error) {
      std::cerr << "Cannot load symbol createPlugin: " << dlsym_error << std::endl;
      dlclose(handle);
      return;
    }

    auto destroy = reinterpret_cast<PluginDestroyFunc>(dlsym(handle, "destroyPlugin"));
    dlsym_error = dlerror();
    if (dlsym_error) {
      std::cerr << "Cannot load symbol destroyPlugin: " << dlsym_error << std::endl;
      dlclose(handle);
      return;
    }

    // create the plugin
    IPlugin* plugin = create();
    if (!plugin) {
      std::cerr << "Failed to create plugin instance" << std::endl;
      dlclose(handle);
      return;
    }

    plugins.push_back({handle, destroy, plugin});
  }

  void runAll() {
    for (auto& pluginInfo : plugins) {
      std::cout << "Running plugin: " << pluginInfo.plugin->getName() << std::endl;
      pluginInfo.plugin->execute();
    }
  }

 private:
  struct PluginInfo {
    void* handle = nullptr;
    PluginDestroyFunc destroy = nullptr;
    IPlugin* plugin = nullptr;
  };

  std::vector<PluginInfo> plugins;
};

int main() {
  PluginManager manager;
  ConfigManager config;
  std::string config_path = "";

  try {
    config_path =
        std::getenv("MICROKERNEL_CONFIG_PATH") ? std::getenv("MICROKERNEL_CONFIG_PATH") : "";
  } catch (...) {
    std::cout << "Failed to get config path variable" << std::endl;
    return 0;
  }

  if (!config.loadConfig(config_path)) {
    std::cerr << "Failed to load configuration" << std::endl;
    return 1;
  }

  nlohmann::json json = config.getJson();

  config.watchForChanges("logging.level", [](const auto& value) {
    std::cout << "logging changed to: " << value << std::endl;
  });

  bool autoload = json["plugins"]["autoload"];
  std::vector<std::filesystem::path> plugin_files;

  if (autoload) {
    std::string plugin_dir = json["plugins"]["directory"];
    plugin_files = config.getPluginFilesFromDirectory(plugin_dir);
  }

  for (std::filesystem::path file : plugin_files) {
    manager.loadPlugin(file);
  }

  manager.runAll();
  return 0;
}