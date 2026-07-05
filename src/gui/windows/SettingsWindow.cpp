#include "SettingsWindow.hpp"
#include "utils/Config.hpp"
#include "logging/Logger.hpp"
#include "core/security/FimManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QMessageBox>

namespace severance::gui::windows {

SettingsWindow::SettingsWindow(QWidget *parent)
  : QDialog(parent) {
  setWindowTitle("Severance Configuration");
  setMinimumSize(600, 500);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  setupUI();
  loadSettings();
}

SettingsWindow::~SettingsWindow() = default;

// Helper: styled group box
static QGroupBox* createGroup(const QString& title, QWidget* parent) {
  auto* group = new QGroupBox(title, parent);
  group->setStyleSheet(R"(
    QGroupBox {
      color: #8B949E;
      font-size: 12px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 1px;
      border: 1px solid #21262D;
      border-radius: 8px;
      margin-top: 16px;
      padding-top: 24px;
    }
    QGroupBox::title {
      subcontrol-origin: margin;
      left: 12px;
      padding: 0 6px;
    }
  )");
  return group;
}

void SettingsWindow::setupUI() {
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // Title bar
  auto* titleBar = new QWidget(this);
  titleBar->setStyleSheet("background-color: #010409; border-bottom: 1px solid #21262D;");
  auto* titleLayout = new QHBoxLayout(titleBar);
  titleLayout->setContentsMargins(20, 12, 20, 12);
  auto* titleLabel = new QLabel("Severance Configuration", titleBar);
  titleLabel->setStyleSheet("color: #E6EDF3; font-size: 18px; font-weight: 700; background: transparent;");
  titleLayout->addWidget(titleLabel);
  titleLayout->addStretch();
  mainLayout->addWidget(titleBar);

  // Tabs
  m_Tabs = new QTabWidget(this);
  m_Tabs->setStyleSheet(R"(
    QTabWidget::pane { border: none; background-color: #0D1117; padding: 16px; }
    QTabBar::tab {
      background-color: transparent; color: #8B949E; padding: 10px 20px;
      border: none; border-bottom: 2px solid transparent; font-size: 13px; font-weight: 600;
    }
    QTabBar::tab:hover { color: #E6EDF3; }
    QTabBar::tab:selected { color: #58A6FF; border-bottom-color: #58A6FF; }
  )");
  
  m_Tabs->addTab(createGeneralTab(), "GENERAL");
  m_Tabs->addTab(createMonitoringTab(), "FLOOR MONITORING");
  m_Tabs->addTab(createAppearanceTab(), "TERMINAL DISPLAY");
  m_Tabs->addTab(createSecurityTab(), "COMPLIANCE");
  m_Tabs->addTab(createAdvancedTab(), "CHIP PARAMETERS");
  
  mainLayout->addWidget(m_Tabs, 1);

  // Button bar
  auto* buttonBar = new QWidget(this);
  buttonBar->setStyleSheet("background-color: #010409; border-top: 1px solid #21262D;");
  auto* buttonLayout = new QHBoxLayout(buttonBar);
  buttonLayout->setContentsMargins(20, 12, 20, 12);
  buttonLayout->addStretch();
  
  auto* resetBtn = new QPushButton("Restore Defaults", buttonBar);
  resetBtn->setStyleSheet(R"(
    QPushButton { background: transparent; border: 1px solid #30363D; color: #8B949E; padding: 8px 16px; border-radius: 6px; font-weight: 600; }
    QPushButton:hover { color: #E6EDF3; border-color: #8B949E; }
  )");
  connect(resetBtn, &QPushButton::clicked, this, &SettingsWindow::onReset);
  buttonLayout->addWidget(resetBtn);
  
  auto* applyBtn = new QPushButton("Confirm", buttonBar);
  applyBtn->setStyleSheet(R"(
    QPushButton { background: #238636; border: 1px solid #2EA043; color: #FFFFFF; padding: 8px 24px; border-radius: 6px; font-weight: 600; }
    QPushButton:hover { background: #2EA043; border-color: #3FB950; }
  )");
  connect(applyBtn, &QPushButton::clicked, this, &SettingsWindow::onApply);
  buttonLayout->addWidget(applyBtn);
  
  mainLayout->addWidget(buttonBar);
}

QWidget* SettingsWindow::createGeneralTab() {
  auto* tab = new QWidget();
  auto* layout = new QVBoxLayout(tab);
  layout->setContentsMargins(20, 8, 20, 8);
  
  auto* behaviorGroup = createGroup("Innie Terminal Behavior", tab);
  auto* behaviorLayout = new QVBoxLayout(behaviorGroup);
  
  m_MinimizeToTray = new QCheckBox("Minimize to system tray on severed floor close", behaviorGroup);
  m_StartMinimized = new QCheckBox("Start terminal minimized to tray", behaviorGroup);
  m_LaunchOnBoot = new QCheckBox("Initialize Lumon terminal on system startup", behaviorGroup);
  
  behaviorLayout->addWidget(m_MinimizeToTray);
  behaviorLayout->addWidget(m_StartMinimized);
  behaviorLayout->addWidget(m_LaunchOnBoot);
  layout->addWidget(behaviorGroup);
  
  auto* regionGroup = createGroup("Regional Compliance", tab);
  auto* regionLayout = new QFormLayout(regionGroup);
  
  m_LanguageCombo = new QComboBox(regionGroup);
  m_LanguageCombo->addItems({"English", "Japanese", "German", "French", "Spanish", "Korean", "Mandarin"});
  regionLayout->addRow("Terminal Language:", m_LanguageCombo);
  layout->addWidget(regionGroup);
  
  layout->addStretch();
  return tab;
}

QWidget* SettingsWindow::createMonitoringTab() {
  auto* tab = new QWidget();
  auto* layout = new QVBoxLayout(tab);
  layout->setContentsMargins(20, 8, 20, 8);
  
  auto* intervalGroup = createGroup("Data Collection Frequency", tab);
  auto* intervalLayout = new QFormLayout(intervalGroup);
  
  m_RefreshInterval = new QSpinBox(intervalGroup);
  m_RefreshInterval->setRange(200, 10000);
  m_RefreshInterval->setSuffix(" ms");
  m_RefreshInterval->setSingleStep(100);
  intervalLayout->addRow("Observation interval:", m_RefreshInterval);
  
  m_MaxProcessHistory = new QSpinBox(intervalGroup);
  m_MaxProcessHistory->setRange(100, 100000);
  m_MaxProcessHistory->setSingleStep(1000);
  intervalLayout->addRow("Max innie activity records:", m_MaxProcessHistory);
  
  m_MaxNetworkHistory = new QSpinBox(intervalGroup);
  m_MaxNetworkHistory->setRange(100, 100000);
  m_MaxNetworkHistory->setSingleStep(1000);
  intervalLayout->addRow("Max perimeter traffic records:", m_MaxNetworkHistory);
  
  layout->addWidget(intervalGroup);
  
  auto* sourcesGroup = createGroup("Surveillance Channels", tab);
  auto* sourcesLayout = new QVBoxLayout(sourcesGroup);
  
  m_MonitorCpu = new QCheckBox("Core utilization metrics (innie workload)", sourcesGroup);
  m_MonitorMemory = new QCheckBox("Memory compartment metrics (allocation)", sourcesGroup);
  m_MonitorNetwork = new QCheckBox("Perimeter integrity metrics (per-adapter)", sourcesGroup);
  m_MonitorDisk = new QCheckBox("Storage compartment metrics (I/O rate)", sourcesGroup);
  m_MonitorGpu = new QCheckBox("Auxiliary processing metrics (VRAM)", sourcesGroup);
  m_MonitorFileSystem = new QCheckBox("Memory compartment events (ETW tracing)", sourcesGroup);
  
  sourcesLayout->addWidget(m_MonitorCpu);
  sourcesLayout->addWidget(m_MonitorMemory);
  sourcesLayout->addWidget(m_MonitorNetwork);
  sourcesLayout->addWidget(m_MonitorDisk);
  sourcesLayout->addWidget(m_MonitorGpu);
  sourcesLayout->addWidget(m_MonitorFileSystem);
  
  layout->addWidget(sourcesGroup);
  layout->addStretch();
  return tab;
}

QWidget* SettingsWindow::createAppearanceTab() {
  auto* tab = new QWidget();
  auto* layout = new QVBoxLayout(tab);
  layout->setContentsMargins(20, 8, 20, 8);
  
  auto* themeGroup = createGroup("Terminal Aesthetic", tab);
  auto* themeLayout = new QFormLayout(themeGroup);
  
  m_ThemeCombo = new QComboBox(themeGroup);
  m_ThemeCombo->addItems({"Lumon Standard (Default)", "Perpetuity Wing", "Break Room", "Wellness Session", "Board Interface"});
  themeLayout->addRow("Color scheme:", m_ThemeCombo);
  
  m_FontSize = new QSpinBox(themeGroup);
  m_FontSize->setRange(8, 24);
  m_FontSize->setSuffix(" px");
  themeLayout->addRow("Terminal font size:", m_FontSize);
  
  layout->addWidget(themeGroup);
  
  auto* effectsGroup = createGroup("Visual Compliance", tab);
  auto* effectsLayout = new QVBoxLayout(effectsGroup);
  
  m_EnableAnimations = new QCheckBox("Enable micro-transitions (Lumon standard)", effectsGroup);
  m_OpenGLGraphs = new QCheckBox("Hardware-accelerated data visualization", effectsGroup);
  
  effectsLayout->addWidget(m_EnableAnimations);
  effectsLayout->addWidget(m_OpenGLGraphs);
  
  layout->addWidget(effectsGroup);
  layout->addStretch();
  return tab;
}

QWidget* SettingsWindow::createSecurityTab() {
  auto* tab = new QWidget();
  auto* layout = new QVBoxLayout(tab);
  layout->setContentsMargins(20, 8, 20, 8);
  
  auto* fimGroup = createGroup("Memory Compartment Integrity Monitoring", tab);
  auto* fimLayout = new QVBoxLayout(fimGroup);
  
  auto* desc = new QLabel("Directories monitored for unauthorized memory leakage:", fimGroup);
  desc->setStyleSheet("color: #8B949E; font-size: 12px;");
  fimLayout->addWidget(desc);
  
  auto* listLayout = new QHBoxLayout();
  m_FimDirectories = new QListWidget(fimGroup);
  m_FimDirectories->setStyleSheet(R"(
    QListWidget { background-color: #0D1117; border: 1px solid #30363D; border-radius: 4px; color: #E6EDF3; }
    QListWidget::item { padding: 4px; border-bottom: 1px solid #21262D; }
    QListWidget::item:selected { background-color: #1F6FEB; }
  )");
  listLayout->addWidget(m_FimDirectories);
  
  auto* btnLayout = new QVBoxLayout();
  auto* addBtn = new QPushButton("Add...", fimGroup);
  addBtn->setStyleSheet("QPushButton { background: #21262D; color: #E6EDF3; padding: 4px 12px; border: 1px solid #30363D; border-radius: 4px; } QPushButton:hover { background: #30363D; }");
  connect(addBtn, &QPushButton::clicked, this, [this]() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Compartment to Monitor");
    if (!dir.isEmpty()) {
      // Check if already exists
      auto items = m_FimDirectories->findItems(dir, Qt::MatchExactly);
      if (items.isEmpty()) {
        m_FimDirectories->addItem(dir);
      }
    }
  });
  
  auto* removeBtn = new QPushButton("Remove", fimGroup);
  removeBtn->setStyleSheet("QPushButton { background: #21262D; color: #DA3633; padding: 4px 12px; border: 1px solid #30363D; border-radius: 4px; } QPushButton:hover { background: #30363D; }");
  connect(removeBtn, &QPushButton::clicked, this, [this]() {
    qDeleteAll(m_FimDirectories->selectedItems());
  });
  
  btnLayout->addWidget(addBtn);
  btnLayout->addWidget(removeBtn);
  btnLayout->addStretch();
  
  listLayout->addLayout(btnLayout);
  fimLayout->addLayout(listLayout);
  
  layout->addWidget(fimGroup);
  layout->addStretch();
  return tab;
}

QWidget* SettingsWindow::createAdvancedTab() {
  auto* tab = new QWidget();
  auto* layout = new QVBoxLayout(tab);
  layout->setContentsMargins(20, 8, 20, 8);
  
  auto* pluginGroup = createGroup("Extension Modules", tab);
  auto* pluginLayout = new QFormLayout(pluginGroup);
  
  m_EnablePlugins = new QCheckBox("Enable extension module system", pluginGroup);
  pluginLayout->addRow("", m_EnablePlugins);
  
  auto* pluginPathLayout = new QHBoxLayout();
  m_PluginPath = new QLineEdit(pluginGroup);
  m_PluginPath->setPlaceholderText("Extension module directory path...");
  auto* browseBtn = new QPushButton("Browse", pluginGroup);
  browseBtn->setFixedWidth(80);
  connect(browseBtn, &QPushButton::clicked, [this]() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Extension Module Directory");
    if (!dir.isEmpty()) {
      m_PluginPath->setText(dir);
    }
  });
  pluginPathLayout->addWidget(m_PluginPath);
  pluginPathLayout->addWidget(browseBtn);
  pluginLayout->addRow("Module path:", pluginPathLayout);
  
  layout->addWidget(pluginGroup);
  
  auto* debugGroup = createGroup("Chip Diagnostics", tab);
  auto* debugLayout = new QFormLayout(debugGroup);
  
  m_VerboseLogging = new QCheckBox("Enable verbose chip telemetry logging", debugGroup);
  debugLayout->addRow("", m_VerboseLogging);
  
  m_ThreadPoolSize = new QSpinBox(debugGroup);
  m_ThreadPoolSize->setRange(1, 32);
  debugLayout->addRow("Processing thread count:", m_ThreadPoolSize);
  
  layout->addWidget(debugGroup);
  layout->addStretch();
  return tab;
}

void SettingsWindow::loadSettings() {
  auto& cfg = utils::Config::GetInstance();
  
  // General
  m_MinimizeToTray->setChecked(cfg.Get("general.minimize_to_tray", "1") == "1");
  m_StartMinimized->setChecked(cfg.Get("general.start_minimized", "0") == "1");
  m_LaunchOnBoot->setChecked(cfg.Get("general.launch_on_boot", "0") == "1");
  
  // Monitoring
  m_RefreshInterval->setValue(std::stoi(cfg.Get("monitoring.refresh_interval_ms", "1000")));
  m_MaxProcessHistory->setValue(std::stoi(cfg.Get("monitoring.max_process_history", "10000")));
  m_MaxNetworkHistory->setValue(std::stoi(cfg.Get("monitoring.max_network_history", "10000")));
  m_MonitorCpu->setChecked(cfg.Get("monitoring.cpu", "1") == "1");
  m_MonitorMemory->setChecked(cfg.Get("monitoring.memory", "1") == "1");
  m_MonitorNetwork->setChecked(cfg.Get("monitoring.network", "1") == "1");
  m_MonitorDisk->setChecked(cfg.Get("monitoring.disk", "1") == "1");
  m_MonitorGpu->setChecked(cfg.Get("monitoring.gpu", "1") == "1");
  m_MonitorFileSystem->setChecked(cfg.Get("monitoring.filesystem", "0") == "1");
  
  // Appearance
  m_FontSize->setValue(std::stoi(cfg.Get("appearance.font_size", "13")));
  m_EnableAnimations->setChecked(cfg.Get("appearance.animations", "1") == "1");
  m_OpenGLGraphs->setChecked(cfg.Get("appearance.opengl_graphs", "1") == "1");
  
  // Security
  m_FimDirectories->clear();
  std::string fimDirsStr = cfg.Get("security.fim_directories", "");
  QStringList dirs = QString::fromStdString(fimDirsStr).split(";", Qt::SkipEmptyParts);
  for (const auto& dir : dirs) {
    m_FimDirectories->addItem(dir);
  }
  
  // Advanced
  m_EnablePlugins->setChecked(cfg.Get("advanced.plugins_enabled", "1") == "1");
  m_PluginPath->setText(QString::fromStdString(cfg.Get("advanced.plugin_path", "plugins")));
  m_VerboseLogging->setChecked(cfg.Get("advanced.verbose_logging", "0") == "1");
  m_ThreadPoolSize->setValue(std::stoi(cfg.Get("advanced.thread_pool_size", "4")));
}

void SettingsWindow::saveSettings() {
  auto& cfg = utils::Config::GetInstance();
  
  // General
  cfg.Set("general.minimize_to_tray", m_MinimizeToTray->isChecked() ? "1" : "0");
  cfg.Set("general.start_minimized", m_StartMinimized->isChecked() ? "1" : "0");
  cfg.Set("general.launch_on_boot", m_LaunchOnBoot->isChecked() ? "1" : "0");
  
  // Monitoring
  cfg.Set("monitoring.refresh_interval_ms", std::to_string(m_RefreshInterval->value()));
  cfg.Set("monitoring.max_process_history", std::to_string(m_MaxProcessHistory->value()));
  cfg.Set("monitoring.max_network_history", std::to_string(m_MaxNetworkHistory->value()));
  cfg.Set("monitoring.cpu", m_MonitorCpu->isChecked() ? "1" : "0");
  cfg.Set("monitoring.memory", m_MonitorMemory->isChecked() ? "1" : "0");
  cfg.Set("monitoring.network", m_MonitorNetwork->isChecked() ? "1" : "0");
  cfg.Set("monitoring.disk", m_MonitorDisk->isChecked() ? "1" : "0");
  cfg.Set("monitoring.gpu", m_MonitorGpu->isChecked() ? "1" : "0");
  cfg.Set("monitoring.filesystem", m_MonitorFileSystem->isChecked() ? "1" : "0");
  
  // Appearance
  cfg.Set("appearance.font_size", std::to_string(m_FontSize->value()));
  cfg.Set("appearance.animations", m_EnableAnimations->isChecked() ? "1" : "0");
  cfg.Set("appearance.opengl_graphs", m_OpenGLGraphs->isChecked() ? "1" : "0");
  
  // Security
  QStringList fimDirsList;
  for(int i = 0; i < m_FimDirectories->count(); ++i) {
      fimDirsList << m_FimDirectories->item(i)->text();
  }
  cfg.Set("security.fim_directories", fimDirsList.join(";").toStdString());
  
  // Apply FIM directly if needed (restarting watchers)
  auto& fim = core::security::FimManager::GetInstance();
  fim.StopAll();
  for (const auto& dir : fimDirsList) {
      fim.StartWatching(dir.toStdString());
  }
  
  // Advanced
  cfg.Set("advanced.plugins_enabled", m_EnablePlugins->isChecked() ? "1" : "0");
  cfg.Set("advanced.plugin_path", m_PluginPath->text().toStdString());
  cfg.Set("advanced.verbose_logging", m_VerboseLogging->isChecked() ? "1" : "0");
  cfg.Set("advanced.thread_pool_size", std::to_string(m_ThreadPoolSize->value()));
  
  SEV_CORE_INFO("Settings saved.");
}

void SettingsWindow::onApply() {
  saveSettings();
  accept();
}

void SettingsWindow::onReset() {
  auto result = QMessageBox::question(this, "Restore Configuration", 
    "Restore all parameters to Lumon factory defaults? This directive cannot be reversed.",
    QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    
  if (result == QMessageBox::Yes) {
    // Reset UI to defaults
    m_MinimizeToTray->setChecked(true);
    m_StartMinimized->setChecked(false);
    m_LaunchOnBoot->setChecked(false);
    m_RefreshInterval->setValue(1000);
    m_MaxProcessHistory->setValue(10000);
    m_MaxNetworkHistory->setValue(10000);
    m_MonitorCpu->setChecked(true);
    m_MonitorMemory->setChecked(true);
    m_MonitorNetwork->setChecked(true);
    m_MonitorDisk->setChecked(true);
    m_MonitorGpu->setChecked(true);
    m_MonitorFileSystem->setChecked(false);
    m_ThemeCombo->setCurrentIndex(0);
    m_FontSize->setValue(13);
    m_EnableAnimations->setChecked(true);
    m_OpenGLGraphs->setChecked(true);
    m_EnablePlugins->setChecked(true);
    m_PluginPath->setText("plugins");
    m_VerboseLogging->setChecked(false);
    m_ThreadPoolSize->setValue(4);
  }
}

} // namespace severance::gui::windows
