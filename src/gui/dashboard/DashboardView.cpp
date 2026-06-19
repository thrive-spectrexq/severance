#include "DashboardView.hpp"
#include "gui/graphs/OpenGLResourceGraph.hpp"
#include "gui/theme/Theme.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>

#ifdef _WIN32
#include "core/metrics/WindowsMetricsProvider.hpp"
#endif

namespace severance::gui::dashboard {

static QFrame* createCard(const QString& title, QWidget* parent) {
  auto* card = new QFrame(parent);
  card->setObjectName("dashboardCard");
  card->setStyleSheet(R"(
    QFrame#dashboardCard {
      background-color: #161B22;
      border: 1px solid #30363D;
      border-radius: 8px;
    }
  )");

  auto* layout = new QVBoxLayout(card);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(8);

  auto* titleLabel = new QLabel(title, card);
  titleLabel->setStyleSheet("color: #8B949E; font-size: 12px; font-weight: 600; text-transform: uppercase;");
  layout->addWidget(titleLabel);

  return card;
}

DashboardView::DashboardView(QWidget *parent) : QWidget(parent) {
#ifdef _WIN32
  m_MetricsProvider = std::make_unique<core::metrics::WindowsMetricsProvider>();
#else
  // Fallback for non-Windows (stub)
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

  // Top Section: Title
  auto* headerLabel = new QLabel("System Overview", this);
  headerLabel->setStyleSheet("color: #E6EDF3; font-size: 24px; font-weight: 700;");
  mainLayout->addWidget(headerLabel);

  // Cards Grid
  auto* grid = new QGridLayout();
  grid->setSpacing(16);

  // CPU Card
  auto* cpuCard = createCard("CPU Usage", this);
  auto* cpuLayout = qobject_cast<QVBoxLayout*>(cpuCard->layout());
  
  auto* cpuHeader = new QHBoxLayout();
  m_CpuValueLabel = new QLabel("--%", cpuCard);
  m_CpuValueLabel->setStyleSheet("color: #E6EDF3; font-size: 32px; font-weight: 300;");
  m_CpuNameLabel = new QLabel("Processor", cpuCard);
  m_CpuNameLabel->setStyleSheet("color: #8B949E; font-size: 12px;");
  
  cpuHeader->addWidget(m_CpuValueLabel);
  cpuHeader->addStretch();
  cpuHeader->addWidget(m_CpuNameLabel, 0, Qt::AlignBottom);
  cpuLayout->addLayout(cpuHeader);
  
  m_CpuGraph = new graphs::OpenGLResourceGraph(cpuCard);
  cpuLayout->addWidget(m_CpuGraph);
  
  grid->addWidget(cpuCard, 0, 0);

  // Memory Card
  auto* memCard = createCard("Memory", this);
  auto* memLayout = qobject_cast<QVBoxLayout*>(memCard->layout());
  
  auto* memHeader = new QHBoxLayout();
  m_MemValueLabel = new QLabel("-- GB", memCard);
  m_MemValueLabel->setStyleSheet("color: #E6EDF3; font-size: 32px; font-weight: 300;");
  m_MemTotalLabel = new QLabel("/ -- GB", memCard);
  m_MemTotalLabel->setStyleSheet("color: #8B949E; font-size: 16px; padding-bottom: 6px;");
  
  memHeader->addWidget(m_MemValueLabel);
  memHeader->addWidget(m_MemTotalLabel, 0, Qt::AlignBottom);
  memHeader->addStretch();
  memLayout->addLayout(memHeader);
  
  m_MemGraph = new graphs::OpenGLResourceGraph(memCard);
  memLayout->addWidget(m_MemGraph);
  
  grid->addWidget(memCard, 0, 1);

  // Network Card
  auto* netCard = createCard("Network Activity", this);
  auto* netLayout = qobject_cast<QVBoxLayout*>(netCard->layout());
  
  m_NetRecvLabel = new QLabel(QString::fromUtf8("↓ 0 Kbps"), netCard);
  m_NetRecvLabel->setStyleSheet("color: #3FB950; font-size: 20px; font-weight: 600;");
  m_NetSentLabel = new QLabel(QString::fromUtf8("↑ 0 Kbps"), netCard);
  m_NetSentLabel->setStyleSheet("color: #58A6FF; font-size: 20px; font-weight: 600;");
  
  netLayout->addSpacing(16);
  netLayout->addWidget(m_NetRecvLabel);
  netLayout->addWidget(m_NetSentLabel);
  netLayout->addStretch();
  
  grid->addWidget(netCard, 1, 0);

  // Add grid to main
  mainLayout->addLayout(grid);
  mainLayout->addStretch();
}

void DashboardView::onRefreshTimer() {
  if (m_MetricsProvider) {
    auto snapshot = m_MetricsProvider->GetSnapshot();
    updateDashboard(snapshot);
  }
}

void DashboardView::updateDashboard(const core::metrics::SystemMetricsSnapshot& snapshot) {
  // Update CPU
  m_CpuValueLabel->setText(QString::number(static_cast<int>(snapshot.cpu.globalUsagePercent)) + "%");
  m_CpuGraph->addDataPoint(snapshot.cpu.globalUsagePercent);
  
  if (!snapshot.cpu.processorName.empty()) {
      m_CpuNameLabel->setText(QString::fromStdString(snapshot.cpu.processorName));
  }

  // Update Memory
  double memUsedGB = snapshot.memory.usedBytes / (1024.0 * 1024.0 * 1024.0);
  double memTotalGB = snapshot.memory.totalBytes / (1024.0 * 1024.0 * 1024.0);
  m_MemValueLabel->setText(QString::number(memUsedGB, 'f', 1) + " GB");
  m_MemTotalLabel->setText(QString("/ %1 GB").arg(QString::number(memTotalGB, 'f', 1)));
  
  double memPercent = (memTotalGB > 0) ? (memUsedGB / memTotalGB) * 100.0 : 0.0;
  m_MemGraph->addDataPoint(memPercent);

  // Update Network (assuming bps)
  double recvKbps = snapshot.network.totalBytesReceivedPerSec * 8.0 / 1000.0;
  double sentKbps = snapshot.network.totalBytesSentPerSec * 8.0 / 1000.0;
  
  m_NetRecvLabel->setText(QString::fromUtf8("↓ %1 Kbps").arg(QString::number(recvKbps, 'f', 1)));
  m_NetSentLabel->setText(QString::fromUtf8("↑ %1 Kbps").arg(QString::number(sentKbps, 'f', 1)));
}

} // namespace severance::gui::dashboard
