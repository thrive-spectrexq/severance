#pragma once

#include <QWidget>
#include <QTreeView>
#include <QLineEdit>
#include <QLabel>
#include <QTimer>
#include <QPushButton>
#include <QSortFilterProxyModel>

namespace severance::gui::process_view {

class ProcessTreeModel; // forward declaration
class ProcessDetailPanel; // forward declaration

class ProcessView : public QWidget {
  Q_OBJECT

public:
  explicit ProcessView(QWidget *parent = nullptr);
  ~ProcessView() = default;

signals:
  void analyzeProcessRequested(uint32_t pid, const QString& processName, const QString& context);

private slots:
  void onRefreshTimer();
  void onSearchTextChanged(const QString &text);
  void onProcessContextMenu(const QPoint &pos);
  void onProcessDoubleClicked(const QModelIndex &index);

private:
  void setupUI();
  void setupToolbar();
  void refreshProcessList();

  // UI elements
  QLineEdit* m_SearchBar{nullptr};
  QTreeView* m_TreeView{nullptr};
  QLabel* m_ProcessCountLabel{nullptr};
  QPushButton* m_KillBtn{nullptr};
  QPushButton* m_RefreshBtn{nullptr};

  // Model
  ProcessTreeModel* m_Model{nullptr};
  QSortFilterProxyModel* m_ProxyModel{nullptr};

  // Detail Panel
  ProcessDetailPanel* m_DetailPanel{nullptr};

  // Refresh timer
  QTimer* m_RefreshTimer{nullptr};
  int m_RefreshIntervalMs{1500};
};

} // namespace severance::gui::process_view
