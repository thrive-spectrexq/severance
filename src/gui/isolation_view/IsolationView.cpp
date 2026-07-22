#include "IsolationView.hpp"
#include <QVBoxLayout>
#include <QTimer>
#include <QString>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>

namespace severance::gui::isolation_view {

class OscilloscopeWidget : public QWidget {
public:
    explicit OscilloscopeWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setFixedHeight(100);
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, [this]() {
            m_phase += 0.2f;
            m_amplitude *= 0.9f;
            if (m_amplitude < 5.0f) m_amplitude = 5.0f;
            update();
        });
        m_timer->start(30);
    }
    
    void bump() {
        m_amplitude = 50.0f;
    }
    
protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor("#0A0A0A"));
        
        QPen pen(QColor("#00FF41"), 2);
        painter.setPen(pen);
        
        QPainterPath path;
        path.moveTo(0, height() / 2);
        
        for (int x = 0; x < width(); x += 2) {
            float distToCenter = 1.0f - std::abs(x - width() / 2.0f) / (width() / 2.0f);
            float actualAmp = m_amplitude * distToCenter;
            float y_val = height() / 2.0f + std::sin(x * 0.05f + m_phase) * actualAmp;
            if (m_amplitude > 20.0f) {
                y_val += (QRandomGenerator::global()->generateDouble() - 0.5f) * actualAmp * 0.5f;
            }
            path.lineTo(x, y_val);
        }
        painter.drawPath(path);
    }
private:
    QTimer* m_timer;
    float m_phase{0.0f};
    float m_amplitude{5.0f};
};


IsolationView::IsolationView(QWidget *parent) : QWidget(parent) {
  setupUI();
}

void IsolationView::setupUI() {
  this->setStyleSheet("background-color: #F0F0F0;");
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(50, 50, 50, 50);
  layout->setAlignment(Qt::AlignCenter);

  m_WarningLabel = new QLabel("You did not mean it. Read it again with sincerity.", this);
  m_WarningLabel->setAlignment(Qt::AlignCenter);
  m_WarningLabel->setStyleSheet("background-color: #D32F2F; color: white; font-family: 'Courier New', Courier, monospace; font-size: 18px; font-weight: bold; padding: 10px; border-radius: 4px;");
  m_WarningLabel->setVisible(false);
  layout->addWidget(m_WarningLabel);

  m_StatementLabel = new QLabel(
    "\"Forgive me for the damage I have done this world.\n"
    "Neither combative nor competitive,\n"
    "I will not be the reason the world decays.\n"
    "My life will have been of no consequence.\n"
    "I will do no harm.\"", this);
  
  m_StatementLabel->setAlignment(Qt::AlignCenter);
  m_StatementLabel->setStyleSheet("font-family: 'Courier New', Courier, monospace; font-size: 24px; font-weight: bold; color: #1A1A1A;");
  layout->addWidget(m_StatementLabel);
  
  layout->addSpacing(20);

  m_InputField = new QLineEdit(this);
  m_InputField->setPlaceholderText("Type the statement here...");
  m_InputField->setStyleSheet("font-family: 'Courier New', Courier, monospace; font-size: 18px; padding: 5px; border: 2px solid #1A1A1A;");
  connect(m_InputField, &QLineEdit::textChanged, this, &IsolationView::onTextChanged);
  layout->addWidget(m_InputField);

  m_SincerityMeter = new QProgressBar(this);
  m_SincerityMeter->setRange(0, 100);
  m_SincerityMeter->setValue(100);
  m_SincerityMeter->setFormat("SINCERITY METER: %p%");
  m_SincerityMeter->setAlignment(Qt::AlignCenter);
  m_SincerityMeter->setStyleSheet(
      "QProgressBar { border: 2px solid #1A1A1A; border-radius: 4px; background-color: #E0E0E0; text-align: center; font-family: 'Courier New'; font-weight: bold; color: black; }"
      "QProgressBar::chunk { background-color: #4CAF50; }"
  );
  layout->addWidget(m_SincerityMeter);

  layout->addSpacing(20);

  m_CounterLabel = new QLabel("READINGS COMPLETED: 0 / 1,000", this);
  m_CounterLabel->setAlignment(Qt::AlignCenter);
  m_CounterLabel->setStyleSheet("font-family: 'Courier New', Courier, monospace; font-size: 18px; color: #1A1A1A;");
  layout->addWidget(m_CounterLabel);

  auto* infoLabel = new QLabel("The statement must be read with sincerity.", this);
  infoLabel->setAlignment(Qt::AlignCenter);
  infoLabel->setStyleSheet("font-family: 'Courier New', Courier, monospace; font-size: 14px; color: #555555;");
  layout->addWidget(infoLabel);

  layout->addSpacing(20);

  m_ReadBtn = new QPushButton("READ STATEMENT", this);
  m_ReadBtn->setFixedSize(200, 40);
  m_ReadBtn->setStyleSheet("background-color: #1A1A1A; color: #F0F0F0; font-family: 'Courier New', Courier, monospace; font-weight: bold; border-radius: 4px;");
  layout->addWidget(m_ReadBtn, 0, Qt::AlignHCenter);

  connect(m_ReadBtn, &QPushButton::clicked, this, &IsolationView::onReadStatement);

  m_Oscilloscope = new OscilloscopeWidget(this);
  layout->addWidget(m_Oscilloscope);

  m_PulseTimer = new QTimer(this);
  connect(m_PulseTimer, &QTimer::timeout, this, &IsolationView::onPulse);
  m_PulseTimer->start(50);
  
  m_LastKeyTimer.start();
}

