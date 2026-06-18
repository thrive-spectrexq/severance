#include "gui/file_view/FileView.hpp"
#include "core/filesystem/EtwMonitor.hpp"
#include "core/events/EventBus.hpp"
#include "core/events/FileActivityEvent.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTimer>

namespace severance::gui::file_view {

FileView::FileView(QWidget* parent) : QWidget(parent) {
  setupUI();

  // Polling timer to batch UI updates
  auto* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &FileView::processPendingEvents);
  timer->start(100); // 100ms UI update interval

  core::events::EventBus::GetInstance().Subscribe(
      core::events::EventType::FileModified,
      [this](std::shared_ptr<core::events::Event> e) {
        auto fe = std::static_pointer_cast<core::events::FileActivityEvent>(e);
        if (fe) {
          appendEvent(fe->fileEvent);
        }
      });
}

FileView::~FileView() = default;

void FileView::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(12);

  // Top Bar
  auto* topBar = new QHBoxLayout();
  m_SearchBox = new QLineEdit(this);
  m_SearchBox->setPlaceholderText("Filter events by process or path...");
  m_SearchBox->setMinimumWidth(300);
  connect(m_SearchBox, &QLineEdit::textChanged, this, &FileView::onSearchTextChanged);
  topBar->addWidget(m_SearchBox);
  topBar->addStretch();
  layout->addLayout(topBar);

  // Table
  m_Table = new QTableWidget(this);
  m_Table->setColumnCount(4);
  m_Table->setHorizontalHeaderLabels({"Process", "PID", "Operation", "Path"});
  m_Table->horizontalHeader()->setStretchLastSection(true);
  m_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_Table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  m_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Table->verticalHeader()->setVisible(false);
  m_Table->setShowGrid(false);

  layout->addWidget(m_Table);
}

void FileView::appendEvent(const core::filesystem::FileEvent& event) {
  std::lock_guard<std::mutex> lock(m_EventsMutex);
  m_PendingEvents.push_back(event);
  
  // Prevent unbounded memory growth if UI is not processing
  if (m_PendingEvents.size() > 5000) {
    m_PendingEvents.erase(m_PendingEvents.begin(), m_PendingEvents.begin() + 1000);
  }
}

void FileView::processPendingEvents() {
  std::vector<core::filesystem::FileEvent> toProcess;
  {
    std::lock_guard<std::mutex> lock(m_EventsMutex);
    if (m_PendingEvents.empty()) return;
    toProcess = std::move(m_PendingEvents);
    m_PendingEvents.clear();
  }

  QString filter = m_SearchBox->text().toLower();

  m_Table->setUpdatesEnabled(false);

  for (const auto& ev : toProcess) {
    QString procName = QString::fromStdString(ev.processName);
    QString path = QString::fromStdString(ev.filePath);

    if (!filter.isEmpty()) {
      bool match = procName.toLower().contains(filter) ||
                   QString::number(ev.pid).contains(filter) ||
                   path.toLower().contains(filter);
      if (!match) continue;
    }

    m_Table->insertRow(0); // Insert at top

    m_Table->setItem(0, 0, new QTableWidgetItem(procName));
    m_Table->setItem(0, 1, new QTableWidgetItem(QString::number(ev.pid)));
    m_Table->setItem(0, 2, new QTableWidgetItem(QString::fromStdString(ev.operation)));
    m_Table->setItem(0, 3, new QTableWidgetItem(path));

    if (m_Table->rowCount() > m_MaxRows) {
      m_Table->removeRow(m_MaxRows);
    }
  }

  m_Table->setUpdatesEnabled(true);
}

void FileView::onSearchTextChanged(const QString& text) {
  // Real-time filtering would require keeping all events.
  // For a streaming log, we just apply the filter to incoming events,
  // or clear the table to start fresh.
  m_Table->setRowCount(0); 
}

} // namespace severance::gui::file_view
