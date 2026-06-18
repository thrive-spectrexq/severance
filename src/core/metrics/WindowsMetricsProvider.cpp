#include "WindowsMetricsProvider.hpp"
#include "logging/Logger.hpp"

#ifdef _WIN32
#include <iphlpapi.h>
#include <pdh.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "pdh.lib")

static uint64_t GetCurrentTimeMs() {
  FILETIME ft;
  GetSystemTimeAsFileTime(&ft);
  ULARGE_INTEGER uli;
  uli.LowPart = ft.dwLowDateTime;
  uli.HighPart = ft.dwHighDateTime;
  return (uli.QuadPart / 10000); // 100-nanosecond intervals to milliseconds
}

static uint64_t FileTimeToUInt64(const FILETIME& ft) {
  ULARGE_INTEGER uli;
  uli.LowPart = ft.dwLowDateTime;
  uli.HighPart = ft.dwHighDateTime;
  return uli.QuadPart;
}

namespace severance::core::metrics {

WindowsMetricsProvider::WindowsMetricsProvider() {
  m_LastTickCount = GetTickCount64();

  // Get Processor Name from Registry
  HKEY hKey;
  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
    char buffer[256] = {};
    DWORD bufferSize = sizeof(buffer);
    if (RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr, reinterpret_cast<LPBYTE>(buffer), &bufferSize) == ERROR_SUCCESS) {
      m_ProcessorName = buffer;
    }
    RegCloseKey(hKey);
  }
}

WindowsMetricsProvider::~WindowsMetricsProvider() {}

SystemMetricsSnapshot WindowsMetricsProvider::GetCurrentMetrics() {
  SystemMetricsSnapshot snapshot;
  snapshot.timestampEpochMs = GetCurrentTimeMs();

#ifdef _WIN32
  UpdateCpuMetrics(snapshot.cpu);
  UpdateMemoryMetrics(snapshot.memory);
  UpdateNetworkMetrics(snapshot.network);
  UpdateDiskMetrics(snapshot.disks);
  UpdateGpuMetrics(snapshot.gpus);
  
  m_LastTickCount = GetTickCount64();
#endif

  return snapshot;
}

#ifdef _WIN32
void WindowsMetricsProvider::UpdateCpuMetrics(CpuMetrics& cpu) {
  cpu.processorName = m_ProcessorName;

  FILETIME idleTime, kernelTime, userTime;
  if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
    ULARGE_INTEGER uIdle, uKernel, uUser;
    uIdle.LowPart = idleTime.dwLowDateTime;
    uIdle.HighPart = idleTime.dwHighDateTime;
    uKernel.LowPart = kernelTime.dwLowDateTime;
    uKernel.HighPart = kernelTime.dwHighDateTime;
    uUser.LowPart = userTime.dwLowDateTime;
    uUser.HighPart = userTime.dwHighDateTime;

    if (m_LastIdleTime.QuadPart != 0) {
      ULONGLONG idleDiff = uIdle.QuadPart - m_LastIdleTime.QuadPart;
      ULONGLONG kernelDiff = uKernel.QuadPart - m_LastKernelTime.QuadPart;
      ULONGLONG userDiff = uUser.QuadPart - m_LastUserTime.QuadPart;
      ULONGLONG sysDiff = kernelDiff + userDiff;

      if (sysDiff > 0) {
        cpu.globalUsagePercent = ((sysDiff - idleDiff) * 100.0) / sysDiff;
      }
    }

    m_LastIdleTime = uIdle;
    m_LastKernelTime = uKernel;
    m_LastUserTime = uUser;
  }

  // To get Process, Thread, and Handle counts we use PDH or NtQuerySystemInformation.
  // For simplicity, we skip these in the minimal initial implementation.
}

void WindowsMetricsProvider::UpdateMemoryMetrics(MemoryMetrics& mem) {
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  if (GlobalMemoryStatusEx(&statex)) {
    mem.totalBytes = statex.ullTotalPhys;
    mem.availableBytes = statex.ullAvailPhys;
    mem.usedBytes = mem.totalBytes - mem.availableBytes;
    mem.usagePercent = (static_cast<double>(mem.usedBytes) / mem.totalBytes) * 100.0;
  }
}

void WindowsMetricsProvider::UpdateNetworkMetrics(NetworkMetrics& net) {
  ULONG outBufLen = 0;
  if (GetIfTable(nullptr, &outBufLen, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
    std::vector<uint8_t> buffer(outBufLen);
    auto* pIfTable = reinterpret_cast<PMIB_IFTABLE>(buffer.data());

    if (GetIfTable(pIfTable, &outBufLen, FALSE) == NO_ERROR) {
      uint64_t currentTick = GetTickCount64();
      double timeDiffSec = (currentTick - m_LastTickCount) / 1000.0;
      if (timeDiffSec <= 0) timeDiffSec = 1.0;

      for (DWORD i = 0; i < pIfTable->dwNumEntries; i++) {
        MIB_IFROW row = pIfTable->table[i];
        
        // Skip loopback or internal interfaces if desired
        if (row.dwType == MIB_IF_TYPE_SOFTWARE_LOOPBACK) continue;

        NetworkAdapterMetrics adapter;
        adapter.name = reinterpret_cast<const char*>(row.bDescr);
        adapter.totalBytesReceived = row.dwInOctets;
        adapter.totalBytesSent = row.dwOutOctets;

        auto& state = m_NetState[row.dwIndex];
        if (m_LastTickCount > 0) {
          if (row.dwInOctets >= state.lastBytesReceived) {
            adapter.bytesReceivedPerSec = (row.dwInOctets - state.lastBytesReceived) / timeDiffSec;
          }
          if (row.dwOutOctets >= state.lastBytesSent) {
            adapter.bytesSentPerSec = (row.dwOutOctets - state.lastBytesSent) / timeDiffSec;
          }
        }

        state.lastBytesReceived = row.dwInOctets;
        state.lastBytesSent = row.dwOutOctets;

        net.adapters.push_back(adapter);
        net.totalBytesReceivedPerSec += adapter.bytesReceivedPerSec;
        net.totalBytesSentPerSec += adapter.bytesSentPerSec;
      }
    }
  }
}

void WindowsMetricsProvider::UpdateDiskMetrics(std::vector<DiskMetrics>& disks) {
  // PDH or WMI required for disk I/O metrics. Skipping for minimal initial impl.
}

void WindowsMetricsProvider::UpdateGpuMetrics(std::vector<GpuMetrics>& gpus) {
  // DXGI required for GPU metrics. Skipping for minimal initial impl.
}

#endif

} // namespace severance::core::metrics
