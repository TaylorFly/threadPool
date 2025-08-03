#include "semaphore.hpp"
#include <mutex>

Semaphore::Semaphore(int limit): resLimit_(limit) {}

void Semaphore::post() {
    std::unique_lock lock(mutex_);
    resLimit_++;
    cond_.notify_all();
}

void Semaphore::wait() {
    std::unique_lock lock(mutex_);
    cond_.wait(lock, [&]() {return resLimit_ > 0;});
    resLimit_--;
    cond_.notify_all();
}
