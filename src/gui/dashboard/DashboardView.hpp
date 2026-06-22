#pragma once

#include "core/metrics/SystemMetrics.hpp"
#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include <QTableWidget>
#include <memory>

namespace severance::core::metrics {
  class MetricsProvider;
}

namespace severance::gui::widgets {
  class DonutChartWidget;
  class HorizontalBarChartWidget;
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

  // KPI Cards (Top Row)
  QLabel* m_KpiProcessCount{nullptr};
  QLabel* m_KpiNetConnections{nullptr};
  QLabel* m_KpiFileHandles{nullptr};
  QLabel* m_KpiAlerts{nullptr};

  // Charts (Middle Row)
  widgets::DonutChartWidget* m_CpuDonut{nullptr};
  widgets::HorizontalBarChartWidget* m_TopProcessesBar{nullptr};

  // Status Labels for Charts
  QLabel* m_CpuValueLabel{nullptr};

  // Bottom Table
  QTableWidget* m_RecentEventsTable{nullptr};

  // Model
  std::unique_ptr<core::metrics::MetricsProvider> m_MetricsProvider;

  // Refresh timer
  QTimer* m_RefreshTimer{nullptr};
};

} // namespace severance::gui::dashboard
