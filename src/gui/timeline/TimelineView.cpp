#include "TimelineView.hpp"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QLabel>

namespace severance::gui::timeline {

TimelineView::TimelineView(QWidget* parent) : QWidget(parent) {
  setupUI();
  loadInitialEvents();
}

TimelineView::~TimelineView() = default;

void TimelineView::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(12);

  // Header
  auto* header = new QLabel("SHIFT LEDGER", this);
  header->setStyleSheet(R"(
    font-family: "Courier New", Courier, monospace;
    font-size: 24px;
    font-weight: 900;
    color: #00E5FF;
    letter-spacing: 4px;
  )");
  header->setAlignment(Qt::AlignCenter);
  layout->addWidget(header);

  // Table Setup
  m_Table = new QTableWidget(this);
  m_Table->setColumnCount(5);
  m_Table->setHorizontalHeaderLabels({"Time", "Event", "Subject", "Department", "Details"});
  m_Table->horizontalHeader()->setStretchLastSection(true);
  m_Table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Table->verticalHeader()->setVisible(false);
  m_Table->setShowGrid(false);
  
  m_Table->setStyleSheet(R"(
    QTableWidget {
      background-color: #0A0F14;
      color: #E0FFFF;
      font-family: "Courier New", Courier, monospace;
      font-size: 13px;
      border: 1px solid #1A7A5C;
      gridline-color: transparent;
    }
    QTableWidget::item:selected {
      background-color: #1A7A5C;
      color: #FFFFFF;
    }
    QHeaderView::section {
      background-color: #0A0F14;
      color: #00E5FF;
      font-weight: bold;
      border: 1px solid #1A7A5C;
    }
  )");

  layout->addWidget(m_Table);
}

void TimelineView::loadInitialEvents() {
  struct FictionalEvent {
      QString time, event, subject, department, details;
  };
  std::vector<FictionalEvent> events = {
      {"08:00:00", "SHIFT START", "All Innies", "MDR", "Morning protocol initiated"},
      {"08:02:14", "ARRIVAL", "Mark S.", "MDR", "Elevator descent confirmed"},
      {"08:02:31", "ARRIVAL", "Helly R.", "MDR", "Elevator descent confirmed"},
      {"08:03:45", "ARRIVAL", "Irving B.", "MDR", "Elevator descent confirmed"},
      {"08:04:01", "ARRIVAL", "Dylan G.", "MDR", "Elevator descent confirmed"},
      {"08:15:00", "REFINEMENT", "Mark S.", "MDR", "Began Coldwater file \xe2\x80\x94 Bin: Woe"},
      {"08:32:17", "ANOMALY", "Helly R.", "MDR", "Attempted unauthorized exploration"},
      {"08:45:00", "WELLNESS", "Ms. Casey", "Wellness", "Session scheduled for Helly R."},
      {"09:12:33", "REFINEMENT", "Dylan G.", "MDR", "Completed Siena batch \xe2\x80\x94 94% quota"},
      {"09:30:00", "BREAK ROOM", "Helly R.", "Compliance", "Statement reading initiated"},
      {"10:15:00", "OBSERVATION", "Milchick", "Management", "Floor inspection complete"},
      {"10:45:22", "REFINEMENT", "Irving B.", "MDR", "Dread bin overflow \xe2\x80\x94 recalibrating"},
      {"11:00:00", "MUSIC DANCE", "Dylan G.", "MDR", "Experience approved by Milchick"},
      {"11:30:00", "PERIMETER", "Unknown", "Security", "Motion detected \xe2\x80\x94 hallway 7-G"},
      {"12:00:00", "SHIFT END", "All Innies", "MDR", "Elevator ascent initiated"}
  };

  m_Table->setUpdatesEnabled(false);
  
  for (const auto& ev : events) {
    int row = m_Table->rowCount();
    m_Table->insertRow(row);
    
    m_Table->setItem(row, 0, new QTableWidgetItem(ev.time));
    m_Table->setItem(row, 1, new QTableWidgetItem(ev.event));
    m_Table->setItem(row, 2, new QTableWidgetItem(ev.subject));
    m_Table->setItem(row, 3, new QTableWidgetItem(ev.department));
    m_Table->setItem(row, 4, new QTableWidgetItem(ev.details));
  }
  m_Table->setUpdatesEnabled(true);
}

} // namespace severance::gui::timeline
