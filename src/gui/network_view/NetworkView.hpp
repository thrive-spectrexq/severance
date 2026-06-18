#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QTimer>
#include <QLineEdit>

namespace severance::gui::network_view {

class NetworkView : public QWidget {
  Q_OBJECT

public:
  explicit NetworkView(QWidget* parent = nullptr);
  ~NetworkView() override;

  void Refresh();

private slots:
  void onSearchTextChanged(const QString& text);
  void onContextMenuRequested(const QPoint& pos);

private:
  void setupUI();
  QString formatState(int state);

  QTableWidget* m_Table{nullptr};
  QLineEdit* m_SearchBox{nullptr};
  QTimer* m_RefreshTimer{nullptr};
};

} // namespace severance::gui::network_view
