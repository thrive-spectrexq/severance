#pragma once

#include "IPlugin.hpp"
#include "severance/plugins/severance_plugin_api.h"
#include <string>
#include <memory>

namespace severance::core::plugins {

/**
 * @brief Adapter that wraps a C-ABI plugin and presents the IPlugin C++ interface.
 *
 * When the PluginManager detects a DLL exporting C-ABI symbols
 * (sev_plugin_create, etc.), it wraps the raw function pointers
 * in a CPluginBridge. From that point on, the plugin is managed
 * identically to a C++ plugin — same lifecycle, same logging,
 * same container.
 */
class CPluginBridge : public IPlugin {
public:
  CPluginBridge(SevPluginCreateFunc    createFn,
                SevPluginDestroyFunc   destroyFn,
                SevPluginGetInfoFunc   getInfoFn,
                SevPluginInitializeFunc initFn,
                SevPluginShutdownFunc  shutdownFn)
      : m_CreateFn(createFn),
        m_DestroyFn(destroyFn),
        m_GetInfoFn(getInfoFn),
        m_InitializeFn(initFn),
        m_ShutdownFn(shutdownFn) {}

  ~CPluginBridge() override {
    if (m_Handle && m_DestroyFn) {
      m_DestroyFn(m_Handle);
      m_Handle = nullptr;
    }
  }

  // Non-copyable, non-movable
  CPluginBridge(const CPluginBridge&) = delete;
  CPluginBridge& operator=(const CPluginBridge&) = delete;

  std::string GetName() const override {
    if (m_Handle && m_GetInfoFn) {
      SevPluginInfo info = m_GetInfoFn(m_Handle);
      return info.name ? info.name : "Unknown C Plugin";
    }
    return "Unknown C Plugin";
  }

  std::string GetVersion() const override {
    if (m_Handle && m_GetInfoFn) {
      SevPluginInfo info = m_GetInfoFn(m_Handle);
      return info.version ? info.version : "0.0.0";
    }
    return "0.0.0";
  }

  bool Initialize(::severance::plugins::IPluginAPI* api) override {
    // Create the plugin instance
    if (m_CreateFn) {
      m_Handle = m_CreateFn();
    }
    if (!m_Handle) {
      return false;
    }

    // Build the C API table, routing to the C++ IPluginAPI
    m_HostApi.api_version_major = SEV_PLUGIN_API_VERSION_MAJOR;
    m_HostApi.api_version_minor = SEV_PLUGIN_API_VERSION_MINOR;

    // Store the C++ API pointer for the static trampolines
    s_CurrentAPI = api;

    m_HostApi.log_info    = &CPluginBridge::LogInfoTrampoline;
    m_HostApi.log_warning = &CPluginBridge::LogWarningTrampoline;
    m_HostApi.log_error   = &CPluginBridge::LogErrorTrampoline;
    m_HostApi.emit_heuristic_alert = &CPluginBridge::EmitHeuristicAlertTrampoline;
    m_HostApi._reserved1  = nullptr;
    m_HostApi._reserved2  = nullptr;
    m_HostApi._reserved3  = nullptr;

    // Keep a reference to the C++ API for this bridge instance
    m_CppApi = api;

    // Initialize the C plugin
    if (m_InitializeFn) {
      int result = m_InitializeFn(m_Handle, &m_HostApi);
      return result != 0;
    }
    return false;
  }

  void Shutdown() override {
    if (m_Handle && m_ShutdownFn) {
      m_ShutdownFn(m_Handle);
    }
  }

private:
  SevPluginHandle*        m_Handle{nullptr};
  SevPluginAPI            m_HostApi{};
  ::severance::plugins::IPluginAPI* m_CppApi{nullptr};

  // C ABI function pointers
  SevPluginCreateFunc     m_CreateFn{nullptr};
  SevPluginDestroyFunc    m_DestroyFn{nullptr};
  SevPluginGetInfoFunc    m_GetInfoFn{nullptr};
  SevPluginInitializeFunc m_InitializeFn{nullptr};
  SevPluginShutdownFunc   m_ShutdownFn{nullptr};

  // Thread-local API pointer for C callbacks
  // This is safe because plugin init is serialized on a single thread
  static inline ::severance::plugins::IPluginAPI* s_CurrentAPI{nullptr};

  static void LogInfoTrampoline(const char* message) {
    if (s_CurrentAPI) s_CurrentAPI->LogInfo(message);
  }
  static void LogWarningTrampoline(const char* message) {
    if (s_CurrentAPI) s_CurrentAPI->LogWarning(message);
  }
  static void LogErrorTrampoline(const char* message) {
    if (s_CurrentAPI) s_CurrentAPI->LogError(message);
  }
  static void EmitHeuristicAlertTrampoline(const char* title, const char* message, int severity) {
    if (s_CurrentAPI) s_CurrentAPI->EmitHeuristicAlert(title, message, severity);
  }
};

} // namespace severance::core::plugins
