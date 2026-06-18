#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <vector>
#include <mutex>
#include <memory>
#include "core/events/Event.hpp"

namespace severance::gui::timeline {

class TimelineView : public QWidget {
  Q_OBJECT

public:
  explicit TimelineView(QWidget* parent = nullptr);
  ~TimelineView() override;

  void appendEvent(std::shared_ptr<core::events::Event> event);

private slots:
  void onSearchTextChanged(const QString& text);
  void onEventTypeFilterChanged(int index);
  void processPendingEvents();

private:
  void setupUI();
  void loadInitialEvents();
  QString formatEventPayload(std::shared_ptr<core::events::Event> event);
  QString getEventTypeName(core::events::EventType type);

  QTableWidget* m_Table{nullptr};
  QLineEdit* m_SearchBox{nullptr};
  QComboBox* m_EventTypeFilter{nullptr};

  std::vector<std::shared_ptr<core::events::Event>> m_PendingEvents;
  std::mutex m_EventsMutex;

  int m_MaxRows{5000};
};

} // namespace severance::gui::timeline
