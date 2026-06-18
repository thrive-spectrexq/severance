#pragma once

#include "ProcessInfo.hpp"
#include <vector>

namespace severance::core::process {

class ProcessManager {
public:
  ProcessManager() = default;
  ~ProcessManager() = default;

  ProcessManager(const ProcessManager &) = delete;
  ProcessManager &operator=(const ProcessManager &) = delete;

  // Get snapshot of all running processes with real system data
  std::vector<ProcessInfo> GetRunningProcesses();

  // Process actions
  bool KillProcess(uint32_t pid);
  bool SuspendProcess(uint32_t pid);
  bool ResumeProcess(uint32_t pid);
};

} // namespace severance::core::process
