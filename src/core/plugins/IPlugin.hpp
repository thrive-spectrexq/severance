#pragma once

#include <string>

namespace severance::core::plugins {

class IPlugin {
public:
  virtual ~IPlugin() = default;

  virtual std::string GetName() const = 0;
  virtual std::string GetVersion() const = 0;

  virtual bool Initialize() = 0;
  virtual void Shutdown() = 0;
};

} // namespace severance::core::plugins
