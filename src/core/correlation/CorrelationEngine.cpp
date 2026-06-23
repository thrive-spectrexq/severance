#include "CorrelationEngine.hpp"
#include "core/events/EventBus.hpp"
#include "core/events/FileActivityEvent.hpp"
#include "core/process/ProcessManager.hpp"
#include "logging/Logger.hpp"

namespace severance::core::correlation {

void CorrelationEngine::Initialize() {
  auto& bus = events::EventBus::GetInstance();
  
  auto callback = [this](std::shared_ptr<events::Event> event) {
    OnEvent(event);
  };

  bus.Subscribe(events::EventType::FileModified, callback);
  bus.Subscribe(events::EventType::ProcessTerminated, callback);
  
  SEV_CORE_INFO("CorrelationEngine initialized.");
}

void CorrelationEngine::Shutdown() {
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Profiles.clear();
  SEV_CORE_INFO("CorrelationEngine shut down.");
}

void CorrelationEngine::OnEvent(std::shared_ptr<events::Event> event) {
  if (event->GetType() == events::EventType::FileModified) {
    HandleFileActivity(event);
  } else if (event->GetType() == events::EventType::ProcessTerminated) {
    HandleProcessTerminated(event);
  }
}

void CorrelationEngine::HandleFileActivity(std::shared_ptr<events::Event> event) {
  auto fae = std::static_pointer_cast<events::FileActivityEvent>(event);
  if (!fae) return;

  uint32_t pid = fae->fileEvent.pid;

  std::lock_guard<std::mutex> lock(m_Mutex);
  auto& profile = m_Profiles[pid];
  profile.pid = pid;
  if (profile.name.empty()) {
    profile.name = fae->fileEvent.processName;
  }
  
  profile.fileActivity.push_back(fae->fileEvent);
  
  // Cap at 1000 events per process to prevent memory exhaustion
  if (profile.fileActivity.size() > 1000) {
    profile.fileActivity.erase(profile.fileActivity.begin(), profile.fileActivity.begin() + 100);
  }
}

void CorrelationEngine::HandleProcessTerminated(std::shared_ptr<events::Event> event) {
  // In a real system we might archive the profile.
  // For now, we clear it when the process exits.
  // We need a proper ProcessTerminatedEvent to get the PID.
  // We'll skip cleanup here for now and rely on manual cleanup if memory gets too high,
  // or we can implement it once we have a robust ProcessTerminatedEvent with the PID.
}

ProcessProfile CorrelationEngine::GetProcessProfile(uint32_t pid) {
  ProcessProfile profile;
  
  // Get File Activity
  {
    std::lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_Profiles.find(pid);
    if (it != m_Profiles.end()) {
      profile = it->second;
    } else {
      profile.pid = pid;
      // Get name from ProcessManager
      process::ProcessManager pm;
      auto procs = pm.GetRunningProcesses();
      for (const auto& p : procs) {
        if (p.pid == pid) {
          profile.name = p.name;
          break;
        }
      }
    }
  }

  // Get current active network connections for this PID
  auto conns = network::NetworkManager::GetInstance().GetActiveConnections();
  for (const auto& c : conns) {
    if (c.pid == pid) {
      profile.networkConnections.push_back(c);
    }
  }

  return profile;
}

} // namespace severance::core::correlation
