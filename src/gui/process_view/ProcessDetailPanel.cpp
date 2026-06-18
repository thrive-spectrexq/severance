#include "ProcessDetailPanel.hpp"
#include "core/correlation/CorrelationEngine.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

namespace severance::gui::process_view {

using namespace core::correlation;

ProcessDetailPanel::ProcessDetailPanel(QWidget* parent) : QWidget(parent) {
  setupUI();
}

ProcessDetailPanel::~ProcessDetailPanel() = default;

void ProcessDetailPanel::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(12);

  // Header
  auto* headerLayout = new QHBoxLayout();
  m_TitleLabel = new QLabel("Select a process", this);
  m_TitleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #E6EDF3;");
  
  m_PidLabel = new QLabel("", this);
  m_PidLabel->setStyleSheet("font-size: 14px; color: #8B949E;");

  headerLayout->addWidget(m_TitleLabel);
  headerLayout->addWidget(m_PidLabel);
  headerLayout->addStretch();
  
  layout->addLayout(headerLayout);

  // Tabs
  m_Tabs = new QTabWidget(this);
  
  // Overview Tab
  auto* overviewTab = new QWidget(this);
  auto* overviewLayout = new QVBoxLayout(overviewTab);
  m_OverviewContent = new QLabel("Overview details will go here...", this);
  m_OverviewContent->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  overviewLayout->addWidget(m_OverviewContent);
  m_Tabs->addTab(overviewTab, "Overview");

  // Files Tab
  m_FilesTable = new QTableWidget(this);
  m_FilesTable->setColumnCount(3);
  m_FilesTable->setHorizontalHeaderLabels({"Time", "Operation", "Path"});
  m_FilesTable->horizontalHeader()->setStretchLastSection(true);
  m_FilesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_FilesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_FilesTable->verticalHeader()->setVisible(false);
  m_Tabs->addTab(m_FilesTable, "Files Touched");

  // Network Tab
  m_NetworkTable = new QTableWidget(this);
  m_NetworkTable->setColumnCount(5);
  m_NetworkTable->setHorizontalHeaderLabels({"Protocol", "Local IP", "Local Port", "Remote IP", "Remote Port"});
  m_NetworkTable->horizontalHeader()->setStretchLastSection(true);
  m_NetworkTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_NetworkTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_NetworkTable->verticalHeader()->setVisible(false);
  m_Tabs->addTab(m_NetworkTable, "Network Connections");

  layout->addWidget(m_Tabs);
}

void ProcessDetailPanel::Clear() {
  m_TitleLabel->setText("Select a process");
  m_PidLabel->setText("");
  m_OverviewContent->setText("Overview details will go here...");
  m_FilesTable->setRowCount(0);
  m_NetworkTable->setRowCount(0);
}

void ProcessDetailPanel::LoadProcess(uint32_t pid) {
  auto profile = CorrelationEngine::GetInstance().GetProcessProfile(pid);

  m_TitleLabel->setText(QString::fromStdString(profile.name));
  m_PidLabel->setText(QString("PID: %1").arg(pid));

  // Populate Files
  m_FilesTable->setUpdatesEnabled(false);
  m_FilesTable->setRowCount(0);
  for (const auto& f : profile.fileActivity) {
    int row = m_FilesTable->rowCount();
    m_FilesTable->insertRow(row);
    m_FilesTable->setItem(row, 0, new QTableWidgetItem(QString::number(f.timestamp)));
    m_FilesTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(f.operation)));
    m_FilesTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(f.filePath)));
  }
  m_FilesTable->setUpdatesEnabled(true);

  // Populate Network
  m_NetworkTable->setUpdatesEnabled(false);
  m_NetworkTable->setRowCount(0);
  for (const auto& n : profile.networkConnections) {
    int row = m_NetworkTable->rowCount();
    m_NetworkTable->insertRow(row);
    QString proto = n.protocol == core::network::ConnectionProtocol::TCP ? "TCP" : "UDP";
    m_NetworkTable->setItem(row, 0, new QTableWidgetItem(proto));
    m_NetworkTable->setItem(row, 1, new QTableWidgetItem(n.localIp));
    m_NetworkTable->setItem(row, 2, new QTableWidgetItem(QString::number(n.localPort)));
    m_NetworkTable->setItem(row, 3, new QTableWidgetItem(n.remoteIp));
    m_NetworkTable->setItem(row, 4, new QTableWidgetItem(QString::number(n.remotePort)));
  }
  m_NetworkTable->setUpdatesEnabled(true);
}

} // namespace severance::gui::process_view
