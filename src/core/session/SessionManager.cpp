#include "SessionManager.hpp"
#include "logging/Logger.hpp"
#include "utils/UUID.hpp"
#include "events/EventBus.hpp"
#include "events/EventTypes.hpp"
#include "events/Event.hpp"
#include <chrono>
#include <fstream>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace severance::core::session {

static std::string FormatTimestamp(uint64_t ms) {
    std::time_t t = ms / 1000;
    std::tm* tm = std::localtime(&t);
    std::ostringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

SessionManager::SessionManager() {
  auto cb = [this](std::shared_ptr<events::Event> e) { this->OnEventReceived(e); };
  auto& bus = events::EventBus::GetInstance();
  bus.Subscribe(events::EventType::ProcessCreated, cb);
  bus.Subscribe(events::EventType::ProcessTerminated, cb);
  bus.Subscribe(events::EventType::NetworkConnectionOpened, cb);
  bus.Subscribe(events::EventType::FileCreated, cb);
  bus.Subscribe(events::EventType::FileModified, cb);
  bus.Subscribe(events::EventType::FileDeleted, cb);
}

SessionManager::~SessionManager() = default;

void SessionManager::OnEventReceived(std::shared_ptr<events::Event> event) {
  if (m_IsRecording) {
    m_RecordedEvents.push_back(event);
    m_CurrentSession.eventCount++;
  }
}

void SessionManager::AddAnnotation(const std::string& note) {
  if (!m_IsRecording) return;
  SessionAnnotation ann;
  ann.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
  ann.note = note;
  m_Annotations.push_back(ann);
  SEV_CORE_INFO("Added session annotation: {}", note);
}

void SessionManager::StartRecording(const std::string& sessionName) {
  if (m_IsRecording) return;
  
  m_IsRecording = true;
  m_CurrentSession.sessionId = utils::UUID::Generate();
  m_CurrentSession.name = sessionName.empty() ? "Unnamed Session" : sessionName;
  m_CurrentSession.startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
  m_CurrentSession.eventCount = 0;
  m_RecordedEvents.clear();
  m_Annotations.clear();
  
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

std::string SessionManager::ExportSession(const std::string& exportPath) {
  SEV_CORE_INFO("Exporting JSON session to: {}", exportPath);
  
  if (!std::filesystem::exists(exportPath)) {
      std::filesystem::create_directories(exportPath);
  }

  std::string fullPath = exportPath + "/session_" + m_CurrentSession.sessionId + ".json";
  std::ofstream file(fullPath);
  if (file.is_open()) {
      file << "{\n";
      file << "  \"sessionId\": \"" << m_CurrentSession.sessionId << "\",\n";
      file << "  \"name\": \"" << m_CurrentSession.name << "\",\n";
      file << "  \"startTime\": " << m_CurrentSession.startTime << ",\n";
      file << "  \"endTime\": " << m_CurrentSession.endTime << ",\n";
      file << "  \"eventCount\": " << m_CurrentSession.eventCount << ",\n";
      file << "  \"annotations\": [\n";
      for (size_t i = 0; i < m_Annotations.size(); ++i) {
          file << "    { \"timestamp\": " << m_Annotations[i].timestamp << ", \"note\": \"" << m_Annotations[i].note << "\" }";
          if (i < m_Annotations.size() - 1) file << ",";
          file << "\n";
      }
      file << "  ],\n";
      file << "  \"events\": [\n";
      for (size_t i = 0; i < m_RecordedEvents.size(); ++i) {
          file << "    { \"type\": " << static_cast<int>(m_RecordedEvents[i]->GetType()) 
               << ", \"name\": \"" << m_RecordedEvents[i]->GetName() << "\" }";
          if (i < m_RecordedEvents.size() - 1) file << ",";
          file << "\n";
      }
      file << "  ]\n";
      file << "}\n";
      file.close();
      SEV_CORE_INFO("Session metadata saved to: {}", fullPath);
  } else {
      SEV_CORE_ERROR("Failed to write session JSON file.");
  }
  return fullPath;
}

std::string SessionManager::ExportMarkdown(const std::string& exportPath) {
  SEV_CORE_INFO("Exporting Markdown session to: {}", exportPath);
  
  if (!std::filesystem::exists(exportPath)) {
      std::filesystem::create_directories(exportPath);
  }

  std::string fullPath = exportPath + "/session_" + m_CurrentSession.sessionId + ".md";
  std::ofstream file(fullPath);
  if (file.is_open()) {
      file << "# Incident Session Report: " << m_CurrentSession.name << "\n\n";
      file << "**Session ID:** `" << m_CurrentSession.sessionId << "`  \n";
      file << "**Start Time:** `" << FormatTimestamp(m_CurrentSession.startTime) << "`  \n";
      file << "**End Time:** `" << FormatTimestamp(m_CurrentSession.endTime) << "`  \n";
      file << "**Total Events Recorded:** `" << m_CurrentSession.eventCount << "`  \n\n";

      file << "## Annotations\n\n";
      if (m_Annotations.empty()) {
          file << "*No annotations recorded during this session.*\n\n";
      } else {
          for (const auto& ann : m_Annotations) {
              file << "- **[" << FormatTimestamp(ann.timestamp) << "]** " << ann.note << "\n";
          }
          file << "\n";
      }

      file << "## Captured Telemetry\n\n";
      file << "| Timestamp | Event Type | Description |\n";
      file << "|-----------|------------|-------------|\n";
      
      if (m_RecordedEvents.empty()) {
          file << "| N/A | N/A | No events captured |\n";
      } else {
          for (const auto& ev : m_RecordedEvents) {
              // We don't have event-level timestamps in the generic Event base class, so we use session bounds ideally
              // For now, put N/A or approximate
              file << "| `Recorded` | `" << static_cast<int>(ev->GetType()) << "` | " << ev->GetName() << " |\n";
          }
      }
      
      file.close();
      SEV_CORE_INFO("Session Markdown saved to: {}", fullPath);
  } else {
      SEV_CORE_ERROR("Failed to write session Markdown file.");
  }
  return fullPath;
}

} // namespace severance::core::session
