#include "ProcessManager.hpp"
#include "logging/Logger.hpp"

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <sddl.h>
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "advapi32.lib")
#endif

#include <unordered_map>
#include <algorithm>
#include "utils/ScopedHandle.hpp"

namespace severance::core::process {

#ifdef _WIN32

// Helper: get username for a process
static std::string GetProcessUser(HANDLE hProcess) {
  utils::ScopedHandle hToken;
  if (!OpenProcessToken(hProcess, TOKEN_QUERY, hToken.GetAddressOf())) {
    return "";
  }

  DWORD tokenInfoLen = 0;
  GetTokenInformation(hToken, TokenUser, nullptr, 0, &tokenInfoLen);
  if (tokenInfoLen == 0) {
    return "";
  }

  std::vector<BYTE> buffer(tokenInfoLen);
  if (!GetTokenInformation(hToken, TokenUser, buffer.data(), tokenInfoLen, &tokenInfoLen)) {
    return "";
  }

  auto* tokenUser = reinterpret_cast<TOKEN_USER*>(buffer.data());

  char name[256] = {};
  char domain[256] = {};
  DWORD nameLen = sizeof(name);
  DWORD domainLen = sizeof(domain);
  SID_NAME_USE sidType;

  if (LookupAccountSidA(nullptr, tokenUser->User.Sid, name, &nameLen, domain, &domainLen, &sidType)) {
    // Return "DOMAIN\User" or just "User"
    std::string user = name;
    return user;
  }

  return "";
}

// Helper: get executable path
static std::string GetProcessPath(HANDLE hProcess) {
  char path[MAX_PATH] = {};
  DWORD size = MAX_PATH;
  if (QueryFullProcessImageNameA(hProcess, 0, path, &size)) {
    return std::string(path);
  }
  return "";
}

// Helper: compute CPU usage between two snapshots
// We store FILETIME per-process and compute delta on next refresh
struct CpuTimeSnapshot {
  ULONGLONG kernelTime{0};
  ULONGLONG userTime{0};
  ULONGLONG timestamp{0};
};

static std::unordered_map<uint32_t, CpuTimeSnapshot> s_PrevCpuTimes;
static ULONGLONG s_PrevSystemTime = 0;
static int s_NumProcessors = 0;

static ULONGLONG FileTimeToULL(const FILETIME& ft) {
  ULARGE_INTEGER uli;
  uli.LowPart = ft.dwLowDateTime;
  uli.HighPart = ft.dwHighDateTime;
  return uli.QuadPart;
}

static double ComputeCpuPercent(HANDLE hProcess, uint32_t pid) {
  FILETIME createTime, exitTime, kernelTime, userTime;
  if (!GetProcessTimes(hProcess, &createTime, &exitTime, &kernelTime, &userTime)) {
    return 0.0;
  }

  ULONGLONG currentKernel = FileTimeToULL(kernelTime);
  ULONGLONG currentUser = FileTimeToULL(userTime);

  FILETIME sysIdle, sysKernel, sysUser;
  GetSystemTimes(&sysIdle, &sysKernel, &sysUser);
  ULONGLONG currentSystemTime = FileTimeToULL(sysKernel) + FileTimeToULL(sysUser);

  if (s_NumProcessors == 0) {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    s_NumProcessors = sysInfo.dwNumberOfProcessors;
    if (s_NumProcessors == 0) s_NumProcessors = 1;
  }

  double cpuPercent = 0.0;
  auto prevIt = s_PrevCpuTimes.find(pid);
  if (prevIt != s_PrevCpuTimes.end() && s_PrevSystemTime > 0) {
    ULONGLONG procDelta = (currentKernel + currentUser) -
                          (prevIt->second.kernelTime + prevIt->second.userTime);
    ULONGLONG sysDelta = currentSystemTime - s_PrevSystemTime;
    if (sysDelta > 0) {
      cpuPercent = (static_cast<double>(procDelta) / static_cast<double>(sysDelta)) * 100.0;
      // Clamp to reasonable range
      if (cpuPercent < 0.0) cpuPercent = 0.0;
      if (cpuPercent > 100.0 * s_NumProcessors) cpuPercent = 100.0 * s_NumProcessors;
    }
  }

  // Store for next delta
  s_PrevCpuTimes[pid] = {currentKernel, currentUser, currentSystemTime};
  s_PrevSystemTime = currentSystemTime;

  return cpuPercent;
}

std::vector<ProcessInfo> ProcessManager::GetRunningProcesses() {
  std::vector<ProcessInfo> processes;

  utils::ScopedHandle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
  if (!snapshot.IsValid()) {
    SEV_CORE_ERROR("Failed to create process snapshot");
    return processes;
  }

  PROCESSENTRY32W pe;
  pe.dwSize = sizeof(PROCESSENTRY32W);

  if (!Process32FirstW(snapshot, &pe)) {
    return processes;
  }

  // Track which PIDs are still alive to clean up old CPU data
  std::vector<uint32_t> alivePids;

  do {
    ProcessInfo info;
    info.pid = pe.th32ProcessID;
    info.ppid = pe.th32ParentProcessID;

    // Convert WCHAR name to std::string (narrow)
    int nameLen = WideCharToMultiByte(CP_UTF8, 0, pe.szExeFile, -1, nullptr, 0, nullptr, nullptr);
    if (nameLen > 0) {
      std::string narrowName(nameLen - 1, '\0');
      WideCharToMultiByte(CP_UTF8, 0, pe.szExeFile, -1, narrowName.data(), nameLen, nullptr, nullptr);
      info.name = std::move(narrowName);
    }

    info.threadCount = pe.cntThreads;

    alivePids.push_back(info.pid);

    // Open process for detailed info (skip PID 0)
    if (info.pid != 0) {
      utils::ScopedHandle hProcess(OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
        FALSE, info.pid));

      if (hProcess.IsValid()) {
        // Memory info
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(hProcess, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc), sizeof(pmc))) {
          info.memoryWorkingSetBytes = pmc.WorkingSetSize;
          info.memoryPrivateBytes = pmc.PrivateUsage;
        }

        // CPU usage
        info.cpuUsagePercent = ComputeCpuPercent(hProcess, info.pid);

        // Path
        info.executablePath = GetProcessPath(hProcess);

        // User
        info.user = GetProcessUser(hProcess);

        // Handle count
        DWORD handleCount = 0;
        if (GetProcessHandleCount(hProcess, &handleCount)) {
          info.handleCount = handleCount;
        }
      }
    }

    info.status = ProcessInfo::Status::Running;
    processes.push_back(std::move(info));

  } while (Process32NextW(snapshot, &pe));

  // Clean up old CPU data for dead processes
  for (auto it = s_PrevCpuTimes.begin(); it != s_PrevCpuTimes.end(); ) {
    if (std::find(alivePids.begin(), alivePids.end(), it->first) == alivePids.end()) {
      it = s_PrevCpuTimes.erase(it);
    } else {
      ++it;
    }
  }

  return processes;
}

