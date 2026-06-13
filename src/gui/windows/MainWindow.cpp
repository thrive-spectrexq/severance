#include "MainWindow.hpp"
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

namespace severance::gui::windows {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Severance");
  resize(800, 600);

  auto centralWidget = new QWidget(this);
  auto layout = new QVBoxLayout(centralWidget);

  auto label = new QLabel("Welcome to Severance Dashboard", this);
  label->setAlignment(Qt::AlignCenter);

  layout->addWidget(label);
  setCentralWidget(centralWidget);
}

MainWindow::~MainWindow() = default;

} // namespace severance::gui::windows
