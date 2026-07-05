#include "HorizontalBarChartWidget.hpp"
#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QFontMetrics>

namespace severance::gui::widgets {

HorizontalBarChartWidget::HorizontalBarChartWidget(QWidget* parent)
  : QWidget(parent) {
  setMinimumSize(250, 150);
}

void HorizontalBarChartWidget::setBars(const std::vector<DataBar>& bars) {
  m_Bars = bars;
  m_MaxValue = 0.0f;
  for (const auto& bar : m_Bars) {
    if (bar.value > m_MaxValue) {
      m_MaxValue = bar.value;
    }
  }
  update();
}

void HorizontalBarChartWidget::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  if (m_Bars.empty()) return;

  int barSpacing = 12;
  int barHeight = 16;
  int totalHeightPerItem = barHeight + barSpacing;

  int y = 10;
  
  QFont labelFont = painter.font();
  labelFont.setPixelSize(12);
  labelFont.setBold(true);
  
  QFont valueFont = painter.font();
  valueFont.setPixelSize(12);

  QFontMetrics labelFm(labelFont);
  QFontMetrics valueFm(valueFont);

  // Find max label width to align the bars
  int maxLabelWidth = 0;
  for (const auto& bar : m_Bars) {
    int w = labelFm.horizontalAdvance(bar.label);
    if (w > maxLabelWidth) maxLabelWidth = w;
  }
  maxLabelWidth += 10; // Padding

  // Find max value width to reserve space on the right
  int maxValueWidth = 0;
  for (const auto& bar : m_Bars) {
    int w = valueFm.horizontalAdvance(bar.displayValue);
    if (w > maxValueWidth) maxValueWidth = w;
  }
  maxValueWidth += 10; // Padding

  int availableBarWidth = width() - maxLabelWidth - maxValueWidth - 20;
  if (availableBarWidth < 10) availableBarWidth = 10;

  for (const auto& bar : m_Bars) {
    // Draw Label
    painter.setFont(labelFont);
    painter.setPen(QColor("#8B9DAF"));
    QRect labelRect(10, y, maxLabelWidth, barHeight);
    painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, bar.label);

    // Draw Background Bar
    QRect bgBarRect(10 + maxLabelWidth + 10, y + 2, availableBarWidth, barHeight - 4);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor("#132A2E")); // Lumon panel background track
    painter.drawRoundedRect(bgBarRect, 4, 4);

    // Draw Foreground Bar
    if (m_MaxValue > 0.0f) {
      int fillWidth = static_cast<int>((bar.value / m_MaxValue) * availableBarWidth);
      if (fillWidth > 0) {
        QRect fgBarRect(10 + maxLabelWidth + 10, y + 2, fillWidth, barHeight - 4);
        painter.setBrush(bar.color);
        painter.drawRoundedRect(fgBarRect, 4, 4);
      }
    }

    // Draw Value
    painter.setFont(valueFont);
    painter.setPen(QColor("#E8ECEF"));
    QRect valueRect(10 + maxLabelWidth + 10 + availableBarWidth + 10, y, maxValueWidth, barHeight);
    painter.drawText(valueRect, Qt::AlignLeft | Qt::AlignVCenter, bar.displayValue);

    y += totalHeightPerItem;
  }
}

} // namespace severance::gui::widgets
