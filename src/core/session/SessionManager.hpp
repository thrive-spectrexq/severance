#pragma once

#include <string>

namespace severance::core::session {

struct SessionInfo {
  std::string sessionId;
  uint64_t startTime;
  uint64_t endTime;
  std::string name;
  std::string description;
  uint64_t eventCount;
};

class SessionManager {
public:
  SessionManager();
  ~SessionManager();

  static SessionManager& GetInstance() {
    static SessionManager instance;
    return instance;
  }

  void StartRecording(const std::string& sessionName);
  void StopRecording();
  bool IsRecording() const;

  std::string ExportSession(const std::string& exportPath);

private:
  bool m_IsRecording{false};
  SessionInfo m_CurrentSession;
};

} // namespace severance::core::session
