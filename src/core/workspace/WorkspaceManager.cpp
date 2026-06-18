#include "WorkspaceManager.hpp"
#include "logging/Logger.hpp"

namespace severance::core::workspace {

WorkspaceManager::WorkspaceManager() {
  // Default profile
  m_Profiles.push_back({"Default", "Default system layout", "{}"});
  m_ActiveProfileName = "Default";
}

WorkspaceManager::~WorkspaceManager() = default;

void WorkspaceManager::LoadProfiles(const std::string& directory) {
  SEV_CORE_INFO("Loading workspace profiles from: {}", directory);
  // Implementation stub
}

void WorkspaceManager::SaveProfile(const WorkspaceProfile& profile) {
  SEV_CORE_INFO("Saving workspace profile: {}", profile.name);
  // Implementation stub
}

std::vector<WorkspaceProfile> WorkspaceManager::GetAvailableProfiles() const {
  return m_Profiles;
}

WorkspaceProfile WorkspaceManager::GetActiveProfile() const {
  for (const auto& p : m_Profiles) {
    if (p.name == m_ActiveProfileName) return p;
  }
  return m_Profiles.empty() ? WorkspaceProfile{} : m_Profiles.front();
}

void WorkspaceManager::SetActiveProfile(const std::string& name) {
  m_ActiveProfileName = name;
  SEV_CORE_INFO("Active workspace profile set to: {}", name);
}

} // namespace severance::core::workspace
