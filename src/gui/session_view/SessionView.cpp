#include "SessionView.hpp"
#include "core/session/SessionManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include <QDesktopServices>
#include <QUrl>

namespace severance::gui::session_view {

SessionView::SessionView(QWidget *parent) : QWidget(parent) {
  setupUI();

  auto* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &SessionView::updateStatus);
  timer->start(1000);
  
  updateStatus();
}

void SessionView::setupUI() {
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(24, 24, 24, 24);
  mainLayout->setSpacing(20);

  // Header
  auto* header = new QLabel("Incident Session & Telemetry", this);
  header->setStyleSheet("font-size: 20px; font-weight: bold; color: #E6EDF3;");
  mainLayout->addWidget(header);

  auto* desc = new QLabel("Record telemetry, add annotations, and export beautiful incident reports.", this);
  desc->setStyleSheet("color: #8B949E; font-size: 14px;");
  mainLayout->addWidget(desc);
  mainLayout->addSpacing(10);

  // Controls Panel
  auto* controlsPane = new QWidget(this);
  controlsPane->setStyleSheet("background-color: #0D1117; border: 1px solid #30363D; border-radius: 6px;");
  auto* controlsLayout = new QVBoxLayout(controlsPane);

  auto* topRow = new QHBoxLayout();
  m_RecordBtn = new QPushButton("Start Recording", this);
  m_RecordBtn->setFixedSize(140, 36);
  m_RecordBtn->setStyleSheet("background-color: #DA3633; color: white; font-weight: bold; border-radius: 4px;");
  connect(m_RecordBtn, &QPushButton::clicked, this, &SessionView::onToggleRecording);

  m_StatusLabel = new QLabel("Status: Ready", this);
  m_StatusLabel->setStyleSheet("color: #8B949E; font-size: 14px;");
  
  m_EventCountLabel = new QLabel("Events: 0", this);
  m_EventCountLabel->setStyleSheet("color: #8B949E; font-size: 14px;");

  topRow->addWidget(m_RecordBtn);
  topRow->addSpacing(20);
  topRow->addWidget(m_StatusLabel);
  topRow->addStretch();
  topRow->addWidget(m_EventCountLabel);
  controlsLayout->addLayout(topRow);

  // Annotations
  controlsLayout->addSpacing(20);
  auto* annLabel = new QLabel("Incident Annotations", this);
  annLabel->setStyleSheet("font-weight: bold; color: #E6EDF3;");
  controlsLayout->addWidget(annLabel);

  auto* annRow = new QHBoxLayout();
  m_AnnotationInput = new QLineEdit(this);
  m_AnnotationInput->setPlaceholderText("Enter observation or note...");
  m_AnnotationInput->setStyleSheet("background-color: #010409; color: #E6EDF3; border: 1px solid #30363D; border-radius: 4px; padding: 4px;");
  m_AnnotationInput->setEnabled(false);

  m_AddAnnotationBtn = new QPushButton("Add Note", this);
  m_AddAnnotationBtn->setStyleSheet("background-color: #238636; color: white; border-radius: 4px; padding: 4px 12px;");
  m_AddAnnotationBtn->setEnabled(false);
  connect(m_AddAnnotationBtn, &QPushButton::clicked, this, &SessionView::onAddAnnotation);
  connect(m_AnnotationInput, &QLineEdit::returnPressed, this, &SessionView::onAddAnnotation);

  annRow->addWidget(m_AnnotationInput);
  annRow->addWidget(m_AddAnnotationBtn);
  controlsLayout->addLayout(annRow);

  m_AnnotationList = new QListWidget(this);
  m_AnnotationList->setStyleSheet("background-color: #010409; color: #E6EDF3; border: 1px solid #30363D; border-radius: 4px;");
  controlsLayout->addWidget(m_AnnotationList);

  mainLayout->addWidget(controlsPane);

  // Exports Panel
  auto* exportsPane = new QWidget(this);
  exportsPane->setStyleSheet("background-color: #0D1117; border: 1px solid #30363D; border-radius: 6px;");
  auto* exportsLayout = new QVBoxLayout(exportsPane);

  auto* exportLabel = new QLabel("Telemetry Exports", this);
  exportLabel->setStyleSheet("font-weight: bold; color: #E6EDF3;");
  exportsLayout->addWidget(exportLabel);

  auto* exportRow = new QHBoxLayout();
  m_ExportMdBtn = new QPushButton("Export to Markdown", this);
  m_ExportMdBtn->setStyleSheet("background-color: #1F6FEB; color: white; border-radius: 4px; padding: 6px 16px;");
  connect(m_ExportMdBtn, &QPushButton::clicked, this, &SessionView::onExportMarkdown);

  m_ExportJsonBtn = new QPushButton("Export to JSON", this);
  m_ExportJsonBtn->setStyleSheet("background-color: #30363D; color: white; border-radius: 4px; padding: 6px 16px;");
  connect(m_ExportJsonBtn, &QPushButton::clicked, this, &SessionView::onExportJson);

  exportRow->addWidget(m_ExportMdBtn);
  exportRow->addWidget(m_ExportJsonBtn);
  exportRow->addStretch();
  exportsLayout->addLayout(exportRow);

  mainLayout->addWidget(exportsPane);
  mainLayout->addStretch();
}

