#include "DashboardView.hpp"
#include "gui/widgets/DonutChartWidget.hpp"
#include "gui/widgets/HorizontalBarChartWidget.hpp"
#include "gui/widgets/NumberGridWidget.hpp"
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

  theme::ApplyDropShadow(card);

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
  mainLayout->setContentsMargins(32, 32, 32, 32); // Increased padding
  mainLayout->setSpacing(24);

  // === Header ===
  auto* headerLabel = new QLabel("Macrodata Refinement Telemetry", this);
  headerLabel->setProperty("cssClass", "heading");
  mainLayout->addWidget(headerLabel);

  // === Top Row: KPI Cards ===
  auto* kpiLayout = new QHBoxLayout();
  kpiLayout->setSpacing(24);
  kpiLayout->addWidget(createKpiCard("ACTIVE INNIES", m_KpiProcessCount, this, theme::Colors::Accent));
  kpiLayout->addWidget(createKpiCard("PERIMETER BREACHES", m_KpiNetConnections, this, theme::Colors::Success));
  kpiLayout->addWidget(createKpiCard("MEMORY COMPARTMENTS", m_KpiFileHandles, this, theme::Colors::ChartTeal));
  kpiLayout->addWidget(createKpiCard("OBSERVATION ALERTS", m_KpiAlerts, this, theme::Colors::Error));
  mainLayout->addLayout(kpiLayout, 0);

  // === Middle Row: Charts ===
  auto* chartsLayout = new QHBoxLayout();
  chartsLayout->setSpacing(16);

  // CPU Donut Chart Card
  auto* donutCard = new QFrame(this);
  donutCard->setProperty("cssClass", "card");
  theme::ApplyDropShadow(donutCard);
  
  auto* donutLayout = new QVBoxLayout(donutCard);
  donutLayout->setContentsMargins(20, 20, 20, 20);
  auto* donutTitle = new QLabel("Cognitive Load Distribution", donutCard);
  donutTitle->setProperty("cssClass", "cardTitle");
  donutLayout->addWidget(donutTitle);
  
  m_CpuDonut = new widgets::DonutChartWidget(donutCard);
  donutLayout->addWidget(m_CpuDonut, 1, Qt::AlignCenter);
  chartsLayout->addWidget(donutCard, 1);

  // Top Processes Bar Chart Card
  auto* barCard = new QFrame(this);
  barCard->setProperty("cssClass", "card");
  theme::ApplyDropShadow(barCard);
  
  auto* barLayout = new QVBoxLayout(barCard);
  barLayout->setContentsMargins(20, 20, 20, 20);
  auto* barTitle = new QLabel("Primary Resource Consumers", barCard);
  barTitle->setProperty("cssClass", "cardTitle");
  barLayout->addWidget(barTitle);

  m_TopProcessesBar = new widgets::HorizontalBarChartWidget(barCard);
  barLayout->addWidget(m_TopProcessesBar, 1);
  chartsLayout->addWidget(barCard, 2); // Takes more space

  mainLayout->addLayout(chartsLayout, 2);

  // === Bottom Row: Number Grid Widget ===
  auto* gridCard = new QFrame(this);
  gridCard->setProperty("cssClass", "card");
  theme::ApplyDropShadow(gridCard);
  
  auto* gridLayout = new QVBoxLayout(gridCard);
  gridLayout->setContentsMargins(20, 20, 20, 20);
  auto* gridTitle = new QLabel("Refinement Feed", gridCard);
  gridTitle->setProperty("cssClass", "cardTitle");
  gridLayout->addWidget(gridTitle);
  gridLayout->addSpacing(8);

  m_NumberGrid = new widgets::NumberGridWidget(gridCard);
  m_NumberGrid->setMinimumHeight(400); // Give it enough space
  
  gridLayout->addWidget(m_NumberGrid, 1);
  mainLayout->addWidget(gridCard, 2);
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
