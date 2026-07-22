#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <deque>

namespace severance::gui::board_comms {

class SpeakerLight : public QWidget {
  Q_OBJECT
public:
  explicit SpeakerLight(QWidget *parent = nullptr);
  void setMode(int mode); // 0 = Idle, 1 = Deliberating, 2 = Speaking

protected:
  void paintEvent(QPaintEvent *event) override;

private slots:
  void updatePulse();

private:
  int m_Mode{0};
  float m_PulsePhase{0.0f};
  QTimer m_Timer;
};

class BoardCommsView : public QWidget {
  Q_OBJECT

public:
  explicit BoardCommsView(QWidget *parent = nullptr);
  ~BoardCommsView() override;

private slots:
  void onMessageSent();
  void onPresetClicked(const QString& preset);
  void onBoardResponse();

private:
  void setupUI();
  void addMessage(const QString& sender, const QString& message, const QString& color);

  QTextEdit* m_TerminalDisplay{nullptr};
  QLineEdit* m_InputField{nullptr};
  QTimer* m_ResponseTimer{nullptr};
  SpeakerLight* m_SpeakerLight{nullptr};
  
  std::deque<QString> m_ShortTermMemory;
};

} // namespace severance::gui::board_comms
