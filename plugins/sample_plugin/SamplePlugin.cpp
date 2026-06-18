#include "core/plugins/IPlugin.hpp"

using namespace severance::core::plugins;
using namespace severance::plugins;

class SamplePlugin : public IPlugin {
public:
  std::string GetName() const override {
    return "Sample Network Observer Plugin";
  }

  std::string GetVersion() const override {
    return "1.0.0";
  }

  bool Initialize(IPluginAPI* api) override {
    m_Api = api;
    if (m_Api) {
      m_Api->LogInfo("Sample Plugin has been successfully initialized!");
    }
    return true;
  }

  void Shutdown() override {
    if (m_Api) {
      m_Api->LogInfo("Sample Plugin shutting down.");
    }
  }

private:
  IPluginAPI* m_Api{nullptr};
};

// Export the factory function with C linkage
SEVERANCE_PLUGIN_EXPORT IPlugin* CreatePlugin() {
  return new SamplePlugin();
}
