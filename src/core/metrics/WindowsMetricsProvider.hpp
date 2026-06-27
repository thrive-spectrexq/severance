#pragma once

#include "SystemMetrics.hpp"

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <unordered_map>
#include <pdh.h>
#endif

namespace severance::core::metrics {

class WindowsMetricsProvider : public MetricsProvider {
public:
  WindowsMetricsProvider();
  ~WindowsMetricsProvider() override;

  SystemMetricsSnapshot GetSnapshot() override;

private:
#ifdef _WIN32
  void UpdateCpuMetrics(CpuMetrics& cpu);
  void UpdateMemoryMetrics(MemoryMetrics& mem);
  void UpdateNetworkMetrics(NetworkMetrics& net);
  void UpdateDiskMetrics(std::vector<DiskMetrics>& disks);
  void UpdateGpuMetrics(std::vector<GpuMetrics>& gpus);

  // State for deltas
  uint64_t m_LastTickCount{0};
  uint64_t m_BootTickCount{0};
  
  // CPU delta state
  ULARGE_INTEGER m_LastIdleTime{};
  ULARGE_INTEGER m_LastKernelTime{};
  ULARGE_INTEGER m_LastUserTime{};

  // Network delta state (using GetIfTable)
  struct NetAdapterState {
    uint64_t lastBytesReceived{0};
    uint64_t lastBytesSent{0};
  };
  std::unordered_map<uint32_t, NetAdapterState> m_NetState;

  // PDH for Disk I/O
  PDH_HQUERY m_PdhQuery{nullptr};
  PDH_HCOUNTER m_DiskReadCounter{nullptr};
  PDH_HCOUNTER m_DiskWriteCounter{nullptr};
  bool m_PdhInitialized{false};

  // Cached system info
  std::string m_ProcessorName;
#endif
};

} // namespace severance::core::metrics
