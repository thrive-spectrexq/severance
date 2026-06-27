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
#include <QPushButton>

namespace severance::gui::timeline {

TimelineView::TimelineView(QWidget* parent) : QWidget(parent) {
  setupUI();

  auto* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &TimelineView::processPendingEvents);
  timer->start(200); // UI update interval

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
  
  auto createFilterBtn = [this](const QString& text) {
    auto btn = new QPushButton(text, this);
    btn->setCheckable(true);
    btn->setChecked(true); // Default all on
    btn->setStyleSheet(R"(
      QPushButton {
        background-color: #21262D;
        border: 1px solid #30363D;
        border-radius: 4px;
        color: #8B949E;
        padding: 4px 12px;
      }
      QPushButton:hover { background-color: #30363D; }
      QPushButton:checked {
        background-color: #1F3A5F;
        border-color: #58A6FF;
        color: #58A6FF;
      }
    )");
    connect(btn, &QPushButton::toggled, this, &TimelineView::onFilterToggled);
    return btn;
  };

  m_FilterProcess = createFilterBtn("Process");
  m_FilterNetwork = createFilterBtn("Network");
  m_FilterFile = createFilterBtn("File");

  topBar->addWidget(m_FilterProcess);
  topBar->addWidget(m_FilterNetwork);
  topBar->addWidget(m_FilterFile);
  topBar->addSpacing(20);

  m_SearchBox = new QLineEdit(this);
  m_SearchBox->setPlaceholderText("Filter events by keyword...");
  m_SearchBox->setMinimumWidth(300);
  connect(m_SearchBox, &QLineEdit::textChanged, this, &TimelineView::onSearchTextChanged);
  topBar->addWidget(m_SearchBox);
  
  topBar->addStretch();
  layout->addLayout(topBar);

  // Histogram
  m_Histogram = new TimelineHistogram(this);
  connect(m_Histogram, &TimelineHistogram::timeRangeSelected, this, &TimelineView::onTimeRangeSelected);
  layout->addWidget(m_Histogram);

  // Splitter
  m_Splitter = new QSplitter(Qt::Horizontal, this);
  layout->addWidget(m_Splitter, 1);

  // Table Setup (Left Side)
  auto* tableContainer = new QWidget(m_Splitter);
  auto* tableLayout = new QVBoxLayout(tableContainer);
  tableLayout->setContentsMargins(0,0,0,0);

  m_Table = new QTableWidget(tableContainer);
  m_Table->setColumnCount(4); // Added hidden timestamp column for filtering
  m_Table->setHorizontalHeaderLabels({"Timestamp", "Type", "Details", "TS_MS"});
  m_Table->horizontalHeader()->setStretchLastSection(true);
  m_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_Table->setColumnHidden(3, true); // Hide raw timestamp MS
  m_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Table->verticalHeader()->setVisible(false);
  m_Table->setShowGrid(false);

  m_Table->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_Table, &QTableWidget::customContextMenuRequested, this, &TimelineView::onContextMenuRequested);
  connect(m_Table->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TimelineView::onSelectionChanged);

  tableLayout->addWidget(m_Table);
  m_Splitter->addWidget(tableContainer);

  // Detail Panel (Right Side)
  m_DetailPanel = new TimelineDetailPanel(m_Splitter);
  m_Splitter->addWidget(m_DetailPanel);
  
  m_Splitter->setStretchFactor(0, 7); // 70% width to table
  m_Splitter->setStretchFactor(1, 3); // 30% width to detail panel
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
  if (event->GetType() == core::events::EventType::FileModified ||
      event->GetType() == core::events::EventType::FileCreated ||
      event->GetType() == core::events::EventType::FileDeleted) {
    auto fae = std::static_pointer_cast<core::events::FileActivityEvent>(event);
    if (fae) {
      return QString("PID: %1 [%2] %3 %4")
        .arg(fae->fileEvent.pid)
        .arg(QString::fromStdString(fae->fileEvent.processName))
        .arg(QString::fromStdString(fae->fileEvent.operation))
        .arg(QString::fromStdString(fae->fileEvent.filePath));
    }
  }
  return QString::fromStdString(event->GetName());
}

