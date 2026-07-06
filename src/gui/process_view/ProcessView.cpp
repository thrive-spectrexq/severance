#include "ProcessView.hpp"
#include "ProcessTreeModel.hpp"
#include "ProcessDetailPanel.hpp"
#include "core/process/ProcessManager.hpp"
#include <QVBoxLayout>
#include <QSplitter>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>
#include <QGraphicsDropShadowEffect>
#include <QRandomGenerator>
#include <algorithm>

namespace severance::gui::process_view {

ProcessView::ProcessView(QWidget *parent) : QWidget(parent) {
  setupUI();

  // Initial data load
  refreshProcessList();

  // Start auto-refresh timer
  m_RefreshTimer = new QTimer(this);
  connect(m_RefreshTimer, &QTimer::timeout, this, &ProcessView::onRefreshTimer);
  m_RefreshTimer->start(m_RefreshIntervalMs);

  // Start Sync Anim timer
  m_SyncAnimTimer = new QTimer(this);
  connect(m_SyncAnimTimer, &QTimer::timeout, this, [this]() {
    if (m_InnieSyncBar) {
      int val = m_InnieSyncBar->value() + QRandomGenerator::global()->bounded(-5, 6);
      m_InnieSyncBar->setValue(std::max(70, std::min(100, val)));
    }
    if (m_OuttieSyncBar) {
      int val = m_OuttieSyncBar->value() + QRandomGenerator::global()->bounded(-2, 3);
      m_OuttieSyncBar->setValue(std::max(5, std::min(20, val)));
    }
  });
  m_SyncAnimTimer->start(150);
}

void ProcessView::setupUI() {
  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(16, 12, 16, 0);
  mainLayout->setSpacing(8);

  // Apply global dark teal theme for the widget
  this->setStyleSheet(R"(
    QWidget {
      background-color: #0A0F14;
      color: #00E5FF;
      font-family: "Consolas", "Courier New", monospace;
    }
    QLabel {
      color: #00E5FF;
    }
    QLineEdit {
      background-color: #05080A;
      border: 1px solid #005F73;
      color: #E0FFFF;
      padding: 4px 8px;
    }
    QLineEdit:focus {
      border: 1px solid #00E5FF;
    }
    QPushButton {
      background-color: #003B46;
      border: 1px solid #005F73;
      color: #00E5FF;
      font-weight: bold;
      padding: 4px 12px;
    }
    QPushButton:hover {
      background-color: #005F73;
      border-color: #00E5FF;
    }
    QPushButton:disabled {
      background-color: #021B21;
      color: #004A55;
      border-color: #002228;
    }
    QTreeView {
      background-color: #05080A;
      alternate-background-color: #080D11;
      border: 1px solid #003B46;
      color: #E0FFFF;
      selection-background-color: #005F73;
      selection-color: #FFFFFF;
      outline: none;
    }
    QTreeView::item:hover {
      background-color: #004554;
    }
    QTreeView::item:selected {
      background-color: #008C9E;
      color: #FFFFFF;
    }
    QHeaderView::section {
      background-color: #001B24;
      color: #00E5FF;
      border: none;
      border-right: 1px solid #003B46;
      border-bottom: 1px solid #003B46;
      padding: 4px;
      font-weight: bold;
    }
    QProgressBar {
      background-color: #021B21;
      border: 1px solid #003B46;
      text-align: center;
      color: transparent;
    }
    QProgressBar::chunk {
      background-color: #00E5FF;
      width: 5px;
      margin: 1px;
    }
  )");

  // Header
  auto headerLayout = new QHBoxLayout();

  auto title = new QLabel("SEVERED WORKFORCE REGISTRY", this);
  title->setStyleSheet("font-size: 20px; font-weight: 900; letter-spacing: 2px;");
  
  auto titleGlow = new QGraphicsDropShadowEffect(this);
  titleGlow->setBlurRadius(15);
  titleGlow->setColor(QColor("#00E5FF"));
  titleGlow->setOffset(0, 0);
  title->setGraphicsEffect(titleGlow);

  headerLayout->addWidget(title);

  headerLayout->addStretch();
  
  // Sync Indicators
  auto syncLayout = new QVBoxLayout();
  syncLayout->setSpacing(2);
  
  auto innieLayout = new QHBoxLayout();
  m_InnieSyncLabel = new QLabel("INNIE SYNC", this);
  m_InnieSyncLabel->setStyleSheet("font-size: 10px; font-weight: bold;");
  m_InnieSyncBar = new QProgressBar(this);
  m_InnieSyncBar->setFixedSize(100, 10);
  m_InnieSyncBar->setRange(0, 100);
  m_InnieSyncBar->setValue(85);
  innieLayout->addWidget(m_InnieSyncLabel);
  innieLayout->addWidget(m_InnieSyncBar);
  
  auto outtieLayout = new QHBoxLayout();
  m_OuttieSyncLabel = new QLabel("OUTTIE SYNC", this);
  m_OuttieSyncLabel->setStyleSheet("font-size: 10px; font-weight: bold; color: #446666;");
  m_OuttieSyncBar = new QProgressBar(this);
  m_OuttieSyncBar->setFixedSize(100, 10);
  m_OuttieSyncBar->setRange(0, 100);
  m_OuttieSyncBar->setValue(12);
  m_OuttieSyncBar->setStyleSheet("QProgressBar::chunk { background-color: #005F73; }");
  outtieLayout->addWidget(m_OuttieSyncLabel);
  outtieLayout->addWidget(m_OuttieSyncBar);
  
  syncLayout->addLayout(innieLayout);
  syncLayout->addLayout(outtieLayout);
  
  headerLayout->addLayout(syncLayout);
  
  headerLayout->addSpacing(20);

  m_ProcessCountLabel = new QLabel("0 active registries", this);
  m_ProcessCountLabel->setStyleSheet("font-size: 13px; font-weight: bold;");
  headerLayout->addWidget(m_ProcessCountLabel);

  mainLayout->addLayout(headerLayout);

  // Toolbar: search + buttons
  setupToolbar();
  auto toolbarLayout = new QHBoxLayout();

  m_SearchBar = new QLineEdit(this);
  m_SearchBar->setPlaceholderText("Filter registry by designation, ID, or operator...");
  m_SearchBar->setClearButtonEnabled(true);
  m_SearchBar->setFixedHeight(36);
  connect(m_SearchBar, &QLineEdit::textChanged, this, &ProcessView::onSearchTextChanged);
  toolbarLayout->addWidget(m_SearchBar, 1);

  m_KillBtn = new QPushButton("Sever Procedure", this);
  m_KillBtn->setProperty("cssClass", "danger");
  m_KillBtn->setFixedHeight(36);
  m_KillBtn->setEnabled(false);
  toolbarLayout->addWidget(m_KillBtn);

  m_RefreshBtn = new QPushButton("Recalibrate", this);
  m_RefreshBtn->setFixedHeight(36);
  connect(m_RefreshBtn, &QPushButton::clicked, this, [this]() {
    refreshProcessList();
  });
  toolbarLayout->addWidget(m_RefreshBtn);

  mainLayout->addLayout(toolbarLayout);

  // Process tree model
  m_Model = new ProcessTreeModel(this);

  // Proxy model for filtering
  m_ProxyModel = new QSortFilterProxyModel(this);
  m_ProxyModel->setSourceModel(m_Model);
  m_ProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
  m_ProxyModel->setRecursiveFilteringEnabled(true);
  m_ProxyModel->setFilterKeyColumn(-1); // search all columns
  m_ProxyModel->setSortRole(Qt::DisplayRole);

  // Tree view
  m_TreeView = new QTreeView(this);
  m_TreeView->setModel(m_ProxyModel);
  m_TreeView->setAlternatingRowColors(true);
  m_TreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_TreeView->setSelectionMode(QAbstractItemView::SingleSelection);
  m_TreeView->setSortingEnabled(true);
  m_TreeView->setAnimated(true);
  m_TreeView->setIndentation(16);
  m_TreeView->setUniformRowHeights(true); // performance optimization
  m_TreeView->setContextMenuPolicy(Qt::CustomContextMenu);

  // Column sizing
  auto header = m_TreeView->header();
  header->setStretchLastSection(true);
  header->setSectionResizeMode(QHeaderView::Interactive);
  header->setDefaultSectionSize(100);
  header->resizeSection(ProcessTreeModel::ColName, 220);
  header->resizeSection(ProcessTreeModel::ColPID, 70);
  header->resizeSection(ProcessTreeModel::ColCPU, 80);
  header->resizeSection(ProcessTreeModel::ColMemory, 100);
  header->resizeSection(ProcessTreeModel::ColThreads, 80);
  header->resizeSection(ProcessTreeModel::ColUser, 120);

  // Default sort by CPU descending
  m_TreeView->sortByColumn(ProcessTreeModel::ColCPU, Qt::DescendingOrder);

  // Connections
  connect(m_TreeView, &QTreeView::customContextMenuRequested,
          this, &ProcessView::onProcessContextMenu);
  connect(m_TreeView, &QTreeView::doubleClicked,
          this, &ProcessView::onProcessDoubleClicked);
  connect(m_TreeView->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, [this]() {
    m_KillBtn->setEnabled(m_TreeView->selectionModel()->hasSelection());
  });

  // Kill button action
  connect(m_KillBtn, &QPushButton::clicked, this, [this]() {
    auto selected = m_TreeView->selectionModel()->currentIndex();
    if (selected.isValid()) {
      auto sourceIndex = m_ProxyModel->mapToSource(selected);
      auto info = m_Model->getProcessInfo(sourceIndex);
      auto reply = QMessageBox::question(this, "Sever Procedure",
        QString("Authorize severance of procedure %1 (ID %2)?")
          .arg(QString::fromStdString(info.name))
          .arg(info.pid),
        QMessageBox::Yes | QMessageBox::No);
      if (reply == QMessageBox::Yes) {
        core::process::ProcessManager mgr;
        mgr.KillProcess(info.pid);
        refreshProcessList();
      }
    }
  });

  auto* splitter = new QSplitter(Qt::Horizontal, this);
  splitter->addWidget(m_TreeView);

  m_DetailPanel = new ProcessDetailPanel(this);
  connect(m_DetailPanel, &ProcessDetailPanel::analyzeProcessRequested, this, [this](uint32_t pid, const QString& name) {
    // Generate context string
    QString context = "Context not implemented yet in the UI."; 
    emit analyzeProcessRequested(pid, name, context);
  });
  splitter->addWidget(m_DetailPanel);
  splitter->setSizes({800, 400}); // Default sizes

  mainLayout->addWidget(splitter, 1);
}

void ProcessView::setupToolbar() {
  // Currently integrated into setupUI, placeholder for future expansion
}

void ProcessView::refreshProcessList() {
  core::process::ProcessManager mgr;
  auto processes = mgr.GetRunningProcesses();
  m_Model->updateProcessList(processes);
  m_ProcessCountLabel->setText(QString::number(m_Model->totalProcessCount()) + " active registries");

  // Expand first level for visibility
  for (int i = 0; i < m_ProxyModel->rowCount(); ++i) {
    // Don't expand all — just the System root
  }
}

void ProcessView::onRefreshTimer() {
  refreshProcessList();
}

void ProcessView::onSearchTextChanged(const QString &text) {
  m_ProxyModel->setFilterFixedString(text);
  if (!text.isEmpty()) {
    m_TreeView->expandAll();
  }
}

void ProcessView::onProcessContextMenu(const QPoint &pos) {
  auto index = m_TreeView->indexAt(pos);
  if (!index.isValid()) return;

  auto sourceIndex = m_ProxyModel->mapToSource(index);
  auto info = m_Model->getProcessInfo(sourceIndex);

  QMenu menu(this);

  auto killAction = menu.addAction("Sever Procedure");
  killAction->setIcon(QIcon()); // Will add icons later
  connect(killAction, &QAction::triggered, this, [this, info]() {
    auto reply = QMessageBox::question(this, "Sever Procedure",
      QString("Authorize severance of procedure %1 (ID %2)?")
        .arg(QString::fromStdString(info.name))
        .arg(info.pid));
    if (reply == QMessageBox::Yes) {
      core::process::ProcessManager mgr;
      mgr.KillProcess(info.pid);
      refreshProcessList();
    }
  });

  auto suspendAction = menu.addAction("Halt Procedure");
  connect(suspendAction, &QAction::triggered, this, [this, info]() {
    core::process::ProcessManager mgr;
    if (mgr.SuspendProcess(info.pid)) {
      refreshProcessList();
    } else {
      QMessageBox::warning(this, "Halt Procedure", "Failure to halt procedure. It may be integral to Lumon operations or already halted.");
    }
  });

  auto resumeAction = menu.addAction("Resume Procedure");
  connect(resumeAction, &QAction::triggered, this, [this, info]() {
    core::process::ProcessManager mgr;
    if (mgr.ResumeProcess(info.pid)) {
      refreshProcessList();
    } else {
      QMessageBox::warning(this, "Resume Procedure", "Failure to resume procedure. Protocol error.");
    }
  });

  menu.addSeparator();

  auto copyPidAction = menu.addAction("Copy PID");
  connect(copyPidAction, &QAction::triggered, this, [info]() {
    QApplication::clipboard()->setText(QString::number(info.pid));
  });

  auto copyNameAction = menu.addAction("Copy Name");
  connect(copyNameAction, &QAction::triggered, this, [info]() {
    QApplication::clipboard()->setText(QString::fromStdString(info.name));
  });

  menu.addSeparator();

  auto propertiesAction = menu.addAction("Registry Details");
  connect(propertiesAction, &QAction::triggered, this, [this, info]() {
    m_DetailPanel->LoadProcess(info.pid);
  });

  menu.exec(m_TreeView->viewport()->mapToGlobal(pos));
}

void ProcessView::onProcessDoubleClicked(const QModelIndex &index) {
  auto sourceIndex = m_ProxyModel->mapToSource(index);
  auto info = m_Model->getProcessInfo(sourceIndex);
  m_DetailPanel->LoadProcess(info.pid);
}

} // namespace severance::gui::process_view
