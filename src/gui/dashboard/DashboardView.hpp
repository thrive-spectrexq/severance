#pragma once

#include "core/metrics/SystemMetrics.hpp"
#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <memory>

namespace severance::core::metrics {
  class MetricsProvider;
}

namespace severance::gui::graphs {
  class OpenGLResourceGraph;
}

namespace severance::gui::dashboard {

class DashboardView : public QWidget {
  Q_OBJECT

public:
  explicit DashboardView(QWidget *parent = nullptr);
  ~DashboardView() override;

private slots:
  void onRefreshTimer();

private:
  void setupUI();
  void updateDashboard(const core::metrics::SystemMetricsSnapshot& snapshot);

  // CPU Card
  QLabel* m_CpuValueLabel{nullptr};
  QLabel* m_CpuNameLabel{nullptr};
  graphs::OpenGLResourceGraph* m_CpuGraph{nullptr};

  // Memory Card
  QLabel* m_MemValueLabel{nullptr};
  QLabel* m_MemTotalLabel{nullptr};
  graphs::OpenGLResourceGraph* m_MemGraph{nullptr};

  // Network Card
  QLabel* m_NetRecvLabel{nullptr};
  QLabel* m_NetSentLabel{nullptr};

  // GPU Card
  QLabel* m_GpuNameLabel{nullptr};
  QLabel* m_GpuValueLabel{nullptr};
  QLabel* m_GpuVramLabel{nullptr};
  QProgressBar* m_GpuVramBar{nullptr};

  // Disk Card
  QLabel* m_DiskNameLabel{nullptr};
  QLabel* m_DiskSpaceLabel{nullptr};
  QProgressBar* m_DiskSpaceBar{nullptr};

  // System Info Card
  QLabel* m_UptimeLabel{nullptr};
  QLabel* m_ProcessCountLabel{nullptr};
  QLabel* m_ThreadCountLabel{nullptr};

  // Model
  std::unique_ptr<core::metrics::MetricsProvider> m_MetricsProvider;

  // Refresh timer
  QTimer* m_RefreshTimer{nullptr};
};

} // namespace severance::gui::dashboard
