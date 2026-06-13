#include "NetworkWidget.hpp"
#include <QLabel>
#include <QVBoxLayout>

namespace severance::gui::widgets {

NetworkWidget::NetworkWidget(QWidget *parent) : QWidget(parent) {
  auto layout = new QVBoxLayout(this);
  auto title = new QLabel("Network Activity");
  title->setStyleSheet("font-weight: bold;");

  auto mockData = new QLabel("0 B/s Up | 0 B/s Down");

  layout->addWidget(title);
  layout->addWidget(mockData);
  layout->addStretch();
}

} // namespace severance::gui::widgets
