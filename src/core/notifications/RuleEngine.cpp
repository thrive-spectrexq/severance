#include "RuleEngine.hpp"
#include "NotificationManager.hpp"
#include "core/events/FileActivityEvent.hpp"
#include "utils/UUID.hpp"
#include <iostream>

namespace severance::core::notifications {

RuleEngine::RuleEngine() {}

RuleEngine::~RuleEngine() {
  Stop();
}

void RuleEngine::Start() {
  if (m_Running) return;
  m_Running = true;

  m_SubscriptionId = events::EventBus::GetInstance().Subscribe(
    events::EventType::FileModified,
    [this](const std::shared_ptr<events::Event>& event) {
      this->OnEvent(event);
    }
  );
  
  // Also subscribe to other types even if we just log/stub them
  events::EventBus::GetInstance().Subscribe(
    events::EventType::ProcessCreated,
    [this](const std::shared_ptr<events::Event>& event) {
      this->OnEvent(event);
    }
  );
  
  events::EventBus::GetInstance().Subscribe(
    events::EventType::NetworkConnectionOpened,
    [this](const std::shared_ptr<events::Event>& event) {
      this->OnEvent(event);
    }
  );
}

void RuleEngine::Stop() {
  if (!m_Running) return;
  m_Running = false;
  events::EventBus::GetInstance().Unsubscribe(m_SubscriptionId);
}

void RuleEngine::OnEvent(const std::shared_ptr<events::Event>& event) {
  if (!m_Running || !event) return;

  std::lock_guard<std::mutex> lock(m_StateMutex);

  switch (event->GetType()) {
    case events::EventType::ProcessCreated:
      CheckSuspiciousPath(event);
      break;
    case events::EventType::FileModified:
    case events::EventType::FileRenamed:
    case events::EventType::FileDeleted:
      CheckRansomwarePattern(event);
      break;
    case events::EventType::NetworkConnectionOpened:
      CheckNetworkBurst(event);
      break;
    default:
      break;
  }
}

void RuleEngine::CheckRansomwarePattern(const std::shared_ptr<events::Event>& event) {
  auto fileEvent = std::dynamic_pointer_cast<events::FileActivityEvent>(event);
  if (!fileEvent) return;

  uint32_t pid = fileEvent->fileEvent.pid;
  uint64_t timestamp = fileEvent->fileEvent.timestamp;

  // Ransomware Pattern: > 20 file modifications in 2 seconds from the same PID
  auto& state = m_FileModCounts[pid];
  
  if (state.count == 0 || (timestamp - state.firstEventTime) > 2000) {
    // Reset window
    state.count = 1;
    state.firstEventTime = timestamp;
  } else {
    state.count++;
    if (state.count == 20) {
      Notification n;
      n.id = utils::GenerateUUID();
      n.timestamp = timestamp;
      n.severity = NotificationSeverity::Critical;
      n.title = "Ransomware Heuristic Triggered";
      n.message = "Process " + std::to_string(pid) + " is rapidly modifying files.";
      n.source = "RuleEngine";
      
      NotificationManager::GetInstance().EmitNotification(n);
    }
  }
}

void RuleEngine::CheckSuspiciousPath(const std::shared_ptr<events::Event>& event) {
  // Stub until ProcessEvent is fully implemented
}

void RuleEngine::CheckNetworkBurst(const std::shared_ptr<events::Event>& event) {
  // Stub until NetworkEvent is fully implemented
}

} // namespace severance::core::notifications
