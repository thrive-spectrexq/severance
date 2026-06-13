#pragma once

#include "FileEvent.hpp"
#include <string>
#include <vector>

namespace severance::core::filesystem {

class FileMonitor {
public:
  FileMonitor() = default;
  ~FileMonitor();

  FileMonitor(const FileMonitor &) = delete;
  FileMonitor &operator=(const FileMonitor &) = delete;

  void Start(const std::string &directory);
  void Stop();

  std::vector<FileEvent> GetRecentEvents();

private:
  bool m_IsRunning{false};
  std::string m_WatchedDirectory;
  // In a real implementation this would hold a buffer of recent events,
  // background threads, and OS API handles.
};

} // namespace severance::core::filesystem
