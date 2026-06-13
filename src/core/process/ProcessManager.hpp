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

  std::vector<ProcessInfo> GetRunningProcesses();
};

} // namespace severance::core::process
