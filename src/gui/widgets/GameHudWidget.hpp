#pragma once

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QHBoxLayout>
#include "core/game/GameEngine.hpp"

namespace severance::gui::widgets {

class GameHudWidget : public QWidget {
  Q_OBJECT

public:
  explicit GameHudWidget(QWidget *parent = nullptr);
  ~GameHudWidget() override = default;

public slots:
  void updateHud();

private slots:
  void onStartClicked();
  void onPauseClicked();
  void onResetClicked();

private:
  void setupUi();

  QLabel* m_TimeLabel{nullptr};
  QProgressBar* m_SuspicionBar{nullptr};
  QLabel* m_SuspicionText{nullptr};
  QProgressBar* m_DefianceBar{nullptr};
  QLabel* m_KeycardsLabel{nullptr};
  QLabel* m_QuotaLabel{nullptr};

  QPushButton* m_StartBtn{nullptr};
  QPushButton* m_PauseBtn{nullptr};
  QPushButton* m_ResetBtn{nullptr};
};

} // namespace severance::gui::widgets
