#include "ProcessTree.hpp"

namespace severance::core::process {

void ProcessTree::AddProcess(const ProcessInfo &process) {
  auto node = std::make_shared<ProcessTreeNode>(process);

  // Try to find a parent among existing roots
  // For now, use a simple flat insertion — parent/child linking
  // can be refined when ppid correlation is implemented.
  bool attached = false;
  if (process.ppid != 0) {
    // Search existing tree for parent
    // (simple linear scan for now)
    for (auto &root : m_Roots) {
      if (root->info.pid == process.ppid) {
        root->children.push_back(node);
        attached = true;
        break;
      }
    }
  }

  if (!attached) {
    m_Roots.push_back(node);
  }
}

std::vector<std::shared_ptr<ProcessTreeNode>> ProcessTree::GetRoots() const {
  return m_Roots;
}

void ProcessTree::Clear() {
  m_Roots.clear();
}

} // namespace severance::core::process
