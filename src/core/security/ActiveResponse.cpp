#include "ActiveResponse.hpp"
#include "core/process/ProcessManager.hpp"
#include "core/notifications/NotificationManager.hpp"
#include "logging/Logger.hpp"

namespace severance::core::security {

void ActiveResponse::HandleSuspiciousProcess(uint32_t pid, const std::string& reason) {
  SEV_CORE_WARN("ActiveResponse triggered for PID {}: {}", pid, reason);

  core::process::ProcessManager procMgr;
  if (procMgr.SuspendProcess(pid)) {
    SEV_CORE_INFO("Successfully suspended suspicious process {}", pid);

    // Prompt user via NotificationManager
    notifications::Notification n;
    n.id = "active_response_" + std::to_string(pid);
    n.severity = notifications::NotificationSeverity::Critical;
    n.title = "Suspicious Process Suspended";
    n.message = "PID " + std::to_string(pid) + " suspended: " + reason + ".\nPlease review and decide whether to kill or resume.";
    n.source = "Active Response";
    
    notifications::NotificationManager::GetInstance().EmitNotification(n);
  } else {
    SEV_CORE_ERROR("Failed to suspend suspicious process {}", pid);
  }
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

} // namespace severance::core::security