bool ProcessManager::KillProcess(uint32_t pid) {
  utils::ScopedHandle hProcess(OpenProcess(PROCESS_TERMINATE, FALSE, pid));
  if (!hProcess.IsValid()) {
    SEV_CORE_ERROR("Failed to open process {} for termination", pid);
    return false;
  }

  BOOL result = TerminateProcess(hProcess, 1);

  if (result) {
    SEV_CORE_INFO("Terminated process {}", pid);
  } else {
    SEV_CORE_ERROR("Failed to terminate process {}", pid);
  }
  return result != FALSE;
}

bool ProcessManager::SuspendProcess(uint32_t pid) {
  // NtSuspendProcess is an undocumented but stable ntdll export.
  // Dynamically load to avoid compile-time ntdll dependency.
  using NtSuspendProcessFn = NTSTATUS(NTAPI*)(HANDLE);
  static NtSuspendProcessFn pNtSuspendProcess = nullptr;

  if (!pNtSuspendProcess) {
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (hNtdll) {
      pNtSuspendProcess = reinterpret_cast<NtSuspendProcessFn>(
        GetProcAddress(hNtdll, "NtSuspendProcess"));
    }
  }

  if (!pNtSuspendProcess) {
    SEV_CORE_ERROR("NtSuspendProcess not available on this system");
    return false;
  }

  utils::ScopedHandle hProcess(OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid));
  if (!hProcess.IsValid()) {
    SEV_CORE_ERROR("Failed to open process {} for suspend (error {})", pid, GetLastError());
    return false;
  }

  NTSTATUS status = pNtSuspendProcess(hProcess.Get());
  if (status == 0) {
    SEV_CORE_INFO("Suspended process {}", pid);
    return true;
  } else {
    SEV_CORE_ERROR("NtSuspendProcess failed for PID {} (NTSTATUS: 0x{:08X})", pid, static_cast<uint32_t>(status));
    return false;
  }
}

bool ProcessManager::ResumeProcess(uint32_t pid) {
  using NtResumeProcessFn = NTSTATUS(NTAPI*)(HANDLE);
  static NtResumeProcessFn pNtResumeProcess = nullptr;

  if (!pNtResumeProcess) {
    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (hNtdll) {
      pNtResumeProcess = reinterpret_cast<NtResumeProcessFn>(
        GetProcAddress(hNtdll, "NtResumeProcess"));
    }
  }

  if (!pNtResumeProcess) {
    SEV_CORE_ERROR("NtResumeProcess not available on this system");
    return false;
  }

  utils::ScopedHandle hProcess(OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid));
  if (!hProcess.IsValid()) {
    SEV_CORE_ERROR("Failed to open process {} for resume (error {})", pid, GetLastError());
    return false;
  }

  NTSTATUS status = pNtResumeProcess(hProcess.Get());
  if (status == 0) {
    SEV_CORE_INFO("Resumed process {}", pid);
    return true;
  } else {
    SEV_CORE_ERROR("NtResumeProcess failed for PID {} (NTSTATUS: 0x{:08X})", pid, static_cast<uint32_t>(status));
    return false;
  }
}

#else
// Non-Windows stub
std::vector<ProcessInfo> ProcessManager::GetRunningProcesses() {
  return {};
}
bool ProcessManager::KillProcess(uint32_t) { return false; }
bool ProcessManager::SuspendProcess(uint32_t) { return false; }
bool ProcessManager::ResumeProcess(uint32_t) { return false; }
#endif

} // namespace severance::core::process
