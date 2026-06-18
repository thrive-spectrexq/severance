#include "SandboxManager.hpp"
#include "logging/Logger.hpp"

#if defined(_WIN32)
#include <windows.h>
#include <sddl.h>
#endif

namespace severance::core::sandbox {

SandboxManager::~SandboxManager() {
    TerminateAll();
}

bool SandboxManager::LaunchProfile(const SandboxProfile &profile) {
  SEV_CORE_INFO("Launching sandbox profile: {} ({})", profile.name, profile.executablePath);

#if defined(_WIN32)
  HANDLE hJob = CreateJobObjectW(NULL, NULL);
  if (!hJob) {
    SEV_CORE_ERROR("Failed to create Job Object. Error: {}", GetLastError());
    return false;
  }

  // Set limits
  JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {0};
  jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

  if (profile.policy.maxMemoryBytes > 0) {
    jeli.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PROCESS_MEMORY;
    jeli.ProcessMemoryLimit = profile.policy.maxMemoryBytes;
  }

  if (!SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli))) {
    SEV_CORE_ERROR("Failed to set Job limits. Error: {}", GetLastError());
    CloseHandle(hJob);
    return false;
  }

  if (profile.policy.maxCpuPercent < 100.0) {
    JOBOBJECT_CPU_RATE_CONTROL_INFORMATION jcpu = {0};
    jcpu.ControlFlags = JOB_OBJECT_CPU_RATE_CONTROL_ENABLE | JOB_OBJECT_CPU_RATE_CONTROL_HARD_CAP;
    jcpu.CpuRate = static_cast<DWORD>(profile.policy.maxCpuPercent * 100.0); // 10,000 = 100%
    if (!SetInformationJobObject(hJob, JobObjectCpuRateControlInformation, &jcpu, sizeof(jcpu))) {
        SEV_CORE_WARN("Failed to set CPU limits. Error: {}", GetLastError());
    }
  }

  // Token Integrity
  HANDLE hToken = NULL;
  HANDLE hNewToken = NULL;
  if (!profile.policy.allowFileSystemWrite) {
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE | TOKEN_ADJUST_DEFAULT | TOKEN_QUERY | TOKEN_ASSIGN_PRIMARY, &hToken)) {
      if (DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hNewToken)) {
        PSID pIntegritySid = NULL;
        if (ConvertStringSidToSidW(L"S-1-16-4096", &pIntegritySid)) { // Low Integrity
          TOKEN_MANDATORY_LABEL tml = {0};
          tml.Label.Attributes = SE_GROUP_INTEGRITY;
          tml.Label.Sid = pIntegritySid;
          SetTokenInformation(hNewToken, TokenIntegrityLevel, &tml, sizeof(tml) + GetLengthSid(pIntegritySid));
          LocalFree(pIntegritySid);
        }
      }
    }
  }

  STARTUPINFOW si = {sizeof(si)};
  PROCESS_INFORMATION pi = {0};
  
  std::wstring wExePath(profile.executablePath.begin(), profile.executablePath.end()); // Simple conversion

  bool success = false;
  DWORD creationFlags = CREATE_SUSPENDED | CREATE_BREAKAWAY_FROM_JOB;

  if (hNewToken) {
    // Note: CreateProcessAsUserW often requires SeAssignPrimaryTokenPrivilege which normal users don't have.
    success = CreateProcessAsUserW(hNewToken, wExePath.c_str(), NULL, NULL, NULL, FALSE, creationFlags, NULL, NULL, &si, &pi);
    if (!success) {
      SEV_CORE_WARN("CreateProcessAsUserW failed (Error {}). Falling back to CreateProcessW.", GetLastError());
    }
  }

  if (!success) {
    success = CreateProcessW(wExePath.c_str(), NULL, NULL, NULL, FALSE, creationFlags, NULL, NULL, &si, &pi);
  }

  if (hNewToken) CloseHandle(hNewToken);
  if (hToken) CloseHandle(hToken);

  if (!success) {
    SEV_CORE_ERROR("Failed to create process. Error: {}", GetLastError());
    CloseHandle(hJob);
    return false;
  }

  if (!AssignProcessToJobObject(hJob, pi.hProcess)) {
    SEV_CORE_ERROR("Failed to assign process to Job. Error: {}", GetLastError());
    TerminateProcess(pi.hProcess, 1);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hJob);
    return false;
  }

  ResumeThread(pi.hThread);
  CloseHandle(pi.hThread);

  ActiveSandbox ctx;
  ctx.profile = profile;
  ctx.hProcess = pi.hProcess;
  ctx.hJob = hJob;
  m_ActiveSandboxes.push_back(ctx);

  return true;
#else
  SEV_CORE_ERROR("Sandboxing is only implemented for Windows in this version.");
  return false;
#endif
}

void SandboxManager::TerminateSandbox(size_t index) {
  if (index >= m_ActiveSandboxes.size()) return;
  auto& ctx = m_ActiveSandboxes[index];
#if defined(_WIN32)
  if (ctx.hProcess) {
    TerminateProcess(ctx.hProcess, 0);
    CloseHandle(ctx.hProcess);
  }
  if (ctx.hJob) CloseHandle(ctx.hJob);
#endif
  m_ActiveSandboxes.erase(m_ActiveSandboxes.begin() + index);
}

void SandboxManager::TerminateAll() {
  SEV_CORE_INFO("Terminating all active sandboxes.");
#if defined(_WIN32)
  for (auto& ctx : m_ActiveSandboxes) {
    if (ctx.hProcess) {
      TerminateProcess(ctx.hProcess, 0);
      CloseHandle(ctx.hProcess);
    }
    if (ctx.hJob) CloseHandle(ctx.hJob);
  }
#endif
  m_ActiveSandboxes.clear();
}

std::vector<SandboxProfile> SandboxManager::GetActiveProfiles() const {
  std::vector<SandboxProfile> profiles;
  for (const auto& ctx : m_ActiveSandboxes) {
    profiles.push_back(ctx.profile);
  }
  return profiles;
}

} // namespace severance::core::sandbox
