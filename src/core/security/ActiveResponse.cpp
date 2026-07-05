#include "ActiveResponse.hpp"
#include "core/process/ProcessManager.hpp"
#include "core/notifications/NotificationManager.hpp"
#include "logging/Logger.hpp"

#include <sstream>

namespace severance::core::security {

void ActiveResponse::LoadDirectivesFromString(const std::string& rulesText) {
  std::lock_guard<std::mutex> lock(directivesMutex_);
  directives_.clear();

  std::istringstream stream(rulesText);
  std::string line;
  ContainmentDirective currentDirective;

  while (std::getline(stream, line)) {
    // Simple text parser
    if (line.empty() || line[0] == '#') continue;

    auto delimPos = line.find(':');
    if (delimPos != std::string::npos) {
      std::string key = line.substr(0, delimPos);
      std::string val = line.substr(delimPos + 1);
      
      // Trim spaces
      key.erase(0, key.find_first_not_of(" \t"));
      key.erase(key.find_last_not_of(" \t") + 1);
      val.erase(0, val.find_first_not_of(" \t"));
      val.erase(val.find_last_not_of(" \t") + 1);

      if (key == "RuleName") {
        if (!currentDirective.name.empty()) {
          directives_.push_back(currentDirective);
          currentDirective = ContainmentDirective{};
        }
        currentDirective.name = val;
      } else if (key == "Condition") {
        currentDirective.condition = val;
      } else if (key == "Action") {
        currentDirective.action = val;
      }
    }
  }

  if (!currentDirective.name.empty()) {
    directives_.push_back(currentDirective);
  }

  SEV_CORE_INFO("ActiveResponse loaded {} custom directives", directives_.size());
}

void ActiveResponse::EvaluateAndRespond(uint32_t pid, const std::string& condition, const std::string& context) {
  std::string chosenAction = "PromptUser"; // Default action
  std::string matchedRule = "Default";

  {
    std::lock_guard<std::mutex> lock(directivesMutex_);
    for (const auto& dir : directives_) {
      if (dir.condition == condition || dir.condition == "Any") {
        chosenAction = dir.action;
        matchedRule = dir.name;
        break;
      }
    }
  }

  SEV_CORE_WARN("[ETW_MARKER] Rule '{}' matched condition '{}' for PID {}. Action: {}", matchedRule, condition, pid, chosenAction);

  core::process::ProcessManager procMgr;

  if (chosenAction == "KillProcess") {
    SEV_CORE_CRITICAL("Action KillProcess executed for PID {} due to rule {}", pid, matchedRule);
    procMgr.KillProcess(pid);
  } else if (chosenAction == "SuspendProcess") {
    SEV_CORE_WARN("Action SuspendProcess executed for PID {} due to rule {}", pid, matchedRule);
    procMgr.SuspendProcess(pid);
  } else if (chosenAction == "LogOnly") {
    SEV_CORE_INFO("LogOnly action for PID {}. Context: {}", pid, context);
  } else {
    // Default / PromptUser behavior
    if (procMgr.SuspendProcess(pid)) {
      notifications::Notification n;
      n.id = "active_response_" + std::to_string(pid);
      n.severity = notifications::NotificationSeverity::Critical;
      n.title = "Suspicious Process Suspended";
      n.message = "PID " + std::to_string(pid) + " matched rule " + matchedRule + ".\nContext: " + context + "\nPlease review.";
      n.source = "Active Response Engine";
      
      notifications::NotificationManager::GetInstance().EmitNotification(n);
    }
  }
}

void ActiveResponse::HandleSuspiciousProcess(uint32_t pid, const std::string& reason) {
  // Legacy bridge
  EvaluateAndRespond(pid, "SuspiciousProcess", reason);
}

void ActiveResponse::OnUserDecision(uint32_t pid, bool kill) {
  core::process::ProcessManager procMgr;
  if (kill) {
    SEV_CORE_INFO("User elected to terminate PID {}", pid);
    procMgr.KillProcess(pid);
  } else {
    SEV_CORE_INFO("User elected to resume PID {}", pid);
    procMgr.ResumeProcess(pid);
  }
}

std::vector<ContainmentDirective> ActiveResponse::GetDirectives() const {
  std::lock_guard<std::mutex> lock(directivesMutex_);
  return directives_;
}

} // namespace severance::core::security
