#include "NotificationManager.hpp"
#include "logging/Logger.hpp"
#include "utils/UUID.hpp"
#include <chrono>

namespace severance::core::notifications {

NotificationManager::NotificationManager() = default;
NotificationManager::~NotificationManager() = default;

void NotificationManager::EmitNotification(const Notification& notification) {
  Notification n = notification;
  if (n.id.empty()) {
    n.id = utils::UUID::Generate();
  }
  if (n.timestamp == 0) {
    n.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  }
  
  m_History.push_back(n);
  
  SEV_CORE_INFO("Notification emitted: [{}] {}", n.title, n.message);

  for (auto& cb : m_Callbacks) {
    cb(n);
  }
}

std::vector<Notification> NotificationManager::GetUnreadNotifications() const {
  std::vector<Notification> unread;
  for (const auto& n : m_History) {
    if (!n.read) unread.push_back(n);
  }
  return unread;
}

void NotificationManager::MarkAsRead(const std::string& id) {
  for (auto& n : m_History) {
    if (n.id == id) {
      n.read = true;
      break;
    }
  }
}

void NotificationManager::MarkAllAsRead() {
  for (auto& n : m_History) {
    n.read = true;
  }
}

void NotificationManager::RegisterCallback(NotificationCallback cb) {
  m_Callbacks.push_back(cb);
}

} // namespace severance::core::notifications
