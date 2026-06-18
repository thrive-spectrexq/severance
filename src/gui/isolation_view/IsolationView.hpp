#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QLineEdit>
#include <memory>
#include "core/sandbox/SandboxManager.hpp"

namespace severance::gui::isolation_view {

class IsolationView : public QWidget {
  Q_OBJECT

public:
  explicit IsolationView(QWidget *parent = nullptr);
  ~IsolationView() override = default;

private slots:
  void onLaunchClicked();
  void onTerminateClicked(int row);

private:
  void setupUI();

  QLineEdit* m_ExecutablePath{nullptr};
  QPushButton* m_BrowseBtn{nullptr};
  QPushButton* m_LaunchBtn{nullptr};
  QTableWidget* m_ActiveSandboxesTable{nullptr};

  std::unique_ptr<core::sandbox::SandboxManager> m_SandboxManager;
};

} // namespace severance::gui::isolation_view
