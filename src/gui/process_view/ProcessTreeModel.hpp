#pragma once

#include "core/process/ProcessInfo.hpp"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <memory>
#include <vector>

namespace severance::gui::process_view {

// Internal tree node for the model
struct ProcessNode {
  core::process::ProcessInfo info;
  ProcessNode* parent{nullptr};
  std::vector<std::unique_ptr<ProcessNode>> children;

  explicit ProcessNode(const core::process::ProcessInfo& pInfo, ProcessNode* parentNode = nullptr)
    : info(pInfo), parent(parentNode) {}
};

class ProcessTreeModel : public QAbstractItemModel {
  Q_OBJECT

public:
  enum Column {
    ColName = 0,
    ColPID,
    ColCPU,
    ColMemory,
    ColThreads,
    ColUser,
    ColPath,
    ColCount  // sentinel — total column count
  };

  explicit ProcessTreeModel(QObject* parent = nullptr);
  ~ProcessTreeModel() override = default;

  // QAbstractItemModel interface
  QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  // Data update
  void updateProcessList(const std::vector<core::process::ProcessInfo>& processes);

  // Accessors
  core::process::ProcessInfo getProcessInfo(const QModelIndex& index) const;
  int totalProcessCount() const { return m_TotalCount; }

private:
  void buildTree(const std::vector<core::process::ProcessInfo>& processes);
  QString formatMemory(uint64_t bytes) const;
  QString formatCpu(double percent) const;

  std::unique_ptr<ProcessNode> m_RootNode;
  int m_TotalCount{0};
};

} // namespace severance::gui::process_view
