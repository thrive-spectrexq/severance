// ============================================================================
// Severance Unit Tests - Core Utilities
// Tests: EventBus, Logger, ThreadPool, UUID, Config
// ============================================================================
#include <catch2/catch_test_macros.hpp>

#include "core/events/Event.hpp"
#include "core/events/EventBus.hpp"
#include "core/events/EventTypes.hpp"
#include "core/logging/Logger.hpp"
#include "utils/Config.hpp"
#include "utils/ThreadPool.hpp"
#include "utils/UUID.hpp"

using namespace severance::core::events;
using namespace severance::utils;

// ---------------------------------------------------------------------------
// Helper: concrete Event subclass for testing the EventBus
// ---------------------------------------------------------------------------
class TestEvent : public Event {
public:
  EventType GetType() const override { return EventType::AppQuit; }
  std::string GetName() const override { return "TestEvent"; }
};

// ===========================================================================
// EventBus
// ===========================================================================
TEST_CASE("EventBus publishes to subscribers", "[EventBus]") {
  bool eventReceived = false;

  EventBus::GetInstance().Subscribe(
      EventType::AppQuit,
      [&](std::shared_ptr<Event> e) { eventReceived = true; });

  auto testEvent = std::make_shared<TestEvent>();
  EventBus::GetInstance().Publish(testEvent);

  REQUIRE(eventReceived == true);
}

TEST_CASE("EventBus delivers correct event type", "[EventBus]") {
  EventType receivedType = EventType::None;

  EventBus::GetInstance().Subscribe(
      EventType::AppQuit,
      [&](std::shared_ptr<Event> e) { receivedType = e->GetType(); });

  auto testEvent = std::make_shared<TestEvent>();
  EventBus::GetInstance().Publish(testEvent);

  REQUIRE(receivedType == EventType::AppQuit);
}

// ===========================================================================
// Logger
// ===========================================================================
TEST_CASE("Logger initializes core and client loggers", "[Logger]") {
  severance::core::logging::Logger::Init();
  REQUIRE(severance::core::logging::Logger::GetCoreLogger() != nullptr);
  REQUIRE(severance::core::logging::Logger::GetClientLogger() != nullptr);
}

// ===========================================================================
// ThreadPool
// ===========================================================================
TEST_CASE("ThreadPool executes tasks and returns results", "[ThreadPool]") {
  ThreadPool pool(2);

  auto result1 = pool.Enqueue([] { return 42; });
  auto result2 = pool.Enqueue([](int a, int b) { return a + b; }, 10, 20);

  REQUIRE(result1.get() == 42);
  REQUIRE(result2.get() == 30);
}

TEST_CASE("ThreadPool handles multiple concurrent tasks", "[ThreadPool]") {
  ThreadPool pool(4);
  constexpr int NUM_TASKS = 50;
  std::vector<std::future<int>> futures;

  for (int i = 0; i < NUM_TASKS; ++i) {
    futures.push_back(pool.Enqueue([i] { return i * i; }));
  }

  for (int i = 0; i < NUM_TASKS; ++i) {
    REQUIRE(futures[i].get() == i * i);
  }
}

// ===========================================================================
// UUID
// ===========================================================================
TEST_CASE("UUID generates valid v4 format", "[UUID]") {
  std::string uuid = severance::utils::UUID::Generate();

  REQUIRE(uuid.length() == 36);
  REQUIRE(uuid[8] == '-');
  REQUIRE(uuid[13] == '-');
  REQUIRE(uuid[14] == '4');  // version nibble
  REQUIRE(uuid[18] == '-');
  REQUIRE(uuid[23] == '-');
}

TEST_CASE("UUID generates unique values", "[UUID]") {
  std::string uuid1 = severance::utils::UUID::Generate();
  std::string uuid2 = severance::utils::UUID::Generate();
  std::string uuid3 = severance::utils::UUID::Generate();

  REQUIRE(uuid1 != uuid2);
  REQUIRE(uuid2 != uuid3);
  REQUIRE(uuid1 != uuid3);
}

// ===========================================================================
// Config
// ===========================================================================
TEST_CASE("Config set and get values", "[Config]") {
  Config::GetInstance().Set("test_key", "test_value");

  REQUIRE(Config::GetInstance().Has("test_key") == true);
  REQUIRE(Config::GetInstance().Get("test_key") == "test_value");
}

TEST_CASE("Config returns default for missing key", "[Config]") {
  REQUIRE(Config::GetInstance().Has("nonexistent_key") == false);
  REQUIRE(Config::GetInstance().Get("nonexistent_key", "fallback") == "fallback");
}
