#pragma once

#include <QWidget>
#include <QLabel>
#include <QTabWidget>
#include <QTextEdit>
#include <QString>
#include <QTreeWidget>

namespace severance::gui::timeline {

class TimelineDetailPanel : public QWidget {
  Q_OBJECT

public:
  explicit TimelineDetailPanel(QWidget* parent = nullptr);
  ~TimelineDetailPanel() override;

  void Clear();
  void LoadEvent(const QString& time, const QString& type, const QString& source, const QString& details);

private:
  void setupUI();

  QLabel* m_TitleLabel;
  QLabel* m_SubtitleLabel;

  QTabWidget* m_Tabs;
  
  // Cross-Context Tab
  QTreeWidget* m_ContextTree;
  
  // Raw Data Tab
  QTextEdit* m_RawContent;
};

} // namespace severance::gui::timeline
