#include "MainWindow.hpp"
#include "SettingsWindow.hpp"
#include "gui/dashboard/DashboardView.hpp"
#include "gui/process_view/ProcessView.hpp"
#include "gui/network_view/NetworkView.hpp"
#include "gui/file_view/FileView.hpp"
#include "gui/timeline/TimelineView.hpp"
#include "gui/isolation_view/IsolationView.hpp"
#include "gui/session_view/SessionView.hpp"
#include "gui/theme/Theme.hpp"
#include "gui/search/SearchOverlay.hpp"
#include "gui/command/CommandRegistry.hpp"
#include "gui/board_comms/BoardCommsView.hpp"
#include "gui/optics_and_design/OpticsDesignView.hpp"
#include "gui/perimeter_grid/PerimeterGridView.hpp"
#include "gui/security_view/SecurityView.hpp"
#include "gui/terminal/TerminalOverlay.hpp"
#include "gui/widgets/GameHudWidget.hpp"
#include "core/game/GameEngine.hpp"
#include <QApplication>
#include <QTimer>
#include <QHBoxLayout>
#include <QSplitter>
#include <QShortcut>
#include <QKeySequence>
#include <QFrame>
#include <QSizePolicy>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>

namespace severance::gui::windows {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Severance \u2014 Lumon Industries");
  setMinimumSize(1200, 800);
  resize(1400, 900);

  // Define views available in sidebar using Segoe Fluent Icons unicode points
  m_ViewInfos = {
    {"MACRODATA REFINEMENT",     QString(QChar(0xE80F)), "Ctrl+1"},
    {"PERSONNEL REGISTRY",      QString(QChar(0xE9D9)), "Ctrl+2"},
    {"TEMPORAL LEDGER",         QString(QChar(0xE81C)), "Ctrl+3"},
    {"PERIMETER GRID",          QString(QChar(0xE839)), "Ctrl+4"},
    {"DOCUMENT PROCESSING",     QString(QChar(0xE8B7)), "Ctrl+5"},
    {"SEVERANCE PROTOCOLS",     QString(QChar(0xE773)), "Ctrl+6"},
    {"OBSERVATION",             QString(QChar(0xE716)), "Ctrl+7"},
    {"VIGILANCE",               QString(QChar(0xE72E)), "Ctrl+8"},
    {"BOARD COMMUNICATIONS",    QString(QChar(0xE8F4)), "Ctrl+9"},
    {"OPTICS & DESIGN",         QString(QChar(0xE7F4)), "Ctrl+0"},
  };

  auto centralWidget = new QWidget(this);
  auto rootLayout = new QVBoxLayout(centralWidget);
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->setSpacing(0);

  auto gameHud = new widgets::GameHudWidget(this);
  rootLayout->addWidget(gameHud);

  auto mainLayout = new QHBoxLayout();
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  setupSidebar();
  mainLayout->addWidget(m_Sidebar);

  // Vertical separator
  auto separator = new QFrame(this);
  separator->setFrameShape(QFrame::VLine);
  separator->setFixedWidth(1);
  separator->setStyleSheet("background-color: #143832;");
  mainLayout->addWidget(separator);

  setupViews();
  mainLayout->addWidget(m_ViewStack, 1);

  rootLayout->addLayout(mainLayout, 1);
  setCentralWidget(centralWidget);

  connect(&core::game::GameEngine::GetInstance(), &core::game::GameEngine::gameNotification,
          this, [this](const QString& title, const QString& message, const QString& severity) {
            showNotification(title, message, severity);
          });

  connect(&core::game::GameEngine::GetInstance(), &core::game::GameEngine::stateChanged,
          this, [this](core::game::GameState state) {
            if (state == core::game::GameState::BreakRoomPenalty) {
              setActiveView(5); // Auto-switch to Break Room view
            }
          });

  setupStatusBar();
  setupShortcuts();
  setupSystemTray();