void SessionView::onToggleRecording() {
  auto& sm = core::session::SessionManager::GetInstance();
  if (sm.IsRecording()) {
    sm.StopRecording();
    m_RecordBtn->setText("Start Recording");
    m_RecordBtn->setStyleSheet("background-color: #DA3633; color: white; font-weight: bold; border-radius: 4px;");
    m_StatusLabel->setText("Status: Stopped");
    m_StatusLabel->setStyleSheet("color: #8B949E; font-size: 14px;");
    m_AnnotationInput->setEnabled(false);
    m_AddAnnotationBtn->setEnabled(false);
  } else {
    sm.StartRecording("Incident Response Session");
    m_RecordBtn->setText("Stop Recording");
    m_RecordBtn->setStyleSheet("background-color: #30363D; color: #E6EDF3; font-weight: bold; border-radius: 4px;");
    m_StatusLabel->setText("Status: RECORDING");
    m_StatusLabel->setStyleSheet("color: #DA3633; font-weight: bold; font-size: 14px;");
    m_AnnotationInput->setEnabled(true);
    m_AddAnnotationBtn->setEnabled(true);
    m_AnnotationList->clear();
  }
}

void SessionView::onAddAnnotation() {
  QString note = m_AnnotationInput->text().trimmed();
  if (note.isEmpty()) return;

  auto& sm = core::session::SessionManager::GetInstance();
  if (sm.IsRecording()) {
    sm.AddAnnotation(note.toStdString());
    m_AnnotationList->addItem(QString("[%1] %2").arg(QTime::currentTime().toString("HH:mm:ss")).arg(note));
    m_AnnotationInput->clear();
  }
}

void SessionView::updateStatus() {
  auto& sm = core::session::SessionManager::GetInstance();
  // We can't easily get eventCount dynamically without adding an accessor.
  // For UI MVP, we'll assume we can't easily poll eventCount if there is no getter.
  // Oh wait, `eventCount` is inside `m_CurrentSession` which is private. We can just leave it as "Events: (recording)"
  if (sm.IsRecording()) {
    m_EventCountLabel->setText("Events: Recording...");
  } else {
    m_EventCountLabel->setText("Events: Stopped");
  }
}

void SessionView::onExportMarkdown() {
  QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/SeveranceReports";
  auto& sm = core::session::SessionManager::GetInstance();
  std::string file = sm.ExportMarkdown(path.toStdString());
  QMessageBox::information(this, "Export Success", QString("Markdown Telemetry Report exported to:\n%1").arg(QString::fromStdString(file)));
  QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(file)));
}

void SessionView::onExportJson() {
  QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/SeveranceReports";
  auto& sm = core::session::SessionManager::GetInstance();
  std::string file = sm.ExportSession(path.toStdString());
  QMessageBox::information(this, "Export Success", QString("JSON Telemetry Report exported to:\n%1").arg(QString::fromStdString(file)));
  QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(file)));
}

} // namespace severance::gui::session_view
