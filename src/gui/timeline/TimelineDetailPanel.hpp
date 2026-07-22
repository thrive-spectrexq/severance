#pragma once
#include <QWidget>

namespace severance::gui::timeline {

class TimelineDetailPanel : public QWidget {
  Q_OBJECT

public:
  explicit TimelineDetailPanel(QWidget* parent = nullptr) : QWidget(parent) {}
  ~TimelineDetailPanel() override = default;
};

} // namespace severance::gui::timeline
