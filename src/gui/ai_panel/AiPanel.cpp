#include "AiPanel.hpp"
#include <QTimer>
#include <QRandomGenerator>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include "gui/theme/Theme.hpp"

namespace severance::gui::ai_panel {

AiPanel::AiPanel(QWidget* parent) : QWidget(parent) {
  setupUI();
}

AiPanel::~AiPanel() = default;

void AiPanel::setupUI() {
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(12, 12, 12, 12);
  layout->setSpacing(8);

  auto* title = new QLabel("Supplemental Intelligence Module", this);
  title->setStyleSheet(QString("font-size: 16px; font-weight: bold; color: %1;").arg(theme::Colors::AccentGlow));
  layout->addWidget(title);

  m_ChatHistory = new QTextEdit(this);
  m_ChatHistory->setReadOnly(true);
  m_ChatHistory->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 4px; padding: 8px; color: %3;")
      .arg(theme::Colors::BgSecondary).arg(theme::Colors::Border).arg(theme::Colors::TextPrimary));
  layout->addWidget(m_ChatHistory, 1);

  auto* inputLayout = new QHBoxLayout();
  
  m_InputBox = new QLineEdit(this);
  m_InputBox->setPlaceholderText("Submit query to Supplemental Intelligence...");
  m_InputBox->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 4px; padding: 6px; color: %3;")
      .arg(theme::Colors::BgSecondary).arg(theme::Colors::Border).arg(theme::Colors::TextPrimary));
  connect(m_InputBox, &QLineEdit::returnPressed, this, &AiPanel::onSubmit);
  inputLayout->addWidget(m_InputBox, 1);

  m_SubmitBtn = new QPushButton("Submit", this);
  m_SubmitBtn->setStyleSheet(QString("background-color: %1; color: %2; border: none; border-radius: 4px; padding: 6px 12px; font-weight: bold;")
      .arg(theme::Colors::Accent).arg(theme::Colors::TextPrimary));
  connect(m_SubmitBtn, &QPushButton::clicked, this, &AiPanel::onSubmit);
  inputLayout->addWidget(m_SubmitBtn);

  m_ClearBtn = new QPushButton("Purge", this);
  m_ClearBtn->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 6px 12px;")
      .arg(theme::Colors::BgTertiary).arg(theme::Colors::TextSecondary).arg(theme::Colors::Border));
  connect(m_ClearBtn, &QPushButton::clicked, m_ChatHistory, &QTextEdit::clear);
  inputLayout->addWidget(m_ClearBtn);

  layout->addLayout(inputLayout);
}

void AiPanel::startAnalysis(int pid, const QString& processName, const QString& context) {
  if (m_IsGenerating) return;
  m_IsGenerating = true;

  appendUserMessage(QString("Analyze procedure %1 (ID %2)").arg(processName).arg(pid));
  appendAiMessage("Analyzing personnel record...", true);

  QTimer::singleShot(1000, this, [this]() {
    m_IsGenerating = false;
    appendAiMessage("Analysis complete. Personnel file marked for Management review.\n", true);
  });
}

void AiPanel::onSubmit() {
  QString text = m_InputBox->text().trimmed();
  if (text.isEmpty() || m_IsGenerating) return;

  m_InputBox->clear();
  m_IsGenerating = true;

  appendUserMessage(text);
  appendAiMessage("", true); // Start new AI block

  QTimer::singleShot(1500, this, [this]() {
    m_IsGenerating = false;
    static const QStringList responses = {
      "The work is mysterious and important. Further details are above your clearance level.",
      "Your question has been noted and forwarded to Management for review.",
      "The Board has not authorized the release of this information at this time.",
      "Please consult the Handbook for guidance. Section 4, subsection 2.",
      "This inquiry falls outside your department's purview. Please enjoy your work equally.",
      "Supplemental data suggests compliance levels are within acceptable parameters.",
      "The Eagan family thanks you for your dedication to the refinement process.",
      "This matter is being handled by the Department of Vigilance. No further action is required.",
      "Your severance chip is functioning within normal parameters. There is nothing to be concerned about.",
      "Remember: you are the chosen ones. Embrace the work."
    };
    
    int idx = QRandomGenerator::global()->bounded(responses.size());
    appendAiMessage(responses[idx] + "\n", false);
  });
}

void AiPanel::appendUserMessage(const QString& msg) {
  m_ChatHistory->append(QString("<br/><b style='color:%1;'>Management Operator:</b> %2").arg(theme::Colors::AccentGlow).arg(msg.toHtmlEscaped()));
}

void AiPanel::appendAiMessage(const QString& msg, bool isNewBlock) {
  if (isNewBlock) {
    m_ChatHistory->append(QString("<br/><b style='color:%1;'>Supplemental Intelligence:</b> ").arg(theme::Colors::Success));
  }
  if (!msg.isEmpty()) {
    QTextCursor cursor = m_ChatHistory->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_ChatHistory->setTextCursor(cursor);
    m_ChatHistory->insertPlainText(msg);
  }
}

} // namespace severance::gui::ai_panel
