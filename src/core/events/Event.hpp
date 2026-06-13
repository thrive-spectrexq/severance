#pragma once

#include "EventTypes.hpp"
#include <string>

namespace severance::core::events {

class Event {
public:
  virtual ~Event() = default;

  virtual EventType GetType() const = 0;
  virtual std::string GetName() const = 0;

  bool handled = false;
};

} // namespace severance::core::events
