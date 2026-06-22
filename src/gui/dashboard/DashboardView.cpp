#include "DashboardView.hpp"
#include "gui/graphs/OpenGLResourceGraph.hpp"
#include "gui/theme/Theme.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFrame>
#include <QProgressBar>
#include <QString>

#ifdef _WIN32
#include "core/metrics/WindowsMetricsProvider.hpp"
#endif

namespace severance::gui::dashboard {

static QFrame* createCard(const QString& title, QWidget* parent) {
  auto* card = new QFrame(parent);
  card->setProperty("cssClass", "card");

  auto* layout = new QVBoxLayout(card);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(8);

  auto* titleLabel = new QLabel(title, card);
  titleLabel->setProperty("cssClass", "cardTitle");
  layout->addWidget(titleLabel);

  return card;
}

static QString formatUptime(uint64_t totalSeconds) {
  uint64_t days = totalSeconds / 86400;
  uint64_t hours = (totalSeconds % 86400) / 3600;
  uint64_t minutes = (totalSeconds % 3600) / 60;

  if (days > 0) {
    return QString("%1d %2h %3m").arg(days).arg(hours).arg(minutes);
  } else if (hours > 0) {
    return QString("%1h %2m").arg(hours).arg(minutes);
  } else {
    return QString("%1m").arg(minutes);
  }
}

static QString formatBytes(uint64_t bytes) {
  if (bytes >= 1099511627776ULL) {
    return QString("%1 TB").arg(static_cast<double>(bytes) / 1099511627776.0, 0, 'f', 1);
  } else if (bytes >= 1073741824ULL) {
    return QString("%1 GB").arg(static_cast<double>(bytes) / 1073741824.0, 0, 'f', 1);
  } else if (bytes >= 1048576ULL) {
    return QString("%1 MB").arg(static_cast<double>(bytes) / 1048576.0, 0, 'f', 0);
  }
  return QString("%1 KB").arg(bytes / 1024);
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
  headerLabel->setProperty("cssClass", "heading");
  mainLayout->addWidget(headerLabel);

  // Cards Grid — 3 columns
  auto* grid = new QGridLayout();
  grid->setSpacing(16);

  // ── Row 0: CPU, Memory, System Info ──

  // CPU Card
  auto* cpuCard = createCard("CPU Usage", this);
  auto* cpuLayout = qobject_cast<QVBoxLayout*>(cpuCard->layout());
  
  auto* cpuHeader = new QHBoxLayout();
  m_CpuValueLabel = new QLabel("--%", cpuCard);
  m_CpuValueLabel->setProperty("cssClass", "cardValue");
  m_CpuNameLabel = new QLabel("Processor", cpuCard);
  m_CpuNameLabel->setProperty("cssClass", "cardUnit");
  
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
  m_MemValueLabel->setProperty("cssClass", "cardValue");
  m_MemTotalLabel = new QLabel("/ -- GB", memCard);
  m_MemTotalLabel->setProperty("cssClass", "cardUnit");
  
  memHeader->addWidget(m_MemValueLabel);
  memHeader->addWidget(m_MemTotalLabel, 0, Qt::AlignBottom);
  memHeader->addStretch();
  memLayout->addLayout(memHeader);
  
  m_MemGraph = new graphs::OpenGLResourceGraph(memCard);
  memLayout->addWidget(m_MemGraph);
  
  grid->addWidget(memCard, 0, 1);

  // System Info Card
  auto* sysCard = createCard("System", this);
  auto* sysLayout = qobject_cast<QVBoxLayout*>(sysCard->layout());
  
  sysLayout->addSpacing(8);

  auto makeInfoRow = [sysCard, sysLayout](const QString& label, QLabel*& valueLabel) {
    auto* row = new QHBoxLayout();
    auto* lbl = new QLabel(label, sysCard);
    lbl->setStyleSheet("color: #8B949E; font-size: 13px;");
    valueLabel = new QLabel("--", sysCard);
    valueLabel->setStyleSheet("color: #E6EDF3; font-size: 13px; font-weight: 600;");
    row->addWidget(lbl);
    row->addStretch();
    row->addWidget(valueLabel);
    sysLayout->addLayout(row);
  };

  makeInfoRow("Uptime", m_UptimeLabel);
  makeInfoRow("Processes", m_ProcessCountLabel);
  makeInfoRow("Threads", m_ThreadCountLabel);
  
  sysLayout->addStretch();
  grid->addWidget(sysCard, 0, 2);

  // ── Row 1: Network, GPU, Disk ──

  // Network Card
  auto* netCard = createCard("Network Activity", this);
  auto* netLayout = qobject_cast<QVBoxLayout*>(netCard->layout());
  
  m_NetRecvLabel = new QLabel(QString::fromUtf8("\xe2\x86\x93 0 Kbps"), netCard);
  m_NetRecvLabel->setProperty("cssClass", "cardValue");
  m_NetRecvLabel->setStyleSheet("color: #3FB950; font-size: 22px; font-weight: 700;");
  m_NetSentLabel = new QLabel(QString::fromUtf8("\xe2\x86\x91 0 Kbps"), netCard);
  m_NetSentLabel->setProperty("cssClass", "cardValue");
  m_NetSentLabel->setStyleSheet("color: #58A6FF; font-size: 22px; font-weight: 700;");
  
  netLayout->addSpacing(8);
  netLayout->addWidget(m_NetRecvLabel);
  netLayout->addWidget(m_NetSentLabel);
  netLayout->addStretch();
  
  grid->addWidget(netCard, 1, 0);

  // GPU Card
  auto* gpuCard = createCard("GPU", this);
  auto* gpuLayout = qobject_cast<QVBoxLayout*>(gpuCard->layout());
  
  m_GpuNameLabel = new QLabel("Detecting...", gpuCard);
  m_GpuNameLabel->setStyleSheet("color: #8B949E; font-size: 12px;");
  m_GpuNameLabel->setWordWrap(true);
  gpuLayout->addWidget(m_GpuNameLabel);
  
  gpuLayout->addSpacing(4);
  
  auto* vramRow = new QHBoxLayout();
  auto* vramTitle = new QLabel("VRAM", gpuCard);
  vramTitle->setStyleSheet("color: #8B949E; font-size: 12px;");
  m_GpuVramLabel = new QLabel("-- / --", gpuCard);
  m_GpuVramLabel->setStyleSheet("color: #E6EDF3; font-size: 13px; font-weight: 600;");
  vramRow->addWidget(vramTitle);
  vramRow->addStretch();
  vramRow->addWidget(m_GpuVramLabel);
  gpuLayout->addLayout(vramRow);
  
  m_GpuVramBar = new QProgressBar(gpuCard);
  m_GpuVramBar->setRange(0, 100);
  m_GpuVramBar->setValue(0);
  m_GpuVramBar->setTextVisible(false);
  m_GpuVramBar->setFixedHeight(8);
  m_GpuVramBar->setStyleSheet(R"(
    QProgressBar { background-color: #21262D; border: none; border-radius: 4px; }
    QProgressBar::chunk { background-color: #A371F7; border-radius: 4px; }
  )");
  gpuLayout->addWidget(m_GpuVramBar);
  
  gpuLayout->addStretch();
  grid->addWidget(gpuCard, 1, 1);

  // Disk Card
  auto* diskCard = createCard("Storage", this);
  auto* diskLayout = qobject_cast<QVBoxLayout*>(diskCard->layout());
  
  m_DiskNameLabel = new QLabel("Detecting...", diskCard);
  m_DiskNameLabel->setStyleSheet("color: #8B949E; font-size: 12px;");
  diskLayout->addWidget(m_DiskNameLabel);
  
  diskLayout->addSpacing(4);
  
  m_DiskSpaceLabel = new QLabel("-- / --", diskCard);
  m_DiskSpaceLabel->setStyleSheet("color: #E6EDF3; font-size: 13px; font-weight: 600;");
  diskLayout->addWidget(m_DiskSpaceLabel);
  
  m_DiskSpaceBar = new QProgressBar(diskCard);
  m_DiskSpaceBar->setRange(0, 100);
  m_DiskSpaceBar->setValue(0);
  m_DiskSpaceBar->setTextVisible(false);
  m_DiskSpaceBar->setFixedHeight(8);
  m_DiskSpaceBar->setStyleSheet(R"(
    QProgressBar { background-color: #21262D; border: none; border-radius: 4px; }
    QProgressBar::chunk { background-color: #3FB950; border-radius: 4px; }
  )");
  diskLayout->addWidget(m_DiskSpaceBar);
  
  diskLayout->addStretch();
  grid->addWidget(diskCard, 1, 2);

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
  
  m_NetRecvLabel->setText(QString::fromUtf8("\xe2\x86\x93 %1 Kbps").arg(QString::number(recvKbps, 'f', 1)));
  m_NetSentLabel->setText(QString::fromUtf8("\xe2\x86\x91 %1 Kbps").arg(QString::number(sentKbps, 'f', 1)));

  // Update System Info
  m_UptimeLabel->setText(formatUptime(snapshot.uptimeSeconds));
  m_ProcessCountLabel->setText(QString::number(snapshot.cpu.processCount));
  m_ThreadCountLabel->setText(QString::number(snapshot.cpu.threadCount));

  // Update GPU (use first GPU if available)
  if (!snapshot.gpus.empty()) {
    const auto& gpu = snapshot.gpus[0];
    m_GpuNameLabel->setText(QString::fromStdString(gpu.name));
    
    QString vramUsed = formatBytes(gpu.dedicatedMemoryUsedBytes);
    QString vramTotal = formatBytes(gpu.dedicatedMemoryTotalBytes);
    m_GpuVramLabel->setText(vramUsed + " / " + vramTotal);
    
    int vramPercent = 0;
    if (gpu.dedicatedMemoryTotalBytes > 0) {
      vramPercent = static_cast<int>((static_cast<double>(gpu.dedicatedMemoryUsedBytes) / gpu.dedicatedMemoryTotalBytes) * 100.0);
    }
    m_GpuVramBar->setValue(vramPercent);
    
    // Color the bar based on usage
    if (vramPercent > 85) {
      m_GpuVramBar->setStyleSheet(R"(
        QProgressBar { background-color: #21262D; border: none; border-radius: 4px; }
        QProgressBar::chunk { background-color: #F85149; border-radius: 4px; }
      )");
    } else if (vramPercent > 60) {
      m_GpuVramBar->setStyleSheet(R"(
        QProgressBar { background-color: #21262D; border: none; border-radius: 4px; }
        QProgressBar::chunk { background-color: #D29922; border-radius: 4px; }
      )");
    } else {
      m_GpuVramBar->setStyleSheet(R"(
        QProgressBar { background-color: #21262D; border: none; border-radius: 4px; }
        QProgressBar::chunk { background-color: #A371F7; border-radius: 4px; }
      )");
    }
  } else {
    m_GpuNameLabel->setText("No GPU detected");
    m_GpuVramLabel->setText("N/A");
    m_GpuVramBar->setValue(0);
  }

  // Update Disk (use primary / C: drive if available, else first)
  if (!snapshot.disks.empty()) {
    // Find C: drive, or use first
    const core::metrics::DiskMetrics* primaryDisk = &snapshot.disks[0];
    for (const auto& disk : snapshot.disks) {
      if (disk.name == "C:") {
        primaryDisk = &disk;
        break;
      }
    }

    m_DiskNameLabel->setText(QString::fromStdString(primaryDisk->name) + " Drive");
    
    uint64_t usedSpace = primaryDisk->totalSpaceBytes - primaryDisk->freeSpaceBytes;
    QString usedStr = formatBytes(usedSpace);
    QString totalStr = formatBytes(primaryDisk->totalSpaceBytes);
    m_DiskSpaceLabel->setText(usedStr + " / " + totalStr);
    
    int diskPercent = 0;
    if (primaryDisk->totalSpaceBytes > 0) {
      diskPercent = static_cast<int>((static_cast<double>(usedSpace) / primaryDisk->totalSpaceBytes) * 100.0);
    }
    m_DiskSpaceBar->setValue(diskPercent);
    
    // Color based on usage
    if (diskPercent > 90) {
      m_DiskSpaceBar->setStyleSheet(R"(
        QProgressBar { background-color: #21262D; border: none; border-radius: 4px; }
        QProgressBar::chunk { background-color: #F85149; border-radius: 4px; }
      )");
    } else if (diskPercent > 75) {
      m_DiskSpaceBar->setStyleSheet(R"(
        QProgressBar { background-color: #21262D; border: none; border-radius: 4px; }
        QProgressBar::chunk { background-color: #D29922; border-radius: 4px; }
      )");
    } else {
      m_DiskSpaceBar->setStyleSheet(R"(
        QProgressBar { background-color: #21262D; border: none; border-radius: 4px; }
        QProgressBar::chunk { background-color: #3FB950; border-radius: 4px; }
      )");
    }
  } else {
    m_DiskNameLabel->setText("No disks detected");
    m_DiskSpaceLabel->setText("N/A");
    m_DiskSpaceBar->setValue(0);
  }
}

} // namespace severance::gui::dashboard
