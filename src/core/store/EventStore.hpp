#pragma once

#include "core/events/Event.hpp"
#include <QString>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace severance::core::store {

struct StoredEvent {
  int64_t id{0};
  int64_t timestamp{0};
  int eventType{0};
  QString eventName;
  QString payloadJson; // Basic JSON payload representation for now
};

struct BufferedEvent {
  int64_t timestamp;
  std::shared_ptr<events::Event> event;
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

  // Tiered Memory System
  void StoreEntityFact(const QString& key, const QString& value);
  QString GetEntityFact(const QString& key);
  void StoreConversationSummary(const QString& summary);
  std::vector<QString> GetRecentSummaries(int limit = 5);

private:
  EventStore() = default;
  ~EventStore();

  EventStore(const EventStore&) = delete;
  EventStore& operator=(const EventStore&) = delete;

  bool CreateSchema();

  QString m_ConnectionName;
  QString m_DbPath;
  bool m_Initialized{false};
  bool m_UseSqlite{false};

  // In-memory fallback storage
  std::vector<StoredEvent> m_InMemoryEvents;
  std::vector<QString> m_InMemorySummaries;

  // Background flush
  void FlushThreadLoop(std::stop_token stoken);
  void FlushEvents(const std::vector<BufferedEvent>& eventsToFlush, const QString& connName);

  std::jthread m_FlushThread;
  std::mutex m_BufferMutex;
  std::condition_variable_any m_BufferCV;
  std::vector<BufferedEvent> m_EventBuffer;
};

} // namespace severance::core::store
