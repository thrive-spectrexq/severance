#pragma once

#include <cstdint>

namespace severance::core::sandbox {

struct IsolationPolicy {
  bool allowNetworkAccess{false};
  bool allowFileSystemWrite{false};
  uint64_t maxMemoryBytes{0}; // 0 implies no limit
  double maxCpuPercent{100.0};
};

} // namespace severance::core::sandbox
