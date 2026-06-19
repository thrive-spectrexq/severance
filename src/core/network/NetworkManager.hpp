#pragma once

#include "ConnectionInfo.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <QString>

namespace severance::core::network {

// ConnectionProtocol mirrors Protocol from ConnectionInfo.hpp
// Used by the GUI layer (with QString fields)
enum class ConnectionProtocol {
  TCP,
  UDP
};

struct NetworkConnection {
  ConnectionProtocol protocol;
  QString localIp;
  uint16_t localPort{0};
  QString remoteIp;
  uint16_t remotePort{0};
  ConnectionState state{ConnectionState::UNKNOWN};
  uint32_t pid{0};
  QString processName; // Fetched lazily or via correlation
};

class NetworkManager {
public:
  static NetworkManager& GetInstance() {
    static NetworkManager instance;
    return instance;
  }

  std::vector<NetworkConnection> GetActiveConnections();

private:
  NetworkManager() = default;
  ~NetworkManager() = default;

  NetworkManager(const NetworkManager&) = delete;
  NetworkManager& operator=(const NetworkManager&) = delete;

  QString FormatIpAddress(uint32_t ipAddr);
  QString FormatIpv6Address(const uint8_t* ipAddr);
  ConnectionState MapTcpState(uint32_t state);
};

} // namespace severance::core::network
