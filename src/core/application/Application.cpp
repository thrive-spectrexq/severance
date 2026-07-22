#include "Application.hpp"
#include "events/EventBus.hpp"
#include "events/EventTypes.hpp"
#include "store/EventStore.hpp"
#include "logging/Logger.hpp"
#include "concurrency/ThreadPool.hpp"
#include <iostream>

namespace severance::core::application {

Application::Application() : m_Running(true) {
  logging::Logger::Init();
  SEV_CORE_INFO("Lumon Workstation Core initialized.");

  // Initialize Thread Pool and Task Scheduler
  concurrency::ThreadPool::GetInstance().Initialize();
  concurrency::TaskScheduler::GetInstance().Initialize();

  // Initialize Event Store
  store::EventStore::GetInstance().Initialize();

  // Subscribe Event Store to all events
  auto eventCallback = [](std::shared_ptr<events::Event> e) {
    store::EventStore::GetInstance().RecordEvent(e);
  };
  
  auto& bus = events::EventBus::GetInstance();
  bus.Subscribe(events::EventType::AppQuit, eventCallback);
  bus.Subscribe(events::EventType::MacrodataRefined, eventCallback);

  // Subscribe to app quit event
  bus.Subscribe(
      events::EventType::AppQuit, [this](std::shared_ptr<events::Event> e) {
        SEV_CORE_INFO("AppQuit event received, shutting down.");
        m_Running = false;
        e->handled = true;
      });
}

Application::~Application() { 
  SEV_CORE_INFO("Lumon Workstation Core shutting down."); 
  store::EventStore::GetInstance().Shutdown();
  concurrency::TaskScheduler::GetInstance().Shutdown();
  concurrency::ThreadPool::GetInstance().Shutdown();
}

void Application::Run() {
  SEV_CORE_INFO("Application running...");
}

} // namespace severance::core::application
