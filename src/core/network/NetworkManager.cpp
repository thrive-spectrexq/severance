#include "NetworkManager.hpp"
#include "logging/Logger.hpp"

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

namespace severance::core::network {

ConnectionState NetworkManager::MapTcpState(uint32_t state) {
  switch (state) {
    case MIB_TCP_STATE_CLOSED: return ConnectionState::CLOSED;
    case MIB_TCP_STATE_LISTEN: return ConnectionState::LISTEN;
    case MIB_TCP_STATE_SYN_SENT: return ConnectionState::SYN_SENT;
    case MIB_TCP_STATE_SYN_RCVD: return ConnectionState::SYN_RCVD;
    case MIB_TCP_STATE_ESTAB: return ConnectionState::ESTABLISHED;
    case MIB_TCP_STATE_FIN_WAIT1: return ConnectionState::FIN_WAIT1;
    case MIB_TCP_STATE_FIN_WAIT2: return ConnectionState::FIN_WAIT2;
    case MIB_TCP_STATE_CLOSE_WAIT: return ConnectionState::CLOSE_WAIT;
    case MIB_TCP_STATE_CLOSING: return ConnectionState::CLOSING;
    case MIB_TCP_STATE_LAST_ACK: return ConnectionState::LAST_ACK;
    case MIB_TCP_STATE_TIME_WAIT: return ConnectionState::TIME_WAIT;
    case MIB_TCP_STATE_DELETE_TCB: return ConnectionState::DELETE_TCB;
    default: return ConnectionState::UNKNOWN;
  }
}

QString NetworkManager::FormatIpAddress(uint32_t ipAddr) {
  struct in_addr ip;
  ip.S_un.S_addr = ipAddr;
  char str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &ip, str, INET_ADDRSTRLEN);
  return QString::fromLatin1(str);
}

QString NetworkManager::FormatIpv6Address(const uint8_t* ipAddr) {
  struct in6_addr ip;
  memcpy(ip.u.Byte, ipAddr, 16);
  char str[INET6_ADDRSTRLEN];
  inet_ntop(AF_INET6, &ip, str, INET6_ADDRSTRLEN);
  return QString::fromLatin1(str);
}

