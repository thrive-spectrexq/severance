#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QTimer>
#include <QLineEdit>
#include <QSplitter>

namespace severance::gui::network_view {

class NetworkView : public QWidget {
  Q_OBJECT

public:
  explicit NetworkView(QWidget* parent = nullptr);
  ~NetworkView() override = default;

  void Refresh();

private slots:
  void onSearchTextChanged(const QString& text);
  void onContextMenuRequested(const QPoint& pos);
  void onSelectionChanged();
  void onTimerTick();

private:
  void setupUI();
  QString formatState(int stateEnum);

  QTableWidget* m_Table{nullptr};
  QLineEdit* m_SearchBox{nullptr};
  QTimer* m_RefreshTimer{nullptr};
  QSplitter* m_Splitter{nullptr};
};

} // namespace severance::gui::network_view
