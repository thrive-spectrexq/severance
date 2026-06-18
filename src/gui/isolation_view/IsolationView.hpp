#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QLineEdit>

namespace severance::gui::isolation_view {

class IsolationView : public QWidget {
  Q_OBJECT

public:
  explicit IsolationView(QWidget *parent = nullptr);
  ~IsolationView() override = default;

private slots:
  void onLaunchClicked();

private:
  void setupUI();

  QLineEdit* m_ExecutablePath{nullptr};
  QPushButton* m_BrowseBtn{nullptr};
  QPushButton* m_LaunchBtn{nullptr};
  QTableWidget* m_ActiveSandboxesTable{nullptr};
};

} // namespace severance::gui::isolation_view
