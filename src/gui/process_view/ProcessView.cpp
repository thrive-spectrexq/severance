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

namespace severance::gui::process_view {

ProcessView::ProcessView(QWidget *parent) : QWidget(parent) {
  setupUI();

  // Initial data load
  refreshProcessList();

  // Start auto-refresh timer
  m_RefreshTimer = new QTimer(this);
  connect(m_RefreshTimer, &QTimer::timeout, this, &ProcessView::onRefreshTimer);
  m_RefreshTimer->start(m_RefreshIntervalMs);
}

void ProcessView::setupUI() {
  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(16, 12, 16, 0);
  mainLayout->setSpacing(8);

  // Header
  auto headerLayout = new QHBoxLayout();

  auto title = new QLabel("Process Explorer", this);
  title->setStyleSheet("font-size: 18px; font-weight: 600; color: #E6EDF3;");
  headerLayout->addWidget(title);

  headerLayout->addStretch();

  m_ProcessCountLabel = new QLabel("0 processes", this);
  m_ProcessCountLabel->setStyleSheet("font-size: 13px; color: #8B949E;");
  headerLayout->addWidget(m_ProcessCountLabel);

  mainLayout->addLayout(headerLayout);

  // Toolbar: search + buttons
  setupToolbar();
  auto toolbarLayout = new QHBoxLayout();

  m_SearchBar = new QLineEdit(this);
  m_SearchBar->setPlaceholderText("Filter processes by name, PID, or user...");
  m_SearchBar->setClearButtonEnabled(true);
  m_SearchBar->setFixedHeight(36);
  connect(m_SearchBar, &QLineEdit::textChanged, this, &ProcessView::onSearchTextChanged);
  toolbarLayout->addWidget(m_SearchBar, 1);

  m_KillBtn = new QPushButton("End Process", this);
  m_KillBtn->setProperty("cssClass", "danger");
  m_KillBtn->setFixedHeight(36);
  m_KillBtn->setEnabled(false);
  toolbarLayout->addWidget(m_KillBtn);

  m_RefreshBtn = new QPushButton("Refresh", this);
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
      auto reply = QMessageBox::question(this, "End Process",
        QString("Terminate %1 (PID %2)?")
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
  m_ProcessCountLabel->setText(QString::number(m_Model->totalProcessCount()) + " processes");

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

  auto killAction = menu.addAction("End Process");
  killAction->setIcon(QIcon()); // Will add icons later
  connect(killAction, &QAction::triggered, this, [this, info]() {
    auto reply = QMessageBox::question(this, "End Process",
      QString("Terminate %1 (PID %2)?")
        .arg(QString::fromStdString(info.name))
        .arg(info.pid));
    if (reply == QMessageBox::Yes) {
      core::process::ProcessManager mgr;
      mgr.KillProcess(info.pid);
      refreshProcessList();
    }
  });

  auto suspendAction = menu.addAction("Suspend Process");
  connect(suspendAction, &QAction::triggered, this, [this, info]() {
    core::process::ProcessManager mgr;
    if (mgr.SuspendProcess(info.pid)) {
      refreshProcessList();
    } else {
      QMessageBox::warning(this, "Suspend Process", "Failed to suspend process. It may be protected or already suspended.");
    }
  });

  auto resumeAction = menu.addAction("Resume Process");
  connect(resumeAction, &QAction::triggered, this, [this, info]() {
    core::process::ProcessManager mgr;
    if (mgr.ResumeProcess(info.pid)) {
      refreshProcessList();
    } else {
      QMessageBox::warning(this, "Resume Process", "Failed to resume process.");
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

  auto propertiesAction = menu.addAction("Properties");
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
