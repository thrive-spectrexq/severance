#include "core/events/Event.hpp"
#include "core/events/EventBus.hpp"
#include "core/events/EventTypes.hpp"
#include "core/logging/Logger.hpp"
#include "utils/Config.hpp"
#include "utils/ThreadPool.hpp"
#include "utils/UUID.hpp"
#include <catch2/catch_test_macros.hpp>
#include <chrono>

using namespace severance::core::events;
using namespace severance::utils;

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
  std::string uuid1 = UUID::Generate();
  std::string uuid2 = UUID::Generate();

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
