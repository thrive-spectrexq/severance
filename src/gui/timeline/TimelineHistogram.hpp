#pragma once
#include <QWidget>

namespace severance::gui::timeline {

class TimelineHistogram : public QWidget {
  Q_OBJECT

public:
  explicit TimelineHistogram(QWidget* parent = nullptr);
  ~TimelineHistogram() override = default;

protected:
  void paintEvent(QPaintEvent* event) override;
};

} // namespace severance::gui::timeline
