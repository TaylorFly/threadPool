#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

// 异步写日志队列
template<class T>
class LockQueue {
public:
    void push(const T &data) {
        std::lock_guard lock(mutex_);
        queue_.push(data);
        cv.notify_one();
    }
    T pop() {
        std::unique_lock lock(mutex_);
        while (queue_.empty()) {
            // wait会释放锁,不会导致push进程的阻塞.
            cv.wait(lock);
        }
        T data = queue_.front();
        queue_.pop();
        return data;
    }
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable cv;
};