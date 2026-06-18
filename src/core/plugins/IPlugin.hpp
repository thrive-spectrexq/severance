#pragma once

#include <string>
#include <memory>
#include "severance/plugins/IPluginAPI.hpp"

// Standard export macro for Windows plugins
#if defined(_WIN32)
  #define SEVERANCE_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
  #define SEVERANCE_PLUGIN_EXPORT extern "C" __attribute__((visibility("default")))
#endif

namespace severance::core::plugins {

class IPlugin {
public:
  virtual ~IPlugin() = default;

  virtual std::string GetName() const = 0;
  virtual std::string GetVersion() const = 0;

  // Initialize now receives the API proxy from the host
  virtual bool Initialize(::severance::plugins::IPluginAPI* api) = 0;
  virtual void Shutdown() = 0;
};

} // namespace severance::core::plugins

// The standard factory function that every plugin DLL must export
// Signature: IPlugin* CreatePlugin()
typedef severance::core::plugins::IPlugin* (*CreatePluginFunc)();

