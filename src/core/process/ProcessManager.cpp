#include "ProcessManager.hpp"
#include "ProcessTree.hpp"

namespace severance::core::process {

void ProcessTree::AddProcess(const ProcessInfo &process) {
  auto node = std::make_shared<ProcessTreeNode>(process);
  // Naive implementation for skeleton: just add everything as root for now
  m_Roots.push_back(node);
}

std::vector<std::shared_ptr<ProcessTreeNode>> ProcessTree::GetRoots() const {
  return m_Roots;
}

void ProcessTree::Clear() { m_Roots.clear(); }

std::vector<ProcessInfo> ProcessManager::GetRunningProcesses() {
  // Return mock data for skeleton
  std::vector<ProcessInfo> processes;

  processes.push_back(ProcessInfo{1, 0, "systemd", "root", 1024000, 0.1});
  processes.push_back(ProcessInfo{100, 1, "severance", "user", 50480000, 2.5});
  processes.push_back(
      ProcessInfo{101, 100, "severance-worker", "user", 12400000, 5.0});

  return processes;
}

} // namespace severance::core::process
