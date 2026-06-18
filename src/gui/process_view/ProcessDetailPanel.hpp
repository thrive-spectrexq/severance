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

  void LoadProcess(uint32_t pid);
  void Clear();

private:
  void setupUI();

  QLabel* m_TitleLabel{nullptr};
  QLabel* m_PidLabel{nullptr};
  QTabWidget* m_Tabs{nullptr};
  
  // Overview Tab
  QLabel* m_OverviewContent{nullptr};
  
  // Files Tab
  QTableWidget* m_FilesTable{nullptr};
  
  // Network Tab
  QTableWidget* m_NetworkTable{nullptr};
};

} // namespace severance::gui::process_view
