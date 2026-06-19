#pragma once

#include "IPlugin.hpp"
#include "CPluginBridge.hpp"
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace severance::core::plugins {

class PluginManager {
public:
  PluginManager();
  ~PluginManager();

  void LoadPlugins(const std::string &directory);
  void UnloadAll();

  std::vector<IPlugin*> GetActivePlugins() const;

private:
  struct PluginContext {
    IPlugin* plugin{nullptr};
    bool     ownedByCBridge{false}; // true if wrapped in CPluginBridge
#if defined(_WIN32)
    HMODULE handle{nullptr};
#else
    void* handle{nullptr};
#endif
  };

  std::vector<PluginContext> m_LoadedPlugins;
  std::unique_ptr<::severance::plugins::IPluginAPI> m_ApiProxy;
};

} // namespace severance::core::plugins
