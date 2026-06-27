#pragma once

#include <QWidget>
#include <QLabel>
#include <QTabWidget>
#include <QTextEdit>
#include <QString>

namespace severance::gui::network_view {

class NetworkDetailPanel : public QWidget {
  Q_OBJECT

public:
  explicit NetworkDetailPanel(QWidget* parent = nullptr);
  ~NetworkDetailPanel() override;

  void Clear();
  void LoadConnection(uint32_t pid, const QString& procName, const QString& localIp, uint16_t localPort, const QString& remoteIp, uint16_t remotePort, const QString& state);

private:
  void setupUI();

  QLabel* m_TitleLabel;
  QLabel* m_SubtitleLabel;

  QTabWidget* m_Tabs;
  
  // Overview Tab
  QTextEdit* m_OverviewContent;
  
  // Traffic Graph Tab (Placeholder)
  QLabel* m_GraphPlaceholder;
  
  // DNS Tab (Placeholder)
  QTextEdit* m_DnsContent;
};

} // namespace severance::gui::network_view
