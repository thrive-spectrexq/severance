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
  SEV_CORE_INFO("Event store initialized successfully at {}", dbPath.toStdString());
  return true;
}

void EventStore::Shutdown() {
  if (!m_Initialized) return;

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
  )";

  if (!query.exec(schema)) {
    SEV_CORE_ERROR("SQL error during schema creation: {}", query.lastError().text().toStdString());
    return false;
  }
  return true;
}

void EventStore::RecordEvent(std::shared_ptr<events::Event> event) {
  if (!m_Initialized || !event) return;

  QSqlDatabase db = QSqlDatabase::database(m_ConnectionName);
  QSqlQuery query(db);
  query.prepare("INSERT INTO events (timestamp, event_type, event_name, payload) VALUES (?, ?, ?, ?)");
  
  query.addBindValue(QDateTime::currentMSecsSinceEpoch());
  query.addBindValue(static_cast<int>(event->GetType()));
  query.addBindValue(QString::fromStdString(event->GetName()));
  // In a real implementation, we'd serialize event specific data to JSON.
  query.addBindValue("{}");

  if (!query.exec()) {
    SEV_CORE_WARN("Failed to record event {}: {}", event->GetName(), query.lastError().text().toStdString());
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

} // namespace severance::core::store
