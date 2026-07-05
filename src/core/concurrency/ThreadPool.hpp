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

namespace severance::core::concurrency {

class ThreadPool {
public:
    static ThreadPool& GetInstance();

    void Initialize(size_t numThreads = std::thread::hardware_concurrency());
    void Shutdown();

    template<typename F, typename... Args>
    auto EnqueueTask(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

private:
    ThreadPool() = default;
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    std::vector<std::thread> m_Workers;
    std::queue<std::function<void()>> m_Tasks;

    std::mutex m_QueueMutex;
    std::condition_variable m_Condition;
    bool m_Stop = false;
};

template<typename F, typename... Args>
auto ThreadPool::EnqueueTask(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        [func = std::bind(std::forward<F>(f), std::forward<Args>(args)...)]() mutable {
            return func();
        }
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(m_QueueMutex);
        if (m_Stop) {
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
