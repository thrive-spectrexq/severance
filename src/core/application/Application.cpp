#include "Application.hpp"
#include "events/EventBus.hpp"
#include "events/EventTypes.hpp"
#include "store/EventStore.hpp"
#include "logging/Logger.hpp"
#include <iostream>

namespace severance::core::application {

Application::Application() : m_Running(true) {
  logging::Logger::Init();
  SEV_CORE_INFO("Application initialized.");

  // Initialize Event Store
  store::EventStore::GetInstance().Initialize();

  // Subscribe Event Store to all events
  auto eventCallback = [](std::shared_ptr<events::Event> e) {
    store::EventStore::GetInstance().RecordEvent(e);
  };
  
  // We can subscribe to specific events or modify EventBus to support wildcard subscriptions.
  // For now, let's subscribe to the known ones:
  auto& bus = events::EventBus::GetInstance();
  bus.Subscribe(events::EventType::ProcessCreated, eventCallback);
  bus.Subscribe(events::EventType::ProcessTerminated, eventCallback);
  bus.Subscribe(events::EventType::AppQuit, eventCallback);

  // Example subscribe to app quit event
  bus.Subscribe(
      events::EventType::AppQuit, [this](std::shared_ptr<events::Event> e) {
        SEV_CORE_INFO("AppQuit event received, shutting down.");
        m_Running = false;
        e->handled = true;
      });
}

Application::~Application() { 
  SEV_CORE_INFO("Application shutting down."); 
  store::EventStore::GetInstance().Shutdown();
}

void Application::Run() {
  SEV_CORE_INFO("Application running...");
  // Main loop would go here, currently controlled by Qt elsewhere if Qt is main
  // loop
}

} // namespace severance::core::application
