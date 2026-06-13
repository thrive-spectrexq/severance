#pragma once

#include <cstdint>
#include <string>

namespace severance::core::process {

struct ProcessInfo {
  uint32_t pid{0};
  uint32_t ppid{0};
  std::string name;
  std::string user;
  uint64_t memoryUsageBytes{0};
  double cpuUsagePercent{0.0};
};

} // namespace severance::core::process
