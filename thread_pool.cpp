#include "thread_pool.hpp"
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>

int Thread::static_thread_id_ = 0;

Thread::Thread(ThreadFunc func): 
    func_(func),
    this_thread_id_(static_thread_id_++) {}

u32 Thread::get_id() const {
    return this_thread_id_;
}

void Thread::start() {
    std::thread t(func_, this_thread_id_);
    t.detach();
}

ThreadPool::ThreadPool()
    : thread_cnt_(0)
    , idle_thread_cnt_(0)
    , task_que_threshold_(MAX_TASK_SIZE)
    , init_thread_cnt_(0)
    , is_running_(true)
    , pool_mode_(PoolMode::STATIC)
{}

ThreadPool::~ThreadPool() {
    is_running_ = false;
    std::unique_lock lock(task_que_mutex_);
    not_empty_.notify_all();
    exit_.wait(lock, [&]() {
        return threads_.empty();
    });
}

void ThreadPool::start(size_t init_thread_size) {
    init_thread_cnt_ = init_thread_size;
    for (int i = 0; i < init_thread_size; i++) {
        Thread thread(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
        u32 thread_id = thread.get_id();
        threads_.emplace(thread_id, thread);
    }
    for (int i = 0; i < init_thread_size; i++) {
        threads_[i].start();
    }
    idle_thread_cnt_ = init_thread_size;
}

void ThreadPool::ThreadFunc(const u32 threadId) {
    // LOG_INFO("Start Thread: %d Started", threadId);
    auto lastTaskTime = std::chrono::high_resolution_clock::now();
    while (true) {
        Task task;
        {
            std::unique_lock lock(task_que_mutex_);
            while (task_que_.empty()) {
                if (!is_running_) {
                    // LOG_INFO("Deconstruct: Thread %d Exit", threadId);
                    threads_.erase(threadId);
                    exit_.notify_all();
                    return ;
                }

                if (pool_mode_ == PoolMode::DYNAMIC) {
                    if (std::cv_status::timeout ==
                        not_empty_.wait_for(lock, std::chrono::seconds(1)))
                    {
                        auto cur = std::chrono::high_resolution_clock::now();
                        auto duration = std::chrono::duration_cast<std::chrono::seconds>(cur - lastTaskTime);
                        if (duration.count() >= MAX_IDLE_SECONDS && thread_cnt_ > init_thread_cnt_) {
                            // LOG_INFO("Timeout: Thread %d Exit", threadId);
                            threads_.erase(threadId);
                            --thread_cnt_;
                            --idle_thread_cnt_;
                            exit_.notify_all();
                            return ;
                        }
                    }
                } else {
                    //wait会释放锁, 当前线程并不一定会得到锁,因为要判断任务队列是否为空
                    not_empty_.wait(lock);
                }
            }

            --idle_thread_cnt_;
            task = task_que_.front();
            task_que_.pop();
            not_full_.notify_all();
            if (!task_que_.empty()) {
                not_empty_.notify_all();
            }
        }
        if (task != nullptr) {
            task();
        }
        ++idle_thread_cnt_;
        lastTaskTime = std::chrono::high_resolution_clock::now();
    }
}

void ThreadPool::setMode(PoolMode mode) {
    pool_mode_ = mode;
}
