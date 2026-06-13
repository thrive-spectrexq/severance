#include "MemoryWidget.hpp"
#include <QLabel>
#include <QVBoxLayout>

namespace severance::gui::widgets {

MemoryWidget::MemoryWidget(QWidget *parent) : QWidget(parent) {
  auto layout = new QVBoxLayout(this);
  auto title = new QLabel("Memory Usage");
  title->setStyleSheet("font-weight: bold;");

  auto mockData = new QLabel("0 MB / 0 MB");

  layout->addWidget(title);
  layout->addWidget(mockData);
  layout->addStretch();
}

} // namespace severance::gui::widgets
