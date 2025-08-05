#include "semaphore.hpp"
#include <mutex>

Semaphore::Semaphore(int limit): resLimit_(limit), isExit_(false) {}

void Semaphore::post() {
    if (isExit_) return ;
    std::unique_lock lock(mutex_);
    resLimit_++;
    cond_.notify_all();
}

void Semaphore::wait() {
    if (isExit_) return ;
    std::unique_lock lock(mutex_);
    cond_.wait(lock, [&]() {return resLimit_ > 0;});
    resLimit_--;
    cond_.notify_all();
}