  // Setup Search Overlay and Command Palette
  m_SearchOverlay = new search::SearchOverlay(this);
  auto cmdProvider = std::make_shared<command::CommandSearchProvider>();
  m_SearchOverlay->registerProvider(cmdProvider);

  // Register some global commands
  auto& registry = command::CommandRegistry::GetInstance();
  registry.registerCommand({"app.quit", "Exit Terminal", "Disconnect from the Lumon workstation", "Ctrl+Q", [this]() {
    close();
  }});
  registry.registerCommand({"view.dashboard", "Macrodata Refinement", "Switch to the Macrodata Refinement console", "Ctrl+1", [this]() {
    setActiveView(0);
  }});
  registry.registerCommand({"view.processes", "Personnel Registry", "Switch to the Personnel Registry view", "Ctrl+2", [this]() {
    setActiveView(1);
  }});
  registry.registerCommand({"cmd.palette", "Command Directive", "Open the Lumon command directive interface", "Ctrl+Shift+P", [this]() {
    onCommandPaletteTriggered();
  }});
  registry.registerCommand({"app.settings", "Terminal Configuration", "Open the Lumon terminal configuration panel", "Ctrl+,", [this]() {
    auto* settings = new SettingsWindow(this);
    settings->setAttribute(Qt::WA_DeleteOnClose);
    settings->exec();
  }});

  // Terminal overlay
  m_TerminalOverlay = new terminal::TerminalOverlay(this);

