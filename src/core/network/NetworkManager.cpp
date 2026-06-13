#include "NetworkManager.hpp"

namespace severance::core::network {

std::vector<ConnectionInfo> NetworkManager::GetActiveConnections() {
  // Return mock data for skeleton
  std::vector<ConnectionInfo> connections;

  connections.push_back({Protocol::TCP, "127.0.0.1", 8080, "0.0.0.0", 0,
                         ConnectionState::Listen, 100});
  connections.push_back({Protocol::TCP, "192.168.1.5", 54321, "93.184.216.34",
                         443, ConnectionState::Established, 101});
  connections.push_back(
      {Protocol::UDP, "0.0.0.0", 53, "0.0.0.0", 0, ConnectionState::Listen, 1});

  return connections;
}

} // namespace severance::core::network