void IsolationView::onTextChanged(const QString& text) {
    Q_UNUSED(text);
    qint64 elapsed = m_LastKeyTimer.restart();
    m_Oscilloscope->bump();

    if (elapsed < 50) {
        // Typing too fast or pasting
        m_SincerityScore -= 15;
    } else if (elapsed > 1000) {
        // Paused too long
        m_SincerityScore -= 5;
    } else {
        // Steady
        m_SincerityScore += 2;
    }
    
    if (m_SincerityScore > 100) m_SincerityScore = 100;
    if (m_SincerityScore < 0) m_SincerityScore = 0;
    
    m_SincerityMeter->setValue(m_SincerityScore);
    
    if (m_SincerityScore < 40) {
        m_SincerityMeter->setStyleSheet(
            "QProgressBar { border: 2px solid #1A1A1A; border-radius: 4px; background-color: #E0E0E0; text-align: center; font-family: 'Courier New'; font-weight: bold; color: black; }"
            "QProgressBar::chunk { background-color: #F44336; }" // Red
        );
        showWarning();
    } else {
        m_SincerityMeter->setStyleSheet(
            "QProgressBar { border: 2px solid #1A1A1A; border-radius: 4px; background-color: #E0E0E0; text-align: center; font-family: 'Courier New'; font-weight: bold; color: black; }"
            "QProgressBar::chunk { background-color: #4CAF50; }" // Green
        );
        hideWarning();
    }
}

void IsolationView::showWarning() {
    if (!m_WarningLabel->isVisible()) {
        m_WarningLabel->setVisible(true);
        m_Reads = 0;
        m_CounterLabel->setText("READINGS COMPLETED: 0 / 1,000");
    }
}

void IsolationView::hideWarning() {
    if (m_WarningLabel->isVisible() && m_SincerityScore > 60) {
        m_WarningLabel->setVisible(false);
    }
}

void IsolationView::onReadStatement() {
  if (m_SincerityScore >= 40) {
      m_Reads++;
      m_CounterLabel->setText(QString("READINGS COMPLETED: %1 / 1,000").arg(m_Reads));
      m_InputField->clear();
      m_SincerityScore = 100;
      m_SincerityMeter->setValue(100);
  } else {
      showWarning();
  }
}

void IsolationView::onPulse() {
  if (m_GlowUp) {
    m_GlowIntensity += 2;
    if (m_GlowIntensity >= 200) m_GlowUp = false;
  } else {
    m_GlowIntensity -= 2;
    if (m_GlowIntensity <= 50) m_GlowUp = true;
  }
  
  int r = 26 + (0 - 26) * m_GlowIntensity / 200;
  int g = 26 + (229 - 26) * m_GlowIntensity / 200;
  int b = 26 + (255 - 26) * m_GlowIntensity / 200;
  
  QString color = QString("#%1%2%3")
                  .arg(r, 2, 16, QChar('0'))
                  .arg(g, 2, 16, QChar('0'))
                  .arg(b, 2, 16, QChar('0'));
  m_StatementLabel->setStyleSheet(QString("font-family: 'Courier New', Courier, monospace; font-size: 24px; font-weight: bold; color: %1;").arg(color));
}

} // namespace severance::gui::isolation_view
