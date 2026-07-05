#pragma once

#include "Event.hpp"
#include <string>

namespace severance::core::events {

class MacrodataRefinedEvent : public Event {
public:
  MacrodataRefinedEvent(const std::string& binName, int count)
      : m_BinName(binName), m_Count(count) {}

  EventType GetType() const override { return EventType::MacrodataRefined; }
  std::string GetName() const override { return "MacrodataRefinedEvent"; }

  std::string GetPayload() const override {
    return std::string("{\"bin\": \"") + m_BinName + "\", \"count\": " + std::to_string(m_Count) + "}";
  }

  std::string GetBinName() const { return m_BinName; }
  int GetCount() const { return m_Count; }

private:
  std::string m_BinName;
  int m_Count;
};

} // namespace severance::core::events
