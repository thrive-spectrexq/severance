#include "Application.hpp"
#include "events/EventBus.hpp"
#include "events/EventTypes.hpp"
#include "logging/Logger.hpp"
#include <iostream>

namespace severance::core::application {

Application::Application() : m_Running(true) {
  logging::Logger::Init();
  SEV_CORE_INFO("Application initialized.");

  // Example subscribe to app quit event
  events::EventBus::GetInstance().Subscribe(
      events::EventType::AppQuit, [this](std::shared_ptr<events::Event> e) {
        SEV_CORE_INFO("AppQuit event received, shutting down.");
        m_Running = false;
        e->handled = true;
      });
}

Application::~Application() { SEV_CORE_INFO("Application shutting down."); }

void Application::Run() {
  SEV_CORE_INFO("Application running...");
  // Main loop would go here, currently controlled by Qt elsewhere if Qt is main
  // loop
}

} // namespace severance::core::application
