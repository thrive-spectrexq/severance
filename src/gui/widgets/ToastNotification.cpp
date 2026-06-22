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
  titleLabel->setStyleSheet("color: #F85149; font-size: 14px; font-weight: bold;");
  mainLayout->addWidget(titleLabel);

  auto* msgLabel = new QLabel(message, this);
  msgLabel->setStyleSheet("color: #E6EDF3; font-size: 12px;");
  msgLabel->setWordWrap(true);
  mainLayout->addWidget(msgLabel);

  auto* btnLayout = new QHBoxLayout();
  btnLayout->addStretch();

  auto* resumeBtn = new QPushButton("Resume", this);
  resumeBtn->setStyleSheet(R"(
    QPushButton { background-color: #21262D; color: #E6EDF3; border: 1px solid #30363D; border-radius: 4px; padding: 4px 12px; }
    QPushButton:hover { background-color: #30363D; }
  )");
  connect(resumeBtn, &QPushButton::clicked, this, [this]() {
    emit actionChosen(m_Pid, false);
    emit closed();
    close();
  });
  btnLayout->addWidget(resumeBtn);

  auto* killBtn = new QPushButton("Terminate", this);
  killBtn->setStyleSheet(R"(
    QPushButton { background-color: #DA3633; color: white; border: none; border-radius: 4px; padding: 4px 12px; font-weight: bold; }
    QPushButton:hover { background-color: #F85149; }
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

  painter.fillPath(path, QColor(22, 27, 34, 240)); // Dark semi-transparent background
  painter.setPen(QPen(QColor(48, 54, 61), 1));
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
