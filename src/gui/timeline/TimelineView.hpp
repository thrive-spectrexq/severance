#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QHeaderView>

namespace severance::gui::timeline {

class TimelineView : public QWidget {
  Q_OBJECT

public:
  explicit TimelineView(QWidget* parent = nullptr);
  ~TimelineView() override;

  void addShiftEvent(const QString& time, const QString& eventType, const QString& subject, const QString& department, const QString& details);

private slots:
  void onSearchTextChanged(const QString& text);
  void onExportLedger();

private:
  void setupUI();
  void loadInitialEvents();

  QTableWidget* m_Table{nullptr};
  QLineEdit* m_SearchInput{nullptr};
  QTimer* m_LiveTimer{nullptr};
};

} // namespace severance::gui::timeline
