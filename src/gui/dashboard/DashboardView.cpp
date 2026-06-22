#include "DashboardView.hpp"
#include "gui/widgets/DonutChartWidget.hpp"
#include "gui/widgets/HorizontalBarChartWidget.hpp"
#include "gui/theme/Theme.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QTableWidget>
#include <QHeaderView>
#include <QString>

#ifdef _WIN32
#include "core/metrics/WindowsMetricsProvider.hpp"
#endif

namespace severance::gui::dashboard {

static QFrame* createKpiCard(const QString& title, QLabel*& valueLabel, QWidget* parent, const char* accentColor) {
  auto* card = new QFrame(parent);
  card->setProperty("cssClass", "card");
  card->setStyleSheet(QString("QFrame.card { border-top: 2px solid %1; }").arg(accentColor));

  auto* layout = new QVBoxLayout(card);
  layout->setContentsMargins(16, 12, 16, 12);
  layout->setSpacing(4);

  auto* titleLabel = new QLabel(title, card);
  titleLabel->setProperty("cssClass", "cardTitle");
  layout->addWidget(titleLabel);

  valueLabel = new QLabel("0", card);
  valueLabel->setProperty("cssClass", "cardValue");
  layout->addWidget(valueLabel);

  return card;
}

DashboardView::DashboardView(QWidget *parent) : QWidget(parent) {
#ifdef _WIN32
  m_MetricsProvider = std::make_unique<core::metrics::WindowsMetricsProvider>();
#else
  // Fallback
#endif

  setupUI();

  m_RefreshTimer = new QTimer(this);
  connect(m_RefreshTimer, &QTimer::timeout, this, &DashboardView::onRefreshTimer);
  m_RefreshTimer->start(1000); // 1s refresh
}

DashboardView::~DashboardView() = default;

void DashboardView::setupUI() {
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(24, 24, 24, 24);
  mainLayout->setSpacing(16);

  // === Top Row: KPI Cards ===
  auto* kpiLayout = new QHBoxLayout();
  kpiLayout->setSpacing(16);
  kpiLayout->addWidget(createKpiCard("PROCESSES", m_KpiProcessCount, this, theme::Colors::Accent));
  kpiLayout->addWidget(createKpiCard("NETWORK CONNECTIONS", m_KpiNetConnections, this, theme::Colors::Success));
  kpiLayout->addWidget(createKpiCard("FILE HANDLES", m_KpiFileHandles, this, theme::Colors::ChartPurple));
  kpiLayout->addWidget(createKpiCard("ALERTS", m_KpiAlerts, this, theme::Colors::Error));
  mainLayout->addLayout(kpiLayout, 0);

  // === Middle Row: Charts ===
  auto* chartsLayout = new QHBoxLayout();
  chartsLayout->setSpacing(16);

  // CPU Donut Chart Card
  auto* donutCard = new QFrame(this);
  donutCard->setProperty("cssClass", "card");
  auto* donutLayout = new QVBoxLayout(donutCard);
  auto* donutTitle = new QLabel("CPU Distribution", donutCard);
  donutTitle->setProperty("cssClass", "cardTitle");
  donutLayout->addWidget(donutTitle);
  
  m_CpuDonut = new widgets::DonutChartWidget(donutCard);
  donutLayout->addWidget(m_CpuDonut, 1, Qt::AlignCenter);
  chartsLayout->addWidget(donutCard, 1);

  // Top Processes Bar Chart Card
  auto* barCard = new QFrame(this);
  barCard->setProperty("cssClass", "card");
  auto* barLayout = new QVBoxLayout(barCard);
  auto* barTitle = new QLabel("Top Memory Consumers", barCard);
  barTitle->setProperty("cssClass", "cardTitle");
  barLayout->addWidget(barTitle);

  m_TopProcessesBar = new widgets::HorizontalBarChartWidget(barCard);
  barLayout->addWidget(m_TopProcessesBar, 1);
  chartsLayout->addWidget(barCard, 2); // Takes more space

  mainLayout->addLayout(chartsLayout, 2);

  // === Bottom Row: Recent Events Table ===
  auto* tableCard = new QFrame(this);
  tableCard->setProperty("cssClass", "card");
  auto* tableLayout = new QVBoxLayout(tableCard);
  auto* tableTitle = new QLabel("Recent Activity", tableCard);
  tableTitle->setProperty("cssClass", "cardTitle");
  tableLayout->addWidget(tableTitle);

  m_RecentEventsTable = new QTableWidget(5, 4, tableCard);
  m_RecentEventsTable->setHorizontalHeaderLabels({"TIME", "TYPE", "PROCESS", "DETAILS"});
  m_RecentEventsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
  m_RecentEventsTable->verticalHeader()->setVisible(false);
  m_RecentEventsTable->setShowGrid(false);
  m_RecentEventsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_RecentEventsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  
  // Dummy data for visual
  QStringList processes = {"chrome.exe", "svchost.exe", "Severance.exe", "MsMpEng.exe", "SearchIndexer.exe"};
  QStringList events = {"Network Conn", "File Read", "Process Start", "Registry Write", "File Write"};
  for (int i = 0; i < 5; ++i) {
    m_RecentEventsTable->setItem(i, 0, new QTableWidgetItem("12:34:56"));
    
    auto* typeItem = new QTableWidgetItem(events[i]);
    if (i == 0) typeItem->setForeground(QColor(theme::Colors::Info));
    else if (i == 2) typeItem->setForeground(QColor(theme::Colors::Success));
    else if (i == 3) typeItem->setForeground(QColor(theme::Colors::Warning));
    m_RecentEventsTable->setItem(i, 1, typeItem);
    
    m_RecentEventsTable->setItem(i, 2, new QTableWidgetItem(processes[i]));
    m_RecentEventsTable->setItem(i, 3, new QTableWidgetItem("Detail about event " + QString::number(i)));
  }
  
  tableLayout->addWidget(m_RecentEventsTable, 1);
  mainLayout->addWidget(tableCard, 2);
}

void DashboardView::onRefreshTimer() {
  if (m_MetricsProvider) {
    auto snapshot = m_MetricsProvider->GetSnapshot();
    updateDashboard(snapshot);
  }
}

void DashboardView::updateDashboard(const core::metrics::SystemMetricsSnapshot& snapshot) {
  // Update KPIs
  m_KpiProcessCount->setText(QString::number(snapshot.cpu.processCount));
  m_KpiNetConnections->setText(QString::number(snapshot.network.totalBytesSentPerSec > 0 ? 12 : 5)); // Dummy conn count
  m_KpiFileHandles->setText(QString::number(snapshot.cpu.threadCount * 2)); // Dummy handles
  m_KpiAlerts->setText(snapshot.cpu.globalUsagePercent > 80 ? "1" : "0");

  // Update CPU Donut
  float userCpu = snapshot.cpu.globalUsagePercent * 0.7f;
  float sysCpu = snapshot.cpu.globalUsagePercent * 0.3f;
  float idleCpu = 100.0f - snapshot.cpu.globalUsagePercent;

  std::vector<widgets::DonutChartWidget::DataSegment> cpuSegments = {
    {"User", userCpu, QColor(theme::Colors::Accent)},
    {"System", sysCpu, QColor(theme::Colors::ChartPurple)},
    {"Idle", idleCpu, QColor(theme::Colors::BgHover)}
  };
  m_CpuDonut->setSegments(cpuSegments);
  m_CpuDonut->setCenterText(QString::number(static_cast<int>(snapshot.cpu.globalUsagePercent)) + "%");

  // Update Top Processes Bar Chart
  // Dummy data simulating top memory processes since snapshot doesn't expose process list yet
  std::vector<widgets::HorizontalBarChartWidget::DataBar> topProcs = {
    {"chrome.exe", 1540.0f, "1.5 GB", QColor(theme::Colors::ChartBlue)},
    {"Severance.exe", 420.0f, "420 MB", QColor(theme::Colors::ChartGreen)},
    {"MsMpEng.exe", 310.0f, "310 MB", QColor(theme::Colors::ChartYellow)},
    {"Code.exe", 280.0f, "280 MB", QColor(theme::Colors::ChartOrange)},
    {"explorer.exe", 150.0f, "150 MB", QColor(theme::Colors::Accent)}
  };
  m_TopProcessesBar->setBars(topProcs);
}

} // namespace severance::gui::dashboard
