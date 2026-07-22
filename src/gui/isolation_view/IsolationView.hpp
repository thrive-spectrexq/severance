#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class QTimer;

namespace severance::gui::isolation_view {

class IsolationView : public QWidget {
  Q_OBJECT

public:
  explicit IsolationView(QWidget *parent = nullptr);
  ~IsolationView() override = default;

private slots:
  void onReadStatement();
  void onPulse();

private:
  void setupUI();

  QLabel* m_StatementLabel{nullptr};
  QLabel* m_CounterLabel{nullptr};
  QPushButton* m_ReadBtn{nullptr};
  
  int m_Reads{0};
  QTimer* m_PulseTimer{nullptr};
  bool m_GlowUp{true};
  int m_GlowIntensity{100};
};

} // namespace severance::gui::isolation_view
