#include "TimelineView.hpp"
#include "core/store/EventStore.hpp"
#include "core/events/MacrodataRefinedEvent.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>
#include <QRandomGenerator>

namespace severance::gui::timeline {

TimelineView::TimelineView(QWidget* parent) : QWidget(parent) {
  setupUI();
  loadInitialEvents();

  // Periodically add live floor telemetry events to the ledger
  m_LiveTimer = new QTimer(this);
  connect(m_LiveTimer, &QTimer::timeout, this, [this]() {
    static const QStringList subjects = {"Mark S.", "Helly R.", "Irving B.", "Dylan G.", "Ms. Casey", "Milchick"};
    static const QStringList depts = {"MDR", "O&D", "Wellness", "Security", "Management"};
    static const QStringList eventTypes = {"REFINEMENT", "TELEMETRY", "PERIMETER", "CHECK-IN", "MONITOR"};
    static const QStringList detailMsgs = {
      "Processed number cluster — 98% accuracy",
      "Perimeter door sensor ping logged",
      "Biometric pulse nominal",
      "Elevator cabin transit logged",
      "Handbook compliance verified"
    };

    auto rng = QRandomGenerator::global();
    if (rng->bounded(100) < 35) {
      QString t = QDateTime::currentDateTime().toString("HH:mm:ss");
      QString ev = eventTypes.at(rng->bounded(eventTypes.size()));
      QString subj = subjects.at(rng->bounded(subjects.size()));
      QString dept = depts.at(rng->bounded(depts.size()));
      QString det = detailMsgs.at(rng->bounded(detailMsgs.size()));
      addShiftEvent(t, ev, subj, dept, det);
    }
  });
  m_LiveTimer->start(4000); // Check every 4 seconds
}

TimelineView::~TimelineView() = default;

void TimelineView::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(12);

  // Header
  auto* header = new QLabel("TEMPORAL LEDGER — SHIFT RECORD", this);
  header->setStyleSheet(R"(
    font-family: "Courier New", Courier, monospace;
    font-size: 24px;
    font-weight: 900;
    color: #00E5FF;
    letter-spacing: 4px;
  )");
  header->setAlignment(Qt::AlignCenter);
  layout->addWidget(header);

  // Top control bar (Search + Export)
  auto* controlLayout = new QHBoxLayout();
  
  m_SearchInput = new QLineEdit(this);
  m_SearchInput->setPlaceholderText("Filter ledger by time, event, subject, department, or details...");
  m_SearchInput->setStyleSheet("QLineEdit { background-color: #061018; color: #00E5FF; border: 1px solid #1A5C4A; padding: 6px; font-size: 13px; font-family: monospace; }");
  connect(m_SearchInput, &QLineEdit::textChanged, this, &TimelineView::onSearchTextChanged);
  controlLayout->addWidget(m_SearchInput, 1);

  auto* exportBtn = new QPushButton("[ EXPORT SHIFT LEDGER ]", this);
  exportBtn->setStyleSheet(R"(
    QPushButton {
      background-color: #161B22;
      color: #39FF14;
      border: 1px solid #30363D;
      padding: 6px 16px;
      font-family: monospace;
      font-weight: bold;
    }
    QPushButton:hover {
      background-color: #39FF14;
      color: #0D1117;
    }
  )");
  connect(exportBtn, &QPushButton::clicked, this, &TimelineView::onExportLedger);
  controlLayout->addWidget(exportBtn);

