#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QTableWidget>
#include <QLabel>

namespace severance::gui::process_view {

class ProcessDetailPanel : public QWidget {
  Q_OBJECT

public:
  explicit ProcessDetailPanel(QWidget* parent = nullptr);
  ~ProcessDetailPanel() override;

  void Clear();
  void LoadProcess(uint32_t pid);

signals:
  void analyzeProcessRequested(uint32_t pid, const QString& processName);

private:
  void setupUI();

  QLabel* m_TitleLabel{nullptr};
  QLabel* m_PidLabel{nullptr};
  QPushButton* m_AiAnalyzeBtn{nullptr};
  QTabWidget* m_Tabs{nullptr};
  
  uint32_t m_CurrentPid{0};
  
  // Overview Tab
  QLabel* m_OverviewContent{nullptr};
  
  // Files Tab
  QTableWidget* m_FilesTable{nullptr};
  
  // Network Tab
  QTableWidget* m_NetworkTable{nullptr};
};

} // namespace severance::gui::process_view
