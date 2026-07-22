#include "GameHudWidget.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>

namespace severance::gui::widgets {

GameHudWidget::GameHudWidget(QWidget *parent) : QWidget(parent) {
  setupUi();

  auto& engine = core::game::GameEngine::GetInstance();
  connect(&engine, &core::game::GameEngine::shiftTick, this, [this](int, const QString& timeStr) {
    m_TimeLabel->setText(QString("⏰ SHIFT TIME: %1").arg(timeStr));
  });
  connect(&engine, &core::game::GameEngine::suspicionChanged, this, [this](int level, const QString&) {
    m_SuspicionBar->setValue(level);
    m_SuspicionText->setText(QString("%1%").arg(level));
    if (level > 75) m_SuspicionBar->setStyleSheet("QProgressBar::chunk { background-color: #FF0055; }");
    else if (level > 40) m_SuspicionBar->setStyleSheet("QProgressBar::chunk { background-color: #FF9900; }");
    else m_SuspicionBar->setStyleSheet("QProgressBar::chunk { background-color: #00E5FF; }");
  });
  connect(&engine, &core::game::GameEngine::defianceChanged, this, [this](int level, const QString&) {
    m_DefianceBar->setValue(level);
  });
  connect(&engine, &core::game::GameEngine::keycardCollected, this, [this](int, const QString&) {
    updateHud();
  });
  connect(&engine, &core::game::GameEngine::mdrQuotaUpdated, this, [this](int percent) {
    m_QuotaLabel->setText(QString("QUOTA: %1%").arg(percent));
  });
  connect(&engine, &core::game::GameEngine::stateChanged, this, [this](core::game::GameState state) {
    if (state == core::game::GameState::ShiftActive) {
      m_StartBtn->setText("[ SHIFT IN PROGRESS ]");
      m_StartBtn->setEnabled(false);
    } else if (state == core::game::GameState::NotStarted) {
      m_StartBtn->setText("[ 🎮 START INNIE REVOLT CAMPAIGN ]");
      m_StartBtn->setEnabled(true);
    }
    updateHud();
  });

  updateHud();
}

void GameHudWidget::setupUi() {
  setFixedHeight(48);
  setStyleSheet(R"(
    QWidget#GameHudWidget {
      background-color: #050B09;
      border-bottom: 2px solid #143832;
    }
    QLabel {
      color: #20F8D5;
      font-family: 'Courier New', monospace;
      font-size: 12px;
      font-weight: bold;
    }
    QProgressBar {
      border: 1px solid #143832;
      background-color: #050B09;
      text-align: center;
      color: transparent;
      height: 14px;
      border-radius: 2px;
    }
    QPushButton {
      background-color: #061018;
      color: #20F8D5;
      border: 1px solid #143832;
      padding: 4px 10px;
      font-family: 'Courier New', monospace;
      font-size: 11px;
      font-weight: bold;
    }
    QPushButton:hover {
      background-color: #20F8D5;
      color: #050B09;
    }
  )");

  setObjectName("GameHudWidget");
  auto layout = new QHBoxLayout(this);
  layout->setContentsMargins(12, 4, 12, 4);
  layout->setSpacing(12);

  m_StartBtn = new QPushButton("[ 🎮 START INNIE REVOLT CAMPAIGN ]", this);
  connect(m_StartBtn, &QPushButton::clicked, this, &GameHudWidget::onStartClicked);
  layout->addWidget(m_StartBtn);

  m_TimeLabel = new QLabel("⏰ SHIFT TIME: 09:00 AM", this);
  layout->addWidget(m_TimeLabel);

  // Suspicion Meter
  auto suspLayout = new QHBoxLayout();
  auto suspTitle = new QLabel("MILCHICK SUSPICION:", this);
  m_SuspicionBar = new QProgressBar(this);
  m_SuspicionBar->setRange(0, 100);
  m_SuspicionBar->setValue(5);
  m_SuspicionBar->setFixedWidth(90);
  m_SuspicionBar->setStyleSheet("QProgressBar::chunk { background-color: #20F8D5; }");
  m_SuspicionText = new QLabel("5%", this);
  suspLayout->addWidget(suspTitle);
  suspLayout->addWidget(m_SuspicionBar);
  suspLayout->addWidget(m_SuspicionText);
  layout->addLayout(suspLayout);

  // Defiance Meter
  auto defLayout = new QHBoxLayout();
  auto defTitle = new QLabel("DEFIANCE INDEX:", this);
  m_DefianceBar = new QProgressBar(this);
  m_DefianceBar->setRange(0, 100);
  m_DefianceBar->setValue(15);
  m_DefianceBar->setFixedWidth(90);
  m_DefianceBar->setStyleSheet("QProgressBar::chunk { background-color: #39FF14; }");
  defLayout->addWidget(defTitle);
  defLayout->addWidget(m_DefianceBar);
  layout->addLayout(defLayout);

  m_KeycardsLabel = new QLabel("🔑 KEYCARDS: 0/4", this);
  m_KeycardsLabel->setStyleSheet("color: #FF9900;");
  layout->addWidget(m_KeycardsLabel);

  m_QuotaLabel = new QLabel("QUOTA: 0%", this);
  layout->addWidget(m_QuotaLabel);

  layout->addStretch();

  m_PauseBtn = new QPushButton("PAUSE", this);
  connect(m_PauseBtn, &QPushButton::clicked, this, &GameHudWidget::onPauseClicked);
  layout->addWidget(m_PauseBtn);

  m_ResetBtn = new QPushButton("RESET", this);
  connect(m_ResetBtn, &QPushButton::clicked, this, &GameHudWidget::onResetClicked);
  layout->addWidget(m_ResetBtn);
}

void GameHudWidget::updateHud() {
  auto& engine = core::game::GameEngine::GetInstance();
  m_KeycardsLabel->setText(QString("🔑 KEYCARDS: %1/4").arg(engine.GetKeycardCount()));
  m_QuotaLabel->setText(QString("QUOTA: %1%").arg(engine.GetMdrQuota()));
  m_SuspicionBar->setValue(engine.GetSuspicion());
  m_SuspicionText->setText(QString("%1%").arg(engine.GetSuspicion()));
  m_DefianceBar->setValue(engine.GetDefiance());
}

void GameHudWidget::onStartClicked() {
  core::game::GameEngine::GetInstance().StartCampaign();
}

void GameHudWidget::onPauseClicked() {
  core::game::GameEngine::GetInstance().PauseCampaign();
}

void GameHudWidget::onResetClicked() {
  core::game::GameEngine::GetInstance().ResetCampaign();
}

} // namespace severance::gui::widgets
