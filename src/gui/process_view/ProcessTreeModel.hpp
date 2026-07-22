#pragma once

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <vector>
#include <memory>
#include <QString>

namespace severance::gui::process_view {

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
    ColCount
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
};

} // namespace severance::gui::process_view
