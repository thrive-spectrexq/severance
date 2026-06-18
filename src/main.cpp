#include "core/application/Application.hpp"
#include "gui/windows/MainWindow.hpp"
#include "gui/theme/Theme.hpp"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication qtApp(argc, argv);

  severance::core::application::Application app;
  app.Run();

  severance::gui::theme::ApplyDarkTheme(&qtApp);

  severance::gui::windows::MainWindow mainWindow;
  mainWindow.show();

  return qtApp.exec();
}
