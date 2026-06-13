#include "core/events/Event.hpp"
#include "core/events/EventBus.hpp"
#include "core/events/EventTypes.hpp"
#include "core/logging/Logger.hpp"
#include <catch2/catch_test_macros.hpp>

using namespace severance::core::events;

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
