#pragma once

#include "ProcessInfo.hpp"
#include <memory>
#include <vector>

namespace severance::core::process {

struct ProcessTreeNode {
  ProcessInfo info;
  std::vector<std::shared_ptr<ProcessTreeNode>> children;

  explicit ProcessTreeNode(const ProcessInfo &pInfo) : info(pInfo) {}
};

class ProcessTree {
public:
  void AddProcess(const ProcessInfo &process);
  std::vector<std::shared_ptr<ProcessTreeNode>> GetRoots() const;
  void Clear();

private:
  std::vector<std::shared_ptr<ProcessTreeNode>> m_Roots;
  // Potentially a flat map for quick parent lookup could be added here
};

} // namespace severance::core::process
