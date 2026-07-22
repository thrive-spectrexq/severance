#pragma once

#include <QWidget>
#include <QString>
#include <QModelIndex>
#include <QPoint>

namespace severance::gui::process_view {

class ProcessTreeModel;
class ProcessDetailPanel;

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
};

} // namespace severance::gui::process_view
