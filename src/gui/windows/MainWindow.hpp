#pragma once

#include <QMainWindow>

namespace severance::gui::windows {

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
};

} // namespace severance::gui::windows
