#pragma once

#include <mutex>
#include <condition_variable>

class Semaphore {
public:
    Semaphore(int limit = 0);
    ~Semaphore() = default;
    void wait();
    void post();
private:
    int resLimit_;
    std::mutex mutex_;
    std::condition_variable cond_;
};