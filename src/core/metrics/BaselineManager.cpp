#include "BaselineManager.hpp"
#include "core/ai/AiEngine.hpp"
#include "core/notifications/NotificationManager.hpp"
#include <spdlog/spdlog.h>
#include <numeric>

#ifdef _WIN32
#include "WindowsMetricsProvider.hpp"
#endif

namespace severance::core::metrics {

BaselineManager::BaselineManager(QObject* parent) : QObject(parent) {
#ifdef _WIN32
  m_MetricsProvider = std::make_unique<WindowsMetricsProvider>();
#endif
  
  m_Timer = new QTimer(this);
  connect(m_Timer, &QTimer::timeout, this, &BaselineManager::onPoll);
}

BaselineManager::~BaselineManager() {
  Stop();
}

void BaselineManager::Start() {
  if (m_Running || !m_MetricsProvider) return;
  
  m_Running = true;
  m_BaselineEstablished = false;
  m_SamplesCollected = 0;
  
  m_CpuSamples.clear();
  m_MemSamples.clear();
  m_NetSamples.clear();
  
  m_Timer->start(1000); // Poll every 1 second
  spdlog::info("BaselineManager started. Recording baseline for {} seconds.", m_RequiredSamples);
}

void BaselineManager::Stop() {
  m_Running = false;
  if (m_Timer) m_Timer->stop();
}

void BaselineManager::onPoll() {
  if (!m_MetricsProvider) return;
  
  auto snapshot = m_MetricsProvider->GetSnapshot();
  
  if (!m_BaselineEstablished) {
    recordBaseline(snapshot);
  } else {
    monitorForAnomalies(snapshot);
  }
}

void BaselineManager::recordBaseline(const SystemMetricsSnapshot& snapshot) {
  m_CpuSamples.push_back(snapshot.cpu.globalUsagePercent);
  m_MemSamples.push_back(static_cast<double>(snapshot.memory.usedBytes));
  m_NetSamples.push_back(static_cast<double>(snapshot.network.totalBytesReceivedPerSec));
  
  m_SamplesCollected++;
  
  if (m_SamplesCollected >= m_RequiredSamples) {
    m_AvgCpu = std::accumulate(m_CpuSamples.begin(), m_CpuSamples.end(), 0.0) / m_RequiredSamples;
    m_AvgMem = std::accumulate(m_MemSamples.begin(), m_MemSamples.end(), 0.0) / m_RequiredSamples;
    m_AvgNetRecv = std::accumulate(m_NetSamples.begin(), m_NetSamples.end(), 0.0) / m_RequiredSamples;
    
    m_BaselineEstablished = true;
    spdlog::info("Baseline Established. CPU Avg: {:.2f}%, Mem Avg: {:.2f} GB", m_AvgCpu, m_AvgMem / 1073741824.0);
  }
}

void BaselineManager::monitorForAnomalies(const SystemMetricsSnapshot& snapshot) {
  // Simple heuristics:
  // - CPU usage spikes to > 90% when average was < 50%
  // - Or CPU usage is 2x the average + 20%
  
  double currentCpu = snapshot.cpu.globalUsagePercent;
  
  bool anomaly = false;
  QString explanation;
  
  if (currentCpu > (m_AvgCpu * 2.0 + 20.0) || (currentCpu > 90.0 && m_AvgCpu < 50.0)) {
    anomaly = true;
    explanation += QString("Sudden CPU Spike Detected. Current: %1%, Baseline: %2%.\n")
                      .arg(currentCpu, 0, 'f', 1).arg(m_AvgCpu, 0, 'f', 1);
  }
  
  auto currentMem = static_cast<double>(snapshot.memory.usedBytes);
  if (currentMem > (m_AvgMem * 1.5)) { // 50% increase in total system memory usage
    anomaly = true;
    explanation += QString("Massive Memory Consumption Detected. Current: %1 GB, Baseline: %2 GB.\n")
                      .arg(currentMem / 1073741824.0, 0, 'f', 1).arg(m_AvgMem / 1073741824.0, 0, 'f', 1);
  }
  
  if (anomaly) {
    spdlog::warn("Baseline Anomaly Detected: {}", explanation.toStdString());
    
    // 1. Emit notification
    notifications::Notification n;
    n.id = "baseline_anomaly"; // simplistic
    n.timestamp = snapshot.timestampEpochMs;
    n.severity = notifications::NotificationSeverity::Warning;
    n.title = "System Baseline Anomaly";
    n.message = explanation.toStdString();
    n.source = "BaselineManager";
    notifications::NotificationManager::GetInstance().EmitNotification(n);

    // 2. Request AI Explanation
    ai::AiEngine::GetInstance().explainAnomaly(explanation);
    
    emit baselineAnomalyDetected(explanation);
    
    // Reset baseline so we don't spam
    // In a real system, you might flag it and continue monitoring, or establish a new baseline
    m_BaselineEstablished = false;
    m_SamplesCollected = 0;
    m_CpuSamples.clear();
    m_MemSamples.clear();
    m_NetSamples.clear();
  }
}

} // namespace severance::core::metrics