  layout->addLayout(controlLayout);

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

void TimelineView::addShiftEvent(const QString& time, const QString& eventType, const QString& subject, const QString& department, const QString& details) {
  int row = m_Table->rowCount();
  m_Table->insertRow(row);
  
  m_Table->setItem(row, 0, new QTableWidgetItem(time));
  
  auto* eventItem = new QTableWidgetItem(eventType);
  if (eventType == "ANOMALY" || eventType == "BREAK ROOM") {
    eventItem->setForeground(QColor("#FF0055"));
  } else if (eventType == "SHIFT START" || eventType == "SHIFT END") {
    eventItem->setForeground(QColor("#00E5FF"));
  } else if (eventType == "REFINEMENT" || eventType == "MUSIC DANCE") {
    eventItem->setForeground(QColor("#39FF14"));
  }
  m_Table->setItem(row, 1, eventItem);

  m_Table->setItem(row, 2, new QTableWidgetItem(subject));
  m_Table->setItem(row, 3, new QTableWidgetItem(department));
  m_Table->setItem(row, 4, new QTableWidgetItem(details));

  // Apply search filter if active
  if (m_SearchInput && !m_SearchInput->text().isEmpty()) {
    onSearchTextChanged(m_SearchInput->text());
  }

  m_Table->scrollToBottom();
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
      {"08:15:00", "REFINEMENT", "Mark S.", "MDR", "Began Coldwater file — Bin: Woe"},
      {"08:32:17", "ANOMALY", "Helly R.", "MDR", "Attempted unauthorized exploration"},
      {"08:45:00", "WELLNESS", "Ms. Casey", "Wellness", "Session scheduled for Helly R."},
      {"09:12:33", "REFINEMENT", "Dylan G.", "MDR", "Completed Siena batch — 94% quota"},
      {"09:30:00", "BREAK ROOM", "Helly R.", "Compliance", "Statement reading initiated"},
      {"10:15:00", "OBSERVATION", "Milchick", "Management", "Floor inspection complete"},
      {"10:45:22", "REFINEMENT", "Irving B.", "MDR", "Dread bin overflow — recalibrating"},
      {"11:00:00", "MUSIC DANCE", "Dylan G.", "MDR", "Experience approved by Milchick"},
      {"11:30:00", "PERIMETER", "Unknown", "Security", "Motion detected — hallway 7-G"},
      {"12:00:00", "SHIFT END", "All Innies", "MDR", "Elevator ascent initiated"}
  };

  m_Table->setUpdatesEnabled(false);
  for (const auto& ev : events) {
    addShiftEvent(ev.time, ev.event, ev.subject, ev.department, ev.details);
  }
  m_Table->setUpdatesEnabled(true);
}

void TimelineView::onSearchTextChanged(const QString& text) {
  QString searchStr = text.trimmed().toLower();
  for (int r = 0; r < m_Table->rowCount(); ++r) {
    bool match = searchStr.isEmpty();
    if (!match) {
      for (int c = 0; c < m_Table->columnCount(); ++c) {
        if (m_Table->item(r, c)->text().toLower().contains(searchStr)) {
          match = true;
          break;
        }
      }
    }
    m_Table->setRowHidden(r, !match);
  }
}

void TimelineView::onExportLedger() {
  QString fileName = QFileDialog::getSaveFileName(this, "Export Temporal Shift Ledger", "shift_ledger.txt", "Text Files (*.txt);;All Files (*)");
  if (fileName.isEmpty()) {
    return;
  }

  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Export Failed", "Unable to open file for writing.");
    return;
  }

  QTextStream out(&file);
  out << "========================================================\n";
  out << "        LUMON INDUSTRIES — TEMPORAL SHIFT LEDGER       \n";
  out << "========================================================\n\n";

  for (int r = 0; r < m_Table->rowCount(); ++r) {
    out << QString("[%1]  %-12s  %-10s  %-10s  %s\n")
           .arg(m_Table->item(r, 0)->text())
           .arg(m_Table->item(r, 1)->text())
           .arg(m_Table->item(r, 2)->text())
           .arg(m_Table->item(r, 3)->text())
           .arg(m_Table->item(r, 4)->text());
  }

  file.close();
  QMessageBox::information(this, "Ledger Exported", QString("Shift ledger successfully filed to:\n%1").arg(fileName));
}

} // namespace severance::gui::timeline
