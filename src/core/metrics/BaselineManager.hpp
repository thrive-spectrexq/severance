#pragma once

#include "core/metrics/SystemMetrics.hpp"
#include <QObject>
#include <QTimer>
#include <memory>
#include <vector>

namespace severance::core::metrics {

class BaselineManager : public QObject {
  Q_OBJECT

public:
  BaselineManager(QObject* parent = nullptr);
  ~BaselineManager() override;

  void Start();
  void Stop();

signals:
  void baselineAnomalyDetected(const QString& anomalyData);

private slots:
  void onPoll();

private:
  void recordBaseline(const SystemMetricsSnapshot& snapshot);
  void monitorForAnomalies(const SystemMetricsSnapshot& snapshot);

  std::unique_ptr<MetricsProvider> m_MetricsProvider;
  QTimer* m_Timer{nullptr};
  
  bool m_Running{false};
  bool m_BaselineEstablished{false};
  
  int m_SamplesCollected{0};
  const int m_RequiredSamples{30}; // 30 seconds at 1Hz

  // Baseline Aggregates
  double m_AvgCpu{0.0};
  double m_AvgMem{0.0};
  double m_AvgNetRecv{0.0};
  
  std::vector<double> m_CpuSamples;
  std::vector<double> m_MemSamples;
  std::vector<double> m_NetSamples;
};

} // namespace severance::core::metrics
