#pragma once

#include "IsolationPolicy.hpp"
#include <string>

namespace severance::core::sandbox {

struct SandboxProfile {
  std::string name;
  IsolationPolicy policy;
  std::string executablePath;
};

} // namespace severance::core::sandbox