void TimelineView::loadInitialEvents() {
  auto events = core::store::EventStore::GetInstance().GetRecentEvents(100);
  m_Table->setUpdatesEnabled(false);
  std::vector<uint64_t> timestamps;
  
  for (const auto& ev : events) {
    int row = m_Table->rowCount();
    m_Table->insertRow(row);
    
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(ev.timestamp);
    QString timeStr = dt.toString("HH:mm:ss.zzz");
    
    m_Table->setItem(row, 0, new QTableWidgetItem(timeStr));
    m_Table->setItem(row, 1, new QTableWidgetItem(getEventTypeName(static_cast<core::events::EventType>(ev.eventType))));
    m_Table->setItem(row, 2, new QTableWidgetItem(ev.payloadJson));
    m_Table->setItem(row, 3, new QTableWidgetItem(QString::number(ev.timestamp)));
    
    timestamps.push_back(ev.timestamp);
  }
  m_Table->setUpdatesEnabled(true);
  m_Histogram->setEvents(timestamps);
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

  m_Table->setUpdatesEnabled(false);

  for (const auto& ev : toProcess) {
    QString typeStr = getEventTypeName(ev->GetType());
    QString detailsStr = formatEventPayload(ev);
    uint64_t ts = static_cast<uint64_t>(QDateTime::currentMSecsSinceEpoch());

    m_Histogram->addEvent(ts);

    m_Table->insertRow(0); // Insert at top
    
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(ts);
    QString timeStr = dt.toString("HH:mm:ss.zzz");
    
    m_Table->setItem(0, 0, new QTableWidgetItem(timeStr));
    m_Table->setItem(0, 1, new QTableWidgetItem(typeStr));
    m_Table->setItem(0, 2, new QTableWidgetItem(detailsStr));
    m_Table->setItem(0, 3, new QTableWidgetItem(QString::number(ts)));

    if (m_Table->rowCount() > m_MaxRows) {
      m_Table->removeRow(m_MaxRows);
    }
  }

  updateTableVisibility();
  m_Table->setUpdatesEnabled(true);
}

void TimelineView::updateTableVisibility() {
  QString filter = m_SearchBox->text().toLower();
  bool showProcess = m_FilterProcess->isChecked();
  bool showNetwork = m_FilterNetwork->isChecked();
  bool showFile = m_FilterFile->isChecked();

  for (int i = 0; i < m_Table->rowCount(); ++i) {
    bool visible = true;
    
    // Type filtering
    QString typeStr = m_Table->item(i, 1)->text();
    if (!showProcess && typeStr.startsWith("Process")) visible = false;
    if (!showNetwork && typeStr.startsWith("Network")) visible = false;
    if (!showFile && typeStr.startsWith("File")) visible = false;
    
    // Time filtering
    if (visible && m_FilterStartMS > 0 && m_FilterEndMS > 0) {
        uint64_t ts = m_Table->item(i, 3)->text().toULongLong();
        if (ts < m_FilterStartMS || ts > m_FilterEndMS) {
            visible = false;
        }
    }

    // Text filtering
    if (visible && !filter.isEmpty()) {
      QString detailsStr = m_Table->item(i, 2)->text();
      if (!typeStr.toLower().contains(filter) && !detailsStr.toLower().contains(filter)) {
        visible = false;
      }
    }

    m_Table->setRowHidden(i, !visible);
  }
}

void TimelineView::onSearchTextChanged(const QString& text) {
  updateTableVisibility();
}

void TimelineView::onFilterToggled() {
  updateTableVisibility();
}

void TimelineView::onTimeRangeSelected(uint64_t startMS, uint64_t endMS) {
  m_FilterStartMS = startMS;
  m_FilterEndMS = endMS;
  updateTableVisibility();
}

void TimelineView::onSelectionChanged() {
  auto items = m_Table->selectedItems();
  if (items.isEmpty()) {
    m_DetailPanel->Clear();
    return;
  }

  int row = items.first()->row();
  QString time = m_Table->item(row, 0)->text();
  QString type = m_Table->item(row, 1)->text();
  QString source = "System"; // Source is currently not a dedicated column, it's baked into details
  QString details = m_Table->item(row, 2)->text();

  m_DetailPanel->LoadEvent(time, type, source, details);
}

} // namespace severance::gui::timeline
