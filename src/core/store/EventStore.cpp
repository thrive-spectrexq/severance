#include "EventStore.hpp"
#include "logging/Logger.hpp"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QVariant>
#include <QUuid>

namespace severance::core::store {

EventStore::~EventStore() {
  Shutdown();
}

bool EventStore::Initialize(const QString& dbPath) {
  if (m_Initialized) return true;

  m_DbPath = dbPath;
  m_ConnectionName = QUuid::createUuid().toString();
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_ConnectionName);
  db.setDatabaseName(dbPath);

  if (!db.open()) {
    SEV_CORE_ERROR("Failed to open SQLite event store at {}: {}", 
                   dbPath.toStdString(), db.lastError().text().toStdString());
    return false;
  }

  if (!CreateSchema()) {
    SEV_CORE_ERROR("Failed to create schema for SQLite event store.");
    db.close();
    return false;
  }

  m_Initialized = true;
  m_FlushThread = std::jthread([this](std::stop_token stoken) {
    FlushThreadLoop(stoken);
  });
  
  SEV_CORE_INFO("Event store initialized successfully at {}", dbPath.toStdString());
  return true;
}

void EventStore::Shutdown() {
  if (!m_Initialized) return;

  m_FlushThread.request_stop();
  m_BufferCV.notify_one();
  if (m_FlushThread.joinable()) {
    m_FlushThread.join();
  }

  {
    QSqlDatabase db = QSqlDatabase::database(m_ConnectionName);
    if (db.isOpen()) {
      db.close();
    }
  }
  QSqlDatabase::removeDatabase(m_ConnectionName);
  m_Initialized = false;
}

bool EventStore::CreateSchema() {
  QSqlDatabase db = QSqlDatabase::database(m_ConnectionName);
  QSqlQuery query(db);

  const char* schema = R"(
    CREATE TABLE IF NOT EXISTS events (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      timestamp INTEGER NOT NULL,
      event_type INTEGER NOT NULL,
      event_name TEXT NOT NULL,
      payload TEXT
    );
    CREATE INDEX IF NOT EXISTS idx_events_timestamp ON events(timestamp);

    CREATE TABLE IF NOT EXISTS entity_memory (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      timestamp INTEGER NOT NULL,
      fact_key TEXT UNIQUE NOT NULL,
      fact_value TEXT NOT NULL
    );
    
    CREATE TABLE IF NOT EXISTS conversation_memory (
      id INTEGER PRIMARY KEY AUTOINCREMENT,
      timestamp INTEGER NOT NULL,
      summary TEXT NOT NULL
    );
  )";

  if (!query.exec(schema)) {
    SEV_CORE_ERROR("SQL error during schema creation: {}", query.lastError().text().toStdString());
    return false;
  }
  return true;
}

void EventStore::RecordEvent(std::shared_ptr<events::Event> event) {
  if (!m_Initialized || !event) return;

  int64_t ts = QDateTime::currentMSecsSinceEpoch();

  std::unique_lock<std::mutex> lock(m_BufferMutex);
  m_EventBuffer.push_back({ts, std::move(event)});
  if (m_EventBuffer.size() >= 1000) {
    m_BufferCV.notify_one();
  }
}

void EventStore::FlushThreadLoop(std::stop_token stoken) {
  QString threadConnName = QUuid::createUuid().toString();
  {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", threadConnName);
    db.setDatabaseName(m_DbPath);
    if (!db.open()) {
      SEV_CORE_ERROR("Flush thread failed to open DB.");
      return;
    }
  }

  std::vector<BufferedEvent> localBuffer;
  localBuffer.reserve(1000);

  while (!stoken.stop_requested()) {
    std::unique_lock<std::mutex> lock(m_BufferMutex);
    m_BufferCV.wait_for(lock, stoken, std::chrono::milliseconds(500), [this]() {
      return m_EventBuffer.size() >= 1000;
    });
    
    if (m_EventBuffer.empty()) {
      continue;
    }

    localBuffer.swap(m_EventBuffer);
    lock.unlock();

    FlushEvents(localBuffer, threadConnName);
    localBuffer.clear();
  }

  // Final flush on stop
  {
    std::unique_lock<std::mutex> lock(m_BufferMutex);
    localBuffer.swap(m_EventBuffer);
  }
  if (!localBuffer.empty()) {
    FlushEvents(localBuffer, threadConnName);
  }

  {
    QSqlDatabase db = QSqlDatabase::database(threadConnName);
    if (db.isOpen()) {
      db.close();
    }
  }
  QSqlDatabase::removeDatabase(threadConnName);
}

