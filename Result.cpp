#include "Result.hpp"
#include <memory>
#include <utility>
#include "threadPool.hpp"
#include <string>
Result::Result(std::shared_ptr<Task> task, bool isValid) 
    : isValid_(isValid), task_(task)
{
    task_->setResult(this);
}

Any Result::get() {
    if (!isValid_) {
        std::string ret = "";
        return ret;
    }
    sem_.wait();
    return std::move(any_);
}

void Result::setVal(Any any) {
    this->any_ = std::move(any);
    sem_.post();
}