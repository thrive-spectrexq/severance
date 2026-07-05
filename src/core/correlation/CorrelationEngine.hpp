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

namespace severance::core::security {
  struct FimEvent;
}


namespace severance::core::correlation {

enum class InnieState {
  Idle,
  Macrodata_Refinement_Detected,
  Kier_Alert_Triggered
};

template <typename T, size_t Capacity>
class RingBuffer {
public:
  void push(const T& item) {
    m_Buffer[m_Head] = item;
    m_Head = (m_Head + 1) % Capacity;
    if (m_Size < Capacity) m_Size++;
  }

  size_t size() const { return m_Size; }
  const T& operator[](size_t index) const {
    return m_Buffer[(m_Head + Capacity - m_Size + index) % Capacity];
  }

  std::vector<T> to_vector() const {
    std::vector<T> vec;
    vec.reserve(m_Size);
    for (size_t i = 0; i < m_Size; ++i) {
      vec.push_back((*this)[i]);
    }
    return vec;
  }

private:
  std::array<T, Capacity> m_Buffer;
  size_t m_Head{0};
  size_t m_Size{0};
};

struct ProcessProfile {
  uint32_t pid{0};
  std::string name;
  InnieState state{InnieState::Idle};
  int64_t lastMacrodataRefinementTime{0};
  
  RingBuffer<filesystem::FileEvent, 1024> fileActivity;
  RingBuffer<network::NetworkConnection, 1024> networkConnections;
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

  void NetworkPollingThread();

  std::unordered_map<uint32_t, ProcessProfile> m_Profiles;
  RingBuffer<uint32_t, 4096> m_ProcessHistory;
  std::mutex m_Mutex;
  std::atomic<bool> m_Running{false};
  std::thread m_PollThread;
};

} // namespace severance::core::correlation
