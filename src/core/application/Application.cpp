#include "Application.hpp"
#include "events/EventBus.hpp"
#include "events/EventTypes.hpp"
#include "store/EventStore.hpp"
#include "logging/Logger.hpp"
#include "filesystem/EtwMonitor.hpp"
#include "events/FileActivityEvent.hpp"
#include "correlation/CorrelationEngine.hpp"
#include "notifications/RuleEngine.hpp"
#include "metrics/BaselineManager.hpp"
#include "concurrency/ThreadPool.hpp"
#include <iostream>

namespace severance::core::application {

Application::Application() : m_Running(true) {
  logging::Logger::Init();
  SEV_CORE_INFO("Application initialized.");

  // Initialize Thread Pool and Task Scheduler
  concurrency::ThreadPool::GetInstance().Initialize();
  concurrency::TaskScheduler::GetInstance().Initialize();

  // Initialize Event Store
  store::EventStore::GetInstance().Initialize();

  // ETW Monitoring
  filesystem::EtwMonitor::GetInstance().SetCallback([](const filesystem::FileEvent& fe) {
    events::EventBus::GetInstance().Publish(std::make_shared<events::FileActivityEvent>(fe));
  });
  filesystem::EtwMonitor::GetInstance().Start();

  // Correlation Engine
  correlation::CorrelationEngine::GetInstance().Initialize();

  // Rule Engine (Smart Notifications)
  m_RuleEngine = std::make_unique<notifications::RuleEngine>();
  m_RuleEngine->Start();

  // Subscribe Event Store to all events
  auto eventCallback = [](std::shared_ptr<events::Event> e) {
    store::EventStore::GetInstance().RecordEvent(e);
  };
  
  // We can subscribe to specific events or modify EventBus to support wildcard subscriptions.
  // For now, let's subscribe to the known ones:
  auto& bus = events::EventBus::GetInstance();
  bus.Subscribe(events::EventType::ProcessCreated, eventCallback);
  bus.Subscribe(events::EventType::ProcessTerminated, eventCallback);
  bus.Subscribe(events::EventType::FileModified, eventCallback);
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
  m_RuleEngine->Stop();
  correlation::CorrelationEngine::GetInstance().Shutdown();
  filesystem::EtwMonitor::GetInstance().Stop();
  store::EventStore::GetInstance().Shutdown();
  concurrency::TaskScheduler::GetInstance().Shutdown();
  concurrency::ThreadPool::GetInstance().Shutdown();
}

void Application::Run() {
  SEV_CORE_INFO("Application running...");
  // Main loop would go here, currently controlled by Qt elsewhere if Qt is main
  // loop
}

} // namespace severance::core::application
