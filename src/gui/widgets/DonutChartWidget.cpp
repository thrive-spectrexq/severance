#include "DonutChartWidget.hpp"
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QRectF>
#include <cmath>

namespace severance::gui::widgets {

DonutChartWidget::DonutChartWidget(QWidget* parent)
  : QWidget(parent) {
  setMinimumSize(150, 150);
}

void DonutChartWidget::setSegments(const std::vector<DataSegment>& segments) {
  m_Segments = segments;
  m_TotalValue = 0.0f;
  for (const auto& seg : m_Segments) {
    m_TotalValue += seg.value;
  }
  update();
}

void DonutChartWidget::setCenterText(const QString& text) {
  m_CenterText = text;
  update();
}

void DonutChartWidget::setDonutThickness(int thickness) {
  m_Thickness = thickness;
  update();
}

void DonutChartWidget::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  int size = std::min(width(), height()) - 10;
  if (size <= 0) return;

  QRectF rect(width() / 2.0 - size / 2.0, height() / 2.0 - size / 2.0, size, size);

  if (m_TotalValue <= 0.0f) {
    // Empty state
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#2A3441"));
    painter.drawPie(rect, 0, 360 * 16);
  } else {
    float startAngle = 90.0f; // Start at top
    for (const auto& seg : m_Segments) {
      if (seg.value <= 0) continue;

      float spanAngle = (seg.value / m_TotalValue) * -360.0f; // Negative to draw clockwise

      painter.setPen(Qt::NoPen);
      painter.setBrush(seg.color);
      painter.drawPie(rect, static_cast<int>(startAngle * 16), static_cast<int>(spanAngle * 16));

      startAngle += spanAngle;
    }
  }

  // Draw inner circle to make it a donut
  float innerSize = size - (m_Thickness * 2);
  if (innerSize > 0) {
    QRectF innerRect(width() / 2.0 - innerSize / 2.0, height() / 2.0 - innerSize / 2.0, innerSize, innerSize);
    painter.setBrush(QColor("#151A23")); // Background color of card
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(innerRect);
  }

  // Draw center text
  if (!m_CenterText.isEmpty()) {
    painter.setPen(QColor("#F1F5F9"));
    QFont font = painter.font();
    font.setPixelSize(18);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(rect, Qt::AlignCenter, m_CenterText);
  }
}

} // namespace severance::gui::widgets
