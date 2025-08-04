#include "threadPool.hpp"
#include "Result.hpp"

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

constexpr int MAX_QUE_THRESHHOLD = INT32_MAX;
constexpr int THREAD_MAX_THREHOLD = 100;
constexpr int THREAD_MAX_IDLE_TIME = 60; // seconds

int Thread::generateId_ = 0;

ThreadPool::ThreadPool()
    : initThreadSize_(4),
      taskCnt_(0),
      taskQueSizeMaxThreshHold_(MAX_QUE_THRESHHOLD),
      mode_(PoolMode::MODE_FIXED),
      isPoolRunning_(false),
      idleThreadSize_(0),
      threadSizeThreshold_(200),
      threadSize_(0)
      {}

void ThreadPool::setMode(PoolMode mode) { 
    if (!checkRunningState()) {
        mode_ = mode; 
    }
}
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

    /**
    * 当前是cached模式
    * 等待执行的task的数量比空闲线程数量更大
    * 不会超过线程的阈值
    */
    if (PoolMode::MODE_CACHED == mode_
        && taskCnt_ > idleThreadSize_
        && threadSize_ < threadSizeThreshold_
    ) {
        std::cout << ">>> create new thread\n";
        //创建新的线程对象
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        int threadId = ptr->getId();
        threads_.emplace(threadId, std::move(ptr));
        //启动线程
        threads_[threadId]->start();
        // 修改线程个数
        threadSize_++;
        idleThreadSize_++;
    }

    return Result(sp);
}

void ThreadPool::start(size_t initThreadSize) {
    // 记录初始线程个数
    initThreadSize_ = initThreadSize;
    threadSize_ = initThreadSize;
    // 创建线程对象
    for (int i = 0; i < initThreadSize_; i++) {
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
        threads_.emplace(ptr->getId(), std::move(ptr));
    }
    for (int i = 0; i < initThreadSize_; i++) {
        threads_[i]->start();
        idleThreadSize_++;
    }
    isPoolRunning_ = true;
}

// 启动线程
void Thread::start() {
    std::thread t(func_, threadId_);
    t.detach();
}

int Thread::getId() const {
    return threadId_;
}

/**
 * threads_中的每个thread会执行这个
 */
void ThreadPool::threadFunc(int threadId) {
    auto lastTime = std::chrono::high_resolution_clock().now();
    while (isPoolRunning_) {
        std::shared_ptr<Task> task;
        {
            std::unique_lock lock(taskQueMtx_);
            std::cerr << "tid: " << std::this_thread::get_id() << " is waiting\n";

            /**
            * 如果是cache模式,需要考虑回收线程的问题
            * 回收线程的情况显然是当前没有任务需要执行
            * 且线程距离上次执行任务的时间超过了线程的存活时间
            * 并且不会超过初始的线程数量,则回收线程
            */
            while (taskQue_.empty()) {
                if (mode_ == PoolMode::MODE_CACHED) {
                    //超时返回
                    if (std::cv_status::timeout == cv_noempty_.wait_for(lock, std::chrono::seconds(1))) {
                        auto now = std::chrono::high_resolution_clock().now();
                        auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
                        if (dur.count() >= THREAD_MAX_IDLE_TIME && threadSize_ > initThreadSize_) {
                            //回收当前线程
                            threads_.erase(threadId);
                            threadSize_--;
                            idleThreadSize_--;
                            std::cout << "thread id: " << std::this_thread::get_id() << " exit!\n";
                            return ;
                        }
                    }
                } else {
                    cv_noempty_.wait(lock);
                }

                if (!isPoolRunning_) {
                    threads_.erase(threadId);
                    cv_exit_.notify_one();
                    std::cout << "thread id: " << std::this_thread::get_id() << " exit!\n";
                    return ;
                }
            }

            --idleThreadSize_;
            task = taskQue_.front();
            taskQue_.pop();
            --taskCnt_;

            if (!taskQue_.empty()) {
                cv_noempty_.notify_all();
            }

            cv_nofull_.notify_all();
        }
        if (task != nullptr) {
            task->exec();
        }
        ++idleThreadSize_;
    }
    cv_exit_.notify_one();
}

void ThreadPool::setThreadSizeThreshold(int threshold) {
    if (checkRunningState()) return ;
    if (mode_ == PoolMode::MODE_CACHED) {
        threadSizeThreshold_ = threshold;
    }
}

bool ThreadPool::checkRunningState() const {
    return isPoolRunning_;
}

ThreadPool::~ThreadPool() {
    isPoolRunning_ = false;
    // 唤醒所有线程处理任务
    cv_noempty_.notify_all();
    //等待线程池中的所有的线程返回
    std::unique_lock lock(taskQueMtx_);
    cv_exit_.wait(lock, [&]() {return threads_.size() == 0;});
}

Thread::Thread(ThreadFunc func) : func_(func), threadId_(generateId_++) {}
Thread::~Thread() {}

void Task::exec() {
    if (result_ != nullptr) {
        result_->setVal(run());
    }
}

void Task::setResult(Result *result) {
    result_ = result;
}

Task::Task(): result_(nullptr){}