void EventStore::FlushEvents(const std::vector<BufferedEvent>& eventsToFlush, const QString& connName) {
  QSqlDatabase db = QSqlDatabase::database(connName);
  db.transaction();

  QSqlQuery query(db);
  query.prepare("INSERT INTO events (timestamp, event_type, event_name, payload) VALUES (?, ?, ?, ?)");
  
  for (const auto& ev : eventsToFlush) {
    query.addBindValue(ev.timestamp);
    query.addBindValue(static_cast<int>(ev.event->GetType()));
    query.addBindValue(QString::fromStdString(ev.event->GetName()));
    query.addBindValue(QString::fromStdString(ev.event->GetPayload()));
    if (!query.exec()) {
      SEV_CORE_WARN("Failed to flush event {}: {}", ev.event->GetName(), query.lastError().text().toStdString());
    }
  }

  if (!db.commit()) {
    SEV_CORE_ERROR("Failed to commit event batch: {}", db.lastError().text().toStdString());
  } else {
    SEV_CORE_INFO("Successfully flushed {} events to the Lumon event store.", eventsToFlush.size());
  }
}

std::vector<StoredEvent> EventStore::GetRecentEvents(int limit) {
  std::vector<StoredEvent> results;
  if (!m_Initialized) return results;

  QSqlDatabase db = QSqlDatabase::database(m_ConnectionName);
  QSqlQuery query(db);
  query.prepare("SELECT id, timestamp, event_type, event_name, payload FROM events ORDER BY timestamp DESC LIMIT ?");
  query.addBindValue(limit);

  if (query.exec()) {
    while (query.next()) {
      StoredEvent e;
      e.id = query.value(0).toLongLong();
      e.timestamp = query.value(1).toLongLong();
      e.eventType = query.value(2).toInt();
      e.eventName = query.value(3).toString();
      e.payloadJson = query.value(4).toString();
      results.push_back(std::move(e));
    }
  } else {
    SEV_CORE_ERROR("Failed to fetch recent events: {}", query.lastError().text().toStdString());
  }

  return results;
}

void EventStore::StoreEntityFact(const QString& key, const QString& value) {
  if (!m_Initialized) return;

  QSqlDatabase db = QSqlDatabase::database(m_ConnectionName);
  QSqlQuery query(db);
  // Using UPSERT syntax for SQLite
  query.prepare("INSERT INTO entity_memory (timestamp, fact_key, fact_value) VALUES (?, ?, ?) "
                "ON CONFLICT(fact_key) DO UPDATE SET fact_value = excluded.fact_value, timestamp = excluded.timestamp");
  
  query.addBindValue(QDateTime::currentMSecsSinceEpoch());
  query.addBindValue(key);
  query.addBindValue(value);

  if (!query.exec()) {
    SEV_CORE_WARN("Failed to store entity fact '{}': {}", key.toStdString(), query.lastError().text().toStdString());
  } else {
    SEV_CORE_INFO("Stored entity fact '{}' successfully for the Innie.", key.toStdString());
  }
}

QString EventStore::GetEntityFact(const QString& key) {
  if (!m_Initialized) return QString();

  QSqlDatabase db = QSqlDatabase::database(m_ConnectionName);
  QSqlQuery query(db);
  query.prepare("SELECT fact_value FROM entity_memory WHERE fact_key = ?");
  query.addBindValue(key);

  if (query.exec() && query.next()) {
    return query.value(0).toString();
  }
  return QString();
}

void EventStore::StoreConversationSummary(const QString& summary) {
  if (!m_Initialized) return;

  QSqlDatabase db = QSqlDatabase::database(m_ConnectionName);
  QSqlQuery query(db);
  query.prepare("INSERT INTO conversation_memory (timestamp, summary) VALUES (?, ?)");
  
  query.addBindValue(QDateTime::currentMSecsSinceEpoch());
  query.addBindValue(summary);

  if (!query.exec()) {
    SEV_CORE_WARN("Failed to store conversation summary: {}", query.lastError().text().toStdString());
  } else {
    SEV_CORE_INFO("Stored conversation summary for the Innie.");
  }
}

std::vector<QString> EventStore::GetRecentSummaries(int limit) {
  std::vector<QString> results;
  if (!m_Initialized) return results;

  QSqlDatabase db = QSqlDatabase::database(m_ConnectionName);
  QSqlQuery query(db);
  query.prepare("SELECT summary FROM conversation_memory ORDER BY timestamp DESC LIMIT ?");
  query.addBindValue(limit);

  if (query.exec()) {
    while (query.next()) {
      results.push_back(query.value(0).toString());
    }
  } else {
    SEV_CORE_ERROR("Failed to fetch recent summaries: {}", query.lastError().text().toStdString());
  }

  return results;
}

} // namespace severance::core::store
