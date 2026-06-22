#include "EventLogManager.hpp"
#include "logging/Logger.hpp"

#ifdef _WIN32
#include <windows.h>
#include <winevt.h>
#pragma comment(lib, "wevtapi.lib")
#endif

namespace severance::core::security {

EventLogManager::EventLogManager() {}

EventLogManager::~EventLogManager() {
  UnsubscribeAll();
}

void EventLogManager::RegisterCallback(EventCallback cb) {
  std::lock_guard<std::mutex> lock(m_Mutex);
  m_Callbacks.push_back(cb);
}

bool EventLogManager::Subscribe(const std::string& channel, const std::string& query) {
#ifdef _WIN32
  int chanLen = MultiByteToWideChar(CP_UTF8, 0, channel.c_str(), -1, NULL, 0);
  std::wstring wChannel(chanLen, 0);
  MultiByteToWideChar(CP_UTF8, 0, channel.c_str(), -1, &wChannel[0], chanLen);

  int queryLen = MultiByteToWideChar(CP_UTF8, 0, query.c_str(), -1, NULL, 0);
  std::wstring wQuery(queryLen, 0);
  MultiByteToWideChar(CP_UTF8, 0, query.c_str(), -1, &wQuery[0], queryLen);

  EVT_HANDLE hSubscription = EvtSubscribe(
      NULL,
      NULL,
      wChannel.c_str(),
      wQuery.c_str(),
      NULL,
      this, // Context
      (EVT_SUBSCRIBE_CALLBACK)SubscriptionCallback,
      EvtSubscribeToFutureEvents
  );

  if (!hSubscription) {
    SEV_CORE_ERROR("Failed to subscribe to Windows Event Log channel: {}. Error: {}", channel, GetLastError());
    return false;
  }

  std::lock_guard<std::mutex> lock(m_Mutex);
  SubscriptionContext ctx;
  ctx.channel = channel;
  ctx.hSubscription = hSubscription;
  m_Subscriptions.push_back(ctx);

  SEV_CORE_INFO("Subscribed to Windows Event Log channel: {}", channel);
  return true;
#else
  SEV_CORE_WARN("EventLogManager is only supported on Windows.");
  return false;
#endif
}

void EventLogManager::Unsubscribe(const std::string& channel) {
#ifdef _WIN32
  std::lock_guard<std::mutex> lock(m_Mutex);
  for (auto it = m_Subscriptions.begin(); it != m_Subscriptions.end(); ) {
    if (it->channel == channel) {
      if (it->hSubscription) {
        EvtClose((EVT_HANDLE)it->hSubscription);
      }
      it = m_Subscriptions.erase(it);
    } else {
      ++it;
    }
  }
#endif
}

void EventLogManager::UnsubscribeAll() {
#ifdef _WIN32
  std::lock_guard<std::mutex> lock(m_Mutex);
  for (auto& sub : m_Subscriptions) {
    if (sub.hSubscription) {
      EvtClose((EVT_HANDLE)sub.hSubscription);
    }
  }
  m_Subscriptions.clear();
#endif
}

#ifdef _WIN32
unsigned long __stdcall EventLogManager::SubscriptionCallback(
    int action,
    void* userContext,
    void* eventHandle) {
    
  if (action == EvtSubscribeActionDeliver) {
    EventLogManager* mgr = reinterpret_cast<EventLogManager*>(userContext);
    if (mgr && eventHandle) {
      mgr->HandleEvent(eventHandle);
    }
  }
  return 0; // ERROR_SUCCESS
}

void EventLogManager::HandleEvent(void* eventHandle) {
  EVT_HANDLE hEvent = (EVT_HANDLE)eventHandle;
  
  DWORD bufferSize = 0;
  DWORD bufferUsed = 0;
  DWORD propertyCount = 0;

  // Render to XML
  EvtRender(NULL, hEvent, EvtRenderEventXml, bufferSize, NULL, &bufferUsed, &propertyCount);
  if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
    bufferSize = bufferUsed;
    std::vector<WCHAR> buffer(bufferSize / sizeof(WCHAR));
    if (EvtRender(NULL, hEvent, EvtRenderEventXml, bufferSize, buffer.data(), &bufferUsed, &propertyCount)) {
      std::wstring wXml(buffer.data());
      
      // Convert WString to String
      int size_needed = WideCharToMultiByte(CP_UTF8, 0, wXml.c_str(), -1, NULL, 0, NULL, NULL);
      std::string xmlStr(size_needed, 0);
      WideCharToMultiByte(CP_UTF8, 0, wXml.c_str(), -1, &xmlStr[0], size_needed, NULL, NULL);

      WindowsEvent ev;
      ev.xmlData = xmlStr;
      
      std::lock_guard<std::mutex> lock(m_Mutex);
      for (auto& cb : m_Callbacks) {
        cb(ev);
      }
    }
  }
}
#else
void EventLogManager::HandleEvent(void* eventHandle) {}
unsigned long EventLogManager::SubscriptionCallback(int action, void* userContext, void* eventHandle) { return 0; }
#endif

} // namespace severance::core::security
