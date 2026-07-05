#include "CorrelationEngine.hpp"
#include "core/events/EventBus.hpp"
#include "core/events/FileActivityEvent.hpp"
#include "core/process/ProcessManager.hpp"
#include "logging/Logger.hpp"
#include <chrono>
#include <algorithm>

namespace severance::core::correlation {

void CorrelationEngine::Initialize() {
  auto& bus = events::EventBus::GetInstance();
  
  auto callback = [this](std::shared_ptr<events::Event> event) {
    OnEvent(event);
  };

  bus.Subscribe(events::EventType::FileModified, callback);
  bus.Subscribe(events::EventType::ProcessTerminated, callback);
  
  m_Running = true;
  m_PollThread = std::thread(&CorrelationEngine::NetworkPollingThread, this);
  
  SEV_CORE_INFO("CorrelationEngine initialized.");
}

void CorrelationEngine::Shutdown() {
  m_Running = false;
  if (m_PollThread.joinable()) {
    m_PollThread.join();
  }

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
  if (profile.pid == 0) {
    m_ProcessHistory.push(pid);
  }
  profile.pid = pid;
  if (profile.name.empty()) {
    profile.name = fae->fileEvent.processName;
  }
  
  profile.fileActivity.push(fae->fileEvent);
  
  std::string lowerPath = fae->fileEvent.filePath;
  std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

  if (fae->fileEvent.operation == "Write" && 
      (lowerPath.find("macrodata") != std::string::npos || lowerPath.find("sensitive") != std::string::npos)) {
      
      if (profile.state == InnieState::Idle) {
          profile.state = InnieState::Macrodata_Refinement_Detected;
          profile.lastMacrodataRefinementTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
          SEV_CORE_WARN("Macrodata Refinement Detected for Innie PID {0} ({1})", pid, profile.name);
      }
  }
}

void CorrelationEngine::HandleProcessTerminated(std::shared_ptr<events::Event> event) {
  // In a real system we might archive the profile.
  // For now, we clear it when the process exits.
}

void CorrelationEngine::NetworkPollingThread() {
    while (m_Running) {
        auto conns = network::NetworkManager::GetInstance().GetActiveConnections();
        
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            auto now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();

            for (const auto& conn : conns) {
                if (m_Profiles.find(conn.pid) != m_Profiles.end()) {
                    auto& profile = m_Profiles[conn.pid];
                    profile.networkConnections.push(conn);
                    
                    if (profile.state == InnieState::Macrodata_Refinement_Detected) {
                        bool isExternal = false;
                        if (conn.remoteIp != "127.0.0.1" && conn.remoteIp != "0.0.0.0" && conn.remoteIp != "::1" && conn.remoteIp != "*") {
                            isExternal = true;
                        }
                        
                        if (isExternal && (now - profile.lastMacrodataRefinementTime) <= 60) {
                            profile.state = InnieState::Kier_Alert_Triggered;
                            SEV_CORE_CRITICAL("Macrodata Anomaly: Kier Alert Triggered! Innie PID {0} ({1}) exfiltrating data to {2}:{3}", 
                                              profile.pid, profile.name, conn.remoteIp.toStdString(), conn.remotePort);
                        }
                    }
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
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

  // Active network connections
  auto conns = network::NetworkManager::GetInstance().GetActiveConnections();
  for (const auto& c : conns) {
    if (c.pid == pid) {
      profile.networkConnections.push(c);
    }
  }

  return profile;
}

} // namespace severance::core::correlation
