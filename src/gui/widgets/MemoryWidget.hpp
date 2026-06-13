#pragma once

#include <QWidget>

namespace severance::gui::widgets {

class MemoryWidget : public QWidget {
  Q_OBJECT
public:
  explicit MemoryWidget(QWidget *parent = nullptr);
};

} // namespace severance::gui::widgets
