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
    appendOutput("AVAILABLE DIRECTIVES:");
    appendOutput("  clear       - Sanitize output buffer");
    appendOutput("  status      - Display corporate integrity status");
    appendOutput("  waffle      - Request waffle party authorization (WIP)");
    appendOutput("  sever       - Initiate severance protocol (Requires Kier authorization)");
    appendOutput("  help        - Show this message");
  } else if (lowerCmd == "clear") {
    m_OutputArea->clear();
    appendOutput("LUMON INDUSTRIES CLI TERMINAL v1.0.4\nType 'help' for available directives.\n");
  } else if (lowerCmd == "status") {
    appendOutput("SYSTEM STATUS: NOMINAL");
    appendOutput("MACRODATA REFINEMENT: 89% QUOTA MET");
  } else if (lowerCmd == "waffle") {
    appendOutput("<font color='#F85149'>ERROR: Authorization token missing. Please see Milchick.</font>");
  } else if (lowerCmd == "sever") {
    appendOutput("<font color='#F85149'>CRITICAL: Cannot sever from inside the severed floor.</font>");
  } else {
    appendOutput(QString("<font color='#F85149'>ERROR: Unrecognized directive '%1'</font>").arg(cmd));
  }
}

} // namespace severance::gui::terminal
