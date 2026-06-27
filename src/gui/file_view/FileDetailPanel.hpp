#pragma once

#include <QWidget>
#include <QLabel>
#include <QTabWidget>
#include <QTextEdit>
#include <QString>

namespace severance::gui::file_view {

class FileDetailPanel : public QWidget {
  Q_OBJECT

public:
  explicit FileDetailPanel(QWidget* parent = nullptr);
  ~FileDetailPanel() override;

  void Clear();
  void LoadFileEvent(const QString& time, const QString& procName, uint32_t pid, const QString& operation, const QString& path);

private:
  void setupUI();

  QLabel* m_TitleLabel;
  QLabel* m_SubtitleLabel;

  QTabWidget* m_Tabs;
  
  // Overview Tab
  QTextEdit* m_OverviewContent;
  
  // Diff Tab (Placeholder)
  QTextEdit* m_DiffContent;
  
  // Bulk Write Tab (Placeholder)
  QLabel* m_BulkWriteLbl;
};

} // namespace severance::gui::file_view
