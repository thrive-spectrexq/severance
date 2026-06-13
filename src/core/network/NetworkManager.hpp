#pragma once

#include "ConnectionInfo.hpp"
#include <vector>

namespace severance::core::network {

class NetworkManager {
public:
  NetworkManager() = default;
  ~NetworkManager() = default;

  NetworkManager(const NetworkManager &) = delete;
  NetworkManager &operator=(const NetworkManager &) = delete;

  std::vector<ConnectionInfo> GetActiveConnections();
};

} // namespace severance::core::network
