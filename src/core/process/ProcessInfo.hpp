#pragma once

#include <cstdint>
#include <string>

namespace severance::core::process {

struct ProcessInfo {
  uint32_t pid{0};
  uint32_t ppid{0};
  std::string name;
  std::string executablePath;
  std::string user;
  std::string commandLine;
  uint64_t memoryWorkingSetBytes{0};
  uint64_t memoryPrivateBytes{0};
  double cpuUsagePercent{0.0};
  uint64_t diskReadBytesPerSec{0};
  uint64_t diskWriteBytesPerSec{0};
  uint32_t threadCount{0};
  uint32_t handleCount{0};
  uint64_t startTimeEpochMs{0};

  enum class Status { Running, Suspended, Zombie };
  Status status{Status::Running};
};

} // namespace severance::core::process
