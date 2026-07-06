#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <memory>
#include "core/sandbox/SandboxManager.hpp"

class QPropertyAnimation;
class QGraphicsColorizeEffect;

namespace severance::gui::isolation_view {

class IsolationView : public QWidget {
  Q_OBJECT

public:
  explicit IsolationView(QWidget *parent = nullptr);
  ~IsolationView() override = default;

private slots:
  void onLaunchClicked();
  void onTerminateClicked(int row);
  void onActiveSandboxClicked(int row, int column);
  void onOvertimeContingencyClicked();

private:
  void setupUI();
  void triggerOvertimeAlarm();

  QLineEdit* m_ExecutablePath{nullptr};
  QPushButton* m_BrowseBtn{nullptr};
  QPushButton* m_LaunchBtn{nullptr};
  QComboBox* m_ProfileCombo{nullptr};
  QSpinBox* m_MemSpin{nullptr};
  QSpinBox* m_CpuSpin{nullptr};
  QTableWidget* m_ActiveSandboxesTable{nullptr};

  std::unique_ptr<core::sandbox::SandboxManager> m_SandboxManager;

  // Security Analysis Pane
  QWidget* m_AnalysisPane{nullptr};
  QLabel*  m_AnalysisTitle{nullptr};
  QLabel*  m_AnalysisDetails{nullptr};

  // Overtime Contingency
  QPushButton* m_OvertimeBtn{nullptr};
  QGraphicsColorizeEffect* m_AlarmEffect{nullptr};
  QPropertyAnimation* m_AlarmAnimation{nullptr};
};

} // namespace severance::gui::isolation_view
