#include "PluginManager.hpp"
#include "logging/Logger.hpp"
#include <filesystem>

namespace severance::core::plugins {

// Internal implementation of IPluginAPI passed to plugins
class HostAPIProxy : public ::severance::plugins::IPluginAPI {
public:
  void LogInfo(const char* message) override {
    SEV_CORE_INFO("[Plugin] {}", message);
  }
  void LogWarning(const char* message) override {
    SEV_CORE_WARN("[Plugin] {}", message);
  }
  void LogError(const char* message) override {
    SEV_CORE_ERROR("[Plugin] {}", message);
  }
};

PluginManager::PluginManager() {
  m_ApiProxy = std::make_unique<HostAPIProxy>();
}

PluginManager::~PluginManager() { UnloadAll(); }

void PluginManager::LoadPlugins(const std::string &directory) {
  SEV_CORE_INFO("Scanning directory for plugins: {}", directory);
  if (!std::filesystem::exists(directory)) {
    return;
  }

  for (const auto& entry : std::filesystem::directory_iterator(directory)) {
    if (entry.path().extension() == ".dll" || entry.path().extension() == ".so") {
      std::string pathStr = entry.path().string();
      
#if defined(_WIN32)
      HMODULE handle = LoadLibraryA(pathStr.c_str());
      if (!handle) {
        SEV_CORE_ERROR("Failed to load plugin DLL: {}", pathStr);
        continue;
      }

      auto createFunc = reinterpret_cast<CreatePluginFunc>(GetProcAddress(handle, "CreatePlugin"));
      if (!createFunc) {
        SEV_CORE_ERROR("Plugin missing CreatePlugin export: {}", pathStr);
        FreeLibrary(handle);
        continue;
      }
#else
      void* handle = dlopen(pathStr.c_str(), RTLD_NOW);
      if (!handle) {
        SEV_CORE_ERROR("Failed to load plugin SO: {}", pathStr);
        continue;
      }
      auto createFunc = reinterpret_cast<CreatePluginFunc>(dlsym(handle, "CreatePlugin"));
      if (!createFunc) {
        dlclose(handle);
        continue;
      }
#endif

      IPlugin* pluginInstance = createFunc();
      if (pluginInstance && pluginInstance->Initialize(m_ApiProxy.get())) {
        SEV_CORE_INFO("Successfully loaded plugin: {} v{}", pluginInstance->GetName(), pluginInstance->GetVersion());
        m_LoadedPlugins.push_back({pluginInstance, handle});
      } else {
        SEV_CORE_ERROR("Plugin failed to initialize: {}", pathStr);
        if (pluginInstance) {
          delete pluginInstance;
        }
#if defined(_WIN32)
        FreeLibrary(handle);
#else
        dlclose(handle);
#endif
      }
    }
  }
}

void PluginManager::UnloadAll() {
  for (auto &ctx : m_LoadedPlugins) {
    if (ctx.plugin) {
      SEV_CORE_INFO("Shutting down plugin: {}", ctx.plugin->GetName());
      ctx.plugin->Shutdown();
      delete ctx.plugin;
    }
#if defined(_WIN32)
    if (ctx.handle) FreeLibrary(ctx.handle);
#else
    if (ctx.handle) dlclose(ctx.handle);
#endif
  }
  m_LoadedPlugins.clear();
}

std::vector<IPlugin*> PluginManager::GetActivePlugins() const {
  std::vector<IPlugin*> active;
  for (const auto& ctx : m_LoadedPlugins) {
    active.push_back(ctx.plugin);
  }
  return active;
}

} // namespace severance::core::plugins
