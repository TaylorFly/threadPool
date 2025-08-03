#include "threadPool.hpp"

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

constexpr int MAX_QUE_THRESHHOLD = 4;
std::mutex outMux;

ThreadPool::ThreadPool()
    : initThreadSize_(4),
      taskCnt_(0),
      taskQueSizeMaxThreshHold_(MAX_QUE_THRESHHOLD),
      mode_(PoolMode::MODE_FIXED) {}

void ThreadPool::setMode(PoolMode mode) { mode_ = mode; }
void ThreadPool::taskQueSizeMaxThreshHold(size_t threshold) {
    taskQueSizeMaxThreshHold_ = threshold;
}

Result ThreadPool::submitTask(std::shared_ptr<Task> sp) {
    std::unique_lock lock(taskQueMtx_);
    if (!cv_nofull_.wait_for(lock, std::chrono::seconds(1),
                             [&]() { return taskQue_.size() < taskQueSizeMaxThreshHold_; })) {
        // taskQue_ is full after 1 seconds, so wait_for return false
        std::cerr << "taskQue is full, submit task fail\n";
    }
    std::cout << "submit success\n";
    taskQue_.emplace(sp);
    taskCnt_++;
    cv_noempty_.notify_all();

    /*
      return task->getResult()
      这种更是不可以的,因为task在返回之后就被析构了, result同样被析构了
    */
}

void ThreadPool::start(int initThreadSize) {
    // 记录初始线程个数
    initThreadSize_ = initThreadSize;
    // 创建线程对象
    for (int i = 0; i < initThreadSize_; i++) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this));
        this->threads_.emplace_back(std::move(ptr));
    }
    for (int i = 0; i < initThreadSize_; i++) {
        threads_[i]->start();
    }
}

// 启动线程
void Thread::start() {
    std::thread t(func_);
    t.detach();
}

/**
 * threads_中的每个thread会执行这个
 */
void ThreadPool::threadFunc() {
    while (true) {
        std::shared_ptr<Task> task;
        {
            std::unique_lock lock(taskQueMtx_);
            std::cout << "tid: " << std::this_thread::get_id() << " try to get task: \n";
            cv_noempty_.wait(lock, [&]() { return !taskQue_.empty(); });
            task = taskQue_.front();
            std::cout << "tid: " << std::this_thread::get_id() << " success to get task: \n";
            taskQue_.pop();
            taskCnt_--;

            if (taskQue_.size() > 0) {
                cv_noempty_.notify_all();
            }

            cv_nofull_.notify_all();
        }
        if (task != nullptr) {
            task->run();
        }
    }
}

ThreadPool::~ThreadPool() {}
Thread::Thread(ThreadFunc func) : func_(func) {}
Thread::~Thread() {}
