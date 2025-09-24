#include <iostream>
#include <kernel/PluginInterface.hpp>

class Plugin1 : public IPlugin {
 public:
  std::string getName() const override { return "Plugin1"; }

  void execute() override { std::cout << "Hello from Plugin1!" << std::endl; }
};

EXPORT_PLUGIN(Plugin1)