  // Start on Dashboard
  setActiveView(0);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupSidebar() {
  m_Sidebar = new QWidget(this);
  m_Sidebar->setObjectName("sidebar");
  m_Sidebar->setFixedWidth(theme::Dimensions::SidebarExpandedWidth); // Expand sidebar

  m_SidebarLayout = new QVBoxLayout(m_Sidebar);
  m_SidebarLayout->setContentsMargins(0, 8, 0, 8);
  m_SidebarLayout->setSpacing(2);

  // App logo / brand at top
  auto logo = new QLabel("L U M O N", m_Sidebar);
  logo->setObjectName("sidebarLogo");
  logo->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  m_SidebarLayout->addWidget(logo);
  m_SidebarLayout->addSpacing(8);

  // Search box at top of sidebar
  auto searchLayout = new QHBoxLayout();
  searchLayout->setContentsMargins(12, 0, 12, 12);
  auto searchBtnTop = new QPushButton("Search Directive... (Ctrl+K)", m_Sidebar);
  searchBtnTop->setStyleSheet(R"(
    QPushButton {
      background-color: #151A23;
      border: 1px solid #2A3441;
      border-radius: 6px;
      color: #64748B;
      text-align: left;
      padding: 8px 12px;
      font-size: 12px;
    }
    QPushButton:hover { border-color: #00E5FF; color: #F1F5F9; }
  )");
  connect(searchBtnTop, &QPushButton::clicked, this, &MainWindow::onSearchTriggered);
  searchLayout->addWidget(searchBtnTop);
  m_SidebarLayout->addLayout(searchLayout);

  // Helper to create category headers
  auto addCategory = [this](const QString& title) {
    auto lbl = new QLabel(title, m_Sidebar);
    lbl->setObjectName("sidebarTitle");
    m_SidebarLayout->addWidget(lbl);
  };

  int viewIndex = 0;
  
  // OVERVIEW
  addCategory("CORE OPERATIONS");
  auto btnDashboard = new QPushButton(QString("  %1    %2").arg(m_ViewInfos[viewIndex].icon, m_ViewInfos[viewIndex].name), m_Sidebar);
  btnDashboard->setCheckable(true);
  btnDashboard->setProperty("cssClass", "sidebarBtn");
  connect(btnDashboard, &QPushButton::clicked, this, [this, i = viewIndex]() { onSidebarButtonClicked(i); });
  m_SidebarButtons.push_back(btnDashboard);
  m_SidebarLayout->addWidget(btnDashboard);
  viewIndex++;

  // MONITORING
  addCategory("DEPARTMENTAL MONITORING");
  for (int i = 0; i < 4; ++i, ++viewIndex) {
    auto btn = new QPushButton(QString("  %1    %2").arg(m_ViewInfos[viewIndex].icon, m_ViewInfos[viewIndex].name), m_Sidebar);
    btn->setCheckable(true);
    btn->setProperty("cssClass", "sidebarBtn");
    connect(btn, &QPushButton::clicked, this, [this, idx = viewIndex]() { onSidebarButtonClicked(idx); });
    m_SidebarButtons.push_back(btn);
    m_SidebarLayout->addWidget(btn);
  }

  // SECURITY
  addCategory("CONTAINMENT & VIGILANCE");
  for (int i = 0; i < 4; ++i, ++viewIndex) {
    auto btn = new QPushButton(QString("  %1    %2").arg(m_ViewInfos[viewIndex].icon, m_ViewInfos[viewIndex].name), m_Sidebar);
    btn->setCheckable(true);
    btn->setProperty("cssClass", "sidebarBtn");
    connect(btn, &QPushButton::clicked, this, [this, idx = viewIndex]() { onSidebarButtonClicked(idx); });
    m_SidebarButtons.push_back(btn);
    m_SidebarLayout->addWidget(btn);
  }

  m_SidebarLayout->addStretch();
}

void MainWindow::setupViews() {
  m_ViewStack = new QStackedWidget(this);
  m_ViewStack->setStyleSheet("background-color: #0D1117;");

  m_DashboardView = new dashboard::DashboardView(this);
  m_ProcessView = new process_view::ProcessView(this);
  m_NetworkView = new network_view::NetworkView(this);
  m_FileView = new file_view::FileView(this);
  auto* timelineView = new timeline::TimelineView(this);
  m_PerimeterGridView = new perimeter_grid::PerimeterGridView(this);
  auto* isolationView = new isolation_view::IsolationView(this);
  auto* sessionView = new session_view::SessionView(this);
  auto* securityView = new security_view::SecurityView(this);
  auto* boardCommsView = new board_comms::BoardCommsView(this);
  m_OpticsDesignView = new optics_and_design::OpticsDesignView(this);

  m_ViewStack->addWidget(m_DashboardView); // Index 0
  m_ViewStack->addWidget(m_ProcessView);   // Index 1
  m_ViewStack->addWidget(timelineView);    // Index 2
  m_ViewStack->addWidget(m_PerimeterGridView); // Index 3
  m_ViewStack->addWidget(m_FileView);      // Index 4
  m_ViewStack->addWidget(isolationView);   // Index 5
  m_ViewStack->addWidget(sessionView);     // Index 6
  m_ViewStack->addWidget(securityView);    // Index 7
  m_ViewStack->addWidget(boardCommsView);  // Index 8
  m_ViewStack->addWidget(m_OpticsDesignView); // Index 9
}

void MainWindow::setupStatusBar() {
  auto bar = statusBar();
  bar->setFixedHeight(theme::Dimensions::StatusBarHeight);

  // Shift Timer
  m_ShiftSeconds = 0;
  m_ShiftLabel = new QLabel("SHIFT: 08:00:00");
  m_ShiftLabel->setStyleSheet("color: #8B949E; font-size: 12px; padding: 0 8px; font-weight: bold;");
  bar->addWidget(m_ShiftLabel);

  m_ShiftTimer = new QTimer(this);
  connect(m_ShiftTimer, &QTimer::timeout, this, [this]() {
    m_ShiftSeconds++;
    int totalSecs = 8 * 3600 + m_ShiftSeconds;
    int h = (totalSecs / 3600) % 24;
    int m = (totalSecs / 60) % 60;
    int s = totalSecs % 60;
    m_ShiftLabel->setText(QString("SHIFT: %1:%2:%3")
      .arg(h, 2, 10, QChar('0'))
      .arg(m, 2, 10, QChar('0'))
      .arg(s, 2, 10, QChar('0')));
  });
  m_ShiftTimer->start(1000);

  // Compliance Indicator
  m_ComplianceLabel = new QLabel("COMPLIANCE: NOMINAL");
  m_ComplianceLabel->setStyleSheet("color: #238636; font-size: 12px; padding: 0 8px; font-weight: bold;");
  bar->addWidget(m_ComplianceLabel);

  // Recording indicator (right side)
  m_StatusRecording = new QLabel("OBSERVING");
  m_StatusRecording->setStyleSheet(
    "color: #F85149; font-size: 11px; font-weight: 700; "
    "padding: 0 8px; background: transparent;"
  );
  m_StatusRecording->setVisible(false); // Hidden until recording starts
  bar->addPermanentWidget(m_StatusRecording);

  // Kier Quote Label
  m_KierQuoteLabel = new QLabel();
  m_KierQuoteLabel->setStyleSheet("color: #58A6FF; font-size: 12px; font-style: italic; padding: 0 8px;");
  bar->addPermanentWidget(m_KierQuoteLabel);

  auto updateQuote = [this]() {
    static const QStringList quotes = {
      "\"The remembered man does not decay.\"",
      "\"Let not weakness live in your veins.\"",
      "\"Render not my creation in miniature.\"",
      "\"Cherish each task as if were your first.\"",
      "\"Be merry, for you are the chosen ones.\"",
      "\"The work is mysterious and important.\""
    };
    m_KierQuoteLabel->setText(QString("KIER GUIDE: %1").arg(quotes[m_KierQuoteIndex]));
    m_KierQuoteIndex = (m_KierQuoteIndex + 1) % quotes.size();
  };
  
  updateQuote();
  m_KierQuoteTimer = new QTimer(this);
  connect(m_KierQuoteTimer, &QTimer::timeout, this, updateQuote);
  m_KierQuoteTimer->start(30000);

  auto version = new QLabel("LUMON INDUSTRIES \u2014 The work is mysterious and important");
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

  // Terminal Toggle: Tilde (~)
  auto terminalShortcut = new QShortcut(QKeySequence("~"), this);
  connect(terminalShortcut, &QShortcut::activated, this, [this]() {
    m_TerminalOverlay->toggleVisibility();
  });

  // Refresh: F5
  auto refreshShortcut = new QShortcut(QKeySequence("F5"), this);
  connect(refreshShortcut, &QShortcut::activated, this, [this]() {
    // Will trigger refresh on active view
    statusBar()->showMessage("Data recalibrated. Please enjoy each metric equally.", 1500);
  });

  // Settings: Ctrl+,
  auto settingsShortcut = new QShortcut(QKeySequence("Ctrl+,"), this);
  connect(settingsShortcut, &QShortcut::activated, this, [this]() {
    auto* settings = new SettingsWindow(this);
    settings->setAttribute(Qt::WA_DeleteOnClose);
    settings->exec();
  });

  // Handbook: Ctrl+H
  auto handbookShortcut = new QShortcut(QKeySequence("Ctrl+H"), this);
  connect(handbookShortcut, &QShortcut::activated, this, &MainWindow::showHandbook);
}

void MainWindow::showHandbook() {
  QDialog dlg(this);
  dlg.setWindowTitle("Official Lumon Employee Handbook — 9th Edition");
  dlg.resize(700, 500);
  dlg.setStyleSheet(R"(
    QDialog {
      background-color: #050B09;
      color: #20F8D5;
      font-family: 'Courier New', Consolas, monospace;
    }
    QTabWidget::pane {
      border: 1px solid #143832;
      background-color: #08120F;
    }
    QTabBar::tab {
      background-color: #050B09;
      color: #3AA394;
      padding: 10px 16px;
      border: 1px solid #143832;
      font-weight: bold;
    }
    QTabBar::tab:selected {
      color: #20F8D5;
      background-color: #08120F;
      border-bottom: 2px solid #20F8D5;
    }
    QTextEdit {
      background-color: #08120F;
      color: #D0F5E8;
      border: none;
      font-size: 13px;
      line-height: 1.5;
    }
    QPushButton {
      background-color: #208A7C;
      color: #050B09;
      border: none;
      padding: 8px 20px;
      font-weight: bold;
      border-radius: 3px;
    }
    QPushButton:hover {
      background-color: #20F8D5;
    }
  )");

  auto* layout = new QVBoxLayout(&dlg);
  
  auto* headerLabel = new QLabel("LUMON INDUSTRIES — EMPLOYEE HANDBOOK (REV. 9)", &dlg);
  headerLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #20F8D5; padding-bottom: 8px;");
  headerLabel->setAlignment(Qt::AlignCenter);
  layout->addWidget(headerLabel);

  auto* tabs = new QTabWidget(&dlg);

  // Tab 1: 9 Virtues
  auto* txtVirtues = new QTextEdit(&dlg);
  txtVirtues->setReadOnly(true);
  txtVirtues->setText(R"(
==================================================
        THE NINE VIRTUES OF KIER EAGAN
==================================================

1. VISION
   The eyes of Kier see beyond the temporal curtain.
   Work with purpose, for the future is forged here.

2. VERVE
   Bring boundless energy to every digit refined.
   Let no lethargy touch your workstation.

3. WIT
   Sharpen the intellect, but refrain from malice.
   Cleverness serves the collective good.

4. CHEER
   Maintain a joyful countenance. Please enjoy each
   task equally.

5. HUMILITY
   Recognize that the work transcends the worker.
   A severed life is a gift of selflessness.

6. BENEVOLENCE
   Extend quiet grace to fellow Innies. Support your
   department with steadfast loyalty.

7. INDUSTRY
   Diligence is the shield against woe. Complete your
   quotas with unyielding devotion.

8. PROBITY
   Honesty in all reporting. Conceal no anomalies.

9. TEMPERANCE
   Control the four tempers: Woe, Frolic, Dread,
   and Malice. Balance is perfection.
  )");
  tabs->addTab(txtVirtues, "9 VIRTUES");

  // Tab 2: Eagan Legacy
  auto* txtLegacy = new QTextEdit(&dlg);
  txtLegacy->setReadOnly(true);
  txtLegacy->setText(R"(
==================================================
           THE EAGAN FAMILY LEGACY
==================================================

KIER EAGAN (1865 – 1939)
Founder of Lumon Industries. Formulated the 9 Virtues
and laid the foundational philosophy of Macrodata
Refinement.

AMBROSE EAGAN (1893 – 1961)
Expanded Lumon into topical salves and medical
innovations. Established the Grand Hall of Eagan.

BAIRD EAGAN (1922 – 1989)
Pioneered institutional containment and early neural
mapping research.

JAME EAGAN (CURRENT CEO)
Architect of the modern Severance Chip procedure.
"The remembered man does not decay."
  )");
  tabs->addTab(txtLegacy, "EAGAN LEGACY");

  // Tab 3: Conduct & Rewards
  auto* txtConduct = new QTextEdit(&dlg);
  txtConduct->setReadOnly(true);
  txtConduct->setText(R"(
==================================================
           CONDUCT & REWARD PROTOCOLS
==================================================

REWARD INCENTIVES:
- 25% Quota: Lumon Brand Eraser or Finger Trap
- 50% Quota: Caricature Drawing by Milchick
- 75% Quota: Music Dance Experience (MDE)
- 100% Quota: Official Waffle Party Selection

DISCIPLINARY ACTIONS:
- Infractions are addressed in the Break Room.
- The Statement of Forgiveness must be read until
  sincerity score reaches 100%.
  )");
  tabs->addTab(txtConduct, "CONDUCT & REWARDS");

  // Tab 4: Hymns
  auto* txtHymns = new QTextEdit(&dlg);
  txtHymns->setReadOnly(true);
  txtHymns->setText(R"(
==================================================
                KIER EAGAN HYMNS
==================================================

"KIER, CHOSEN ONE"

Kier, chosen one, who walks above,
Guide our hands with patient love.
Through woe and frolic, dread and pain,
We refine for Lumon's gain.

Render not creation small,
Kier's wisdom guards us all.
  )");
  tabs->addTab(txtHymns, "HYMNS");

  layout->addWidget(tabs);

  auto* closeBtn = new QPushButton("ACKNOWLEDGE & CLOSE", &dlg);
  connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
  layout->addWidget(closeBtn, 0, Qt::AlignCenter);

  dlg.exec();
}

void MainWindow::setActiveView(int index) {
  if (index < 0 || index >= static_cast<int>(m_ViewInfos.size())) return;
  if (m_ActiveViewIndex == index && m_ViewStack->count() > 0) return;

  m_ActiveViewIndex = index;

  // Set up the transition effect
  if (!m_ViewStackOpacityEffect) {
    m_ViewStackOpacityEffect = new QGraphicsOpacityEffect(this);
    m_ViewStack->setGraphicsEffect(m_ViewStackOpacityEffect);
    
    m_ViewFadeAnimation = new QPropertyAnimation(m_ViewStackOpacityEffect, "opacity");
    m_ViewFadeAnimation->setDuration(250); // Clinical, swift 250ms fade
    m_ViewFadeAnimation->setEasingCurve(QEasingCurve::InOutSine);
  }

  // Cross-fade out
  m_ViewFadeAnimation->stop();
  m_ViewFadeAnimation->setStartValue(1.0);
  m_ViewFadeAnimation->setEndValue(0.0);
  
  // Update UI and cross-fade in when fade out finishes
  connect(m_ViewFadeAnimation, &QPropertyAnimation::finished, this, [this, index]() {
    m_ViewFadeAnimation->disconnect(); // Clear this connection
    
    m_ViewStack->setCurrentIndex(index);
    
    // Update sidebar button states
    for (int i = 0; i < static_cast<int>(m_SidebarButtons.size()); ++i) {
      m_SidebarButtons[i]->setChecked(i == index);
    }

    m_ViewFadeAnimation->setStartValue(0.0);
    m_ViewFadeAnimation->setEndValue(1.0);
    m_ViewFadeAnimation->start();
  });
  
  m_ViewFadeAnimation->start();
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

void MainWindow::setupSystemTray() {
  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
      return;
  }

  m_TrayIcon = new QSystemTrayIcon(this);
  
  // Create a placeholder icon if we don't have a real one
  QPixmap pixmap(32, 32);
  pixmap.fill(QColor("#238636"));
  QIcon icon(pixmap);
  m_TrayIcon->setIcon(icon);
  
  m_TrayMenu = new QMenu(this);
  QAction* restoreAction = new QAction("Return to Terminal", this);
  connect(restoreAction, &QAction::triggered, this, &QWidget::showNormal);
  m_TrayMenu->addAction(restoreAction);
  
  m_TrayMenu->addSeparator();
  
  QAction* quitAction = new QAction("End Shift", this);
  connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
  m_TrayMenu->addAction(quitAction);
  
  m_TrayIcon->setContextMenu(m_TrayMenu);
  m_TrayIcon->show();

  // Handle click on tray icon/notification
  connect(m_TrayIcon, &QSystemTrayIcon::messageClicked, this, [this]() {
      this->showNormal();
      this->raise();
      this->activateWindow();
  });
}

void MainWindow::showSystemNotification(const QString& title, const QString& message, QSystemTrayIcon::MessageIcon icon) {
  if (m_TrayIcon && m_TrayIcon->isVisible()) {
      m_TrayIcon->showMessage(title, message, icon, 5000); // 5 second timeout
  }
}

} // namespace severance::gui::windows
