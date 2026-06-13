#include "CpuWidget.hpp"
#include <QLabel>
#include <QVBoxLayout>

namespace severance::gui::widgets {

CpuWidget::CpuWidget(QWidget *parent) : QWidget(parent) {
  auto layout = new QVBoxLayout(this);
  auto title = new QLabel("CPU Usage");
  title->setStyleSheet("font-weight: bold;");

  auto mockData = new QLabel("0% (Idle)");

  layout->addWidget(title);
  layout->addWidget(mockData);
  layout->addStretch();
}

} // namespace severance::gui::widgets
