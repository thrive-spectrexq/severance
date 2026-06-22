#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

namespace severance::gui::windows {

class SettingsWindow : public QDialog {
  Q_OBJECT

public:
  explicit SettingsWindow(QWidget *parent = nullptr);
  ~SettingsWindow() override;

private slots:
  void onApply();
  void onReset();

private:
  void setupUI();
  QWidget* createGeneralTab();
  QWidget* createMonitoringTab();
  QWidget* createAppearanceTab();
  QWidget* createAdvancedTab();
  
  void loadSettings();
  void saveSettings();

  QTabWidget* m_Tabs{nullptr};
  
  // General
  QCheckBox* m_MinimizeToTray{nullptr};
  QCheckBox* m_StartMinimized{nullptr};
  QCheckBox* m_LaunchOnBoot{nullptr};
  QComboBox* m_LanguageCombo{nullptr};
  
  // Monitoring
  QSpinBox* m_RefreshInterval{nullptr};
  QCheckBox* m_MonitorCpu{nullptr};
  QCheckBox* m_MonitorMemory{nullptr};
  QCheckBox* m_MonitorNetwork{nullptr};
  QCheckBox* m_MonitorDisk{nullptr};
  QCheckBox* m_MonitorGpu{nullptr};
  QCheckBox* m_MonitorFileSystem{nullptr};
  QSpinBox* m_MaxProcessHistory{nullptr};
  QSpinBox* m_MaxNetworkHistory{nullptr};
  
  // Appearance
  QComboBox* m_ThemeCombo{nullptr};
  QSpinBox* m_FontSize{nullptr};
  QCheckBox* m_EnableAnimations{nullptr};
  QCheckBox* m_OpenGLGraphs{nullptr};
  
  // Advanced
  QLineEdit* m_PluginPath{nullptr};
  QCheckBox* m_EnablePlugins{nullptr};
  QCheckBox* m_VerboseLogging{nullptr};
  QSpinBox* m_ThreadPoolSize{nullptr};
};

} // namespace severance::gui::windows
