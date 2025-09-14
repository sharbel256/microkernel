#include <kernel/PluginInterface.hpp>
#include <iostream>
#include <dlfcn.h>
#include <memory>
#include <vector>
#include <functional>

class PluginManager {
public:
    ~PluginManager() {
        // Clean up all plugins
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
        void* handle = dlopen(path.c_str(), RTLD_LAZY);
        if (!handle) {
            std::cerr << "Cannot load plugin: " << dlerror() << std::endl;
            return;
        }

        // Clear any existing error
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

        // Create the plugin
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
    
    // Load plugins
    manager.loadPlugin("/Users/sharbel/code/microkernel/build/plugins/plugin1/libplugin1.dylib");
    manager.loadPlugin("/Users/sharbel/code/microkernel/build/plugins/plugin2/libplugin2.dylib");
    
    manager.runAll();
    return 0;
}