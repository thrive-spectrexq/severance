#include "TimelineView.hpp"
#include "core/events/EventBus.hpp"
#include "core/events/EventTypes.hpp"
#include "core/events/FileActivityEvent.hpp"
#include "core/store/EventStore.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTimer>
#include <QDateTime>

namespace severance::gui::timeline {

TimelineView::TimelineView(QWidget* parent) : QWidget(parent) {
  setupUI();

  auto* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &TimelineView::processPendingEvents);
  timer->start(200); // UI update interval

  core::events::EventBus::GetInstance().Subscribe(
      core::events::EventType::None, // Subscribe to all? EventBus doesn't support wildcard yet. 
      // We will subscribe to all individual events.
      [this](std::shared_ptr<core::events::Event> e) {
        appendEvent(e);
      });
  
  // Register specifically for events since EventBus doesn't have wildcard
  auto cb = [this](std::shared_ptr<core::events::Event> e) { appendEvent(e); };
  auto& bus = core::events::EventBus::GetInstance();
  bus.Subscribe(core::events::EventType::ProcessCreated, cb);
  bus.Subscribe(core::events::EventType::ProcessTerminated, cb);
  bus.Subscribe(core::events::EventType::NetworkConnectionOpened, cb);
  bus.Subscribe(core::events::EventType::NetworkConnectionClosed, cb);
  bus.Subscribe(core::events::EventType::FileCreated, cb);
  bus.Subscribe(core::events::EventType::FileModified, cb);
  bus.Subscribe(core::events::EventType::FileDeleted, cb);
  bus.Subscribe(core::events::EventType::AppQuit, cb);

  loadInitialEvents();
}

TimelineView::~TimelineView() = default;

void TimelineView::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(12);

  // Top Bar
  auto* topBar = new QHBoxLayout();
  
  m_EventTypeFilter = new QComboBox(this);
  m_EventTypeFilter->addItem("All Events", 0);
  m_EventTypeFilter->addItem("Process Activity", 1);
  m_EventTypeFilter->addItem("Network Activity", 2);
  m_EventTypeFilter->addItem("File Activity", 3);
  connect(m_EventTypeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TimelineView::onEventTypeFilterChanged);
  topBar->addWidget(m_EventTypeFilter);

  m_SearchBox = new QLineEdit(this);
  m_SearchBox->setPlaceholderText("Filter events by keyword...");
  m_SearchBox->setMinimumWidth(300);
  connect(m_SearchBox, &QLineEdit::textChanged, this, &TimelineView::onSearchTextChanged);
  topBar->addWidget(m_SearchBox);
  
  topBar->addStretch();
  layout->addLayout(topBar);

  // Table
  m_Table = new QTableWidget(this);
  m_Table->setColumnCount(3);
  m_Table->setHorizontalHeaderLabels({"Timestamp", "Type", "Details"});
  m_Table->horizontalHeader()->setStretchLastSection(true);
  m_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Table->verticalHeader()->setVisible(false);
  m_Table->setShowGrid(false);

  layout->addWidget(m_Table);
}

QString TimelineView::getEventTypeName(core::events::EventType type) {
  switch (type) {
    case core::events::EventType::ProcessCreated: return "Process Created";
    case core::events::EventType::ProcessTerminated: return "Process Terminated";
    case core::events::EventType::NetworkConnectionOpened: return "Network Opened";
    case core::events::EventType::NetworkConnectionClosed: return "Network Closed";
    case core::events::EventType::FileCreated: return "File Created";
    case core::events::EventType::FileModified: return "File Activity";
    case core::events::EventType::FileDeleted: return "File Deleted";
    case core::events::EventType::AppQuit: return "App Quit";
    default: return "Unknown";
  }
}

