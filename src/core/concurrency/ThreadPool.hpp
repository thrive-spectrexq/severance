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
    
    template<typename F, typename... Args>
    auto ScheduleTask(F&& f, Args&&... args) {
        return ThreadPool::GetInstance().EnqueueTask(std::forward<F>(f), std::forward<Args>(args)...);
    }
};

} // namespace severance::core::concurrency
