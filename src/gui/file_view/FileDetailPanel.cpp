#include "FileDetailPanel.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QDir>

namespace severance::gui::file_view {

FileDetailPanel::FileDetailPanel(QWidget* parent) : QWidget(parent) {
  setupUI();
}

FileDetailPanel::~FileDetailPanel() = default;

void FileDetailPanel::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(12);

  // Header
  auto* headerLayout = new QVBoxLayout();
  headerLayout->setSpacing(4);
  m_TitleLabel = new QLabel("Select a document processing event", this);
  m_TitleLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #E6EDF3;");
  m_TitleLabel->setWordWrap(true);
  
  m_SubtitleLabel = new QLabel("", this);
  m_SubtitleLabel->setStyleSheet("font-size: 13px; color: #8B949E;");

  headerLayout->addWidget(m_TitleLabel);
  headerLayout->addWidget(m_SubtitleLabel);
  layout->addLayout(headerLayout);

  // Tabs
  m_Tabs = new QTabWidget(this);
  
  // Overview Tab
  auto* overviewTab = new QWidget(this);
  auto* overviewLayout = new QVBoxLayout(overviewTab);
  m_OverviewContent = new QTextEdit(this);
  m_OverviewContent->setReadOnly(true);
  m_OverviewContent->setStyleSheet("background: transparent; border: none; color: #F1F5F9; font-size: 13px;");
  overviewLayout->addWidget(m_OverviewContent);
  m_Tabs->addTab(overviewTab, "OVERVIEW");

  // Diff Tab (Placeholder)
  auto* diffTab = new QWidget(this);
  auto* diffLayout = new QVBoxLayout(diffTab);
  m_DiffContent = new QTextEdit(this);
  m_DiffContent->setReadOnly(true);
  m_DiffContent->setStyleSheet("background: transparent; border: none; color: #94A3B8; font-family: monospace;");
  diffLayout->addWidget(m_DiffContent);
  m_Tabs->addTab(diffTab, "DOCUMENT DELTA");

  // Bulk Write Tab (Placeholder)
  auto* bulkTab = new QWidget(this);
  auto* bulkLayout = new QVBoxLayout(bulkTab);
  m_BulkWriteLbl = new QLabel("[Macrodata Refinement Placeholder]\nDetects anomalous high-frequency numerical patterns.", this);
  m_BulkWriteLbl->setAlignment(Qt::AlignCenter);
  m_BulkWriteLbl->setStyleSheet("color: #64748B; font-style: italic;");
  bulkLayout->addWidget(m_BulkWriteLbl);
  m_Tabs->addTab(bulkTab, "MACRODATA ANALYSIS");

  layout->addWidget(m_Tabs);
}

void FileDetailPanel::Clear() {
  m_TitleLabel->setText("Select a document processing event");
  m_SubtitleLabel->setText("");
  m_OverviewContent->clear();
  m_DiffContent->clear();
}

void FileDetailPanel::LoadFileEvent(const QString& time, const QString& procName, uint32_t pid, const QString& operation, const QString& path) {
  QFileInfo fi(path);
  m_TitleLabel->setText(fi.fileName().isEmpty() ? path : fi.fileName());
  m_SubtitleLabel->setText(QString("Operation: %1 | Time: %2").arg(operation).arg(time));

  QString overviewHtml = QString(R"(
    <h3>Surveillance Details</h3>
    <table cellpadding="4">
      <tr><td style="color:#8B949E;">Procedure:</td><td>%1</td></tr>
      <tr><td style="color:#8B949E;">ID:</td><td>%2</td></tr>
      <tr><td style="color:#8B949E;">Designation:</td><td>%3</td></tr>
      <tr><td style="color:#8B949E;">Sector:</td><td>%4</td></tr>
    </table>
  )").arg(procName).arg(pid).arg(path).arg(fi.dir().path());

  m_OverviewContent->setHtml(overviewHtml);

  if (operation.toLower().contains("write") || operation.toLower().contains("modify")) {
    m_DiffContent->setPlainText(QString("File modified. Diffing not available for %1 (Text-only diffs pending ETW payload capture).").arg(fi.fileName()));
  } else {
    m_DiffContent->setPlainText("No diff available for this operation type.");
  }
}

} // namespace severance::gui::file_view
