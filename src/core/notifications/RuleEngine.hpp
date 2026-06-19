#pragma once

#include "core/events/EventBus.hpp"
#include "core/events/Event.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>

namespace severance::core::notifications {

class RuleEngine {
public:
  RuleEngine();
  ~RuleEngine();

  void Start();
  void Stop();

private:
  void OnEvent(const std::shared_ptr<events::Event>& event);

  // Rules
  void CheckRansomwarePattern(const std::shared_ptr<events::Event>& event);
  void CheckSuspiciousPath(const std::shared_ptr<events::Event>& event);
  void CheckNetworkBurst(const std::shared_ptr<events::Event>& event);

  bool m_Running{false};
  uint32_t m_SubscriptionId{0};

  // State for Ransomware Pattern
  struct FileModState {
    uint64_t count{0};
    uint64_t firstEventTime{0};
  };
  std::unordered_map<uint32_t, FileModState> m_FileModCounts; // PID -> State

  // State for Network Burst Pattern
  struct NetConnState {
    uint64_t count{0};
    uint64_t firstEventTime{0};
  };
  std::unordered_map<uint32_t, NetConnState> m_NetConnCounts; // PID -> State

  std::mutex m_StateMutex;
};

} // namespace severance::core::notifications
