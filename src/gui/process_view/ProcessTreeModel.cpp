#include "ProcessTreeModel.hpp"
#include <QColor>
#include <QFont>
#include <algorithm>
#include <unordered_map>

namespace severance::gui::process_view {

ProcessTreeModel::ProcessTreeModel(QObject* parent)
    : QAbstractItemModel(parent) {
  // Create a virtual root node (not visible in the tree)
  core::process::ProcessInfo rootInfo;
  rootInfo.pid = 0;
  rootInfo.name = "root";
  m_RootNode = std::make_unique<ProcessNode>(rootInfo);
}

QModelIndex ProcessTreeModel::index(int row, int column, const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) return {};

  ProcessNode* parentNode = parent.isValid()
    ? static_cast<ProcessNode*>(parent.internalPointer())
    : m_RootNode.get();

  if (row < static_cast<int>(parentNode->children.size())) {
    return createIndex(row, column, parentNode->children[row].get());
  }
  return {};
}

QModelIndex ProcessTreeModel::parent(const QModelIndex& index) const {
  if (!index.isValid()) return {};

  auto* childNode = static_cast<ProcessNode*>(index.internalPointer());
  auto* parentNode = childNode->parent;

  if (!parentNode || parentNode == m_RootNode.get()) return {};

  // Find the row of the parent in its own parent
  auto* grandParent = parentNode->parent;
  if (!grandParent) return {};

  for (int i = 0; i < static_cast<int>(grandParent->children.size()); ++i) {
    if (grandParent->children[i].get() == parentNode) {
      return createIndex(i, 0, parentNode);
    }
  }
  return {};
}

int ProcessTreeModel::rowCount(const QModelIndex& parent) const {
  if (parent.column() > 0) return 0;

  ProcessNode* parentNode = parent.isValid()
    ? static_cast<ProcessNode*>(parent.internalPointer())
    : m_RootNode.get();

  return static_cast<int>(parentNode->children.size());
}

int ProcessTreeModel::columnCount(const QModelIndex& /*parent*/) const {
  return ColCount;
}

QVariant ProcessTreeModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return {};

  auto* node = static_cast<ProcessNode*>(index.internalPointer());
  const auto& info = node->info;

  if (role == Qt::DisplayRole) {
    switch (index.column()) {
      case ColName:    return QString::fromStdString(info.name);
      case ColPID:     return info.pid;
      case ColCPU:     return formatCpu(info.cpuUsagePercent);
      case ColMemory:  return formatMemory(info.memoryWorkingSetBytes);
      case ColThreads: return info.threadCount;
      case ColUser:    return QString::fromStdString(info.user);
      case ColPath:    return QString::fromStdString(info.executablePath);
      default: return {};
    }
  }

  if (role == Qt::ForegroundRole) {
    // Color-code CPU usage
    if (index.column() == ColCPU) {
      if (info.cpuUsagePercent > 50.0) return QColor("#F85149");  // Red
      if (info.cpuUsagePercent > 20.0) return QColor("#D29922");  // Yellow
      return QColor("#8B949E");  // Muted for low usage
    }
    // Muted text for secondary columns
    if (index.column() == ColPath || index.column() == ColUser) {
      return QColor("#6E7681");
    }
    return QColor("#E6EDF3");
  }

  if (role == Qt::TextAlignmentRole) {
    if (index.column() == ColPID || index.column() == ColCPU ||
        index.column() == ColMemory || index.column() == ColThreads) {
      return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
    }
  }

  if (role == Qt::FontRole) {
    if (index.column() == ColName) {
      QFont f;
      f.setWeight(QFont::Medium);
      return f;
    }
  }

  return {};
}

QVariant ProcessTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};

  switch (section) {
    case ColName:    return "Name";
    case ColPID:     return "PID";
    case ColCPU:     return "CPU %";
    case ColMemory:  return "Memory";
    case ColThreads: return "Threads";
    case ColUser:    return "User";
    case ColPath:    return "Path";
    default: return {};
  }
}

Qt::ItemFlags ProcessTreeModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) return Qt::NoItemFlags;
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void ProcessTreeModel::updateProcessList(const std::vector<core::process::ProcessInfo>& processes) {
  beginResetModel();
  buildTree(processes);
  m_TotalCount = static_cast<int>(processes.size());
  endResetModel();
}

void ProcessTreeModel::buildTree(const std::vector<core::process::ProcessInfo>& processes) {
  // Clear existing tree
  m_RootNode->children.clear();

  // Build a lookup map: PID -> node
  std::unordered_map<uint32_t, ProcessNode*> nodeMap;

  // First pass: create all nodes
  std::vector<std::unique_ptr<ProcessNode>> allNodes;
  for (const auto& proc : processes) {
    auto node = std::make_unique<ProcessNode>(proc);
    nodeMap[proc.pid] = node.get();
    allNodes.push_back(std::move(node));
  }

  // Second pass: link parent-child relationships
  for (auto& node : allNodes) {
    auto parentIt = nodeMap.find(node->info.ppid);
    if (parentIt != nodeMap.end() && parentIt->second != node.get()) {
      // Found parent — attach as child
      node->parent = parentIt->second;
      parentIt->second->children.push_back(std::move(node));
    } else {
      // No parent found (or self-referencing) — attach to root
      node->parent = m_RootNode.get();
      m_RootNode->children.push_back(std::move(node));
    }
  }

  // Sort children by name at each level
  std::function<void(ProcessNode*)> sortChildren = [&](ProcessNode* node) {
    std::sort(node->children.begin(), node->children.end(),
      [](const auto& a, const auto& b) {
        return a->info.name < b->info.name;
      });
    for (auto& child : node->children) {
      sortChildren(child.get());
    }
  };
  sortChildren(m_RootNode.get());
}

core::process::ProcessInfo ProcessTreeModel::getProcessInfo(const QModelIndex& index) const {
  if (!index.isValid()) return {};
  auto* node = static_cast<ProcessNode*>(index.internalPointer());
  return node->info;
}

QString ProcessTreeModel::formatMemory(uint64_t bytes) const {
  if (bytes == 0) return "0 B";
  const double kb = bytes / 1024.0;
  const double mb = kb / 1024.0;
  const double gb = mb / 1024.0;

  if (gb >= 1.0) return QString::number(gb, 'f', 1) + " GB";
  if (mb >= 1.0) return QString::number(mb, 'f', 1) + " MB";
  if (kb >= 1.0) return QString::number(kb, 'f', 0) + " KB";
  return QString::number(bytes) + " B";
}

QString ProcessTreeModel::formatCpu(double percent) const {
  if (percent < 0.05) return "0.0";
  return QString::number(percent, 'f', 1);
}

} // namespace severance::gui::process_view
