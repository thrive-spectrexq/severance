#pragma once

#include <QWidget>

namespace severance::gui::widgets {

class CpuWidget : public QWidget {
  Q_OBJECT
public:
  explicit CpuWidget(QWidget *parent = nullptr);
};

} // namespace severance::gui::widgets
