#include "SessionManager.hpp"
#include "logging/Logger.hpp"
#include "utils/UUID.hpp"
#include <chrono>

namespace severance::core::session {

SessionManager::SessionManager() = default;
SessionManager::~SessionManager() = default;

void SessionManager::StartRecording(const std::string& sessionName) {
  if (m_IsRecording) return;
  
  m_IsRecording = true;
  m_CurrentSession.sessionId = utils::UUID::Generate();
  m_CurrentSession.name = sessionName.empty() ? "Unnamed Session" : sessionName;
  m_CurrentSession.startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
  m_CurrentSession.eventCount = 0;
  
  SEV_CORE_INFO("Started session recording: {}", m_CurrentSession.sessionId);
}

void SessionManager::StopRecording() {
  if (!m_IsRecording) return;
  
  m_IsRecording = false;
  m_CurrentSession.endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
    
  SEV_CORE_INFO("Stopped session recording: {}", m_CurrentSession.sessionId);
}

bool SessionManager::IsRecording() const {
  return m_IsRecording;
}

#include <fstream>
#include <filesystem>

std::string SessionManager::ExportSession(const std::string& exportPath) {
  SEV_CORE_INFO("Exporting session to: {}", exportPath);
  
  if (!std::filesystem::exists(exportPath)) {
      std::filesystem::create_directories(exportPath);
  }

  std::string fullPath = exportPath + "/session_" + m_CurrentSession.sessionId + ".json";
  std::ofstream file(fullPath);
  if (file.is_open()) {
      file << "{\n";
      file << "  \"sessionId\": \"" << m_CurrentSession.sessionId << "\",\n";
      file << "  \"name\": \"" << m_CurrentSession.name << "\",\n";
      file << "  \"description\": \"" << m_CurrentSession.description << "\",\n";
      file << "  \"startTime\": " << m_CurrentSession.startTime << ",\n";
      file << "  \"endTime\": " << m_CurrentSession.endTime << ",\n";
      file << "  \"eventCount\": " << m_CurrentSession.eventCount << "\n";
      file << "}\n";
      file.close();
      SEV_CORE_INFO("Session metadata saved to: {}", fullPath);
  } else {
      SEV_CORE_ERROR("Failed to write session file.");
  }
  return fullPath;
}

} // namespace severance::core::session
