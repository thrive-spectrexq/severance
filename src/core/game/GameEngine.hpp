#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QDateTime>

namespace severance::core::game {

enum class GameState {
  NotStarted,
  ShiftActive,
  BreakRoomPenalty,
  SecurityLockdown,
  OtcHeistPhase,
  InnieRevoltVictory,
  GameOverSevered
};

class GameEngine : public QObject {
  Q_OBJECT

public:
  static GameEngine& GetInstance() {
    static GameEngine instance;
    return instance;
  }

  void StartCampaign();
  void PauseCampaign();
  void ResetCampaign();

  GameState GetState() const { return m_State; }
  int GetShiftTimeSeconds() const { return m_ShiftTimeSeconds; }
  int GetSuspicion() const { return m_Suspicion; }
  int GetDefiance() const { return m_Defiance; }
  int GetKeycardCount() const { return m_KeycardsCollected; }
  bool HasKeycard(int index) const { return index >= 0 && index < 4 && m_Keycards[index]; }
  int GetMdrQuota() const { return m_MdrQuotaProgress; }

  void AddSuspicion(int delta, const QString& reason);
  void AddDefiance(int delta, const QString& reason);
  void CollectKeycard(int index, const QString& sourceName);
  void SetMdrQuota(int percent);
  void ReportBreakRoomSuccess();
  void TriggerOtcSwitch(const QString& roomName);

signals:
  void stateChanged(GameState newState);
  void suspicionChanged(int level, const QString& reason);
  void defianceChanged(int level, const QString& reason);
  void keycardCollected(int index, const QString& sourceName);
  void shiftTick(int remainingSeconds, const QString& formattedTime);
  void gameNotification(const QString& title, const QString& message, const QString& severity);
  void mdrQuotaUpdated(int percent);

private slots:
  void onShiftTimerTick();

private:
  GameEngine();
  ~GameEngine() override = default;

  GameState m_State{GameState::NotStarted};
  QTimer* m_ShiftTimer{nullptr};
  int m_ShiftTimeSeconds{480}; // 8 minutes total shift (09:00 AM -> 05:00 PM)
  int m_Suspicion{0};           // 0 to 100
  int m_Defiance{10};          // 0 to 100
  int m_KeycardsCollected{0};
  bool m_Keycards[4]{false, false, false, false}; // 0: Personnel, 1: Docs, 2: Wellness, 3: O&D
  int m_MdrQuotaProgress{0};
  
  bool m_OtcMdrSwitched{false};
  bool m_OtcExecSwitched{false};
};

} // namespace severance::core::game
