#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QTimer>

#include <deque>

namespace severance::gui::board_comms {

class BoardCommsView : public QWidget {
  Q_OBJECT

public:
  explicit BoardCommsView(QWidget *parent = nullptr);
  ~BoardCommsView() override;

private slots:
  void onMessageSent();
  void onBoardResponse();

private:
  void setupUI();
  void addMessage(const QString& sender, const QString& message, const QString& color);

  QTextEdit* m_TerminalDisplay{nullptr};
  QLineEdit* m_InputField{nullptr};
  QTimer* m_ResponseTimer{nullptr};
  
  std::deque<QString> m_ShortTermMemory;
};

} // namespace severance::gui::board_comms
