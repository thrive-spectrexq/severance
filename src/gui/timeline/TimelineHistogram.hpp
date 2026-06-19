#pragma once

#include <QWidget>
#include <vector>
#include <cstdint>

namespace severance::gui::timeline {

class TimelineHistogram : public QWidget {
  Q_OBJECT

public:
  explicit TimelineHistogram(QWidget* parent = nullptr);
  ~TimelineHistogram() override;

  // Add a new event timestamp (ms since epoch)
  void addEvent(uint64_t timestampMS);
  
  // Replace all events (e.g. on load)
  void setEvents(const std::vector<uint64_t>& timestamps);

signals:
  // Emitted when the user selects a time range via dragging or zooming
  // 0 means no filter
  void timeRangeSelected(uint64_t startMS, uint64_t endMS);

protected:
  void paintEvent(QPaintEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  void updateBuckets();

  std::vector<uint64_t> m_Timestamps;
  std::vector<int> m_Buckets; // Computed frequencies for rendering

  uint64_t m_ViewStartMS{0};
  uint64_t m_ViewEndMS{0};
  
  uint64_t m_DataMinMS{0};
  uint64_t m_DataMaxMS{0};

  // Selection
  bool m_IsSelecting{false};
  int m_SelectStartX{0};
  int m_SelectCurrentX{0};

  uint64_t pixelToTime(int px) const;
  int timeToPixel(uint64_t tMS) const;
};

} // namespace severance::gui::timeline
