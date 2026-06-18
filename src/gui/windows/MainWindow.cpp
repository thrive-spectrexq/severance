#include "MainWindow.hpp"
#include "gui/dashboard/DashboardView.hpp"
#include "gui/process_view/ProcessView.hpp"
#include "gui/network_view/NetworkView.hpp"
#include "gui/file_view/FileView.hpp"
#include "gui/timeline/TimelineView.hpp"
#include "gui/theme/Theme.hpp"
#include "gui/search/SearchOverlay.hpp"
#include "gui/command/CommandRegistry.hpp"
#include <QApplication>
#include <QHBoxLayout>
#include <QShortcut>
#include <QKeySequence>
#include <QFrame>
#include <QSizePolicy>

namespace severance::gui::windows {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Severance");
  setMinimumSize(1200, 800);
  resize(1400, 900);

  // Define views available in sidebar
  m_ViewInfos = {
    {"Dashboard",  "D", "Ctrl+1"},
    {"Processes",  "P", "Ctrl+2"},
    {"Timeline",   "T", "Ctrl+3"},
    {"Network",    "N", "Ctrl+4"},
    {"Files",      "F", "Ctrl+5"},
    {"Isolation",  "I", "Ctrl+6"},
  };

  // Build the central layout: sidebar | content
  auto centralWidget = new QWidget(this);
  auto mainLayout = new QHBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  setupSidebar();
  mainLayout->addWidget(m_Sidebar);

  // Vertical separator
  auto separator = new QFrame(this);
  separator->setFrameShape(QFrame::VLine);
  separator->setFixedWidth(1);
  separator->setStyleSheet("background-color: #30363D;");
  mainLayout->addWidget(separator);

  setupViews();
  mainLayout->addWidget(m_ViewStack, 1); // stretch=1, takes remaining space

  setCentralWidget(centralWidget);

  setupStatusBar();
  setupShortcuts();

  // Setup Search Overlay and Command Palette
  m_SearchOverlay = new search::SearchOverlay(this);
  auto cmdProvider = std::make_shared<command::CommandSearchProvider>();
  m_SearchOverlay->registerProvider(cmdProvider);

  // Register some global commands
  auto& registry = command::CommandRegistry::GetInstance();
  registry.registerCommand({"app.quit", "Quit Severance", "Close the application", "Ctrl+Q", [this]() {
    close();
  }});
  registry.registerCommand({"view.dashboard", "Show Dashboard", "Switch to the system dashboard view", "Ctrl+1", [this]() {
    setActiveView(0);
  }});
  registry.registerCommand({"view.processes", "Show Processes", "Switch to the process explorer view", "Ctrl+2", [this]() {
    setActiveView(1);
  }});
  registry.registerCommand({"cmd.palette", "Show Command Palette", "Show the command palette", "Ctrl+Shift+P", [this]() {
    onCommandPaletteTriggered();
  }});

