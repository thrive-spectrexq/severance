#include "DashboardView.hpp"
#include "core/metrics/WindowsMetricsProvider.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>

namespace severance::gui::dashboard {

static QString FormatBytes(uint64_t bytes) {
  const double kb = bytes / 1024.0;
  const double mb = kb / 1024.0;
  const double gb = mb / 1024.0;

  if (gb >= 1.0) return QString::number(gb, 'f', 1) + " GB";
  if (mb >= 1.0) return QString::number(mb, 'f', 1) + " MB";
  if (kb >= 1.0) return QString::number(kb, 'f', 0) + " KB";
  return QString::number(bytes) + " B";
}

static QString FormatSpeed(uint64_t bytesPerSec) {
  const double kb = bytesPerSec / 1024.0;
  const double mb = kb / 1024.0;
  const double gb = mb / 1024.0;

  if (gb >= 1.0) return QString::number(gb, 'f', 1) + " GB/s";
  if (mb >= 1.0) return QString::number(mb, 'f', 1) + " MB/s";
  if (kb >= 1.0) return QString::number(kb, 'f', 0) + " KB/s";
  return QString::number(bytesPerSec) + " B/s";
}

static QFrame* CreateCard(QWidget* parent, const QString& title, QWidget** outContent) {
  auto card = new QFrame(parent);
  card->setProperty("class", "card");
  
  auto layout = new QVBoxLayout(card);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(8);

  auto titleLabel = new QLabel(title, card);
  titleLabel->setProperty("class", "cardTitle");
  layout->addWidget(titleLabel);

  auto content = new QWidget(card);
  layout->addWidget(content, 1);
  *outContent = content;

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
  m_RefreshTimer->start(1000); // 1 second updates
}

DashboardView::~DashboardView() = default;

void DashboardView::setupUI() {
  auto mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(24, 24, 24, 24);
  mainLayout->setSpacing(24);

  auto title = new QLabel("System Dashboard", this);
  title->setStyleSheet("font-size: 24px; font-weight: 600; color: #E6EDF3;");
  mainLayout->addWidget(title);

  auto grid = new QGridLayout();
  grid->setSpacing(20);

  // CPU Card
  QWidget* cpuContent;
  auto cpuCard = CreateCard(this, "CPU", &cpuContent);
  auto cpuLayout = new QVBoxLayout(cpuContent);
  cpuLayout->setContentsMargins(0,0,0,0);
  m_CpuValueLabel = new QLabel("0.0%", cpuContent);
  m_CpuValueLabel->setProperty("class", "cardValue");
  cpuLayout->addWidget(m_CpuValueLabel);
  m_CpuNameLabel = new QLabel("Processor", cpuContent);
  m_CpuNameLabel->setProperty("class", "cardUnit");
  cpuLayout->addWidget(m_CpuNameLabel);
  cpuLayout->addStretch();
  grid->addWidget(cpuCard, 0, 0);

  // Memory Card
  QWidget* memContent;
  auto memCard = CreateCard(this, "Memory", &memContent);
  auto memLayout = new QVBoxLayout(memContent);
  memLayout->setContentsMargins(0,0,0,0);
  m_MemValueLabel = new QLabel("0.0 GB", memContent);
  m_MemValueLabel->setProperty("class", "cardValue");
  memLayout->addWidget(m_MemValueLabel);
  m_MemTotalLabel = new QLabel("of 0.0 GB", memContent);
  m_MemTotalLabel->setProperty("class", "cardUnit");
  memLayout->addWidget(m_MemTotalLabel);
  memLayout->addStretch();
  grid->addWidget(memCard, 0, 1);

  // Network Card
  QWidget* netContent;
  auto netCard = CreateCard(this, "Network", &netContent);
  auto netLayout = new QVBoxLayout(netContent);
  netLayout->setContentsMargins(0,0,0,0);
  m_NetRecvLabel = new QLabel("↓ 0 B/s", netContent);
  m_NetRecvLabel->setProperty("class", "cardValue");
  m_NetRecvLabel->setStyleSheet("color: #3FB950;"); // Green
  netLayout->addWidget(m_NetRecvLabel);
  m_NetSentLabel = new QLabel("↑ 0 B/s", netContent);
  m_NetSentLabel->setProperty("class", "cardValue");
  m_NetSentLabel->setStyleSheet("color: #58A6FF;"); // Blue
  netLayout->addWidget(m_NetSentLabel);
  netLayout->addStretch();
  grid->addWidget(netCard, 0, 2);

  grid->setColumnStretch(0, 1);
  grid->setColumnStretch(1, 1);
  grid->setColumnStretch(2, 1);

  mainLayout->addLayout(grid);
  mainLayout->addStretch(); // push cards to top
}

void DashboardView::onRefreshTimer() {
  if (m_MetricsProvider) {
    updateDashboard(m_MetricsProvider->GetCurrentMetrics());
  }
}

void DashboardView::updateDashboard(const core::metrics::SystemMetricsSnapshot& snapshot) {
  m_CpuValueLabel->setText(QString::number(snapshot.cpu.globalUsagePercent, 'f', 1) + "%");
  if (!snapshot.cpu.processorName.empty()) {
    m_CpuNameLabel->setText(QString::fromStdString(snapshot.cpu.processorName));
  }

  m_MemValueLabel->setText(FormatBytes(snapshot.memory.usedBytes));
  m_MemTotalLabel->setText("of " + FormatBytes(snapshot.memory.totalBytes));

  m_NetRecvLabel->setText("↓ " + FormatSpeed(snapshot.network.totalBytesReceivedPerSec));
  m_NetSentLabel->setText("↑ " + FormatSpeed(snapshot.network.totalBytesSentPerSec));
}

} // namespace severance::gui::dashboard
