#pragma once

#include <string>
#include <functional>
#include <vector>

namespace severance::core::notifications {

enum class NotificationSeverity {
  Info,
  Warning,
  Error,
  Critical
};

struct Notification {
  std::string id;
  uint64_t timestamp;
  NotificationSeverity severity;
  std::string title;
  std::string message;
  std::string source;
  bool read{false};
};

class NotificationManager {
public:
  NotificationManager();
  ~NotificationManager();

  static NotificationManager& GetInstance() {
    static NotificationManager instance;
    return instance;
  }

  void EmitNotification(const Notification& notification);
  std::vector<Notification> GetUnreadNotifications() const;
  void MarkAsRead(const std::string& id);
  void MarkAllAsRead();

  // Callbacks for UI
  using NotificationCallback = std::function<void(const Notification&)>;
  void RegisterCallback(NotificationCallback cb);

private:
  std::vector<Notification> m_History;
  std::vector<NotificationCallback> m_Callbacks;
};

} // namespace severance::core::notifications
