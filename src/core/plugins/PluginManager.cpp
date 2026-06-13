#include "PluginManager.hpp"
#include "logging/Logger.hpp"

namespace severance::core::plugins {

PluginManager::~PluginManager() { UnloadAll(); }

void PluginManager::LoadPlugins(const std::string &directory) {
  // In a real implementation, we would iterate directory, find .dll/.so,
  // dlopen them, and instantiate plugins.
  SEV_CORE_INFO("Scanning directory for plugins: {}", directory);
}

void PluginManager::UnloadAll() {
  for (auto &plugin : m_Plugins) {
    SEV_CORE_INFO("Shutting down plugin: {}", plugin->GetName());
    plugin->Shutdown();
  }
  m_Plugins.clear();
}

std::vector<std::shared_ptr<IPlugin>> PluginManager::GetActivePlugins() const {
  return m_Plugins;
}

} // namespace severance::core::plugins
