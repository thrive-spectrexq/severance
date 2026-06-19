#include "IsolationView.hpp"
#include "core/sandbox/SandboxManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>

namespace severance::gui::isolation_view {

IsolationView::IsolationView(QWidget *parent) : QWidget(parent) {
  m_SandboxManager = std::make_unique<core::sandbox::SandboxManager>();
  setupUI();
}

void IsolationView::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(16);

  // Header
  auto* header = new QLabel("Sandbox & Isolation", this);
  header->setStyleSheet("font-size: 20px; font-weight: bold; color: #E6EDF3;");
  layout->addWidget(header);

  auto* desc = new QLabel("Launch applications in an isolated environment with restricted filesystem and network access.", this);
  desc->setStyleSheet("color: #8B949E;");
  layout->addWidget(desc);

  // Launch Area
  auto* launchLayout = new QHBoxLayout();
  m_ExecutablePath = new QLineEdit(this);
  m_ExecutablePath->setPlaceholderText("Path to executable (e.g. C:\\Windows\\System32\\cmd.exe)");
  launchLayout->addWidget(m_ExecutablePath);

  m_BrowseBtn = new QPushButton("Browse...", this);
  connect(m_BrowseBtn, &QPushButton::clicked, this, [this]() {
    QString path = QFileDialog::getOpenFileName(this, "Select Executable", "", "Executables (*.exe);;All Files (*)");
    if (!path.isEmpty()) {
      m_ExecutablePath->setText(path);
    }
  });
  launchLayout->addWidget(m_BrowseBtn);

  m_LaunchBtn = new QPushButton("Launch Isolated", this);
  m_LaunchBtn->setStyleSheet("background-color: #238636; color: white; font-weight: bold; padding: 6px 12px; border-radius: 4px;");
  connect(m_LaunchBtn, &QPushButton::clicked, this, &IsolationView::onLaunchClicked);
  launchLayout->addWidget(m_LaunchBtn);

  layout->addLayout(launchLayout);

  // Configuration Area
  auto* configLayout = new QHBoxLayout();
  
  auto* profileLabel = new QLabel("Profile:", this);
  profileLabel->setStyleSheet("color: #E6EDF3;");
  m_ProfileCombo = new QComboBox(this);
  m_ProfileCombo->addItems({"Strict", "Restricted", "Unrestricted"});
  
  auto* memLabel = new QLabel("Memory (MB):", this);
  memLabel->setStyleSheet("color: #E6EDF3;");
  m_MemSpin = new QSpinBox(this);
  m_MemSpin->setRange(0, 16384);
  m_MemSpin->setValue(256);
  
  auto* cpuLabel = new QLabel("CPU (%):", this);
  cpuLabel->setStyleSheet("color: #E6EDF3;");
  m_CpuSpin = new QSpinBox(this);
  m_CpuSpin->setRange(1, 100);
  m_CpuSpin->setValue(50);
  
  configLayout->addWidget(profileLabel);
  configLayout->addWidget(m_ProfileCombo);
  configLayout->addSpacing(16);
  configLayout->addWidget(memLabel);
  configLayout->addWidget(m_MemSpin);
  configLayout->addSpacing(16);
  configLayout->addWidget(cpuLabel);
  configLayout->addWidget(m_CpuSpin);
  configLayout->addStretch();
  
  layout->addLayout(configLayout);

  connect(m_ProfileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
    if (index == 0) { // Strict
      m_MemSpin->setValue(128); m_CpuSpin->setValue(20);
    } else if (index == 1) { // Restricted
      m_MemSpin->setValue(512); m_CpuSpin->setValue(50);
    } else { // Unrestricted
      m_MemSpin->setValue(0); m_CpuSpin->setValue(100);
    }
  });

  // Active Sandboxes & Analysis
  layout->addSpacing(16);
  auto* subheader = new QLabel("Active Sandboxes & Analysis", this);
  subheader->setStyleSheet("font-size: 16px; font-weight: bold; color: #E6EDF3;");
  layout->addWidget(subheader);

  auto* splitLayout = new QHBoxLayout();

  m_ActiveSandboxesTable = new QTableWidget(this);
  m_ActiveSandboxesTable->setColumnCount(4);
  m_ActiveSandboxesTable->setHorizontalHeaderLabels({"Profile Name", "Executable", "Status", "Action"});
  m_ActiveSandboxesTable->horizontalHeader()->setStretchLastSection(true);
  m_ActiveSandboxesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_ActiveSandboxesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  splitLayout->addWidget(m_ActiveSandboxesTable, 2);

  connect(m_ActiveSandboxesTable, &QTableWidget::cellClicked, this, &IsolationView::onActiveSandboxClicked);

  m_AnalysisPane = new QWidget(this);
  m_AnalysisPane->setStyleSheet("background-color: #0D1117; border: 1px solid #30363D; border-radius: 6px;");
  auto* analysisLayout = new QVBoxLayout(m_AnalysisPane);
  
  m_AnalysisTitle = new QLabel("Security Analysis", m_AnalysisPane);
  m_AnalysisTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: #58A6FF; border: none;");
  m_AnalysisDetails = new QLabel("Select a sandbox to view isolation details and behavior.", m_AnalysisPane);
  m_AnalysisDetails->setWordWrap(true);
  m_AnalysisDetails->setStyleSheet("color: #8B949E; border: none; margin-top: 8px;");
  m_AnalysisDetails->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  
  analysisLayout->addWidget(m_AnalysisTitle);
  analysisLayout->addWidget(m_AnalysisDetails);
  analysisLayout->addStretch();
  
  splitLayout->addWidget(m_AnalysisPane, 1);
  layout->addLayout(splitLayout, 1);
}

