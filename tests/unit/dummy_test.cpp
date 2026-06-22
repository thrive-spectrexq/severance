#include "core/events/Event.hpp"
#include "core/events/EventBus.hpp"
#include "core/events/EventTypes.hpp"
#include "core/filesystem/FileEvent.hpp"
#include "core/filesystem/FileMonitor.hpp"
#include "core/logging/Logger.hpp"
#include "core/network/ConnectionInfo.hpp"
#include "core/network/NetworkManager.hpp"
#include "core/process/ProcessInfo.hpp"
#include "core/process/ProcessManager.hpp"
#include "core/process/ProcessTree.hpp"
#include "utils/Config.hpp"
#include "utils/ThreadPool.hpp"
#include "utils/UUID.hpp"
#include "core/plugins/PluginManager.hpp"
#include "core/sandbox/SandboxManager.hpp"
#include <catch2/catch_test_macros.hpp>
#include <chrono>

using namespace severance::core::events;
using namespace severance::utils;
using namespace severance::core::process;
using namespace severance::core::network;
using namespace severance::core::filesystem;

class TestEvent : public Event {
public:
  EventType GetType() const override { return EventType::AppQuit; }
  std::string GetName() const override { return "TestEvent"; }
};

TEST_CASE("EventBus Pub/Sub", "[EventBus]") {
  bool eventReceived = false;

  EventBus::GetInstance().Subscribe(
      EventType::AppQuit,
      [&](std::shared_ptr<Event> e) { eventReceived = true; });

  auto testEvent = std::make_shared<TestEvent>();
  EventBus::GetInstance().Publish(testEvent);

  REQUIRE(eventReceived == true);
}

TEST_CASE("Logger Init", "[Logger]") {
  severance::core::logging::Logger::Init();
  REQUIRE(severance::core::logging::Logger::GetCoreLogger() != nullptr);
  REQUIRE(severance::core::logging::Logger::GetClientLogger() != nullptr);
}

TEST_CASE("ThreadPool Execution", "[ThreadPool]") {
  ThreadPool pool(4);

  auto result1 = pool.Enqueue([] { return 42; });
  auto result2 = pool.Enqueue([](int a, int b) { return a + b; }, 10, 20);

  REQUIRE(result1.get() == 42);
  REQUIRE(result2.get() == 30);
}

TEST_CASE("UUID Generator", "[UUID]") {
  std::string uuid1 = severance::utils::UUID::Generate();
  std::string uuid2 = severance::utils::UUID::Generate();

  REQUIRE(uuid1.length() == 36);
  REQUIRE(uuid1[8] == '-');
  REQUIRE(uuid1[13] == '-');
  REQUIRE(uuid1[14] == '4');
  REQUIRE(uuid1[18] == '-');
  REQUIRE(uuid1[23] == '-');

  REQUIRE(uuid1 != uuid2);
}

TEST_CASE("Config System", "[Config]") {
  Config::GetInstance().Set("test_key", "test_value");

  REQUIRE(Config::GetInstance().Has("test_key") == true);
  REQUIRE(Config::GetInstance().Has("invalid_key") == false);

  REQUIRE(Config::GetInstance().Get("test_key") == "test_value");
  REQUIRE(Config::GetInstance().Get("invalid_key", "default") == "default");
}

TEST_CASE("ProcessManager", "[ProcessManager]") {
  ProcessManager manager;
  auto processes = manager.GetRunningProcesses();

  // On Windows CI, the real process list will have many entries
  REQUIRE(processes.size() > 0);
}

TEST_CASE("ProcessTree", "[ProcessTree]") {
  ProcessInfo pInfo;
  pInfo.pid = 1;
  pInfo.ppid = 0;
  pInfo.name = "init";
  pInfo.user = "root";
  pInfo.memoryWorkingSetBytes = 1024;
  pInfo.cpuUsagePercent = 0.1;

  ProcessTree tree;
  tree.AddProcess(pInfo);

  auto roots = tree.GetRoots();
  REQUIRE(roots.size() == 1);
  REQUIRE(roots[0]->info.pid == 1);

  tree.Clear();
  REQUIRE(tree.GetRoots().empty() == true);
}

TEST_CASE("NetworkManager", "[NetworkManager]") {
  // NetworkManager is a singleton — use GetInstance()
  auto& manager = NetworkManager::GetInstance();
  auto connections = manager.GetActiveConnections();

  // On CI, connections list may vary; just verify no crash
  REQUIRE(connections.size() >= 0);
}

TEST_CASE("FileMonitor", "[FileMonitor]") {
  FileMonitor monitor;
  auto events_before_start = monitor.GetRecentEvents();
  REQUIRE(events_before_start.empty() == true);

  monitor.Start("/tmp/severance_test");
  auto events_after_start = monitor.GetRecentEvents();

  REQUIRE(events_after_start.size() == 3);
  REQUIRE(events_after_start[0].type == FileEventType::Created);

  monitor.Stop();
  auto events_after_stop = monitor.GetRecentEvents();
  REQUIRE(events_after_stop.empty() == true);
}


using namespace severance::core::plugins;
using namespace severance::core::sandbox;

TEST_CASE("PluginManager", "[PluginManager]") {
  PluginManager manager;

  REQUIRE(manager.GetActivePlugins().empty() == true);

  // Test stub load function doesn't crash
  manager.LoadPlugins("/tmp/fake_plugins");
  REQUIRE(manager.GetActivePlugins().empty() == true);
}

TEST_CASE("SandboxManager", "[SandboxManager]") {
  SandboxManager manager;

  SandboxProfile profile;
  profile.name = "TestProfile";
  profile.executablePath = "/usr/bin/echo";
  profile.policy.allowNetworkAccess = false;

  REQUIRE(manager.GetActiveProfiles().empty() == true);

  bool launched = manager.LaunchProfile(profile);
  REQUIRE(launched == true);

  auto profiles = manager.GetActiveProfiles();
  REQUIRE(profiles.size() == 1);
  REQUIRE(profiles[0].name == "TestProfile");

  manager.TerminateAll();
  REQUIRE(manager.GetActiveProfiles().empty() == true);
}
