#include "WindowsMetricsProvider.hpp"
#include "logging/Logger.hpp"

#ifdef _WIN32
#include <iphlpapi.h>
#include <pdh.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "dxgi.lib")

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

  // Get system boot time for uptime calculation
  m_BootTickCount = GetTickCount64();
}

WindowsMetricsProvider::~WindowsMetricsProvider() {}

SystemMetricsSnapshot WindowsMetricsProvider::GetSnapshot() {
  SystemMetricsSnapshot snapshot;
  snapshot.timestampEpochMs = GetCurrentTimeMs();

  UpdateCpuMetrics(snapshot.cpu);
  UpdateMemoryMetrics(snapshot.memory);
  UpdateNetworkMetrics(snapshot.network);
  UpdateDiskMetrics(snapshot.disks);
  UpdateGpuMetrics(snapshot.gpus);
  
  // System uptime from boot
  snapshot.uptimeSeconds = GetTickCount64() / 1000;
  
  m_LastTickCount = GetTickCount64();

  return snapshot;
}


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

  // Get process and thread counts from snapshot
  DWORD procCount = 0;
  DWORD threadCount = 0;
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnapshot != INVALID_HANDLE_VALUE) {
    PROCESSENTRY32W pe;
    pe.dwSize = sizeof(PROCESSENTRY32W);
    if (Process32FirstW(hSnapshot, &pe)) {
      do {
        procCount++;
        threadCount += pe.cntThreads;
      } while (Process32NextW(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
  }
  cpu.processCount = procCount;
  cpu.threadCount = threadCount;
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
        if (row.dwType == IF_TYPE_SOFTWARE_LOOPBACK) continue;

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
  // Enumerate logical drives
  DWORD driveMask = GetLogicalDrives();
  char driveLetter[] = "A:\\";
  
  for (int i = 0; i < 26; ++i) {
    if (!(driveMask & (1 << i))) continue;
    
    driveLetter[0] = 'A' + i;
    
    UINT driveType = GetDriveTypeA(driveLetter);
    if (driveType != DRIVE_FIXED && driveType != DRIVE_REMOVABLE) continue;
    
    ULARGE_INTEGER freeBytesAvail, totalBytes, totalFreeBytes;
    if (GetDiskFreeSpaceExA(driveLetter, &freeBytesAvail, &totalBytes, &totalFreeBytes)) {
      DiskMetrics dm;
      dm.name = std::string(1, driveLetter[0]) + ":";
      dm.totalSpaceBytes = totalBytes.QuadPart;
      dm.freeSpaceBytes = totalFreeBytes.QuadPart;
      
      // Compute I/O rates using performance counters (PDH)
      // For the initial implementation, we report space usage only.
      // Disk I/O rate tracking requires PDH queries with delta time,
      // which we'll add in a follow-up iteration.
      dm.readBytesPerSec = 0;
      dm.writeBytesPerSec = 0;
      dm.activeTimePercent = 0.0;
      
      disks.push_back(dm);
    }
  }
}

void WindowsMetricsProvider::UpdateGpuMetrics(std::vector<GpuMetrics>& gpus) {
  // Use DXGI to enumerate GPUs and get memory info
  IDXGIFactory1* pFactory = nullptr;
  HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pFactory));
  if (FAILED(hr) || !pFactory) return;
  
  IDXGIAdapter1* pAdapter = nullptr;
  for (UINT adapterIndex = 0; pFactory->EnumAdapters1(adapterIndex, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++adapterIndex) {
    DXGI_ADAPTER_DESC1 desc;
    if (SUCCEEDED(pAdapter->GetDesc1(&desc))) {
      // Skip software/basic adapters
      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        pAdapter->Release();
        continue;
      }
      
      GpuMetrics gm;
      
      // Convert wide string name to narrow
      int nameLen = WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, nullptr, 0, nullptr, nullptr);
      if (nameLen > 0) {
        std::string narrowName(nameLen - 1, '\0');
        WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, narrowName.data(), nameLen, nullptr, nullptr);
        gm.name = std::move(narrowName);
      }
      
      gm.dedicatedMemoryTotalBytes = desc.DedicatedVideoMemory;
      
      // Query current VRAM usage via DXGI 1.4 (IDXGIAdapter3)
      IDXGIAdapter3* pAdapter3 = nullptr;
      if (SUCCEEDED(pAdapter->QueryInterface(__uuidof(IDXGIAdapter3), reinterpret_cast<void**>(&pAdapter3)))) {
        DXGI_QUERY_VIDEO_MEMORY_INFO memInfo;
        if (SUCCEEDED(pAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memInfo))) {
          gm.dedicatedMemoryUsedBytes = memInfo.CurrentUsage;
          
          // Calculate usage percentage
          if (gm.dedicatedMemoryTotalBytes > 0) {
            gm.usagePercent = (static_cast<double>(gm.dedicatedMemoryUsedBytes) / gm.dedicatedMemoryTotalBytes) * 100.0;
          }
        }
        pAdapter3->Release();
      }
      
      // GPU temperature requires NVML or vendor-specific APIs — not available through DXGI
      gm.temperatureCelsius = 0.0;
      
      gpus.push_back(gm);
    }
    pAdapter->Release();
  }
  
  pFactory->Release();
}

#endif // _WIN32

} // namespace severance::core::metrics
