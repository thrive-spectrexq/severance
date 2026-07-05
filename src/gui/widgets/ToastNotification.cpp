#include "ToastNotification.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPainterPath>

namespace severance::gui::widgets {

ToastNotification::ToastNotification(const QString& title, const QString& message, uint32_t pid, QWidget* parent)
    : QWidget(parent), m_Pid(pid) {
  // Use Tool flag so it floats above its parent but doesn't create a separate taskbar entry
  setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_DeleteOnClose);
  setFixedSize(350, 140);

  setupUI(title, message);
}

ToastNotification::~ToastNotification() = default;

void ToastNotification::setupUI(const QString& title, const QString& message) {
  auto* mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(15, 15, 15, 15);
  mainLayout->setSpacing(8);

  auto* titleLabel = new QLabel(title, this);
  titleLabel->setStyleSheet("color: #C0392B; font-size: 14px; font-weight: bold; text-transform: uppercase; letter-spacing: 1px;");
  mainLayout->addWidget(titleLabel);

  auto* msgLabel = new QLabel(message, this);
  msgLabel->setStyleSheet("color: #E8ECEF; font-size: 12px;");
  msgLabel->setWordWrap(true);
  mainLayout->addWidget(msgLabel);

  auto* btnLayout = new QHBoxLayout();
  btnLayout->addStretch();

  auto* resumeBtn = new QPushButton("Resume", this);
  resumeBtn->setStyleSheet(R"(
    QPushButton { background-color: #0F1A1F; color: #E8ECEF; border: 1px solid #1C2E38; border-radius: 3px; padding: 4px 12px; text-transform: uppercase; letter-spacing: 0.5px; font-size: 11px; }
    QPushButton:hover { background-color: rgba(26, 122, 92, 0.25); border-color: #1A7A5C; color: #7FDBCA; }
  )");
  connect(resumeBtn, &QPushButton::clicked, this, [this]() {
    emit actionChosen(m_Pid, false);
    emit closed();
    close();
  });
  btnLayout->addWidget(resumeBtn);

  auto* killBtn = new QPushButton("Sever", this);
  killBtn->setStyleSheet(R"(
    QPushButton { background-color: #C0392B; color: #E8ECEF; border: none; border-radius: 3px; padding: 4px 12px; font-weight: bold; text-transform: uppercase; letter-spacing: 0.5px; font-size: 11px; }
    QPushButton:hover { background-color: #E74C3C; }
  )");
  connect(killBtn, &QPushButton::clicked, this, [this]() {
    emit actionChosen(m_Pid, true);
    emit closed();
    close();
  });
  btnLayout->addWidget(killBtn);

  mainLayout->addLayout(btnLayout);
}

void ToastNotification::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QPainterPath path;
  path.addRoundedRect(rect(), 8, 8);

  painter.fillPath(path, QColor(15, 26, 31, 240)); // Lumon dark teal semi-transparent
  painter.setPen(QPen(QColor(28, 46, 56), 1));       // Lumon border
  painter.drawPath(path);
}

void ToastNotification::showWithAnimation() {
  if (!parentWidget()) {
      show();
      return;
  }

  // Calculate position relative to parent window globally
  QPoint parentPos = parentWidget()->mapToGlobal(QPoint(0,0));
  int startX = parentPos.x() + parentWidget()->width() - width() - 20;
  int startY = parentPos.y() + parentWidget()->height();
  int endY = parentPos.y() + parentWidget()->height() - height() - 20;

  setGeometry(startX, startY, width(), height());
  show();

  m_Animation = new QPropertyAnimation(this, "pos", this);
  m_Animation->setDuration(300);
  m_Animation->setStartValue(QPoint(startX, startY));
  m_Animation->setEndValue(QPoint(startX, endY));
  m_Animation->setEasingCurve(QEasingCurve::OutCubic);
  m_Animation->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace severance::gui::widgets
