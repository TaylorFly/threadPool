#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <unordered_map>
#include <queue>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>
// #include "logger.hpp"

constexpr int MAX_THREAD_SIZE  = INT32_MAX;
constexpr int MAX_TASK_SIZE    = 64;
constexpr int MAX_IDLE_SECONDS = 60;

using u32 = unsigned int;

enum class PoolMode {
    STATIC, DYNAMIC
};

class Thread {
public:
    using ThreadFunc = std::function<void(int)>;
    static int static_thread_id_;
    Thread() = default;
    Thread(ThreadFunc); 
    u32 get_id() const;
    void start();
private:
    ThreadFunc func_;
    u32 this_thread_id_;
};

class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    void start(size_t init_thread_size);


    template<typename Func, typename... Args>
    auto submitTask(Func&& func, Args&& ...args) -> std::future<decltype(func(args...))> {
        using RType = decltype(func(args...));
        std::shared_ptr<std::packaged_task<RType()>> task = std::make_shared<std::packaged_task<RType()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...)
        );
        std::future<RType> result = task->get_future();
        std::unique_lock lock(task_que_mutex_);
        if (!not_full_.wait_for(lock, std::chrono::seconds(1), [&]() {
            return task_que_.size() < task_que_threshold_;
        })) {
            // LOG_INFO("Task Queue is Full, Submit Fail");
            task = std::make_shared<std::packaged_task<RType()>>(
                [](){return RType();}
            );
            (*task)();
            return task->get_future();
        }
        // LOG_INFO("New Task Emplace To Task Queue");
        task_que_.emplace([task](){(*task)();});
        not_empty_.notify_all();

        if (pool_mode_ == PoolMode::DYNAMIC
            && thread_cnt_ < MAX_THREAD_SIZE
            && task_que_.size() > idle_thread_cnt_) {
            Thread thread(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
            threads_.emplace(thread.get_id(), thread);
            thread.start();
            ++idle_thread_cnt_;
            ++thread_cnt_;
        }

        return result;
    }

    void ThreadFunc(u32 threadId);
    void setMode(PoolMode mode);
private:
    PoolMode pool_mode_;
    std::unordered_map<u32, Thread> threads_;
    std::atomic_int thread_cnt_;
    std::atomic_int idle_thread_cnt_;
    std::atomic_int init_thread_cnt_;

    using Task = std::function<void()>;
    std::queue<Task> task_que_;
    std::mutex task_que_mutex_;
    std::atomic_int task_que_threshold_;

    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    std::condition_variable exit_;

    std::atomic_bool is_running_;
};

#endif