#include "ResourceGraph.hpp"
#include <QPainter>
#include <QPainterPath>
#include <algorithm>

namespace severance::gui::graphs {

ResourceGraph::ResourceGraph(QWidget *parent) : QWidget(parent) {
  setMinimumHeight(100);
  
  // Seed with 0s
  for (size_t i = 0; i < m_MaxPoints; ++i) {
      m_Data.push_back(0.0);
  }

  m_Timer = new QTimer(this);
  connect(m_Timer, &QTimer::timeout, this, &ResourceGraph::updateData);
  m_Timer->start(1000); // 1 Hz update
}

void ResourceGraph::addDataPoint(double value) {
  m_Data.push_back(value);
  if (m_Data.size() > m_MaxPoints) {
    m_Data.erase(m_Data.begin());
  }
  update();
}

void ResourceGraph::clear() {
  std::fill(m_Data.begin(), m_Data.end(), 0.0);
  update();
}

void ResourceGraph::updateData() {
  // Simulate live data for demonstration (e.g. 20-80% usage)
  // In a real scenario, this would query System/Process stats
  double randomValue = 20.0 + (rand() % 60);
  addDataPoint(randomValue);
}

void ResourceGraph::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // Background
  painter.fillRect(rect(), QColor("#0A0E14"));

  if (m_Data.empty()) return;

  // Find max for scaling
  double maxVal = *std::max_element(m_Data.begin(), m_Data.end());
  if (maxVal < 1.0) maxVal = 1.0;

  float stepX = width() / static_cast<float>(m_MaxPoints - 1);
  float scaleY = height() / static_cast<float>(maxVal);

  QPainterPath path;
  path.moveTo(0, height()); // Start at bottom left

  for (size_t i = 0; i < m_Data.size(); ++i) {
    float x = i * stepX;
    float y = height() - (m_Data[i] * scaleY);
    if (i == 0) {
      path.moveTo(x, y);
    } else {
      path.lineTo(x, y);
    }
  }

  // Fill area under the curve
  QPainterPath fillPath = path;
  fillPath.lineTo(width(), height());
  fillPath.lineTo(0, height());
  
  QColor fillColor("#1A7A5C");
  fillColor.setAlpha(50);
  painter.fillPath(fillPath, fillColor);

  painter.setPen(QPen(QColor("#7FDBCA"), 2));
  painter.drawPath(path);
}

} // namespace severance::gui::graphs
