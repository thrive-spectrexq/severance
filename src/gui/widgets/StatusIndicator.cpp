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
    case Status::Online: return QColor("#2ECC71");  // Lumon bright green — neural impulse active
    case Status::Warning: return QColor("#D4A017"); // Institutional amber
    case Status::Error: return QColor("#C0392B");   // Clinical red
    case Status::Offline:
    default: return QColor("#3D4F5F"); // Faint gray — dormant
  }
}

} // namespace severance::gui::widgets
