#include "SandboxManager.hpp"
#include "logging/Logger.hpp"

namespace severance::core::sandbox {

bool SandboxManager::LaunchProfile(const SandboxProfile &profile) {
  SEV_CORE_INFO("Launching sandbox profile: {} ({})", profile.name,
                profile.executablePath);
  // Real implementation would invoke OS-level isolation (cgroups, namespaces,
  // Windows Job Objects)
  m_ActiveProfiles.push_back(profile);
  return true;
}

void SandboxManager::TerminateAll() {
  SEV_CORE_INFO("Terminating all active sandboxes.");
  m_ActiveProfiles.clear();
}

std::vector<SandboxProfile> SandboxManager::GetActiveProfiles() const {
  return m_ActiveProfiles;
}

} // namespace severance::core::sandbox
