// ============================================================================
// Severance Tests - Global Setup
// Ensures Logger and other singletons are initialized before any test runs
// ============================================================================
#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include "core/logging/Logger.hpp"

class TestSetupListener : public Catch::EventListenerBase {
public:
  using Catch::EventListenerBase::EventListenerBase;

  void testRunStarting(Catch::TestRunInfo const&) override {
    severance::core::logging::Logger::Init();
  }
};

CATCH_REGISTER_LISTENER(TestSetupListener)
