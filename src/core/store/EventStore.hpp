#pragma once

#include "core/events/Event.hpp"
#include <QString>
#include <memory>
#include <vector>

namespace severance::core::store {

struct StoredEvent {
  int64_t id{0};
  int64_t timestamp{0};
  int eventType{0};
  QString eventName;
  QString payloadJson; // Basic JSON payload representation for now
};

class EventStore {
public:
  static EventStore& GetInstance() {
    static EventStore instance;
    return instance;
  }

  bool Initialize(const QString& dbPath = "severance_events.db");
  void Shutdown();

  // Sink for EventBus
  void RecordEvent(std::shared_ptr<events::Event> event);

  // Retrieval
  std::vector<StoredEvent> GetRecentEvents(int limit = 100);

private:
  EventStore() = default;
  ~EventStore();

  EventStore(const EventStore&) = delete;
  EventStore& operator=(const EventStore&) = delete;

  bool CreateSchema();

  QString m_ConnectionName;
  bool m_Initialized{false};
};

} // namespace severance::core::store
