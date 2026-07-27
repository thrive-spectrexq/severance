#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>

namespace severance::core::filesystem {
  struct FileEvent {
      int dummy;
  };
}

namespace severance::gui::file_view {

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
  void onDocumentDoubleClicked(int row, int column);
  void filterByClassification(const QString& classification);

private:
  void setupUI();
  void openDocumentViewer(int row);

  QTableWidget* m_Table{nullptr};
  QLineEdit* m_SearchInput{nullptr};
  QString m_CurrentFilter{"ALL"};
};

} // namespace severance::gui::file_view
