#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QSplitter>
#include <QTimer>
#include <vector>
#include <mutex>
#include <memory>
#include "TimelineHistogram.hpp"
#include "TimelineDetailPanel.hpp"
#include "core/events/Event.hpp"
#include "core/events/EventTypes.hpp"

namespace severance::gui::timeline {

class TimelineView : public QWidget {
  Q_OBJECT

public:
  explicit TimelineView(QWidget* parent = nullptr);
  ~TimelineView() override;

  void appendEvent(std::shared_ptr<core::events::Event> event);

private slots:
  void onSearchTextChanged(const QString& text);
  void onFilterToggled();
  void onTimeRangeSelected(uint64_t startMS, uint64_t endMS);
  void processPendingEvents();
  void onContextMenuRequested(const QPoint& pos);
  void onSelectionChanged();

private:
  void setupUI();
  void loadInitialEvents();
  QString formatEventPayload(std::shared_ptr<core::events::Event> event);
  QString getEventTypeName(core::events::EventType type);
  void updateTableVisibility();

  QTableWidget* m_Table{nullptr};
  QLineEdit* m_SearchBox{nullptr};
  
  QPushButton* m_FilterProcess{nullptr};
  QPushButton* m_FilterNetwork{nullptr};
  QPushButton* m_FilterFile{nullptr};

  TimelineHistogram* m_Histogram{nullptr};
  QSplitter* m_Splitter{nullptr};
  TimelineDetailPanel* m_DetailPanel{nullptr};

  std::vector<std::shared_ptr<core::events::Event>> m_PendingEvents;
  std::mutex m_EventsMutex;

  int m_MaxRows{5000};
  uint64_t m_FilterStartMS{0};
  uint64_t m_FilterEndMS{0};
};

} // namespace severance::gui::timeline
