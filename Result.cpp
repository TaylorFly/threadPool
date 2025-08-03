#include "Result.hpp"
#include <memory>
#include <utility>

Result::Result(std::shared_ptr<Task> task, bool isValid) 
    : isValid_(isValid), task_(task)
{
    
}

Any Result::get() {
    if (!isValid_) {
        return "";
    }
    sem_.wait();
}

void Result::setVal(Any any) {

}