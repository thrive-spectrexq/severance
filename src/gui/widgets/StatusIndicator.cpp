#include "StatusIndicator.hpp"
#include <QPainter>

namespace severance::gui::widgets {

StatusIndicator::StatusIndicator(QWidget *parent) : QWidget(parent) {
  setFixedSize(12, 12);
}

void StatusIndicator::setStatus(Status status) {
  m_Status = status;
  update();
}

void StatusIndicator::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QColor color = getColorForStatus(m_Status);
  
  painter.setBrush(color);
  painter.setPen(Qt::NoPen);
  painter.drawEllipse(rect());
}

QColor StatusIndicator::getColorForStatus(Status status) const {
  switch (status) {
    case Status::Online: return QColor("#238636"); // Green
    case Status::Warning: return QColor("#D29922"); // Yellow
    case Status::Error: return QColor("#F85149");   // Red
    case Status::Offline:
    default: return QColor("#8B949E"); // Gray
  }
}

} // namespace severance::gui::widgets
