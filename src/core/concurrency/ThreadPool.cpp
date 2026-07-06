#include "ThreadPool.hpp"
#include "../logging/Logger.hpp"

namespace severance::core::concurrency {

ThreadPool& ThreadPool::GetInstance() {
    static ThreadPool instance;
    return instance;
}

ThreadPool::~ThreadPool() {
    Shutdown();
}

void ThreadPool::Initialize(size_t numThreads) {
    SEV_CORE_INFO("ThreadPool initializing with {} threads.", numThreads);
    
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    if (!m_Workers.empty()) {
        return; // Already initialized
    }
    
    // Recreate stop source if previously stopped
    if (m_StopSource.stop_requested()) {
        m_StopSource = std::stop_source();
    }

    for (size_t i = 0; i < numThreads; ++i) {
        m_Workers.emplace_back([this](std::stop_token stoken) {
            while (!stoken.stop_requested()) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->m_QueueMutex);
                    // condition_variable_any seamlessly integrates with stop_token
                    this->m_Condition.wait(lock, stoken, [this] { 
                        return !this->m_Tasks.empty(); 
                    });
                    
                    if (stoken.stop_requested() && this->m_Tasks.empty()) {
                        return;
                    }
                    
                    // We might have been woken up but task queue is empty
                    if (this->m_Tasks.empty()) continue;
                    
                    task = std::move(this->m_Tasks.front());
                    this->m_Tasks.pop();
                }
                
                try {
                    task();
                } catch (const std::exception& e) {
                    SEV_CORE_ERROR("Exception in ThreadPool task: {}", e.what());
                } catch (...) {
                    SEV_CORE_ERROR("Unknown exception in ThreadPool task.");
                }
            }
        });
    }
}

void ThreadPool::Shutdown() {
    m_StopSource.request_stop();
    m_Condition.notify_all();
    
    // std::jthread joins automatically on destruction, but we clear them 
    // to eagerly join and clean up the pool.
    m_Workers.clear();
    SEV_CORE_INFO("ThreadPool shut down.");
}

TaskScheduler& TaskScheduler::GetInstance() {
    static TaskScheduler instance;
    return instance;
}

} // namespace severance::core::concurrency
