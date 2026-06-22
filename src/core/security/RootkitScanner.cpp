#include "RootkitScanner.hpp"
#include "core/process/ProcessManager.hpp"
#include "logging/Logger.hpp"
#include "ActiveResponse.hpp"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#endif

#include <algorithm>
#include <set>

namespace severance::core::security {

std::vector<uint32_t> RootkitScanner::GetPidsFromEnumProcesses() {
  std::vector<uint32_t> pids;
#ifdef _WIN32
  DWORD aProcesses[1024], cbNeeded, cProcesses;
  if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
    SEV_CORE_ERROR("RootkitScanner: EnumProcesses failed");
    return pids;
  }
  
  cProcesses = cbNeeded / sizeof(DWORD);
  for (unsigned int i = 0; i < cProcesses; i++) {
    if (aProcesses[i] != 0) {
      pids.push_back(aProcesses[i]);
    }
  }
#endif
  return pids;
}

std::vector<HiddenProcess> RootkitScanner::ScanForHiddenProcesses() {
  std::vector<HiddenProcess> anomalies;
#ifdef _WIN32
  core::process::ProcessManager procMgr;
  auto toolhelpList = procMgr.GetRunningProcesses();
  auto enumProcList = GetPidsFromEnumProcesses();

  std::set<uint32_t> thPids;
  for (const auto& p : toolhelpList) {
    thPids.insert(p.pid);
  }

  std::set<uint32_t> enumPids(enumProcList.begin(), enumProcList.end());

  // Check for PIDs hidden from Toolhelp32 but visible in EnumProcesses
  for (uint32_t pid : enumPids) {
    if (thPids.find(pid) == thPids.end()) {
      HiddenProcess hp;
      hp.pid = pid;
      hp.visibleInToolhelp = false;
      hp.visibleInEnumProcesses = true;
      hp.visibleInNtQuerySystemInformation = false;
      anomalies.push_back(hp);
      
      SEV_CORE_WARN("RootkitScanner: Process {} is hidden from CreateToolhelp32Snapshot!", pid);
      
      // Trigger active response
      ActiveResponse::GetInstance().HandleSuspiciousProcess(pid, "Hidden process detected (Toolhelp32 evasion)");
    }
  }

  // We could also check the reverse, but Toolhelp32 can sometimes report stale PIDs if a process exits during the scan.
#endif
  return anomalies;
}

} // namespace severance::core::security
