#pragma once

#include <QWidget>

namespace severance::gui::widgets {

class NetworkWidget : public QWidget {
  Q_OBJECT
public:
  explicit NetworkWidget(QWidget *parent = nullptr);
};

} // namespace severance::gui::widgets
