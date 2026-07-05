#include "AiPanel.hpp"
#include "core/ai/AiEngine.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>

namespace severance::gui::ai_panel {

AiPanel::AiPanel(QWidget* parent) : QWidget(parent) {
  setupUI();

  auto& engine = core::ai::AiEngine::GetInstance();
  engine.initialize();

  connect(&engine, &core::ai::AiEngine::responseChunkReceived, this, &AiPanel::onChunkReceived);
  connect(&engine, &core::ai::AiEngine::responseFinished, this, &AiPanel::onResponseFinished);
  connect(&engine, &core::ai::AiEngine::errorOccurred, this, &AiPanel::onError);
}

AiPanel::~AiPanel() = default;

void AiPanel::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(12, 12, 12, 12);
  layout->setSpacing(8);

  auto* title = new QLabel("Supplemental Intelligence Module", this);
  title->setStyleSheet("font-size: 16px; font-weight: bold; color: #58A6FF;");
  layout->addWidget(title);

  m_ChatHistory = new QTextEdit(this);
  m_ChatHistory->setReadOnly(true);
  m_ChatHistory->setStyleSheet("background-color: #0D1117; border: 1px solid #30363D; border-radius: 6px; padding: 8px; color: #C9D1D9;");
  layout->addWidget(m_ChatHistory, 1);

  auto* inputLayout = new QHBoxLayout();
  
  m_InputBox = new QLineEdit(this);
  m_InputBox->setPlaceholderText("Submit query to Supplemental Intelligence...");
  m_InputBox->setStyleSheet("background-color: #0D1117; border: 1px solid #30363D; border-radius: 4px; padding: 6px; color: #C9D1D9;");
  connect(m_InputBox, &QLineEdit::returnPressed, this, &AiPanel::onSubmit);
  inputLayout->addWidget(m_InputBox, 1);

  m_SubmitBtn = new QPushButton("Submit", this);
  m_SubmitBtn->setStyleSheet("background-color: #238636; color: white; border: none; border-radius: 4px; padding: 6px 12px; font-weight: bold;");
  connect(m_SubmitBtn, &QPushButton::clicked, this, &AiPanel::onSubmit);
  inputLayout->addWidget(m_SubmitBtn);

  m_ClearBtn = new QPushButton("Purge", this);
  m_ClearBtn->setStyleSheet("background-color: #21262D; color: #C9D1D9; border: 1px solid #30363D; border-radius: 4px; padding: 6px 12px;");
  connect(m_ClearBtn, &QPushButton::clicked, m_ChatHistory, &QTextEdit::clear);
  inputLayout->addWidget(m_ClearBtn);

  layout->addLayout(inputLayout);
}

void AiPanel::startAnalysis(int pid, const QString& processName, const QString& context) {
  if (m_IsGenerating) return;
  m_IsGenerating = true;

  appendUserMessage(QString("Analyze procedure %1 (ID %2)").arg(processName).arg(pid));
  appendAiMessage("Analyzing...", true);

  core::ai::AiEngine::GetInstance().analyzeProcess(pid, processName, context);
}

void AiPanel::onSubmit() {
  QString text = m_InputBox->text().trimmed();
  if (text.isEmpty() || m_IsGenerating) return;

  m_InputBox->clear();
  m_IsGenerating = true;

  appendUserMessage(text);
  appendAiMessage("", true); // Start new AI block

  core::ai::AiEngine::GetInstance().systemQuery(text);
}

void AiPanel::onChunkReceived(const QString& chunk) {
  // If the last text was "Analyzing...", clear it first
  QString currentHtml = m_ChatHistory->toHtml();
  if (currentHtml.contains("Analyzing...")) {
      // Very naive clear for the placeholder. In a real app we'd manage blocks better.
      // But QTextEdit append logic makes this tricky without a full model.
      // We will just append the chunk.
  }
  
  // To avoid HTML injection and make it stream nicely, we insert plaintext at the end.
  QTextCursor cursor = m_ChatHistory->textCursor();
  cursor.movePosition(QTextCursor::End);
  m_ChatHistory->setTextCursor(cursor);
  m_ChatHistory->insertPlainText(chunk);
  
  // Auto-scroll to bottom
  QScrollBar *vScrollBar = m_ChatHistory->verticalScrollBar();
  vScrollBar->setValue(vScrollBar->maximum());
}

void AiPanel::onResponseFinished() {
  m_IsGenerating = false;
  // Add an extra newline at the end of the AI's response for spacing
  appendAiMessage("\n", false);
}

void AiPanel::onError(const QString& errorMsg) {
  m_IsGenerating = false;
  appendAiMessage(QString("\n[Error: %1]").arg(errorMsg), false);
}

void AiPanel::appendUserMessage(const QString& msg) {
  m_ChatHistory->append(QString("<br/><b style='color:#58A6FF;'>Operator:</b> %1").arg(msg.toHtmlEscaped()));
}

void AiPanel::appendAiMessage(const QString& msg, bool isNewBlock) {
  if (isNewBlock) {
    m_ChatHistory->append("<br/><b style='color:#3FB950;'>Supplemental Intelligence:</b> ");
  }
  if (!msg.isEmpty()) {
    QTextCursor cursor = m_ChatHistory->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_ChatHistory->setTextCursor(cursor);
    m_ChatHistory->insertPlainText(msg);
  }
}

} // namespace severance::gui::ai_panel