void IsolationView::onLaunchClicked() {
  QString path = m_ExecutablePath->text();
  if (path.isEmpty()) {
    QMessageBox::warning(this, "Error", "Please select an executable to launch.");
    return;
  }

  core::sandbox::SandboxProfile profile;
  profile.name = m_ProfileCombo->currentText().toStdString() + " Sandbox " + std::to_string(m_ActiveSandboxesTable->rowCount() + 1);
  profile.executablePath = path.toStdString();
  
  // Use UI configured limits
  profile.policy.maxMemoryBytes = static_cast<uint64_t>(m_MemSpin->value()) * 1024 * 1024; // MB to Bytes
  profile.policy.maxCpuPercent = static_cast<double>(m_CpuSpin->value());
  
  // Strict/Restricted flags
  if (m_ProfileCombo->currentIndex() == 0) { // Strict
    profile.policy.allowNetworkAccess = false;
    profile.policy.allowFileSystemWrite = false;
  } else if (m_ProfileCombo->currentIndex() == 1) { // Restricted
    profile.policy.allowNetworkAccess = true;
    profile.policy.allowFileSystemWrite = false;
  } else { // Unrestricted
    profile.policy.allowNetworkAccess = true;
    profile.policy.allowFileSystemWrite = true;
  }

  if (m_SandboxManager->LaunchProfile(profile)) {
    int row = m_ActiveSandboxesTable->rowCount();
    m_ActiveSandboxesTable->insertRow(row);
    m_ActiveSandboxesTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(profile.name)));
    m_ActiveSandboxesTable->setItem(row, 1, new QTableWidgetItem(path));
    m_ActiveSandboxesTable->setItem(row, 2, new QTableWidgetItem("Running"));

    auto* termBtn = new QPushButton("Terminate", this);
    termBtn->setStyleSheet("background-color: #DA3633; color: white; border-radius: 4px;");
    connect(termBtn, &QPushButton::clicked, this, [this, row]() {
      onTerminateClicked(row);
    });
    m_ActiveSandboxesTable->setCellWidget(row, 3, termBtn);
  }
}

void IsolationView::onActiveSandboxClicked(int row, int column) {
  (void)column;
  if (row >= 0 && row < m_ActiveSandboxesTable->rowCount()) {
    auto profiles = m_SandboxManager->GetActiveProfiles();
    if (row < profiles.size()) {
      const auto& p = profiles[row];
      
      QString details = QString(
        "<b>Profile:</b> %1<br><br>"
        "<b>Target Executable:</b><br>%2<br><br>"
        "<b>Enforced Limits:</b><br>"
        "- Memory Cap: %3 MB<br>"
        "- CPU Cap: %4%<br>"
        "- FS Write Allowed: %5<br>"
        "- Net Access Allowed: %6<br><br>"
        "<b>Security Analysis:</b><br>"
        "This process is running under a Windows Job Object. "
      ).arg(QString::fromStdString(p.name))
       .arg(QString::fromStdString(p.executablePath))
       .arg(p.policy.maxMemoryBytes > 0 ? QString::number(p.policy.maxMemoryBytes / (1024*1024)) : "Unlimited")
       .arg(QString::number(p.policy.maxCpuPercent))
       .arg(p.policy.allowFileSystemWrite ? "Yes" : "No")
       .arg(p.policy.allowNetworkAccess ? "Yes" : "No");

      if (!p.policy.allowFileSystemWrite) {
        details += "A Low Integrity Restricted Token is enforced, preventing writes to medium/high integrity locations. ";
      }

      m_AnalysisDetails->setText(details);
    }
  }
}

void IsolationView::onTerminateClicked(int row) {
  if (row >= 0 && row < m_ActiveSandboxesTable->rowCount()) {
    m_SandboxManager->TerminateSandbox(row);
    m_ActiveSandboxesTable->removeRow(row);
    
    // We must re-bind the lambda row indices for remaining rows
    for (int i = row; i < m_ActiveSandboxesTable->rowCount(); ++i) {
        auto* btn = qobject_cast<QPushButton*>(m_ActiveSandboxesTable->cellWidget(i, 3));
        if (btn) {
            btn->disconnect();
            connect(btn, &QPushButton::clicked, this, [this, i]() {
                onTerminateClicked(i);
            });
        }
    }
  }
}

} // namespace severance::gui::isolation_view
