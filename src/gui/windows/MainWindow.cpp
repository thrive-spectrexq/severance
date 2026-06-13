#include "MainWindow.hpp"
#include "gui/dashboard/DashboardView.hpp"

namespace severance::gui::windows {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Severance");
  resize(800, 600);

  auto dashboard = new dashboard::DashboardView(this);
  setCentralWidget(dashboard);
}

MainWindow::~MainWindow() = default;

} // namespace severance::gui::windows
