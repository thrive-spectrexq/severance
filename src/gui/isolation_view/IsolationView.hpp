#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QProgressBar>
#include <QElapsedTimer>

class QTimer;

namespace severance::gui::isolation_view {

class OscilloscopeWidget;

class IsolationView : public QWidget {
  Q_OBJECT

public:
  explicit IsolationView(QWidget *parent = nullptr);
  ~IsolationView() override = default;

private slots:
  void onReadStatement();
  void onPulse();
  void onTextChanged(const QString& text);

private:
  void setupUI();
  void showWarning();
  void hideWarning();

  QLabel* m_StatementLabel{nullptr};
  QLabel* m_CounterLabel{nullptr};
  QPushButton* m_ReadBtn{nullptr};
  
  QLineEdit* m_InputField{nullptr};
  QProgressBar* m_SincerityMeter{nullptr};
  QLabel* m_WarningLabel{nullptr};
  OscilloscopeWidget* m_Oscilloscope{nullptr};
  
  int m_Reads{0};
  QTimer* m_PulseTimer{nullptr};
  bool m_GlowUp{true};
  int m_GlowIntensity{100};
  
  QElapsedTimer m_LastKeyTimer;
  int m_SincerityScore{100};
};

} // namespace severance::gui::isolation_view
