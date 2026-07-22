#pragma once

#include <QWidget>
#include <QTableWidget>

namespace severance::core::filesystem {
  struct FileEvent {
      int dummy;
  };
}

namespace severance::gui::file_view {

class FileDetailPanel;

class FileView : public QWidget {
  Q_OBJECT

public:
  explicit FileView(QWidget* parent = nullptr);
  ~FileView() override = default;

  void appendEvent(const severance::core::filesystem::FileEvent& event);

private slots:
  void onSearchTextChanged(const QString& text);
  void onContextMenuRequested(const QPoint& pos);
  void onSelectionChanged();
  void processPendingEvents();
  void updateReceptors();

private:
  void setupUI();

  QTableWidget* m_Table{nullptr};
};

} // namespace severance::gui::file_view
