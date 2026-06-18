#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <vector>
#include <mutex>

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
  void processPendingEvents();

private:
  void setupUI();

  QTableWidget* m_Table{nullptr};
  QLineEdit* m_SearchBox{nullptr};

  std::vector<severance::core::filesystem::FileEvent> m_PendingEvents;
  std::mutex m_EventsMutex;

  int m_MaxRows{1000};
};

} // namespace severance::gui::file_view
