#include "TimelineDetailPanel.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>

namespace severance::gui::timeline {

TimelineDetailPanel::TimelineDetailPanel(QWidget* parent) : QWidget(parent) {
  setupUI();
}

TimelineDetailPanel::~TimelineDetailPanel() = default;

void TimelineDetailPanel::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(12);

  // Header
  auto* headerLayout = new QVBoxLayout();
  headerLayout->setSpacing(4);
  m_TitleLabel = new QLabel("Select a temporal event", this);
  m_TitleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #E6EDF3;");
  m_TitleLabel->setWordWrap(true);
  
  m_SubtitleLabel = new QLabel("", this);
  m_SubtitleLabel->setStyleSheet("font-size: 13px; color: #8B949E;");

  headerLayout->addWidget(m_TitleLabel);
  headerLayout->addWidget(m_SubtitleLabel);
  layout->addLayout(headerLayout);

  // Tabs
  m_Tabs = new QTabWidget(this);
  
  // Cross-Context Tab
  auto* contextTab = new QWidget(this);
  auto* contextLayout = new QVBoxLayout(contextTab);
  m_ContextTree = new QTreeWidget(this);
  m_ContextTree->setHeaderHidden(true);
  m_ContextTree->setStyleSheet("background: transparent; border: none; color: #F1F5F9; font-size: 13px;");
  contextLayout->addWidget(m_ContextTree);
  m_Tabs->addTab(contextTab, "CROSS-CONTEXT CHAIN");

  // Raw Data Tab
  auto* rawTab = new QWidget(this);
  auto* rawLayout = new QVBoxLayout(rawTab);
  m_RawContent = new QTextEdit(this);
  m_RawContent->setReadOnly(true);
  m_RawContent->setStyleSheet("background: transparent; border: none; color: #94A3B8; font-family: monospace;");
  rawLayout->addWidget(m_RawContent);
  m_Tabs->addTab(rawTab, "RAW LEDGER DATA");

  layout->addWidget(m_Tabs);
}

void TimelineDetailPanel::Clear() {
  m_TitleLabel->setText("Select a temporal event");
  m_SubtitleLabel->setText("");
  m_ContextTree->clear();
  m_RawContent->clear();
}

void TimelineDetailPanel::LoadEvent(const QString& time, const QString& type, const QString& source, const QString& details) {
  m_TitleLabel->setText(type);
  m_SubtitleLabel->setText(QString("Time: %1").arg(time));

  // Populate Cross-Context Tree (Mock representation of process -> net -> file correlation)
  m_ContextTree->clear();
  
  auto* rootItem = new QTreeWidgetItem(m_ContextTree);
  rootItem->setText(0, QString("Procedure: %1").arg(source));
  rootItem->setExpanded(true);
  
  auto* fileItem = new QTreeWidgetItem(rootItem);
  fileItem->setText(0, "Document Surveillance");
  fileItem->setExpanded(true);
  
  auto* fileDetail = new QTreeWidgetItem(fileItem);
  if (type == "FileAccess") {
    fileDetail->setText(0, details);
  } else {
    fileDetail->setText(0, "No correlated file writes in this execution block.");
  }
  
  auto* netItem = new QTreeWidgetItem(rootItem);
  netItem->setText(0, "Grid Communications");
  netItem->setExpanded(true);
  
  auto* netDetail = new QTreeWidgetItem(netItem);
  if (type == "NetworkConnect") {
    netDetail->setText(0, details);
  } else {
    netDetail->setText(0, "No correlated outbound connections detected.");
  }

  // Raw Json Mock
  QString rawJson = QString(R"({
  "timestamp": "%1",
  "event_type": "%2",
  "source": "%3",
  "payload": "%4",
  "correlation_id": "c-7b2x91a",
  "confidence": 0.99
})").arg(time).arg(type).arg(source).arg(QString(details).replace("\"", "\\\""));

  m_RawContent->setPlainText(rawJson);
}

} // namespace severance::gui::timeline