  // Start on Dashboard
  setActiveView(0);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupSidebar() {
  m_Sidebar = new QWidget(this);
  m_Sidebar->setObjectName("sidebar");
  m_Sidebar->setFixedWidth(theme::Dimensions::SidebarWidth);

  m_SidebarLayout = new QVBoxLayout(m_Sidebar);
  m_SidebarLayout->setContentsMargins(0, 8, 0, 8);
  m_SidebarLayout->setSpacing(2);

  // App logo / brand at top
  auto logo = new QLabel("S", m_Sidebar);
  logo->setAlignment(Qt::AlignCenter);
  logo->setFixedHeight(40);
  logo->setStyleSheet(
    "font-size: 20px; font-weight: 800; color: #58A6FF; "
    "background: transparent; padding-top: 4px;"
  );
  m_SidebarLayout->addWidget(logo);
  m_SidebarLayout->addSpacing(12);

  // Navigation buttons
  for (int i = 0; i < static_cast<int>(m_ViewInfos.size()); ++i) {
    auto btn = new QPushButton(m_ViewInfos[i].icon, m_Sidebar);
    btn->setCheckable(true);
    btn->setFixedSize(40, 40);
    btn->setToolTip(m_ViewInfos[i].name + "  (" + m_ViewInfos[i].shortcut + ")");
    btn->setStyleSheet(R"(
      QPushButton {
        background-color: transparent;
        border: none;
        border-radius: 8px;
        color: #8B949E;
        font-size: 16px;
        font-weight: 700;
        margin: 2px 8px;
      }
      QPushButton:hover {
        background-color: #21262D;
        color: #E6EDF3;
      }
      QPushButton:checked {
        background-color: #1F3A5F;
        color: #58A6FF;
      }
    )");

    connect(btn, &QPushButton::clicked, this, [this, i]() {
      onSidebarButtonClicked(i);
    });

    m_SidebarButtons.push_back(btn);
    m_SidebarLayout->addWidget(btn, 0, Qt::AlignHCenter);
  }

  m_SidebarLayout->addStretch(); // Push remaining items to bottom

  // Search button at bottom
  auto searchBtn = new QPushButton("?", m_Sidebar);
  searchBtn->setFixedSize(40, 40);
  searchBtn->setToolTip("Search  (Ctrl+K)");
  searchBtn->setStyleSheet(R"(
    QPushButton {
      background-color: transparent;
      border: none;
      border-radius: 8px;
      color: #6E7681;
      font-size: 16px;
      font-weight: 700;
      margin: 2px 8px;
    }
    QPushButton:hover {
      background-color: #21262D;
      color: #E6EDF3;
    }
  )");
  connect(searchBtn, &QPushButton::clicked, this, &MainWindow::onSearchTriggered);
  m_SidebarLayout->addWidget(searchBtn, 0, Qt::AlignHCenter);
  m_SidebarLayout->addSpacing(4);
}

void MainWindow::setupViews() {
  m_ViewStack = new QStackedWidget(this);
  m_ViewStack->setStyleSheet("background-color: #0D1117;");

  m_DashboardView = new dashboard::DashboardView(this);
  m_ProcessView = new process_view::ProcessView(this);
  m_NetworkView = new network_view::NetworkView(this);
  m_FileView = new file_view::FileView(this);
  auto* timelineView = new timeline::TimelineView(this);
  auto* isolationView = new QLabel("Isolation View (Coming Soon)", this);

  isolationView->setAlignment(Qt::AlignCenter);

  m_ViewStack->addWidget(m_DashboardView); // Index 0
  m_ViewStack->addWidget(m_ProcessView);   // Index 1
  m_ViewStack->addWidget(timelineView);    // Index 2
  m_ViewStack->addWidget(m_NetworkView);   // Index 3
  m_ViewStack->addWidget(m_FileView);      // Index 4
  m_ViewStack->addWidget(isolationView);   // Index 5
}

void MainWindow::setupStatusBar() {
  auto bar = statusBar();
  bar->setFixedHeight(theme::Dimensions::StatusBarHeight);

  m_StatusCpu = new QLabel("CPU: ---%");
  m_StatusCpu->setStyleSheet("color: #8B949E; font-size: 12px; padding: 0 8px;");
  bar->addWidget(m_StatusCpu);

  m_StatusMem = new QLabel("MEM: ---");
  m_StatusMem->setStyleSheet("color: #8B949E; font-size: 12px; padding: 0 8px;");
  bar->addWidget(m_StatusMem);

  m_StatusProcessCount = new QLabel("Processes: ---");
  m_StatusProcessCount->setStyleSheet("color: #8B949E; font-size: 12px; padding: 0 8px;");
  bar->addWidget(m_StatusProcessCount);

  // Recording indicator (right side)
  m_StatusRecording = new QLabel("REC");
  m_StatusRecording->setStyleSheet(
    "color: #F85149; font-size: 11px; font-weight: 700; "
    "padding: 0 8px; background: transparent;"
  );
  m_StatusRecording->setVisible(false); // Hidden until recording starts
  bar->addPermanentWidget(m_StatusRecording);

  auto version = new QLabel("v0.2.0-dev");
  version->setStyleSheet("color: #6E7681; font-size: 11px; padding: 0 8px;");
  bar->addPermanentWidget(version);
}

void MainWindow::setupShortcuts() {
  // View switching shortcuts: Ctrl+1 through Ctrl+6
  for (int i = 0; i < static_cast<int>(m_ViewInfos.size()); ++i) {
    auto shortcut = new QShortcut(QKeySequence(m_ViewInfos[i].shortcut), this);
    connect(shortcut, &QShortcut::activated, this, [this, i]() {
      setActiveView(i);
    });
  }

  // Global Search: Ctrl+K
  auto searchShortcut = new QShortcut(QKeySequence("Ctrl+K"), this);
  connect(searchShortcut, &QShortcut::activated, this, &MainWindow::onSearchTriggered);

  // Command Palette: Ctrl+Shift+P
  auto cmdShortcut = new QShortcut(QKeySequence("Ctrl+Shift+P"), this);
  connect(cmdShortcut, &QShortcut::activated, this, &MainWindow::onCommandPaletteTriggered);

  // Refresh: F5
  auto refreshShortcut = new QShortcut(QKeySequence("F5"), this);
  connect(refreshShortcut, &QShortcut::activated, this, [this]() {
    // Will trigger refresh on active view
    statusBar()->showMessage("Refreshed", 1500);
  });

  // Settings: Ctrl+,
  auto settingsShortcut = new QShortcut(QKeySequence("Ctrl+,"), this);
  connect(settingsShortcut, &QShortcut::activated, this, [this]() {
    statusBar()->showMessage("Settings (coming soon)", 1500);
  });
}

void MainWindow::setActiveView(int index) {
  if (index < 0 || index >= static_cast<int>(m_ViewInfos.size())) return;

  m_ActiveViewIndex = index;
  m_ViewStack->setCurrentIndex(index);

  // Update sidebar button states
  for (int i = 0; i < static_cast<int>(m_SidebarButtons.size()); ++i) {
    m_SidebarButtons[i]->setChecked(i == index);
  }
}

void MainWindow::onSidebarButtonClicked(int index) {
  setActiveView(index);
}

void MainWindow::onSearchTriggered() {
  m_SearchOverlay->showOverlay();
}

void MainWindow::onCommandPaletteTriggered() {
  // For now, Command Palette is just Search Overlay but we could pre-fill with "> "
  m_SearchOverlay->showOverlay();
}

} // namespace severance::gui::windows
