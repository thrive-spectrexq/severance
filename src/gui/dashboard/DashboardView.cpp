#include "DashboardView.hpp"
#include "gui/widgets/NumberGridWidget.hpp"
#include <QVBoxLayout>

namespace severance::gui::dashboard {

DashboardView::DashboardView(QWidget *parent) : QWidget(parent) {
  setupUI();
}

DashboardView::~DashboardView() = default;

void DashboardView::setupUI() {
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  m_NumberGrid = new widgets::NumberGridWidget(this);
  mainLayout->addWidget(m_NumberGrid);
}

} // namespace severance::gui::dashboard
