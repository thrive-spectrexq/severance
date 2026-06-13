#include "StatusBarWidget.hpp"
#include <QHBoxLayout>
#include <QLabel>

namespace severance::gui::widgets {

StatusBarWidget::StatusBarWidget(QWidget *parent) : QWidget(parent) {
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(5, 5, 5, 5);

  auto statusText = new QLabel("System status: OK");
  layout->addWidget(statusText);
  layout->addStretch();
}

} // namespace severance::gui::widgets
