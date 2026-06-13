#include "DashboardView.hpp"
#include "gui/widgets/CpuWidget.hpp"
#include "gui/widgets/MemoryWidget.hpp"
#include "gui/widgets/NetworkWidget.hpp"
#include "gui/widgets/StatusBarWidget.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>

namespace severance::gui::dashboard {

DashboardView::DashboardView(QWidget *parent) : QWidget(parent) {
  auto mainLayout = new QVBoxLayout(this);

  // Top row with hardware metrics
  auto metricsLayout = new QHBoxLayout();
  metricsLayout->addWidget(new widgets::CpuWidget(this));
  metricsLayout->addWidget(new widgets::MemoryWidget(this));
  metricsLayout->addWidget(new widgets::NetworkWidget(this));

  mainLayout->addLayout(metricsLayout);
  mainLayout->addStretch(); // Push everything to top

  // Bottom status bar
  mainLayout->addWidget(new widgets::StatusBarWidget(this));
}

} // namespace severance::gui::dashboard
