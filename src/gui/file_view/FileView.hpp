#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QTimer>
#include <QLineEdit>
#include <QSplitter>
#include <vector>
#include <mutex>
#include "FileDetailPanel.hpp"

namespace severance::core::filesystem {
  struct FileEvent;
}

namespace severance::gui::file_view {

class FileView : public QWidget {
  Q_OBJECT

public:
  explicit FileView(QWidget* parent = nullptr);
  ~FileView() override;

  void appendEvent(const severance::core::filesystem::FileEvent& event);

private slots:
  void onSearchTextChanged(const QString& text);
  void onContextMenuRequested(const QPoint& pos);
  void onSelectionChanged();
  void processPendingEvents();

private:
  void setupUI();

  QTimer* m_RefreshTimer;
  QLineEdit* m_SearchBox;
  QTableWidget* m_Table;
  QSplitter* m_Splitter;
  FileDetailPanel* m_DetailPanel;

  std::vector<severance::core::filesystem::FileEvent> m_PendingEvents;
  std::mutex m_EventsMutex;

  int m_MaxRows{1000};
};

} // namespace severance::gui::file_view
