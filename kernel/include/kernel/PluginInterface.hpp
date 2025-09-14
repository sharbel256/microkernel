#pragma once

#include <string>
#include <memory>
#include <functional>

// Plugin interface
class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual std::string getName() const = 0;
    virtual void execute() = 0;
};

// Plugin function signatures
using PluginCreateFunc = IPlugin*(*)();
using PluginDestroyFunc = void(*)(IPlugin*);

// Plugin API macros (to be used in plugins)
#define EXPORT_PLUGIN(PluginClass) \
    extern "C" { \
        IPlugin* createPlugin() { \
            return new PluginClass(); \
        } \
        void destroyPlugin(IPlugin* plugin) { \
            delete plugin; \
        } \
    }