std::vector<NetworkConnection> NetworkManager::GetActiveConnections() {
  std::vector<NetworkConnection> connections;

  // Initialize Winsock if not done (though Qt does it usually, it's safer)
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);

  // 1. TCP IPv4
  PMIB_TCPTABLE_OWNER_PID pTcpTable = nullptr;
  DWORD dwSize = 0;
  if (GetExtendedTcpTable(pTcpTable, &dwSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == ERROR_INSUFFICIENT_BUFFER) {
    pTcpTable = (PMIB_TCPTABLE_OWNER_PID)MALLOC(dwSize);
    if (pTcpTable != nullptr) {
      if (GetExtendedTcpTable(pTcpTable, &dwSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
        for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++) {
          NetworkConnection conn;
          conn.protocol = ConnectionProtocol::TCP;
          conn.state = MapTcpState(pTcpTable->table[i].dwState);
          conn.localIp = FormatIpAddress(pTcpTable->table[i].dwLocalAddr);
          conn.localPort = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
          conn.remoteIp = FormatIpAddress(pTcpTable->table[i].dwRemoteAddr);
          conn.remotePort = ntohs((u_short)pTcpTable->table[i].dwRemotePort);
          conn.pid = pTcpTable->table[i].dwOwningPid;
          connections.push_back(conn);
        }
      }
      FREE(pTcpTable);
    }
  }

  // 2. UDP IPv4
  PMIB_UDPTABLE_OWNER_PID pUdpTable = nullptr;
  dwSize = 0;
  if (GetExtendedUdpTable(pUdpTable, &dwSize, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0) == ERROR_INSUFFICIENT_BUFFER) {
    pUdpTable = (PMIB_UDPTABLE_OWNER_PID)MALLOC(dwSize);
    if (pUdpTable != nullptr) {
      if (GetExtendedUdpTable(pUdpTable, &dwSize, FALSE, AF_INET, UDP_TABLE_OWNER_PID, 0) == NO_ERROR) {
        for (DWORD i = 0; i < pUdpTable->dwNumEntries; i++) {
          NetworkConnection conn;
          conn.protocol = ConnectionProtocol::UDP;
          conn.state = ConnectionState::UNKNOWN; // UDP is connectionless
          conn.localIp = FormatIpAddress(pUdpTable->table[i].dwLocalAddr);
          conn.localPort = ntohs((u_short)pUdpTable->table[i].dwLocalPort);
          conn.remoteIp = "*";
          conn.remotePort = 0;
          conn.pid = pUdpTable->table[i].dwOwningPid;
          connections.push_back(conn);
        }
      }
      FREE(pUdpTable);
    }
  }

  // 3. TCP IPv6
  PMIB_TCP6TABLE_OWNER_PID pTcp6Table = nullptr;
  dwSize = 0;
  if (GetExtendedTcpTable(pTcp6Table, &dwSize, FALSE, AF_INET6, TCP_TABLE_OWNER_PID_ALL, 0) == ERROR_INSUFFICIENT_BUFFER) {
    pTcp6Table = (PMIB_TCP6TABLE_OWNER_PID)MALLOC(dwSize);
    if (pTcp6Table != nullptr) {
      if (GetExtendedTcpTable(pTcp6Table, &dwSize, FALSE, AF_INET6, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
        for (DWORD i = 0; i < pTcp6Table->dwNumEntries; i++) {
          NetworkConnection conn;
          conn.protocol = ConnectionProtocol::TCP;
          conn.state = MapTcpState(pTcp6Table->table[i].dwState);
          conn.localIp = FormatIpv6Address(pTcp6Table->table[i].ucLocalAddr);
          conn.localPort = ntohs((u_short)pTcp6Table->table[i].dwLocalPort);
          conn.remoteIp = FormatIpv6Address(pTcp6Table->table[i].ucRemoteAddr);
          conn.remotePort = ntohs((u_short)pTcp6Table->table[i].dwRemotePort);
          conn.pid = pTcp6Table->table[i].dwOwningPid;
          connections.push_back(conn);
        }
      }
      FREE(pTcp6Table);
    }
  }

  // 4. UDP IPv6
  PMIB_UDP6TABLE_OWNER_PID pUdp6Table = nullptr;
  dwSize = 0;
  if (GetExtendedUdpTable(pUdp6Table, &dwSize, FALSE, AF_INET6, UDP_TABLE_OWNER_PID, 0) == ERROR_INSUFFICIENT_BUFFER) {
    pUdp6Table = (PMIB_UDP6TABLE_OWNER_PID)MALLOC(dwSize);
    if (pUdp6Table != nullptr) {
      if (GetExtendedUdpTable(pUdp6Table, &dwSize, FALSE, AF_INET6, UDP_TABLE_OWNER_PID, 0) == NO_ERROR) {
        for (DWORD i = 0; i < pUdp6Table->dwNumEntries; i++) {
          NetworkConnection conn;
          conn.protocol = ConnectionProtocol::UDP;
          conn.state = ConnectionState::UNKNOWN;
          conn.localIp = FormatIpv6Address(pUdp6Table->table[i].ucLocalAddr);
          conn.localPort = ntohs((u_short)pUdp6Table->table[i].dwLocalPort);
          conn.remoteIp = "*";
          conn.remotePort = 0;
          conn.pid = pUdp6Table->table[i].dwOwningPid;
          connections.push_back(conn);
        }
      }
      FREE(pUdp6Table);
    }
  }

  return connections;
}

} // namespace severance::core::network
#else
namespace severance::core::network {
ConnectionState NetworkManager::MapTcpState(uint32_t state) { return ConnectionState::UNKNOWN; }
QString NetworkManager::FormatIpAddress(uint32_t ipAddr) { return ""; }
QString NetworkManager::FormatIpv6Address(const uint8_t* ipAddr) { return ""; }
std::vector<NetworkConnection> NetworkManager::GetActiveConnections() { return {}; }
}
#endif
