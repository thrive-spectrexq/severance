#include "FileMonitor.hpp"

namespace severance::core::filesystem {

FileMonitor::~FileMonitor() { Stop(); }

void FileMonitor::Start(const std::string &directory) {
  m_WatchedDirectory = directory;
  m_IsRunning = true;
  // Real implementation would register OS watchers here
}

void FileMonitor::Stop() {
  m_IsRunning = false;
  // Real implementation would unregister OS watchers here
}

std::vector<FileEvent> FileMonitor::GetRecentEvents() {
  std::vector<FileEvent> events;
  if (m_IsRunning) {
    // Mock data for skeleton
    events.push_back({FileEventType::Created, m_WatchedDirectory + "/test.txt",
                      "", 1620000000});
    events.push_back({FileEventType::Modified, m_WatchedDirectory + "/test.txt",
                      "", 1620000010});
    events.push_back({FileEventType::Deleted, m_WatchedDirectory + "/old.log",
                      "", 1620000015});
  }
  return events;
}

} // namespace severance::core::filesystem
