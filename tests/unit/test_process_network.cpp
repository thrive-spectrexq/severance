// ============================================================================
// Severance Unit Tests - Process & Network
// Tests: ProcessManager, ProcessTree, NetworkManager
// ============================================================================
#include <catch2/catch_test_macros.hpp>

#include "core/process/ProcessInfo.hpp"
#include "core/process/ProcessManager.hpp"
#include "core/process/ProcessTree.hpp"
#include "core/network/ConnectionInfo.hpp"
#include "core/network/NetworkManager.hpp"

using namespace severance::core::process;
using namespace severance::core::network;

// ===========================================================================
// ProcessManager
// ===========================================================================
TEST_CASE("ProcessManager returns running processes", "[ProcessManager]") {
  ProcessManager manager;
  auto processes = manager.GetRunningProcesses();

  // Windows CI will always have running processes
  REQUIRE(processes.size() > 0);
}

TEST_CASE("ProcessManager process entries have valid PIDs", "[ProcessManager]") {
  ProcessManager manager;
  auto processes = manager.GetRunningProcesses();

  REQUIRE(!processes.empty());
  // At least some processes should have non-zero PIDs
  bool foundValidPid = false;
  for (const auto& p : processes) {
    if (p.pid > 0) {
      foundValidPid = true;
      break;
    }
  }
  REQUIRE(foundValidPid);
}

// ===========================================================================
// ProcessTree
// ===========================================================================
TEST_CASE("ProcessTree adds single root process", "[ProcessTree]") {
  ProcessTree tree;

  ProcessInfo pInfo;
  pInfo.pid = 1;
  pInfo.ppid = 0;
  pInfo.name = "init";
  pInfo.user = "root";
  pInfo.memoryWorkingSetBytes = 1024;
  pInfo.cpuUsagePercent = 0.1;

  tree.AddProcess(pInfo);

  auto roots = tree.GetRoots();
  REQUIRE(roots.size() == 1);
  REQUIRE(roots[0]->info.pid == 1);
  REQUIRE(roots[0]->info.name == "init");
}

TEST_CASE("ProcessTree handles parent-child relationship", "[ProcessTree]") {
  ProcessTree tree;

  ProcessInfo parent;
  parent.pid = 100;
  parent.ppid = 0;
  parent.name = "parent_proc";

  ProcessInfo child;
  child.pid = 200;
  child.ppid = 100;
  child.name = "child_proc";

  tree.AddProcess(parent);
  tree.AddProcess(child);

  auto roots = tree.GetRoots();
  REQUIRE(roots.size() == 1);
  REQUIRE(roots[0]->info.pid == 100);
  REQUIRE(roots[0]->children.size() == 1);
  REQUIRE(roots[0]->children[0]->info.pid == 200);
}

TEST_CASE("ProcessTree clear removes all entries", "[ProcessTree]") {
  ProcessTree tree;

  ProcessInfo pInfo;
  pInfo.pid = 1;
  pInfo.ppid = 0;
  pInfo.name = "test";

  tree.AddProcess(pInfo);
  REQUIRE(!tree.GetRoots().empty());

  tree.Clear();
  REQUIRE(tree.GetRoots().empty());
}

TEST_CASE("ProcessTree orphan processes become roots", "[ProcessTree]") {
  ProcessTree tree;

  ProcessInfo orphan;
  orphan.pid = 500;
  orphan.ppid = 9999;  // parent not in tree
  orphan.name = "orphan";

  tree.AddProcess(orphan);

  auto roots = tree.GetRoots();
  REQUIRE(roots.size() == 1);
  REQUIRE(roots[0]->info.pid == 500);
}

// ===========================================================================
// NetworkManager
// ===========================================================================
TEST_CASE("NetworkManager returns connections without crashing", "[NetworkManager]") {
  auto& manager = NetworkManager::GetInstance();
  auto connections = manager.GetActiveConnections();

  // Connections may be empty or populated — just ensure no crash
  REQUIRE(connections.size() >= 0);
}
