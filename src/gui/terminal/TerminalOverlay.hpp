#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>

namespace severance::gui::terminal {

class TerminalOverlay : public QWidget {
  Q_OBJECT

public:
  explicit TerminalOverlay(QWidget *parent = nullptr);
  ~TerminalOverlay() override;

  void toggleVisibility();

protected:
  void resizeEvent(QResizeEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
  void onReturnPressed();
  void triggerGlitch();
  void purgeAnomaly();
  void applyGlitchEffect();

private:
  void setupUi();
  void appendOutput(const QString& text);
  void executeCommand(const QString& cmd);
  void updateGeometry();

  QTextEdit* m_OutputArea{nullptr};
  QLineEdit* m_InputLine{nullptr};
  QPropertyAnimation* m_SlideAnimation{nullptr};
  bool m_IsVisible{false};
  int m_TargetHeight{400};
  
  QPushButton* m_PurgeButton{nullptr};
  QTimer* m_GlitchTimer{nullptr};
  QTimer* m_RandomGlitchTrigger{nullptr};
  bool m_IsGlitching{false};
};

} // namespace severance::gui::terminal
