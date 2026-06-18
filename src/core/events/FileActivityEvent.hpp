#pragma once

#include "Event.hpp"
#include "core/filesystem/EtwMonitor.hpp"

namespace severance::core::events {

class FileActivityEvent : public Event {
public:
  explicit FileActivityEvent(const filesystem::FileEvent& fe) : fileEvent(fe) {}

  EventType GetType() const override { return EventType::FileModified; } // Re-using FileModified
  std::string GetName() const override { return "FileActivityEvent"; }

  filesystem::FileEvent fileEvent;
};

} // namespace severance::core::events