QString TimelineView::formatEventPayload(std::shared_ptr<core::events::Event> event) {
  if (event->GetType() == core::events::EventType::FileModified) {
    auto fae = std::static_pointer_cast<core::events::FileActivityEvent>(event);
    if (fae) {
      return QString("PID: %1 [%2] %3 %4")
        .arg(fae->fileEvent.pid)
        .arg(QString::fromStdString(fae->fileEvent.processName))
        .arg(QString::fromStdString(fae->fileEvent.operation))
        .arg(QString::fromStdString(fae->fileEvent.filePath));
    }
  }
  // Fallback
  return QString::fromStdString(event->GetName());
}

void TimelineView::loadInitialEvents() {
  auto events = core::store::EventStore::GetInstance().GetRecentEvents(100);
  m_Table->setUpdatesEnabled(false);
  for (const auto& ev : events) {
    int row = m_Table->rowCount();
    m_Table->insertRow(row);
    
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(ev.timestamp);
    QString timeStr = dt.toString("HH:mm:ss.zzz");
    
    m_Table->setItem(row, 0, new QTableWidgetItem(timeStr));
    m_Table->setItem(row, 1, new QTableWidgetItem(getEventTypeName(static_cast<core::events::EventType>(ev.eventType))));
    m_Table->setItem(row, 2, new QTableWidgetItem(ev.payloadJson));
  }
  m_Table->setUpdatesEnabled(true);
}

void TimelineView::appendEvent(std::shared_ptr<core::events::Event> event) {
  std::lock_guard<std::mutex> lock(m_EventsMutex);
  m_PendingEvents.push_back(event);
  if (m_PendingEvents.size() > 5000) {
    m_PendingEvents.erase(m_PendingEvents.begin(), m_PendingEvents.begin() + 1000);
  }
}

void TimelineView::processPendingEvents() {
  std::vector<std::shared_ptr<core::events::Event>> toProcess;
  {
    std::lock_guard<std::mutex> lock(m_EventsMutex);
    if (m_PendingEvents.empty()) return;
    toProcess = std::move(m_PendingEvents);
    m_PendingEvents.clear();
  }

  QString filter = m_SearchBox->text().toLower();
  int typeFilterIdx = m_EventTypeFilter->currentIndex();

  m_Table->setUpdatesEnabled(false);

  for (const auto& ev : toProcess) {
    QString typeStr = getEventTypeName(ev->GetType());
    QString detailsStr = formatEventPayload(ev);

    // Apply filters
    if (typeFilterIdx == 1) { // Process
      if (ev->GetType() != core::events::EventType::ProcessCreated && 
          ev->GetType() != core::events::EventType::ProcessTerminated) continue;
    } else if (typeFilterIdx == 2) { // Network
      if (ev->GetType() != core::events::EventType::NetworkConnectionOpened && 
          ev->GetType() != core::events::EventType::NetworkConnectionClosed) continue;
    } else if (typeFilterIdx == 3) { // File
      if (ev->GetType() != core::events::EventType::FileCreated && 
          ev->GetType() != core::events::EventType::FileModified &&
          ev->GetType() != core::events::EventType::FileDeleted) continue;
    }

    if (!filter.isEmpty()) {
      if (!typeStr.toLower().contains(filter) && !detailsStr.toLower().contains(filter)) {
        continue;
      }
    }

    m_Table->insertRow(0); // Insert at top (newest first)
    
    QDateTime dt = QDateTime::currentDateTime();
    QString timeStr = dt.toString("HH:mm:ss.zzz");
    
    m_Table->setItem(0, 0, new QTableWidgetItem(timeStr));
    m_Table->setItem(0, 1, new QTableWidgetItem(typeStr));
    m_Table->setItem(0, 2, new QTableWidgetItem(detailsStr));

    if (m_Table->rowCount() > m_MaxRows) {
      m_Table->removeRow(m_MaxRows);
    }
  }

  m_Table->setUpdatesEnabled(true);
}

void TimelineView::onSearchTextChanged(const QString& text) {
  // Clearing and re-fetching from EventStore is better, but requires implementing full filtering in DB.
  // For now, we rely on live filtering of new events.
}

void TimelineView::onEventTypeFilterChanged(int index) {
  // Same as search
}

} // namespace severance::gui::timeline
