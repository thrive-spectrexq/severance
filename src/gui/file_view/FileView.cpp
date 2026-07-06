#include "gui/file_view/FileView.hpp"
#include "core/filesystem/EtwMonitor.hpp"
#include "core/events/EventBus.hpp"
#include "core/events/FileActivityEvent.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTimer>
#include <QTime>
#include <QLabel>
#include <QPropertyAnimation>
#include <QRandomGenerator>

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

  m_ReceptorTimer = new QTimer(this);
  connect(m_ReceptorTimer, &QTimer::timeout, this, &FileView::updateReceptors);
  m_ReceptorTimer->start(100);
}

FileView::~FileView() = default;

void FileView::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(12);
  this->setStyleSheet("background-color: #050A0F; color: #00FFD1; font-family: 'Courier New';");

  // Top Bar Layout
  auto* topBar = new QVBoxLayout();

  // Title & Receptors
  auto* titleLayout = new QHBoxLayout();
  m_TitleLabel = new QLabel("MACRODATA ARCHIVAL SCANNER", this);
  m_TitleLabel->setStyleSheet("font-size: 24px; font-weight: bold; letter-spacing: 4px; color: #00FFD1;");
  
  m_ReceptorLabel1 = new QLabel("REC_A: 0x0000", this);
  m_ReceptorLabel2 = new QLabel("REC_B: 0x0000", this);
  QString recStyle = "font-size: 14px; color: #007A65; background-color: #021814; border: 1px solid #004D40; padding: 4px;";
  m_ReceptorLabel1->setStyleSheet(recStyle);
  m_ReceptorLabel2->setStyleSheet(recStyle);

  titleLayout->addWidget(m_TitleLabel);
  titleLayout->addStretch();
  titleLayout->addWidget(m_ReceptorLabel1);
  titleLayout->addWidget(m_ReceptorLabel2);

  // Scanner Line Container
  auto* scannerContainer = new QWidget(this);
  scannerContainer->setFixedHeight(4);
  scannerContainer->setStyleSheet("background-color: #021814; border: none;");
  m_ScannerLine = new QWidget(scannerContainer);
  m_ScannerLine->setFixedSize(200, 4);
  m_ScannerLine->setStyleSheet("background-color: #00FFD1; border-radius: 2px;");

  m_ScannerAnim = new QPropertyAnimation(m_ScannerLine, "pos", this);
  m_ScannerAnim->setDuration(2500);
  m_ScannerAnim->setStartValue(QPoint(0, 0));
  m_ScannerAnim->setEndValue(QPoint(1200, 0)); // Approx width
  m_ScannerAnim->setEasingCurve(QEasingCurve::InOutSine);
  m_ScannerAnim->setLoopCount(-1);
  m_ScannerAnim->start();

  topBar->addLayout(titleLayout);
  topBar->addWidget(scannerContainer);

  // Search
  auto* searchLayout = new QHBoxLayout();
  m_SearchBox = new QLineEdit(this);
  m_SearchBox->setPlaceholderText("Filter surveillance by procedure or designation...");
  m_SearchBox->setMinimumWidth(400);
  m_SearchBox->setStyleSheet("background-color: #021814; color: #00FFD1; border: 1px solid #004D40; padding: 6px;");
  connect(m_SearchBox, &QLineEdit::textChanged, this, &FileView::onSearchTextChanged);
  searchLayout->addWidget(m_SearchBox);
  searchLayout->addStretch();
  
  topBar->addLayout(searchLayout);
  layout->addLayout(topBar);

  // Splitter
  m_Splitter = new QSplitter(Qt::Horizontal, this);
  layout->addWidget(m_Splitter, 1);

  // Table Setup (Left Side)
  auto* tableContainer = new QWidget(m_Splitter);
  auto* tableLayout = new QVBoxLayout(tableContainer);
  tableLayout->setContentsMargins(0,0,0,0);

  m_Table = new QTableWidget(tableContainer);
  m_Table->setColumnCount(5);
  m_Table->setHorizontalHeaderLabels({"TIMESTAMP", "PROCEDURE", "ID", "OPERATION", "DESIGNATION"});
  m_Table->horizontalHeader()->setStretchLastSection(true);
  m_Table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_Table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_Table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
  m_Table->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_Table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_Table->verticalHeader()->setVisible(false);
  m_Table->setShowGrid(true);
  
  m_Table->setStyleSheet(
      "QTableWidget { background-color: #050A0F; color: #00FFD1; gridline-color: #004D40; border: 1px solid #004D40; }"
      "QHeaderView::section { background-color: #021814; color: #00FFD1; padding: 4px; border: 1px solid #004D40; font-weight: bold; text-transform: uppercase; }"
      "QTableWidget::item:selected { background-color: #004D40; color: #FFFFFF; }"
      "QScrollBar:vertical { background: #021814; width: 12px; }"
      "QScrollBar::handle:vertical { background: #004D40; border-radius: 6px; }"
  );

  m_Table->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_Table, &QTableWidget::customContextMenuRequested, this, &FileView::onContextMenuRequested);
  connect(m_Table->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FileView::onSelectionChanged);

  tableLayout->addWidget(m_Table);
  m_Splitter->addWidget(tableContainer);

  // Detail Panel (Right Side)
  m_DetailPanel = new FileDetailPanel(m_Splitter);
  m_Splitter->addWidget(m_DetailPanel);
  
  m_Splitter->setStretchFactor(0, 7); // 70% width to table
  m_Splitter->setStretchFactor(1, 3); // 30% width to detail panel
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

    QTime now = QTime::currentTime();
    m_Table->setItem(0, 0, new QTableWidgetItem(now.toString("HH:mm:ss")));
    m_Table->setItem(0, 1, new QTableWidgetItem(procName));
    m_Table->setItem(0, 2, new QTableWidgetItem(QString::number(ev.pid)));
    m_Table->setItem(0, 3, new QTableWidgetItem(QString::fromStdString(ev.operation)));
    m_Table->setItem(0, 4, new QTableWidgetItem(path));

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

void FileView::onSelectionChanged() {
  auto items = m_Table->selectedItems();
  if (items.isEmpty()) {
    m_DetailPanel->Clear();
    return;
  }

  int row = items.first()->row();
  QString time = m_Table->item(row, 0)->text();
  QString procName = m_Table->item(row, 1)->text();
  uint32_t pid = m_Table->item(row, 2)->text().toUInt();
  QString operation = m_Table->item(row, 3)->text();
  QString path = m_Table->item(row, 4)->text();

  m_DetailPanel->LoadFileEvent(time, procName, pid, operation, path);
}

void FileView::onContextMenuRequested(const QPoint& pos) {
  // TODO: Implement context menu for FileView
}

void FileView::updateReceptors() {
  uint32_t val1 = QRandomGenerator::global()->generate() % 0xFFFF;
  uint32_t val2 = QRandomGenerator::global()->generate() % 0xFFFF;
  m_ReceptorLabel1->setText(QString("ENCRYPT_ST_1: 0x%1").arg(val1, 4, 16, QChar('0')).toUpper());
  m_ReceptorLabel2->setText(QString("ENCRYPT_ST_2: 0x%1").arg(val2, 4, 16, QChar('0')).toUpper());
}

} // namespace severance::gui::file_view
