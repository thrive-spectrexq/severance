#include "WorkspaceManager.hpp"
#include "logging/Logger.hpp"
#include <fstream>
#include <filesystem>
#include <sstream>

namespace severance::core::workspace {

WorkspaceManager::WorkspaceManager() {
  m_Profiles.push_back({"Default", "Default system layout", "{}"});
  m_ActiveProfileName = "Default";
}

WorkspaceManager::~WorkspaceManager() = default;

void WorkspaceManager::LoadProfiles(const std::string& directory) {
  SEV_CORE_INFO("Loading workspace profiles from: {}", directory);
  if (!std::filesystem::exists(directory)) {
      std::filesystem::create_directories(directory);
      return;
  }
  
  for (const auto& entry : std::filesystem::directory_iterator(directory)) {
      if (entry.path().extension() == ".json") {
          std::ifstream file(entry.path());
          if (file.is_open()) {
              std::stringstream buffer;
              buffer << file.rdbuf();
              
              WorkspaceProfile profile;
              profile.name = entry.path().stem().string();
              profile.layoutJson = buffer.str();
              profile.description = "Loaded from disk";
              
              // Simplistic parsing for MVP: we just load the whole file as layoutJson
              // If it already exists, replace it, else push it
              bool found = false;
              for (auto& p : m_Profiles) {
                  if (p.name == profile.name) {
                      p = profile;
                      found = true;
                      break;
                  }
              }
              if (!found) m_Profiles.push_back(profile);
          }
      }
  }
}

void WorkspaceManager::SaveProfile(const WorkspaceProfile& profile, const std::string& directory) {
  SEV_CORE_INFO("Saving workspace profile: {}", profile.name);
  if (!std::filesystem::exists(directory)) {
      std::filesystem::create_directories(directory);
  }
  
  std::filesystem::path path = std::filesystem::path(directory) / (profile.name + ".json");
  std::ofstream file(path);
  if (file.is_open()) {
      file << profile.layoutJson;
      file.close();
  }
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
