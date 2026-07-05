#include "BoardCommsView.hpp"
#include <QLabel>
#include <QKeyEvent>
#include <QDateTime>
#include <QRandomGenerator>

namespace severance::gui::board_comms {

BoardCommsView::BoardCommsView(QWidget *parent) : QWidget(parent) {
  setupUI();

  m_ResponseTimer = new QTimer(this);
  m_ResponseTimer->setSingleShot(true);
  connect(m_ResponseTimer, &QTimer::timeout, this, &BoardCommsView::onBoardResponse);
}

BoardCommsView::~BoardCommsView() = default;

void BoardCommsView::setupUI() {
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(10);

  auto titleLabel = new QLabel("BOARD COMMUNICATIONS INTERFACE", this);
  titleLabel->setStyleSheet("color: #00E5FF; font-family: monospace; font-size: 16px; font-weight: bold; letter-spacing: 2px;");
  layout->addWidget(titleLabel);

  m_TerminalDisplay = new QTextEdit(this);
  m_TerminalDisplay->setReadOnly(true);
  m_TerminalDisplay->setStyleSheet(R"(
    QTextEdit {
      background-color: #0D1117;
      color: #39FF14;
      font-family: monospace;
      font-size: 14px;
      border: 1px solid #30363D;
      padding: 10px;
    }
  )");
  layout->addWidget(m_TerminalDisplay, 1);

  m_InputField = new QLineEdit(this);
  m_InputField->setPlaceholderText("Enter transmission...");
  m_InputField->setStyleSheet(R"(
    QLineEdit {
      background-color: #161B22;
      color: #F1F5F9;
      font-family: monospace;
      font-size: 14px;
      border: 1px solid #30363D;
      padding: 8px;
    }
    QLineEdit:focus {
      border: 1px solid #00E5FF;
    }
  )");
  connect(m_InputField, &QLineEdit::returnPressed, this, &BoardCommsView::onMessageSent);
  layout->addWidget(m_InputField);

  addMessage("SYSTEM", "ENCRYPTED CHANNEL ESTABLISHED. THE BOARD IS LISTENING.", "#8B949E");
}

void BoardCommsView::addMessage(const QString& sender, const QString& message, const QString& color) {
  QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
  QString html = QString("<span style='color:#6E7681;'>[%1]</span> <strong style='color:%2;'>%3:</strong> <span style='color:%2;'>%4</span>")
                  .arg(timeStr, color, sender, message);
  m_TerminalDisplay->append(html);
}

void BoardCommsView::onMessageSent() {
  QString msg = m_InputField->text().trimmed();
  if (msg.isEmpty()) return;

  m_InputField->clear();
  m_InputField->setDisabled(true);

  addMessage("MANAGEMENT", msg, "#F1F5F9");

  // Simulate typing/processing delay
  int delayMs = QRandomGenerator::global()->bounded(2000, 5000);
  m_ResponseTimer->start(delayMs);
}

void BoardCommsView::onBoardResponse() {
  QStringList responses = {
    "The Board is satisfied.",
    "The Board has ended the transmission.",
    "The Board does not find this query relevant.",
    "Your diligence is noted.",
    "Do not inquire further on this matter.",
    "The work is mysterious and important.",
    "Focus on your immediate directives.",
    "The Board reminds you that questions are a burden to others.",
    "Acknowledge and comply.",
    "The Board requires results, not correspondence."
  };

  QString response = responses.at(QRandomGenerator::global()->bounded(responses.size()));
  addMessage("THE BOARD", response, "#F85149");

  m_InputField->setDisabled(false);
  m_InputField->setFocus();
}

} // namespace severance::gui::board_comms
