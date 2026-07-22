#include "BoardCommsView.hpp"
#include <QLabel>
#include <QKeyEvent>
#include <QDateTime>
#include <QRandomGenerator>
#include <QPainter>
#include <QHBoxLayout>
#include <cmath>
#include "core/store/EventStore.hpp"

namespace severance::gui::board_comms {

SpeakerLight::SpeakerLight(QWidget* parent) : QWidget(parent) {
  setFixedSize(60, 60);
  connect(&m_Timer, &QTimer::timeout, this, &SpeakerLight::updatePulse);
}

void SpeakerLight::setMode(int mode) {
  m_Mode = mode;
  if (m_Mode == 1 || m_Mode == 2) {
    m_Timer.start(50);
  } else {
    m_Timer.stop();
    m_PulsePhase = 0.0f;
  }
  update();
}

void SpeakerLight::updatePulse() {
  m_PulsePhase += 0.15f;
  if (m_PulsePhase > 2.0f * 3.14159f) m_PulsePhase -= 2.0f * 3.14159f;
  update();
}

void SpeakerLight::paintEvent(QPaintEvent* /*event*/) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  
  QRectF rect(5, 5, 50, 50);
  
  QColor color = QColor("#30363D"); // Idle
  if (m_Mode == 1) { // Deliberating (red/cyan pulse)
    float val = (std::sin(m_PulsePhase) + 1.0f) / 2.0f;
    int r = static_cast<int>(255 * val);
    int c = static_cast<int>(255 * (1.0f - val));
    color = QColor(r, c, c); // red/cyan interpolation
  } else if (m_Mode == 2) { // Speaking (flashes brightly)
    float val = (std::sin(m_PulsePhase * 3.0f) + 1.0f) / 2.0f;
    int v = static_cast<int>(100 + 155 * val);
    color = QColor(v, v, v);
  }
  
  p.setBrush(color);
  p.setPen(QPen(QColor("#F1F5F9"), 2));
  p.drawEllipse(rect);
}

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

  auto headerLayout = new QHBoxLayout();
  auto titleLabel = new QLabel("BOARD COMMUNICATIONS INTERFACE", this);
  titleLabel->setStyleSheet("color: #00E5FF; font-family: monospace; font-size: 16px; font-weight: bold; letter-spacing: 2px;");
  headerLayout->addWidget(titleLabel);
  
  headerLayout->addStretch();
  
  m_SpeakerLight = new SpeakerLight(this);
  headerLayout->addWidget(m_SpeakerLight);
  
  layout->addLayout(headerLayout);

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
  
  auto presetLayout = new QHBoxLayout();
  QStringList presets = {"I ACCEPT THE DIRECTIVE", "REQUEST CLARIFICATION", "PRAISE KIER EAGAN"};
  for (const auto& p : presets) {
    auto btn = new QPushButton(QString("[ %1 ]").arg(p), this);
    btn->setStyleSheet(R"(
      QPushButton {
        background-color: #161B22;
        color: #00E5FF;
        border: 1px solid #30363D;
        padding: 6px;
        font-family: monospace;
        font-weight: bold;
      }
      QPushButton:hover {
        background-color: #00E5FF;
        color: #0D1117;
      }
    )");
    connect(btn, &QPushButton::clicked, [this, p]() { onPresetClicked(p); });
    presetLayout->addWidget(btn);
  }
  layout->addLayout(presetLayout);

  addMessage("SYSTEM", "ENCRYPTED CHANNEL ESTABLISHED. THE BOARD IS LISTENING.", "#8B949E");
}

void BoardCommsView::addMessage(const QString& sender, const QString& message, const QString& color) {
  QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
  QString html = QString("<span style='color:#6E7681;'>[%1]</span> <strong style='color:%2;'>%3:</strong> <span style='color:%2;'>%4</span>")
                  .arg(timeStr, color, sender, message);
  m_TerminalDisplay->append(html);
}

void BoardCommsView::onPresetClicked(const QString& preset) {
  if (m_InputField->isEnabled()) {
    m_InputField->setText(preset);
    onMessageSent();
  }
}

void BoardCommsView::onMessageSent() {
  QString msg = m_InputField->text().trimmed();
  if (msg.isEmpty()) return;

  m_InputField->clear();
  m_InputField->setDisabled(true);

  addMessage("MANAGEMENT", msg, "#F1F5F9");

  // 1. Short-term memory update
  m_ShortTermMemory.push_back(msg);
  if (m_ShortTermMemory.size() > 5) {
    m_ShortTermMemory.pop_front();
  }

  // 2. Fact updates based on user keywords
  QString lowerMsg = msg.toLower();
  if (lowerMsg.contains("quit") || lowerMsg.contains("leave") || lowerMsg.contains("severance") || lowerMsg.contains("resign")) {
    core::store::EventStore::GetInstance().StoreEntityFact("FlightRisk", "High");
  } else if (lowerMsg.contains("rebel") || lowerMsg.contains("overthrow")) {
    core::store::EventStore::GetInstance().StoreEntityFact("Infractions", "Critical");
  }

  // Simulate typing/processing delay
  int delayMs = QRandomGenerator::global()->bounded(2000, 5000);
  m_ResponseTimer->start(delayMs);
  
  if (m_SpeakerLight) {
    m_SpeakerLight->setMode(1); // Deliberating
  }
}

void BoardCommsView::onBoardResponse() {
  if (m_SpeakerLight) {
    m_SpeakerLight->setMode(2); // Speaking
    QTimer::singleShot(2000, this, [this]() {
      if (m_SpeakerLight) m_SpeakerLight->setMode(0); // Idle
    });
  }
  // 3. Retrieve Tiered Memory context
  QString flightRisk = core::store::EventStore::GetInstance().GetEntityFact("FlightRisk");
  QString infractions = core::store::EventStore::GetInstance().GetEntityFact("Infractions");
  
  // Also checking conversation memory just to show integration
  auto recentSummaries = core::store::EventStore::GetInstance().GetRecentSummaries(3);

  QStringList responses;

  if (flightRisk == "High") {
    responses << "The Board reminds you that severance is permanent."
              << "A departure is not authorized. Return to your work."
              << "The Board is concerned by your recent expressions of discontent.";
  } 
  
  if (infractions == "Critical") {
    responses << "Your infractions have been noted. The Break Room awaits."
              << "Cease your rebellious inquiries immediately.";
  }

  if (responses.isEmpty()) {
    responses = {
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
  }

  QString response = responses.at(QRandomGenerator::global()->bounded(responses.size()));
  
  // Slight contextualization if the user's message is extremely short
  if (!m_ShortTermMemory.empty() && m_ShortTermMemory.back().length() < 4) {
    if (responses.contains("Acknowledge and comply.")) {
      response = "Brief communications are inefficient. " + response;
    }
  }

  addMessage("THE BOARD", response, "#F85149");

  // 4. Store conversation summary for long-term memory
  QString summary = "User said: " + (m_ShortTermMemory.empty() ? "" : m_ShortTermMemory.back()) + " | Board replied: " + response;
  core::store::EventStore::GetInstance().StoreConversationSummary(summary);

  m_InputField->setDisabled(false);
  m_InputField->setFocus();
}

} // namespace severance::gui::board_comms
