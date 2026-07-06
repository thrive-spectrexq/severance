#pragma once

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QVBoxLayout>

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
};

} // namespace severance::gui::terminal
