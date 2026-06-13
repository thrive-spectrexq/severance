#pragma once

#include <QWidget>

namespace severance::gui::widgets {

class StatusBarWidget : public QWidget {
  Q_OBJECT
public:
  explicit StatusBarWidget(QWidget *parent = nullptr);
};

} // namespace severance::gui::widgets
