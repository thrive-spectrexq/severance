#include "TimelineHistogram.hpp"
#include <QPainter>
#include <QWheelEvent>
#include <QMouseEvent>
#include <algorithm>
#include <QDateTime>

namespace severance::gui::timeline {

TimelineHistogram::TimelineHistogram(QWidget* parent) : QWidget(parent) {
  setMinimumHeight(60);
  setFixedHeight(80);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  setMouseTracking(true); // For hover effects if needed later
}

TimelineHistogram::~TimelineHistogram() = default;

void TimelineHistogram::addEvent(uint64_t timestampMS) {
  m_Timestamps.push_back(timestampMS);
  
  if (m_Timestamps.size() == 1) {
    m_DataMinMS = timestampMS;
    m_DataMaxMS = timestampMS + 1000; // default 1s spread
    m_ViewStartMS = m_DataMinMS;
    m_ViewEndMS = m_DataMaxMS;
  } else {
    m_DataMinMS = std::min(m_DataMinMS, timestampMS);
    m_DataMaxMS = std::max(m_DataMaxMS, timestampMS);
    
    // Auto-scroll if we are looking at the live edge
    if (m_ViewEndMS >= m_DataMaxMS - 5000) { // Within 5 seconds of the edge
       uint64_t span = m_ViewEndMS - m_ViewStartMS;
       m_ViewEndMS = m_DataMaxMS + 1000; // padding
       m_ViewStartMS = m_ViewEndMS > span ? m_ViewEndMS - span : 0;
    }
  }
  
  updateBuckets();
  update();
}

void TimelineHistogram::setEvents(const std::vector<uint64_t>& timestamps) {
  m_Timestamps = timestamps;
  std::sort(m_Timestamps.begin(), m_Timestamps.end());
  
  if (!m_Timestamps.empty()) {
    m_DataMinMS = m_Timestamps.front();
    m_DataMaxMS = m_Timestamps.back() + 1000; // padding
    m_ViewStartMS = m_DataMinMS;
    m_ViewEndMS = m_DataMaxMS;
  }
  
  updateBuckets();
  update();
}

void TimelineHistogram::updateBuckets() {
  if (m_Timestamps.empty() || width() <= 0) {
    m_Buckets.clear();
    return;
  }

  int numBuckets = width();
  m_Buckets.assign(numBuckets, 0);

  uint64_t span = m_ViewEndMS - m_ViewStartMS;
  if (span == 0) span = 1;

  for (auto ts : m_Timestamps) {
    if (ts >= m_ViewStartMS && ts <= m_ViewEndMS) {
      int bucketIdx = static_cast<int>(((ts - m_ViewStartMS) * numBuckets) / span);
      if (bucketIdx >= 0 && bucketIdx < numBuckets) {
        m_Buckets[bucketIdx]++;
      }
    }
  }
}

uint64_t TimelineHistogram::pixelToTime(int px) const {
  if (width() <= 0) return m_ViewStartMS;
  uint64_t span = m_ViewEndMS - m_ViewStartMS;
  return m_ViewStartMS + (static_cast<uint64_t>(px) * span) / width();
}

int TimelineHistogram::timeToPixel(uint64_t tMS) const {
  uint64_t span = m_ViewEndMS - m_ViewStartMS;
  if (span == 0 || width() <= 0) return 0;
  return static_cast<int>(((tMS - m_ViewStartMS) * width()) / span);
}

void TimelineHistogram::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // Background
  painter.fillRect(rect(), QColor("#0D1117"));

  // Draw axis line
  painter.setPen(QColor("#30363D"));
  painter.drawLine(0, height() - 1, width(), height() - 1);

  if (m_Buckets.empty() || width() <= 0) {
    painter.setPen(QColor("#8B949E"));
    painter.drawText(rect(), Qt::AlignCenter, "No Activity Data");
    return;
  }

  // Find max bucket for scaling
  int maxCount = 1;
  for (int count : m_Buckets) {
    if (count > maxCount) maxCount = count;
  }

  // Draw bars
  painter.setPen(Qt::NoPen);
  for (int x = 0; x < m_Buckets.size(); ++x) {
    if (m_Buckets[x] > 0) {
      int barHeight = static_cast<int>((m_Buckets[x] * (height() - 4)) / maxCount);
      if (barHeight < 2) barHeight = 2; // minimum visible bar
      
      QRect barRect(x, height() - 1 - barHeight, 1, barHeight);
      painter.fillRect(barRect, QColor("#58A6FF"));
    }
  }

