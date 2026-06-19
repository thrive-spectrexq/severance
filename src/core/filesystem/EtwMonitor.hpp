#pragma once

#include <string>
#include <vector>
#include <functional>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#include <evntrace.h>
#else
// Forward declare for non-Windows platforms
typedef uint64_t TRACEHANDLE;
#endif

namespace severance::core::filesystem {

struct FileEvent {
  int64_t timestamp;
  uint32_t pid;
  std::string processName;
  std::string operation; // e.g., "Create", "Read", "Write", "Delete"
  std::string filePath;
  std::string details;
};

class EtwMonitor {
public:
  static EtwMonitor& GetInstance() {
    static EtwMonitor instance;
    return instance;
  }

  bool Start();
  void Stop();

  void SetCallback(std::function<void(const FileEvent&)> callback) {
    m_Callback = std::move(callback);
  }

private:
  EtwMonitor() = default;
  ~EtwMonitor() { Stop(); }

  EtwMonitor(const EtwMonitor&) = delete;
  EtwMonitor& operator=(const EtwMonitor&) = delete;

#ifdef _WIN32
  static void WINAPI EventRecordCallback(struct _EVENT_RECORD* pEventRecord);
  void ProcessEvent(struct _EVENT_RECORD* pEventRecord);
#endif

  TRACEHANDLE m_SessionHandle{0};
  TRACEHANDLE m_TraceHandle{0};
  std::atomic<bool> m_Running{false};
  std::string m_SessionName{"SeveranceEtwSession"};

  std::function<void(const FileEvent&)> m_Callback;
};

} // namespace severance::core::filesystem
