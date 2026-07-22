#include "ProcessTreeModel.hpp"

namespace severance::gui::process_view {

ProcessTreeModel::ProcessTreeModel(QObject* parent) : QAbstractItemModel(parent) {}

QModelIndex ProcessTreeModel::index(int, int, const QModelIndex&) const { return QModelIndex(); }
QModelIndex ProcessTreeModel::parent(const QModelIndex&) const { return QModelIndex(); }
int ProcessTreeModel::rowCount(const QModelIndex&) const { return 0; }
int ProcessTreeModel::columnCount(const QModelIndex&) const { return ColCount; }
QVariant ProcessTreeModel::data(const QModelIndex&, int) const { return QVariant(); }
QVariant ProcessTreeModel::headerData(int, Qt::Orientation, int) const { return QVariant(); }
Qt::ItemFlags ProcessTreeModel::flags(const QModelIndex&) const { return Qt::NoItemFlags; }

} // namespace severance::gui::process_view
