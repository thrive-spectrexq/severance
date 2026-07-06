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

TaskScheduler::~TaskScheduler() {
    Shutdown();
}

void TaskScheduler::Initialize() {
    SEV_CORE_INFO("TaskScheduler initializing.");
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    if (m_SchedulerThread.joinable()) {
        return;
    }
    
    if (m_StopSource.stop_requested()) {
        m_StopSource = std::stop_source();
    }
    
    m_SchedulerThread = std::jthread([this](std::stop_token stoken) {
        SchedulerLoop(stoken);
    });
}

void TaskScheduler::Shutdown() {
    m_StopSource.request_stop();
    m_Condition.notify_all();
    
    if (m_SchedulerThread.joinable()) {
        // jthread joins automatically, but we can clear it to be explicit
        m_SchedulerThread = std::jthread{};
    }
    
    std::unique_lock<std::mutex> lock(m_QueueMutex);
    std::priority_queue<ScheduledTask, std::vector<ScheduledTask>, std::greater<ScheduledTask>> emptyQueue;
    std::swap(m_Tasks, emptyQueue);
    
    SEV_CORE_INFO("TaskScheduler shut down.");
}

void TaskScheduler::SchedulerLoop(std::stop_token stoken) {
    while (!stoken.stop_requested()) {
        std::function<void()> taskToExecute;
        std::chrono::milliseconds recurringInterval{0};

        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            if (m_Tasks.empty()) {
                m_Condition.wait(lock, stoken, [this] { return !m_Tasks.empty(); });
            } else {
                auto now = std::chrono::steady_clock::now();
                auto topTaskTime = m_Tasks.top().executionTime;
                
                if (now >= topTaskTime) {
                    auto topTask = m_Tasks.top();
                    m_Tasks.pop();
                    taskToExecute = topTask.task;
                    recurringInterval = topTask.interval;
                } else {
                    m_Condition.wait_until(lock, stoken, topTaskTime, [this, topTaskTime] {
                        return !m_Tasks.empty() && m_Tasks.top().executionTime < topTaskTime;
                    });
                }
            }
        }

        if (stoken.stop_requested()) {
            break;
        }

        if (taskToExecute) {
            ThreadPool::GetInstance().EnqueueTask(taskToExecute);

            if (recurringInterval.count() > 0) {
                auto nextTime = std::chrono::steady_clock::now() + recurringInterval;
                std::unique_lock<std::mutex> lock(m_QueueMutex);
                m_Tasks.push({nextTime, taskToExecute, recurringInterval});
                m_Condition.notify_one();
            }
        }
    }
}

} // namespace severance::core::concurrency
