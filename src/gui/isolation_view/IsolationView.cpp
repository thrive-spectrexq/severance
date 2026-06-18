#include "IsolationView.hpp"
#include "core/sandbox/SandboxManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>

namespace severance::gui::isolation_view {

IsolationView::IsolationView(QWidget *parent) : QWidget(parent) {
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

  // Active Sandboxes Table
  layout->addSpacing(16);
  auto* subheader = new QLabel("Active Sandboxes", this);
  subheader->setStyleSheet("font-size: 16px; font-weight: bold; color: #E6EDF3;");
  layout->addWidget(subheader);

  m_ActiveSandboxesTable = new QTableWidget(this);
  m_ActiveSandboxesTable->setColumnCount(3);
  m_ActiveSandboxesTable->setHorizontalHeaderLabels({"Profile Name", "Executable", "Status"});
  m_ActiveSandboxesTable->horizontalHeader()->setStretchLastSection(true);
  m_ActiveSandboxesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_ActiveSandboxesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  layout->addWidget(m_ActiveSandboxesTable, 1);
}

void IsolationView::onLaunchClicked() {
  QString path = m_ExecutablePath->text();
  if (path.isEmpty()) {
    QMessageBox::warning(this, "Error", "Please select an executable to launch.");
    return;
  }

  core::sandbox::SandboxProfile profile;
  profile.name = "Custom Sandbox " + std::to_string(m_ActiveSandboxesTable->rowCount() + 1);
  profile.executablePath = path.toStdString();
  profile.networkIsolation = true;
  profile.filesystemIsolation = true;

  core::sandbox::SandboxManager mgr;
  if (mgr.LaunchProfile(profile)) {
    int row = m_ActiveSandboxesTable->rowCount();
    m_ActiveSandboxesTable->insertRow(row);
    m_ActiveSandboxesTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(profile.name)));
    m_ActiveSandboxesTable->setItem(row, 1, new QTableWidgetItem(path));
    m_ActiveSandboxesTable->setItem(row, 2, new QTableWidgetItem("Running"));
  }
}

} // namespace severance::gui::isolation_view
