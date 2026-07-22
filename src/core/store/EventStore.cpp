#include "EventStore.hpp"
#include "logging/Logger.hpp"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QVariant>
#include <QUuid>
#include <QLoggingCategory>

namespace severance::core::store {

EventStore::~EventStore() {
  Shutdown();
}

bool EventStore::Initialize(const QString& dbPath) {
  if (m_Initialized) return true;

  // Suppress Qt SQL driver missing warnings
  QLoggingCategory::setFilterRules("qt.sql.qsqldatabase.warning=false");

  m_DbPath = dbPath;
  
  if (QSqlDatabase::isDriverAvailable("QSQLITE")) {
    m_ConnectionName = QUuid::createUuid().toString();
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", m_ConnectionName);
    db.setDatabaseName(dbPath);

    if (db.open() && CreateSchema()) {
      m_UseSqlite = true;
      m_Initialized = true;
      m_FlushThread = std::jthread([this](std::stop_token stoken) {
        FlushThreadLoop(stoken);
      });
      SEV_CORE_INFO("Lumon event store initialized with SQLite storage at {}", dbPath.toStdString());
      return true;
    }
  }

  // Graceful in-memory fallback
  m_UseSqlite = false;
  m_Initialized = true;
  SEV_CORE_INFO("Lumon event store initialized in in-memory session mode.");
  return true;
}

void EventStore::Shutdown() {
  if (!m_Initialized) return;

  if (m_UseSqlite) {
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
  }
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
  if (m_UseSqlite) {
    m_EventBuffer.push_back({ts, std::move(event)});
    if (m_EventBuffer.size() >= 1000) {
      m_BufferCV.notify_one();
    }
  } else {
    StoredEvent se;
    se.id = static_cast<int64_t>(m_InMemoryEvents.size() + 1);
    se.timestamp = ts;
    se.eventType = static_cast<int>(event->GetType());
    se.eventName = QString::fromStdString(event->GetName());
    se.payloadJson = QString::fromStdString(event->GetPayload());
    m_InMemoryEvents.push_back(se);
  }
}

void EventStore::FlushThreadLoop(std::stop_token stoken) {
  QString threadConnName = QUuid::createUuid().toString();
  {
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", threadConnName);
    db.setDatabaseName(m_DbPath);
    if (!db.open()) {
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
    query.exec();
  }

  db.commit();
}

std::vector<StoredEvent> EventStore::GetRecentEvents(int limit) {
  std::vector<StoredEvent> results;
  if (!m_Initialized) return results;

  if (!m_UseSqlite) {
    std::unique_lock<std::mutex> lock(m_BufferMutex);
    int count = static_cast<int>(m_InMemoryEvents.size());
    int start = std::max(0, count - limit);
    for (int i = count - 1; i >= start; --i) {
      results.push_back(m_InMemoryEvents[i]);
    }
    return results;
  }

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
