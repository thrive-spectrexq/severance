#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace severance::core::metrics {

struct CpuMetrics {
  double globalUsagePercent{0.0};
  std::vector<double> coreUsagePercent;
  uint64_t currentClockFreqMhz{0};
  uint32_t processCount{0};
  uint32_t threadCount{0};
  uint32_t handleCount{0};
  std::string processorName;
};

struct MemoryMetrics {
  uint64_t totalBytes{0};
  uint64_t availableBytes{0};
  uint64_t usedBytes{0};
  uint64_t cachedBytes{0};
  double usagePercent{0.0};
};

struct NetworkAdapterMetrics {
  std::string name;
  std::string description;
  std::string macAddress;
  std::vector<std::string> ipAddresses;
  uint64_t bytesReceivedPerSec{0};
  uint64_t bytesSentPerSec{0};
  uint64_t totalBytesReceived{0};
  uint64_t totalBytesSent{0};
};

struct NetworkMetrics {
  uint64_t totalBytesReceivedPerSec{0};
  uint64_t totalBytesSentPerSec{0};
  std::vector<NetworkAdapterMetrics> adapters;
};

struct DiskMetrics {
  std::string name;
  uint64_t totalSpaceBytes{0};
  uint64_t freeSpaceBytes{0};
  uint64_t readBytesPerSec{0};
  uint64_t writeBytesPerSec{0};
  double activeTimePercent{0.0};
};

struct GpuMetrics {
  std::string name;
  double usagePercent{0.0};
  uint64_t dedicatedMemoryTotalBytes{0};
  uint64_t dedicatedMemoryUsedBytes{0};
  double temperatureCelsius{0.0}; // Optional, depending on driver support
};

struct SystemMetricsSnapshot {
  uint64_t timestampEpochMs{0};
  uint64_t uptimeSeconds{0};
  CpuMetrics cpu;
  MemoryMetrics memory;
  NetworkMetrics network;
  std::vector<DiskMetrics> disks;
  std::vector<GpuMetrics> gpus;
};

class MetricsProvider {
public:
  MetricsProvider() = default;
  virtual ~MetricsProvider() = default;

  virtual SystemMetricsSnapshot GetSnapshot() = 0;
};

} // namespace severance::core::metrics
