#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>

namespace severance::gui::timeline {

class TimelineView : public QWidget {
  Q_OBJECT

public:
  explicit TimelineView(QWidget* parent = nullptr);
  ~TimelineView() override;

private:
  void setupUI();
  void loadInitialEvents();

  QTableWidget* m_Table{nullptr};
};

} // namespace severance::gui::timeline
