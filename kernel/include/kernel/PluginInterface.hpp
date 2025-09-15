#pragma once

#include <string>
#include <memory>
#include <functional>

class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual std::string getName() const = 0;
    virtual void execute() = 0;
};

using PluginCreateFunc = IPlugin*(*)();
using PluginDestroyFunc = void(*)(IPlugin*);

#define EXPORT_PLUGIN(PluginClass) \
    extern "C" { \
        IPlugin* createPlugin() { \
            return new PluginClass(); \
        } \
        void destroyPlugin(IPlugin* plugin) { \
            delete plugin; \
        } \
    }