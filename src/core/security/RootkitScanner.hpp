#pragma once

#include <vector>
#include <cstdint>

namespace severance::core::security {

struct HiddenProcess {
  uint32_t pid;
  bool visibleInToolhelp;
  bool visibleInEnumProcesses;
  bool visibleInNtQuerySystemInformation; // Optional extension
};

class RootkitScanner {
public:
  static RootkitScanner& GetInstance() {
    static RootkitScanner instance;
    return instance;
  }

  // Scans for discrepancies in process enumeration APIs
  std::vector<HiddenProcess> ScanForHiddenProcesses();

private:
  RootkitScanner() = default;
  ~RootkitScanner() = default;

  RootkitScanner(const RootkitScanner&) = delete;
  RootkitScanner& operator=(const RootkitScanner&) = delete;

  std::vector<uint32_t> GetPidsFromEnumProcesses();
};

} // namespace severance::core::security
