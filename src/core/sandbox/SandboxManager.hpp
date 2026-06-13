#pragma once

#include "SandboxProfile.hpp"
#include <string>
#include <vector>

namespace severance::core::sandbox {

class SandboxManager {
public:
  SandboxManager() = default;
  ~SandboxManager() = default;

  SandboxManager(const SandboxManager &) = delete;
  SandboxManager &operator=(const SandboxManager &) = delete;

  bool LaunchProfile(const SandboxProfile &profile);
  void TerminateAll();

  std::vector<SandboxProfile> GetActiveProfiles() const;

private:
  std::vector<SandboxProfile> m_ActiveProfiles;
};

} // namespace severance::core::sandbox
