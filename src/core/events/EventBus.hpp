#pragma once

#include "Event.hpp"
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace severance::core::events {

using EventCallback = std::function<void(std::shared_ptr<Event>)>;

class EventBus {
public:
  static EventBus &GetInstance() {
    static EventBus instance;
    return instance;
  }

  void Subscribe(EventType type, EventCallback callback);
  void Publish(std::shared_ptr<Event> event);

  EventBus(const EventBus &) = delete;
  EventBus &operator=(const EventBus &) = delete;

private:
  EventBus() = default;
  ~EventBus() = default;

  std::unordered_map<EventType, std::vector<EventCallback>> m_Subscribers;
  std::mutex m_Mutex;
};

} // namespace severance::core::events
