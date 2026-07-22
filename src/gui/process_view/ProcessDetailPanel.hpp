#pragma once

#include <QWidget>
#include <QString>

namespace severance::gui::process_view {

class ProcessDetailPanel : public QWidget {
  Q_OBJECT

public:
  explicit ProcessDetailPanel(QWidget* parent = nullptr);
  ~ProcessDetailPanel() override = default;

  void Clear();
  void LoadProcess(uint32_t pid);

signals:
  void analyzeProcessRequested(uint32_t pid, const QString& processName);
};

} // namespace severance::gui::process_view
