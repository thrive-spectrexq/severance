#pragma once

#include "IPlugin.hpp"
#include <memory>
#include <vector>

namespace severance::core::plugins {

class PluginManager {
public:
  PluginManager() = default;
  ~PluginManager();

  PluginManager(const PluginManager &) = delete;
  PluginManager &operator=(const PluginManager &) = delete;

  void LoadPlugins(const std::string &directory);
  void UnloadAll();

  std::vector<std::shared_ptr<IPlugin>> GetActivePlugins() const;

private:
  std::vector<std::shared_ptr<IPlugin>> m_Plugins;
};

} // namespace severance::core::plugins
