#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>

namespace severance::gui::ai_panel {

class AiPanel : public QWidget {
  Q_OBJECT

public:
  explicit AiPanel(QWidget* parent = nullptr);
  ~AiPanel() override;

  // Callable by other views to trigger analysis
  void startAnalysis(int pid, const QString& processName, const QString& context);

private slots:
  void onSubmit();
  void onChunkReceived(const QString& chunk);
  void onResponseFinished();
  void onError(const QString& errorMsg);

private:
  void setupUI();
  void appendUserMessage(const QString& msg);
  void appendAiMessage(const QString& msg, bool isNewBlock = false);

  QTextEdit* m_ChatHistory{nullptr};
  QLineEdit* m_InputBox{nullptr};
  QPushButton* m_SubmitBtn{nullptr};
  QPushButton* m_ClearBtn{nullptr};

  bool m_IsGenerating{false};
};

} // namespace severance::gui::ai_panel
