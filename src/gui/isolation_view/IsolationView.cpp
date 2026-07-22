#include "IsolationView.hpp"
#include <QVBoxLayout>
#include <QTimer>
#include <QString>

namespace severance::gui::isolation_view {

IsolationView::IsolationView(QWidget *parent) : QWidget(parent) {
  setupUI();
}

void IsolationView::setupUI() {
  this->setStyleSheet("background-color: #F0F0F0;");
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(50, 50, 50, 50);
  layout->setAlignment(Qt::AlignCenter);

  m_StatementLabel = new QLabel(
    "\"Forgive me for the damage I have done this world.\n"
    "Neither combative nor competitive,\n"
    "I will not be the reason the world decays.\n"
    "My life will have been of no consequence.\n"
    "I will do no harm.\"", this);
  
  m_StatementLabel->setAlignment(Qt::AlignCenter);
  m_StatementLabel->setStyleSheet("font-family: 'Courier New', Courier, monospace; font-size: 24px; font-weight: bold; color: #1A1A1A;");
  layout->addWidget(m_StatementLabel);
  
  layout->addSpacing(40);

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

  m_PulseTimer = new QTimer(this);
  connect(m_PulseTimer, &QTimer::timeout, this, &IsolationView::onPulse);
  m_PulseTimer->start(50);
}

void IsolationView::onReadStatement() {
  m_Reads++;
  m_CounterLabel->setText(QString("READINGS COMPLETED: %1 / 1,000").arg(m_Reads));
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
