#include "EventBus.hpp"
#include <algorithm>

namespace severance::core::events {

void EventBus::Subscribe(EventType type, EventCallback callback) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Subscribers[type].push_back(callback);
}

void EventBus::Publish(std::shared_ptr<Event> event) {
    std::vector<EventCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto it = m_Subscribers.find(event->GetType());
        if (it != m_Subscribers.end()) {
            callbacks = it->second;
        }
    }

    for (auto& callback : callbacks) {
        callback(event);
        if (event->handled) {
            break;
        }
    }
}

} // namespace severance::core::events
