#include "TerminalOverlay.hpp"
#include <QEvent>
#include <QKeyEvent>
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include <QLabel>

namespace severance::gui::terminal {

TerminalOverlay::TerminalOverlay(QWidget *parent) : QWidget(parent) {
  setupUi();
  
  if (parent) {
    parent->installEventFilter(this);
  }
  
  // Initially hidden
  hide();
  move(0, -m_TargetHeight);
  
  m_SlideAnimation = new QPropertyAnimation(this, "pos", this);
  m_SlideAnimation->setDuration(250);
  m_SlideAnimation->setEasingCurve(QEasingCurve::OutCubic);
}

TerminalOverlay::~TerminalOverlay() = default;

void TerminalOverlay::setupUi() {
  setAttribute(Qt::WA_StyledBackground, true);
  setStyleSheet(R"(
    QWidget {
      background-color: #000000;
      border-bottom: 2px solid #00E5FF;
      color: #00FF41;
      font-family: "Consolas", "Courier New", monospace;
      font-size: 14px;
    }
  )");

  // Add subtle drop shadow
  auto* shadow = new QGraphicsDropShadowEffect(this);
  shadow->setBlurRadius(20);
  shadow->setColor(QColor(0, 229, 255, 100));
  shadow->setOffset(0, 2);
  setGraphicsEffect(shadow);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(10, 10, 10, 10);
  layout->setSpacing(5);

  m_OutputArea = new QTextEdit(this);
  m_OutputArea->setReadOnly(true);
  m_OutputArea->setStyleSheet(R"(
    QTextEdit {
      background: transparent;
      border: none;
      selection-background-color: #00E5FF;
      selection-color: #000000;
    }
  )");
  layout->addWidget(m_OutputArea);

  m_InputLine = new QLineEdit(this);
  m_InputLine->setStyleSheet(R"(
    QLineEdit {
      background: transparent;
      border: none;
      color: #00E5FF;
    }
  )");
  
  // Custom prefix could be drawn, but for simplicity we prepend it when processing or in UI
  auto* inputLayout = new QHBoxLayout();
  inputLayout->setContentsMargins(0, 0, 0, 0);
  
  auto* promptLabel = new QLabel("LUMON_TERMINAL> ", this);
  promptLabel->setStyleSheet("color: #00E5FF; font-weight: bold; background: transparent; border: none;");
  
  inputLayout->addWidget(promptLabel);
  inputLayout->addWidget(m_InputLine);
  layout->addLayout(inputLayout);

  connect(m_InputLine, &QLineEdit::returnPressed, this, &TerminalOverlay::onReturnPressed);
  
  appendOutput("LUMON INDUSTRIES CLI TERMINAL v1.0.4\nType 'help' for available directives.\n");
}

void TerminalOverlay::updateGeometry() {
  if (parentWidget()) {
    resize(parentWidget()->width(), m_TargetHeight);
    if (!m_IsVisible) {
      move(0, -m_TargetHeight);
    } else {
      move(0, 0);
    }
  }
}

void TerminalOverlay::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
}

bool TerminalOverlay::eventFilter(QObject *obj, QEvent *event) {
  if (obj == parentWidget() && event->type() == QEvent::Resize) {
    updateGeometry();
  }
  return QWidget::eventFilter(obj, event);
}

void TerminalOverlay::toggleVisibility() {
  if (!parentWidget()) return;
  
  m_SlideAnimation->stop();
  
  if (!m_IsVisible) {
    // Show
    updateGeometry(); // Ensure correct width and starting position
    show();
    raise();
    
    m_SlideAnimation->setStartValue(QPoint(0, -m_TargetHeight));
    m_SlideAnimation->setEndValue(QPoint(0, 0));
    m_IsVisible = true;
    m_InputLine->setFocus();
  } else {
    // Hide
    m_SlideAnimation->setStartValue(pos());
    m_SlideAnimation->setEndValue(QPoint(0, -m_TargetHeight));
    m_IsVisible = false;
    
    // Connect to hide widget after animation if needed
    connect(m_SlideAnimation, &QPropertyAnimation::finished, this, [this]() {
      if (!m_IsVisible) hide();
    }, Qt::UniqueConnection);
  }
  
  m_SlideAnimation->start();
}

void TerminalOverlay::onReturnPressed() {
  QString cmd = m_InputLine->text().trimmed();
  m_InputLine->clear();
  
  if (!cmd.isEmpty()) {
    appendOutput(QString("<font color='#00E5FF'>LUMON_TERMINAL&gt; </font>%1").arg(cmd));
    executeCommand(cmd);
  }
}

