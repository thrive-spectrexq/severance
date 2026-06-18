#pragma once

#include <string>

namespace severance::plugins {

// A proxy interface provided by the core engine to plugins.
// Plugins use this to interact with the host (logging, events, etc.)
class IPluginAPI {
public:
  virtual ~IPluginAPI() = default;

  virtual void LogInfo(const char* message) = 0;
  virtual void LogWarning(const char* message) = 0;
  virtual void LogError(const char* message) = 0;

  // Future expansion:
  // virtual void SubscribeToEvent(EventType type, EventCallback cb) = 0;
  // virtual void PublishEvent(const Event& event) = 0;
};

} // namespace severance::plugins
