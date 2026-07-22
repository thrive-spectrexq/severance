#pragma once
#include <QWidget>

namespace severance::gui::widgets { class NumberGridWidget; }

namespace severance::gui::dashboard {
class DashboardView : public QWidget {
  Q_OBJECT
public:
  explicit DashboardView(QWidget *parent = nullptr);
  ~DashboardView() override;
private:
  void setupUI();
  widgets::NumberGridWidget* m_NumberGrid{nullptr};
};
} // namespace severance::gui::dashboard
