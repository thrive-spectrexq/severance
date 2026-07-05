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
    m_Stop = false;

    for (size_t i = 0; i < numThreads; ++i) {
        m_Workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->m_QueueMutex);
                    this->m_Condition.wait(lock, [this] { return this->m_Stop || !this->m_Tasks.empty(); });
                    
                    if (this->m_Stop && this->m_Tasks.empty()) {
                        return;
                    }
                    
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
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        if (m_Stop) return; // Already stopped
        m_Stop = true;
    }
    
    m_Condition.notify_all();
    
    for (std::thread& worker : m_Workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    m_Workers.clear();
    SEV_CORE_INFO("ThreadPool shut down.");
}

TaskScheduler& TaskScheduler::GetInstance() {
    static TaskScheduler instance;
    return instance;
}

} // namespace severance::core::concurrency
