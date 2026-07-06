#pragma once

#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <type_traits>
#include <memory>
#include <stdexcept>
#include <stop_token>
#include <concepts>
#include <chrono>

namespace severance::core::concurrency {

class ThreadPool {
public:
    static ThreadPool& GetInstance();

    void Initialize(size_t numThreads = std::thread::hardware_concurrency());
    void Shutdown();

    // Modern C++20 approach using concepts and perfect forwarding
    template<std::invocable F>
    auto EnqueueTask(F&& f) -> std::future<std::invoke_result_t<F>>;

private:
    ThreadPool() = default;
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    std::vector<std::jthread> m_Workers;
    std::queue<std::function<void()>> m_Tasks;

    std::mutex m_QueueMutex;
    std::condition_variable_any m_Condition;
    std::stop_source m_StopSource;
};

template<std::invocable F>
auto ThreadPool::EnqueueTask(F&& f) -> std::future<std::invoke_result_t<F>> {
    using return_type = std::invoke_result_t<F>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(std::forward<F>(f));
    std::future<return_type> res = task->get_future();
    
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        if (m_StopSource.stop_requested()) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        m_Tasks.emplace([task]() { (*task)(); });
    }
    m_Condition.notify_one();
    return res;
}

class TaskScheduler {
public:
    static TaskScheduler& GetInstance();
    
    void Initialize();
    void Shutdown();

    template<std::invocable F>
    void ScheduleDelayedTask(std::chrono::milliseconds delay, F&& f);

    template<std::invocable F>
    void ScheduleRecurringTask(std::chrono::milliseconds interval, F&& f);

private:
    TaskScheduler() = default;
    ~TaskScheduler();

    TaskScheduler(const TaskScheduler&) = delete;
    TaskScheduler& operator=(const TaskScheduler&) = delete;

    struct ScheduledTask {
        std::chrono::steady_clock::time_point executionTime;
        std::function<void()> task;
        std::chrono::milliseconds interval{0};

        bool operator>(const ScheduledTask& other) const {
            return executionTime > other.executionTime;
        }
    };

    std::jthread m_SchedulerThread;
    std::priority_queue<ScheduledTask, std::vector<ScheduledTask>, std::greater<ScheduledTask>> m_Tasks;
    std::mutex m_QueueMutex;
    std::condition_variable_any m_Condition;
    std::stop_source m_StopSource;

    void SchedulerLoop(std::stop_token stoken);
};

template<std::invocable F>
void TaskScheduler::ScheduleDelayedTask(std::chrono::milliseconds delay, F&& f) {
    auto execTime = std::chrono::steady_clock::now() + delay;
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_Tasks.push({execTime, std::forward<F>(f), std::chrono::milliseconds(0)});
    }
    m_Condition.notify_one();
}

template<std::invocable F>
void TaskScheduler::ScheduleRecurringTask(std::chrono::milliseconds interval, F&& f) {
    auto execTime = std::chrono::steady_clock::now() + interval;
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        m_Tasks.push({execTime, std::forward<F>(f), interval});
    }
    m_Condition.notify_one();
}

} // namespace severance::core::concurrency
