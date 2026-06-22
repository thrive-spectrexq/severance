#pragma once

#include <QWidget>
#include <QString>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>

namespace severance::gui::widgets {

class ToastNotification : public QWidget {
  Q_OBJECT

public:
  explicit ToastNotification(const QString& title, const QString& message, uint32_t pid, QWidget* parent = nullptr);
  ~ToastNotification() override;

  void showWithAnimation();

signals:
  void actionChosen(uint32_t pid, bool kill);
  void closed();

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  void setupUI(const QString& title, const QString& message);

  uint32_t m_Pid;
  QTimer* m_AutoCloseTimer{nullptr};
  QPropertyAnimation* m_Animation{nullptr};
};

} // namespace severance::gui::widgets
