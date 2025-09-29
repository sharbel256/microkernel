#include <iostream>
#include <kernel/plugin_interface.hpp>

class Plugin2 : public IPlugin {
 public:
  std::string getName() const override { return "Plugin2"; }

  void execute() override { std::cout << "Hello from Plugin2!" << std::endl; }
};

EXPORT_PLUGIN(Plugin2)