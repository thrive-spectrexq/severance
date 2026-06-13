#pragma once

#include <QWidget>

namespace severance::gui::dashboard {

class DashboardView : public QWidget {
  Q_OBJECT
public:
  explicit DashboardView(QWidget *parent = nullptr);
};

} // namespace severance::gui::dashboard
