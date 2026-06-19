#pragma once

#include "SandboxProfile.hpp"
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include "utils/ScopedHandle.hpp"
#endif

namespace severance::core::sandbox {

class SandboxManager {
public:
  SandboxManager() = default;
  ~SandboxManager();

  SandboxManager(const SandboxManager &) = delete;
  SandboxManager &operator=(const SandboxManager &) = delete;

  bool LaunchProfile(const SandboxProfile &profile);
  void TerminateSandbox(size_t index);
  void TerminateAll();

  std::vector<SandboxProfile> GetActiveProfiles() const;

private:
  struct ActiveSandbox {
    SandboxProfile profile;
#if defined(_WIN32)
    utils::ScopedHandle hProcess;
    utils::ScopedHandle hJob;
#else
    void* hProcess{nullptr};
    void* hJob{nullptr};
#endif
  };

  std::vector<ActiveSandbox> m_ActiveSandboxes;
};

} // namespace severance::core::sandbox
