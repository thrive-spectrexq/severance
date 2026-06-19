#pragma once

#include <string>
#include <vector>
#include <memory>

namespace severance::core::events {
  class Event;
}

namespace severance::core::session {

struct SessionAnnotation {
  uint64_t timestamp;
  std::string note;
};

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

  void AddAnnotation(const std::string& note);
  void OnEventReceived(std::shared_ptr<events::Event> event);

  std::string ExportSession(const std::string& exportPath);
  std::string ExportMarkdown(const std::string& exportPath);

private:
  bool m_IsRecording{false};
  SessionInfo m_CurrentSession;
  std::vector<std::shared_ptr<events::Event>> m_RecordedEvents;
  std::vector<SessionAnnotation> m_Annotations;
};

} // namespace severance::core::session
