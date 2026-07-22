#pragma once

#include <QWidget>
#include <QString>

namespace severance::gui::file_view {

class FileDetailPanel : public QWidget {
  Q_OBJECT

public:
  explicit FileDetailPanel(QWidget* parent = nullptr);
  ~FileDetailPanel() override = default;

  void Clear();
  void LoadFileEvent(const QString& time, const QString& procName, uint32_t pid, const QString& operation, const QString& path);

private:
  void setupUI();
};

} // namespace severance::gui::file_view
