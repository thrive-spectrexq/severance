#pragma once

#include <string>
#include <vector>

namespace severance::core::workspace {

struct WorkspaceProfile {
  std::string name;
  std::string description;
  std::string layoutJson;
};

class WorkspaceManager {
public:
  WorkspaceManager();
  ~WorkspaceManager();

  static WorkspaceManager& GetInstance() {
    static WorkspaceManager instance;
    return instance;
  }

  void LoadProfiles(const std::string& directory);
  void SaveProfile(const WorkspaceProfile& profile, const std::string& directory);
  
  std::vector<WorkspaceProfile> GetAvailableProfiles() const;
  WorkspaceProfile GetActiveProfile() const;
  void SetActiveProfile(const std::string& name);

private:
  std::vector<WorkspaceProfile> m_Profiles;
  std::string m_ActiveProfileName;
};

} // namespace severance::core::workspace
