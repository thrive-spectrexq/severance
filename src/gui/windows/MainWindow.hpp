#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QShortcut>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <vector>
#include <memory>

namespace severance::gui::search { class SearchOverlay; }

namespace severance::gui::dashboard { class DashboardView; }
namespace severance::gui::process_view { class ProcessView; }
namespace severance::gui::network_view { class NetworkView; }
namespace severance::gui::file_view { class FileView; }
namespace severance::gui::optics_and_design { class OpticsDesignView; }
namespace severance::gui::perimeter_grid { class PerimeterGridView; }
namespace severance::gui::terminal { class TerminalOverlay; }

namespace severance::gui::windows {

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void onSidebarButtonClicked(int index);
  void onSearchTriggered();
  void onCommandPaletteTriggered();

private:
  void setupSidebar();
  void setupViews();
  void setupStatusBar();
  void setupShortcuts();
  void setupSystemTray();
  void setActiveView(int index);

public slots:
  void showSystemNotification(const QString& title, const QString& message, QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information);
  void showHandbook();

private:
  // Sidebar
  QWidget* m_Sidebar{nullptr};
  QVBoxLayout* m_SidebarLayout{nullptr};
  std::vector<QPushButton*> m_SidebarButtons;
  int m_ActiveViewIndex{0};

  // Central content
  QStackedWidget* m_ViewStack{nullptr};

  // Views
  dashboard::DashboardView* m_DashboardView{nullptr};
  process_view::ProcessView* m_ProcessView{nullptr};
  network_view::NetworkView* m_NetworkView{nullptr};
  file_view::FileView* m_FileView{nullptr};
  optics_and_design::OpticsDesignView* m_OpticsDesignView{nullptr};
  perimeter_grid::PerimeterGridView* m_PerimeterGridView{nullptr};

  // Search
  search::SearchOverlay* m_SearchOverlay{nullptr};

  // Status bar widgets
  QTimer* m_ShiftTimer{nullptr};
  QLabel* m_ShiftLabel{nullptr};
  QLabel* m_ComplianceLabel{nullptr};
  QLabel* m_KierQuoteLabel{nullptr};
  QTimer* m_KierQuoteTimer{nullptr};
  int m_ShiftSeconds{0};
  int m_KierQuoteIndex{0};
  QLabel* m_StatusRecording{nullptr};

  // System Tray
  QSystemTrayIcon* m_TrayIcon{nullptr};
  QMenu* m_TrayMenu{nullptr};

  // Sidebar button labels and icons
  struct ViewInfo {
    QString name;
    QString icon;     // Unicode fallback, will use SVG later
    QString shortcut;
  };
  std::vector<ViewInfo> m_ViewInfos;

  // View transition engine
  QGraphicsOpacityEffect* m_ViewStackOpacityEffect{nullptr};
  QPropertyAnimation* m_ViewFadeAnimation{nullptr};

  // Terminal
  terminal::TerminalOverlay* m_TerminalOverlay{nullptr};
};

} // namespace severance::gui::windows
