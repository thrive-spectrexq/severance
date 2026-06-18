#pragma once

#include "core/filesystem/EtwMonitor.hpp"
#include "core/network/NetworkManager.hpp"
#include "core/events/EventTypes.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <string>

namespace severance::core::events {
  class Event;
}

namespace severance::core::correlation {

struct ProcessProfile {
  uint32_t pid{0};
  std::string name;
  
  std::vector<filesystem::FileEvent> fileActivity;
  std::vector<network::NetworkConnection> networkConnections;
};

class CorrelationEngine {
public:
  static CorrelationEngine& GetInstance() {
    static CorrelationEngine instance;
    return instance;
  }

  void Initialize();
  void Shutdown();

  ProcessProfile GetProcessProfile(uint32_t pid);

private:
  CorrelationEngine() = default;
  ~CorrelationEngine() = default;

  CorrelationEngine(const CorrelationEngine&) = delete;
  CorrelationEngine& operator=(const CorrelationEngine&) = delete;

  void OnEvent(std::shared_ptr<events::Event> event);
  
  void HandleFileActivity(std::shared_ptr<events::Event> event);
  void HandleProcessTerminated(std::shared_ptr<events::Event> event);

  std::unordered_map<uint32_t, ProcessProfile> m_Profiles;
  std::mutex m_Mutex;
};

} // namespace severance::core::correlation
