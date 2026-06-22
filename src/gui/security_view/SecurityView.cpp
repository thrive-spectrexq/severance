#include "SecurityView.hpp"
#include <QVBoxLayout>
#include <QSplitter>
#include <QHeaderView>
#include <QLabel>
#include <QGroupBox>

namespace severance::gui::security_view {

SecurityView::SecurityView(QWidget* parent) : QWidget(parent) {
  setupUI();

  // Register callbacks to receive data
  core::security::FimManager::GetInstance().RegisterCallback(
    [this](const core::security::FimEvent& ev) { this->onFimEvent(ev); });

  core::security::EventLogManager::GetInstance().RegisterCallback(
    [this](const core::security::WindowsEvent& ev) { this->onLogEvent(ev); });

  m_UiTimer = new QTimer(this);
  connect(m_UiTimer, &QTimer::timeout, this, &SecurityView::updateTables);
  m_UiTimer->start(500); // UI update rate
}

SecurityView::~SecurityView() = default;

void SecurityView::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);

  auto* splitter = new QSplitter(Qt::Vertical, this);

  // FIM Section
  auto* fimGroup = new QGroupBox("File Integrity Monitoring (FIM)");
  auto* fimLayout = new QVBoxLayout(fimGroup);
  m_FimTable = new QTableWidget();
  m_FimTable->setColumnCount(4);
  m_FimTable->setHorizontalHeaderLabels({"Action", "File Path", "Old Path", "SHA-256 Hash"});
  m_FimTable->horizontalHeader()->setStretchLastSection(true);
  m_FimTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_FimTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_FimTable->verticalHeader()->setVisible(false);
  m_FimTable->setShowGrid(false);
  fimLayout->addWidget(m_FimTable);
  splitter->addWidget(fimGroup);

  // Event Log Section
  auto* logGroup = new QGroupBox("Windows Event Logs (High Severity Filtered)");
  auto* logLayout = new QVBoxLayout(logGroup);
  m_LogTable = new QTableWidget();
  m_LogTable->setColumnCount(1);
  m_LogTable->setHorizontalHeaderLabels({"Raw XML Data"});
  m_LogTable->horizontalHeader()->setStretchLastSection(true);
  m_LogTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_LogTable->verticalHeader()->setVisible(false);
  m_LogTable->setShowGrid(false);
  logLayout->addWidget(m_LogTable);
  splitter->addWidget(logGroup);

  splitter->setSizes({300, 300});
  layout->addWidget(splitter);
}

void SecurityView::onFimEvent(const core::security::FimEvent& ev) {
  QMutexLocker lock(&m_QueueMutex);
  m_PendingFimEvents.push_back(ev);
}

void SecurityView::onLogEvent(const core::security::WindowsEvent& ev) {
  // VERY basic filtering for now
  // We check the XML for string indicators of high severity
  if (ev.xmlData.find("Level>2</Level>") != std::string::npos || // Error
      ev.xmlData.find("Level>1</Level>") != std::string::npos || // Critical
      ev.xmlData.find("Level>3</Level>") != std::string::npos || // Warning
      ev.xmlData.find("EventID>4624</EventID>") != std::string::npos || // Logon
      ev.xmlData.find("EventID>4625</EventID>") != std::string::npos) { // Failed Logon

    QMutexLocker lock(&m_QueueMutex);
    m_PendingLogEvents.push_back(ev);
  }
}

void SecurityView::updateTables() {
  std::vector<core::security::FimEvent> fimBatch;
  std::vector<core::security::WindowsEvent> logBatch;

  {
    QMutexLocker lock(&m_QueueMutex);
    fimBatch.swap(m_PendingFimEvents);
    logBatch.swap(m_PendingLogEvents);
  }

  // Update FIM Table
  if (!fimBatch.empty()) {
    m_FimTable->setUpdatesEnabled(false);
    for (const auto& ev : fimBatch) {
      int row = 0; // Insert at top
      m_FimTable->insertRow(row);
      
      QString actionStr;
      switch (ev.action) {
        case core::security::FimEvent::Action::Added: actionStr = "Added"; break;
        case core::security::FimEvent::Action::Modified: actionStr = "Modified"; break;
        case core::security::FimEvent::Action::Removed: actionStr = "Removed"; break;
        case core::security::FimEvent::Action::Renamed: actionStr = "Renamed"; break;
      }

      m_FimTable->setItem(row, 0, new QTableWidgetItem(actionStr));
      m_FimTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(ev.filePath)));
      m_FimTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(ev.oldFilePath)));
      m_FimTable->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(ev.fileHash)));
    }
    // Limit to 1000 rows
    while (m_FimTable->rowCount() > 1000) m_FimTable->removeRow(1000);
    m_FimTable->setUpdatesEnabled(true);
  }

  // Update Log Table
  if (!logBatch.empty()) {
    m_LogTable->setUpdatesEnabled(false);
    for (const auto& ev : logBatch) {
      int row = 0;
      m_LogTable->insertRow(row);
      
      m_LogTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(ev.xmlData)));
    }
    // Limit to 1000 rows
    while (m_LogTable->rowCount() > 1000) m_LogTable->removeRow(1000);
    m_LogTable->setUpdatesEnabled(true);
  }
}

} // namespace severance::gui::security_view