  // Draw selection overlay
  if (m_IsSelecting) {
    int startX = std::min(m_SelectStartX, m_SelectCurrentX);
    int endX = std::max(m_SelectStartX, m_SelectCurrentX);
    QRect selRect(startX, 0, endX - startX, height());
    painter.fillRect(selRect, QColor(88, 166, 255, 60)); // Transparent blue
    painter.setPen(QColor("#58A6FF"));
    painter.drawLine(startX, 0, startX, height());
    painter.drawLine(endX, 0, endX, height());
  }

  // Draw simple time labels
  painter.setPen(QColor("#8B949E"));
  QString startLabel = QDateTime::fromMSecsSinceEpoch(m_ViewStartMS).toString("HH:mm:ss");
  QString endLabel = QDateTime::fromMSecsSinceEpoch(m_ViewEndMS).toString("HH:mm:ss");
  painter.drawText(QRect(4, 2, 100, 20), Qt::AlignLeft, startLabel);
  painter.drawText(QRect(width() - 104, 2, 100, 20), Qt::AlignRight, endLabel);
}

void TimelineHistogram::wheelEvent(QWheelEvent* event) {
  int numDegrees = event->angleDelta().y() / 8;
  int numSteps = numDegrees / 15;
  
  if (numSteps == 0) return;

  uint64_t span = m_ViewEndMS - m_ViewStartMS;
  if (span == 0) span = 1000;

  // Zoom factor: 10% per step
  double zoomFactor = 1.0 - (numSteps * 0.1);
  if (zoomFactor < 0.1) zoomFactor = 0.1;
  if (zoomFactor > 2.0) zoomFactor = 2.0;

  uint64_t newSpan = static_cast<uint64_t>(span * zoomFactor);
  if (newSpan < 100) newSpan = 100; // Max zoom: 100ms
  
  // Center zoom on mouse pointer
  uint64_t mouseTime = pixelToTime(event->position().x());
  
  // Ratio of mouse pos to total width
  double mouseRatio = event->position().x() / static_cast<double>(width());
  
  uint64_t newStart = mouseTime - static_cast<uint64_t>(newSpan * mouseRatio);
  uint64_t newEnd = newStart + newSpan;

  // Don't zoom out past data bounds (plus some padding)
  if (newStart < m_DataMinMS) newStart = m_DataMinMS;
  if (newEnd > m_DataMaxMS + 1000) newEnd = m_DataMaxMS + 1000;

  m_ViewStartMS = newStart;
  m_ViewEndMS = newEnd;
  
  updateBuckets();
  update();
  
  // Emit signal with updated view range
  emit timeRangeSelected(m_ViewStartMS, m_ViewEndMS);
}

void TimelineHistogram::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    m_IsSelecting = true;
    m_SelectStartX = event->position().x();
    m_SelectCurrentX = m_SelectStartX;
    update();
  } else if (event->button() == Qt::RightButton) {
    // Reset selection/zoom
    m_ViewStartMS = m_DataMinMS;
    m_ViewEndMS = m_DataMaxMS + 1000;
    updateBuckets();
    update();
    emit timeRangeSelected(0, 0); // Reset filter
  }
}

void TimelineHistogram::mouseMoveEvent(QMouseEvent* event) {
  if (m_IsSelecting) {
    m_SelectCurrentX = event->position().x();
    update();
  }
}

void TimelineHistogram::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton && m_IsSelecting) {
    m_IsSelecting = false;
    m_SelectCurrentX = event->position().x();
    
    if (std::abs(m_SelectStartX - m_SelectCurrentX) > 5) { // Minimum drag width
      int startX = std::min(m_SelectStartX, m_SelectCurrentX);
      int endX = std::max(m_SelectStartX, m_SelectCurrentX);
      
      uint64_t filterStart = pixelToTime(startX);
      uint64_t filterEnd = pixelToTime(endX);
      
      // We could zoom in or just emit filter. Let's zoom in.
      m_ViewStartMS = filterStart;
      m_ViewEndMS = filterEnd;
      updateBuckets();
      
      emit timeRangeSelected(filterStart, filterEnd);
    }
    update();
  }
}

} // namespace severance::gui::timeline
