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
  m_TitleLabel = new QLabel("Select a procedure", this);
  m_TitleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #E6EDF3;");
  
  m_PidLabel = new QLabel("", this);
  m_PidLabel->setStyleSheet("font-size: 14px; color: #8B949E;");

  headerLayout->addWidget(m_TitleLabel);
  headerLayout->addWidget(m_PidLabel);
  headerLayout->addStretch();
  
  m_AiAnalyzeBtn = new QPushButton("✨ Supplemental Intelligence Analysis", this);
  m_AiAnalyzeBtn->setStyleSheet("background-color: #238636; color: white; border: none; border-radius: 4px; padding: 6px 12px; font-weight: bold;");
  m_AiAnalyzeBtn->hide(); // Hide until process loaded
  connect(m_AiAnalyzeBtn, &QPushButton::clicked, this, [this]() {
    emit analyzeProcessRequested(m_CurrentPid, m_TitleLabel->text());
  });
  headerLayout->addWidget(m_AiAnalyzeBtn);

  layout->addLayout(headerLayout);

  // Tabs
  m_Tabs = new QTabWidget(this);
  
  // Overview Tab
  auto* overviewTab = new QWidget(this);
  auto* overviewLayout = new QVBoxLayout(overviewTab);
  m_OverviewContent = new QLabel("Registry particulars pending selection...", this);
  m_OverviewContent->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  overviewLayout->addWidget(m_OverviewContent);
  m_Tabs->addTab(overviewTab, "OVERVIEW");

  // Files Tab
  m_FilesTable = new QTableWidget(this);
  m_FilesTable->setColumnCount(3);
  m_FilesTable->setHorizontalHeaderLabels({"Timestamp", "Operation", "Designation"});
  m_FilesTable->horizontalHeader()->setStretchLastSection(true);
  m_FilesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_FilesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_FilesTable->verticalHeader()->setVisible(false);
  m_Tabs->addTab(m_FilesTable, "DOCUMENT ACTIVITY");

  // Network Tab
  m_NetworkTable = new QTableWidget(this);
  m_NetworkTable->setColumnCount(5);
  m_NetworkTable->setHorizontalHeaderLabels({"Protocol", "Internal IP", "Internal Port", "External IP", "External Port"});
  m_NetworkTable->horizontalHeader()->setStretchLastSection(true);
  m_NetworkTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_NetworkTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_NetworkTable->verticalHeader()->setVisible(false);
  m_Tabs->addTab(m_NetworkTable, "COMMUNICATIONS");

  layout->addWidget(m_Tabs);
}

void ProcessDetailPanel::Clear() {
  m_CurrentPid = 0;
  m_TitleLabel->setText("Select a procedure");
  m_PidLabel->setText("");
  m_AiAnalyzeBtn->hide();
  m_OverviewContent->setText("Registry particulars pending selection...");
  m_FilesTable->setRowCount(0);
  m_NetworkTable->setRowCount(0);
}

void ProcessDetailPanel::LoadProcess(uint32_t pid) {
  m_CurrentPid = pid;
  auto profile = CorrelationEngine::GetInstance().GetProcessProfile(pid);

  m_TitleLabel->setText(QString::fromStdString(profile.name));
  m_PidLabel->setText(QString("ID: %1").arg(pid));
  m_AiAnalyzeBtn->show();

  // Populate Files
  m_FilesTable->setUpdatesEnabled(false);
  m_FilesTable->setRowCount(0);
  for (size_t i = 0; i < profile.fileActivity.size(); ++i) {
    const auto& f = profile.fileActivity[i];
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
  for (size_t i = 0; i < profile.networkConnections.size(); ++i) {
    const auto& n = profile.networkConnections[i];
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
