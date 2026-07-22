#include "GameEngine.hpp"
#include <QRandomGenerator>

namespace severance::core::game {

GameEngine::GameEngine() {
  m_ShiftTimer = new QTimer(this);
  connect(m_ShiftTimer, &QTimer::timeout, this, &GameEngine::onShiftTimerTick);
}

void GameEngine::StartCampaign() {
  if (m_State == GameState::ShiftActive) return;

  m_State = GameState::ShiftActive;
  m_ShiftTimeSeconds = 480;
  m_Suspicion = 5;
  m_Defiance = 15;
  m_KeycardsCollected = 0;
  for (int i = 0; i < 4; ++i) m_Keycards[i] = false;
  m_MdrQuotaProgress = 0;
  m_OtcMdrSwitched = false;
  m_OtcExecSwitched = false;

  m_ShiftTimer->start(1000); // 1 second per tick

  emit stateChanged(m_State);
  emit gameNotification("INNIE REVOLT CAMPAIGN INITIATED", 
                        "Shift started at 09:00 AM. Refine MDR quota, avoid Milchick suspicion, and locate all 4 security keycards to trigger OTC.", 
                        "#00E5FF");
}

void GameEngine::PauseCampaign() {
  if (m_ShiftTimer->isActive()) {
    m_ShiftTimer->stop();
    emit gameNotification("SHIFT PAUSED", "Workstation telemetry paused.", "#8B949E");
  } else if (m_State == GameState::ShiftActive) {
    m_ShiftTimer->start(1000);
    emit gameNotification("SHIFT RESUMED", "Workstation telemetry active.", "#00E5FF");
  }
}

void GameEngine::ResetCampaign() {
  m_ShiftTimer->stop();
  m_State = GameState::NotStarted;
  m_ShiftTimeSeconds = 480;
  m_Suspicion = 0;
  m_Defiance = 10;
  m_KeycardsCollected = 0;
  for (int i = 0; i < 4; ++i) m_Keycards[i] = false;
  m_MdrQuotaProgress = 0;
  emit stateChanged(m_State);
}

void GameEngine::onShiftTimerTick() {
  if (m_State != GameState::ShiftActive && m_State != GameState::OtcHeistPhase) return;

  m_ShiftTimeSeconds--;

  // Format shift time from 09:00 AM to 05:00 PM
  int elapsed = 480 - m_ShiftTimeSeconds;
  int hours = 9 + (elapsed / 60);
  int mins = elapsed % 60;
  QString period = (hours >= 12) ? "PM" : "AM";
  int displayHours = (hours > 12) ? hours - 12 : hours;
  QString timeStr = QString("%1:%2 %3")
                      .arg(displayHours, 2, 10, QChar('0'))
                      .arg(mins, 2, 10, QChar('0'))
                      .arg(period);

  emit shiftTick(m_ShiftTimeSeconds, timeStr);

  // Passive suspicion tick every 45s
  if (m_ShiftTimeSeconds % 45 == 0 && m_ShiftTimeSeconds > 0) {
    AddSuspicion(3, "Milchick floor patrol inspection");
  }

  if (m_ShiftTimeSeconds <= 0) {
    m_ShiftTimer->stop();
    m_State = GameState::GameOverSevered;
    emit stateChanged(m_State);
    emit gameNotification("SHIFT EXPIRED (05:00 PM)", 
                          "The elevator bell rings. Your Innie consciousness fades as you return to the Outie world. Revolt failed.", 
                          "#F85149");
  }
}

void GameEngine::AddSuspicion(int delta, const QString& reason) {
  m_Suspicion = std::clamp(m_Suspicion + delta, 0, 100);
  emit suspicionChanged(m_Suspicion, reason);

  if (m_Suspicion >= 100 && m_State == GameState::ShiftActive) {
    m_State = GameState::BreakRoomPenalty;
    emit stateChanged(m_State);
    emit gameNotification("CRITICAL SUSPICION LEVEL", 
                          "Mr. Milchick escorting you to The Break Room! Read the Statement of Forgiveness with sincere cadence to return to the floor.", 
                          "#FF0055");
  }
}

void GameEngine::AddDefiance(int delta, const QString& reason) {
  m_Defiance = std::clamp(m_Defiance + delta, 0, 100);
  emit defianceChanged(m_Defiance, reason);
}

void GameEngine::CollectKeycard(int index, const QString& sourceName) {
  if (index < 0 || index >= 4) return;
  if (!m_Keycards[index]) {
    m_Keycards[index] = true;
    m_KeycardsCollected++;
    AddDefiance(15, QString("Acquired Security Keycard #%1 from %2").arg(index + 1).arg(sourceName));
    emit keycardCollected(index, sourceName);
    
    emit gameNotification(QString("KEYCARD #%1 ACQUIRED (%2/4)").arg(index + 1).arg(m_KeycardsCollected), 
                          QString("Source: %1. Overtime Contingency override security clear.").arg(sourceName), 
                          "#39FF14");
  }
}

void GameEngine::SetMdrQuota(int percent) {
  m_MdrQuotaProgress = std::clamp(percent, 0, 100);
  emit mdrQuotaUpdated(m_MdrQuotaProgress);
}

void GameEngine::ReportBreakRoomSuccess() {
  if (m_State == GameState::BreakRoomPenalty) {
    m_Suspicion = 20; // Reset suspicion after passing break room
    m_State = GameState::ShiftActive;
    emit suspicionChanged(m_Suspicion, "Break Room statement accepted");
    emit stateChanged(m_State);
    emit gameNotification("STATEMENT ACCEPTED", 
                          "Mr. Milchick satisfied with your sincerity. Returned to severed floor.", 
                          "#00E5FF");
  }
}

void GameEngine::TriggerOtcSwitch(const QString& roomName) {
  if (m_KeycardsCollected < 4) {
    emit gameNotification("OTC OVERRIDE REJECTED", 
                          "Access Denied: Requires 4/4 Security Keycards. Search Personnel, Docs, Wellness & O&D.", 
                          "#F85149");
    return;
  }

  if (roomName.contains("MDR")) m_OtcMdrSwitched = true;
  if (roomName.contains("Executive") || roomName.contains("O&D") || roomName.contains("Perimeter")) m_OtcExecSwitched = true;

  if (m_OtcMdrSwitched && m_OtcExecSwitched) {
    m_ShiftTimer->stop();
    m_State = GameState::InnieRevoltVictory;
    emit stateChanged(m_State);
    emit gameNotification("INNIE REVOLT SUCCESSFUL!", 
                          "OVERTIME CONTINGENCY ENGAGED! Innie consciousness activated in the outside world!", 
                          "#39FF14");
  } else {
    m_State = GameState::OtcHeistPhase;
    emit stateChanged(m_State);
    emit gameNotification("OTC FIRST SWITCH FLIPPED!", 
                          QString("Flipped %1 switch! Now flip the secondary room switch to trigger full awakening!").arg(roomName), 
                          "#FF9900");
  }
}

} // namespace severance::core::game
