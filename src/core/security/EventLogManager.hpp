#pragma once

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>

namespace severance::core::security {

struct WindowsEvent {
  std::string channel;
  uint32_t eventId;
  std::string providerName;
  std::string xmlData; // Raw XML of the event for parsing
  std::string timeCreated;
};

class EventLogManager {
public:
  using EventCallback = std::function<void(const WindowsEvent&)>;

  static EventLogManager& GetInstance() {
    static EventLogManager instance;
    return instance;
  }

  // Subscribe to a specific channel with an optional XPath query
  bool Subscribe(const std::string& channel, const std::string& query = "*");
  void Unsubscribe(const std::string& channel);
  void UnsubscribeAll();

  void RegisterCallback(EventCallback cb);

private:
  EventLogManager();
  ~EventLogManager();

  EventLogManager(const EventLogManager&) = delete;
  EventLogManager& operator=(const EventLogManager&) = delete;

  std::vector<EventCallback> m_Callbacks;
  std::mutex m_Mutex;

  struct SubscriptionContext {
    std::string channel;
    void* hSubscription{nullptr}; // EVT_HANDLE
  };
  std::vector<SubscriptionContext> m_Subscriptions;

  // Internal callback for EvtSubscribe
  static unsigned long __stdcall SubscriptionCallback(
      int action,
      void* userContext,
      void* eventHandle);
      
  void HandleEvent(void* eventHandle);
};

} // namespace severance::core::security
