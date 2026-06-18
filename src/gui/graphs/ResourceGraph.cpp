#include "ResourceGraph.hpp"
#include <QPainter>
#include <QPainterPath>
#include <algorithm>

namespace severance::gui::graphs {

ResourceGraph::ResourceGraph(QWidget *parent) : QWidget(parent) {
  setMinimumHeight(100);
}

void ResourceGraph::addDataPoint(double value) {
  m_Data.push_back(value);
  if (m_Data.size() > m_MaxPoints) {
    m_Data.erase(m_Data.begin());
  }
  update();
}

void ResourceGraph::clear() {
  m_Data.clear();
  update();
}

void ResourceGraph::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // Background
  painter.fillRect(rect(), QColor("#0D1117"));

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

  painter.setPen(QPen(QColor("#58A6FF"), 2));
  painter.drawPath(path);
}

} // namespace severance::gui::graphs
