#include "PluginManager.hpp"
#include "logging/Logger.hpp"
#include "core/notifications/NotificationManager.hpp"
#include <filesystem>
#include <chrono>

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

  void EmitHeuristicAlert(const char* title, const char* message, int severity) override {
    notifications::Notification n;
    n.id = "plugin_heuristic";
    n.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
    n.severity = static_cast<notifications::NotificationSeverity>(severity);
    n.title = title ? title : "Plugin Alert";
    n.message = message ? message : "";
    n.source = "Plugin Engine";
    notifications::NotificationManager::GetInstance().EmitNotification(n);
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

      // ── Try C++ plugin first (CreatePlugin export) ──
      auto createFunc = reinterpret_cast<CreatePluginFunc>(GetProcAddress(handle, "CreatePlugin"));
      if (createFunc) {
        IPlugin* pluginInstance = createFunc();
        if (pluginInstance && pluginInstance->Initialize(m_ApiProxy.get())) {
          SEV_CORE_INFO("Loaded C++ plugin: {} v{}", pluginInstance->GetName(), pluginInstance->GetVersion());
          m_LoadedPlugins.push_back({pluginInstance, false, handle});
        } else {
          SEV_CORE_ERROR("C++ plugin failed to initialize: {}", pathStr);
          if (pluginInstance) delete pluginInstance;
          FreeLibrary(handle);
        }
        continue;
      }

      // ── Try C plugin (sev_plugin_create export) ──
      auto cCreateFn   = reinterpret_cast<SevPluginCreateFunc>(GetProcAddress(handle, "sev_plugin_create"));
      auto cDestroyFn  = reinterpret_cast<SevPluginDestroyFunc>(GetProcAddress(handle, "sev_plugin_destroy"));
      auto cGetInfoFn  = reinterpret_cast<SevPluginGetInfoFunc>(GetProcAddress(handle, "sev_plugin_get_info"));
      auto cInitFn     = reinterpret_cast<SevPluginInitializeFunc>(GetProcAddress(handle, "sev_plugin_initialize"));
      auto cShutdownFn = reinterpret_cast<SevPluginShutdownFunc>(GetProcAddress(handle, "sev_plugin_shutdown"));

      if (cCreateFn && cDestroyFn && cGetInfoFn && cInitFn && cShutdownFn) {
        // Wrap the C plugin in the bridge adapter
        auto* bridge = new CPluginBridge(cCreateFn, cDestroyFn, cGetInfoFn, cInitFn, cShutdownFn);
        if (bridge->Initialize(m_ApiProxy.get())) {
          SEV_CORE_INFO("Loaded C plugin: {} v{}", bridge->GetName(), bridge->GetVersion());
          m_LoadedPlugins.push_back({bridge, true, handle});
        } else {
          SEV_CORE_ERROR("C plugin failed to initialize: {}", pathStr);
          delete bridge;
          FreeLibrary(handle);
        }
      } else {
        SEV_CORE_WARN("DLL has no recognized plugin exports: {}", pathStr);
        FreeLibrary(handle);
      }

#else
      void* handle = dlopen(pathStr.c_str(), RTLD_NOW);
      if (!handle) {
        SEV_CORE_ERROR("Failed to load plugin SO: {}", pathStr);
        continue;
      }

      // ── Try C++ plugin first ──
      auto createFunc = reinterpret_cast<CreatePluginFunc>(dlsym(handle, "CreatePlugin"));
      if (createFunc) {
        IPlugin* pluginInstance = createFunc();
        if (pluginInstance && pluginInstance->Initialize(m_ApiProxy.get())) {
          SEV_CORE_INFO("Loaded C++ plugin: {} v{}", pluginInstance->GetName(), pluginInstance->GetVersion());
          m_LoadedPlugins.push_back({pluginInstance, false, handle});
        } else {
          if (pluginInstance) delete pluginInstance;
          dlclose(handle);
        }
        continue;
      }

      // ── Try C plugin ──
      auto cCreateFn   = reinterpret_cast<SevPluginCreateFunc>(dlsym(handle, "sev_plugin_create"));
      auto cDestroyFn  = reinterpret_cast<SevPluginDestroyFunc>(dlsym(handle, "sev_plugin_destroy"));
      auto cGetInfoFn  = reinterpret_cast<SevPluginGetInfoFunc>(dlsym(handle, "sev_plugin_get_info"));
      auto cInitFn     = reinterpret_cast<SevPluginInitializeFunc>(dlsym(handle, "sev_plugin_initialize"));
      auto cShutdownFn = reinterpret_cast<SevPluginShutdownFunc>(dlsym(handle, "sev_plugin_shutdown"));

      if (cCreateFn && cDestroyFn && cGetInfoFn && cInitFn && cShutdownFn) {
        auto* bridge = new CPluginBridge(cCreateFn, cDestroyFn, cGetInfoFn, cInitFn, cShutdownFn);
        if (bridge->Initialize(m_ApiProxy.get())) {
          SEV_CORE_INFO("Loaded C plugin: {} v{}", bridge->GetName(), bridge->GetVersion());
          m_LoadedPlugins.push_back({bridge, true, handle});
        } else {
          delete bridge;
          dlclose(handle);
        }
      } else {
        dlclose(handle);
      }
#endif
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
