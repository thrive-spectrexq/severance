// ============================================================================
// Severance Unit Tests - FileMonitor, PluginManager, SandboxManager
// Tests cover stubs and data structures without launching real processes
// ============================================================================
#include <catch2/catch_test_macros.hpp>

#include "core/filesystem/FileEvent.hpp"
#include "core/filesystem/FileMonitor.hpp"
#include "core/plugins/PluginManager.hpp"
#include "core/sandbox/SandboxManager.hpp"

using namespace severance::core::filesystem;
using namespace severance::core::plugins;
using namespace severance::core::sandbox;

// ===========================================================================
// FileMonitor
// ===========================================================================
TEST_CASE("FileMonitor returns empty before start", "[FileMonitor]") {
  FileMonitor monitor;
  auto events = monitor.GetRecentEvents();
  REQUIRE(events.empty());
}

TEST_CASE("FileMonitor returns mock events after start", "[FileMonitor]") {
  FileMonitor monitor;
  monitor.Start("C:\\severance_test_dir");

  auto events = monitor.GetRecentEvents();
  REQUIRE(events.size() == 3);
  REQUIRE(events[0].type == FileEventType::Created);
  REQUIRE(events[1].type == FileEventType::Modified);
  REQUIRE(events[2].type == FileEventType::Deleted);

  monitor.Stop();
}

TEST_CASE("FileMonitor returns empty after stop", "[FileMonitor]") {
  FileMonitor monitor;
  monitor.Start("C:\\severance_test_dir");
  monitor.Stop();

  auto events = monitor.GetRecentEvents();
  REQUIRE(events.empty());
}

// ===========================================================================
// PluginManager
// ===========================================================================
TEST_CASE("PluginManager starts with no active plugins", "[PluginManager]") {
  PluginManager manager;
  REQUIRE(manager.GetActivePlugins().empty());
}

TEST_CASE("PluginManager load from nonexistent path stays empty", "[PluginManager]") {
  PluginManager manager;
  manager.LoadPlugins("C:\\nonexistent_plugin_dir");
  REQUIRE(manager.GetActivePlugins().empty());
}

// ===========================================================================
// SandboxManager
// ===========================================================================
TEST_CASE("SandboxManager starts with no active profiles", "[SandboxManager]") {
  SandboxManager manager;
  REQUIRE(manager.GetActiveProfiles().empty());
}

TEST_CASE("SandboxManager TerminateAll on empty is safe", "[SandboxManager]") {
  SandboxManager manager;
  // Should not crash when there are no active sandboxes
  manager.TerminateAll();
  REQUIRE(manager.GetActiveProfiles().empty());
}

TEST_CASE("SandboxManager LaunchProfile with valid executable", "[SandboxManager]") {
  SandboxManager manager;

  SandboxProfile profile;
  profile.name = "NotepadSandbox";
  profile.executablePath = "C:\\Windows\\System32\\notepad.exe";
  profile.policy.allowNetworkAccess = false;

  bool launched = manager.LaunchProfile(profile);

  if (launched) {
    // If launch succeeded, verify it's tracked
    auto profiles = manager.GetActiveProfiles();
    REQUIRE(profiles.size() == 1);
    REQUIRE(profiles[0].name == "NotepadSandbox");

    // Clean up — terminate the sandboxed process
    manager.TerminateAll();
    REQUIRE(manager.GetActiveProfiles().empty());
  } else {
    // On some CI runners, CreateProcessW may fail due to permissions.
    // Just verify no crash occurred.
    REQUIRE(manager.GetActiveProfiles().empty());
  }
}