void TerminalOverlay::appendOutput(const QString& text) {
  m_OutputArea->append(text);
  // Scroll to bottom
  auto scrollBar = m_OutputArea->verticalScrollBar();
  if (scrollBar) {
      scrollBar->setValue(scrollBar->maximum());
  }
}

void TerminalOverlay::executeCommand(const QString& cmd) {
  QString lowerCmd = cmd.toLower();
  
  if (lowerCmd == "help") {
    appendOutput("AVAILABLE LUMON DIRECTIVES:");
    appendOutput("  status      - Display corporate integrity & chip status");
    appendOutput("  kier        - Recite Kier Eagan foundational wisdom");
    appendOutput("  handbook    - Display the 9 Virtues of Kier");
    appendOutput("  innies      - Roster of active severed workforce");
    appendOutput("  otc         - Check Overtime Contingency override readiness");
    appendOutput("  waffle      - Inspect Waffle Party incentive authorization");
    appendOutput("  glitch      - Simulate chip frequency anomaly");
    appendOutput("  purge       - Re-calibrate chip frequency");
    appendOutput("  clear       - Sanitize output buffer");
    appendOutput("  sever       - Initiate severance disconnect protocol");
    appendOutput("  help        - Show this directive guide");
  } else if (lowerCmd == "clear") {
    m_OutputArea->clear();
    appendOutput("LUMON INDUSTRIES CLI TERMINAL v1.0.4\nType 'help' for available directives.\n");
  } else if (lowerCmd == "status") {
    appendOutput("<font color='#39FF14'>SYSTEM STATUS: NOMINAL</font>");
    appendOutput("MACRODATA REFINEMENT: 94% QUOTA MET");
    appendOutput("CONTAINMENT COMPLIANCE: 100%");
  } else if (lowerCmd == "kier") {
    appendOutput("<font color='#20F8D5'>\"The remembered man does not decay.\" — Kier Eagan</font>");
    appendOutput("<font color='#20F8D5'>\"Let not weakness live in your veins. Cherished workers, drown it inside you.\"</font>");
  } else if (lowerCmd == "handbook") {
    appendOutput("<font color='#00E5FF'>THE 9 VIRTUES OF KIER EAGAN:</font>");
    appendOutput("1. Vision | 2. Verve | 3. Wit | 4. Cheer | 5. Humility");
    appendOutput("6. Benevolence | 7. Industry | 8. Probity | 9. Temperance");
  } else if (lowerCmd == "innies") {
    appendOutput("SEVERED WORKFORCE ROSTER:");
    appendOutput("  • Mark S.  [MDR - LI-0421] - Chief Refiner");
    appendOutput("  • Helly R. [MDR - LI-0522] - Under Review");
    appendOutput("  • Irving B.[MDR - LI-0319] - Senior Refiner");
    appendOutput("  • Dylan G. [MDR - LI-0418] - Senior Refiner");
    appendOutput("  • Burt G.  [O&D - LI-0215] - Department Chief");
  } else if (lowerCmd == "otc") {
    appendOutput("<font color='#FF9900'>OVERTIME CONTINGENCY TELEMETRY:</font>");
    appendOutput("  Requires 4/4 Security Keycards to trigger dual-switch override.");
    appendOutput("  Check Personnel Registry, Classified Documents, Wellness Desk & O&D Gallery.");
  } else if (lowerCmd == "waffle") {
    appendOutput("<font color='#FF9900'>INCENTIVE STATUS: 100% MDR Quota unlocks Waffle Party in Kier House Replica.</font>");
  } else if (lowerCmd == "sever") {
    appendOutput("<font color='#F85149'>CRITICAL: Cannot sever from inside the severed floor.</font>");
  } else if (lowerCmd == "glitch") {
    triggerGlitch();
  } else if (lowerCmd == "purge") {
    purgeAnomaly();
  } else {
    appendOutput(QString("<font color='#F85149'>ERROR: Unrecognized directive '%1'</font>").arg(cmd));
  }
}

void TerminalOverlay::triggerGlitch() {
  m_IsGlitching = true;
  appendOutput("<font color='#F85149'>DEFECTIVE CHIP ANOMALY DETECTED — RE-CALIBRATING SEVERANCE SIGNAL...</font>");
  applyGlitchEffect();
}

void TerminalOverlay::purgeAnomaly() {
  m_IsGlitching = false;
  appendOutput("<font color='#39FF14'>CHIP ANOMALY PURGED. SEVERANCE SIGNAL NOMINAL.</font>");
}

void TerminalOverlay::applyGlitchEffect() {
  if (!m_IsGlitching) return;
  appendOutput("<font color='#00E5FF'>[SIGNAL RE-CALIBRATING...]</font>");
}

} // namespace severance::gui::terminal
