#pragma once

#include <cstdint>
#include <string>

namespace severance::core::network {

// Canonical connection state enum — used by both ConnectionInfo and NetworkConnection
enum class ConnectionState {
  UNKNOWN = 0,
  CLOSED,
  LISTEN,
  SYN_SENT,
  SYN_RCVD,
  ESTABLISHED,
  FIN_WAIT1,
  FIN_WAIT2,
  CLOSE_WAIT,
  CLOSING,
  LAST_ACK,
  TIME_WAIT,
  DELETE_TCB
};

enum class Protocol { TCP, UDP, Unknown };

struct ConnectionInfo {
  Protocol protocol{Protocol::Unknown};
  std::string localAddress;
  uint16_t localPort{0};
  std::string remoteAddress;
  uint16_t remotePort{0};
  ConnectionState state{ConnectionState::UNKNOWN};
  uint32_t pid{0};
};

} // namespace severance::core::network
