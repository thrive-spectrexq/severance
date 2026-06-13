#pragma once

#include <cstdint>
#include <string>

namespace severance::core::network {

enum class ConnectionState {
  Established,
  Listen,
  TimeWait,
  CloseWait,
  Closed,
  Unknown
};

enum class Protocol { TCP, UDP, Unknown };

struct ConnectionInfo {
  Protocol protocol{Protocol::Unknown};
  std::string localAddress;
  uint16_t localPort{0};
  std::string remoteAddress;
  uint16_t remotePort{0};
  ConnectionState state{ConnectionState::Unknown};
  uint32_t pid{0};
};

} // namespace severance::core::network
