#include <atomic>
#include "semaphore.hpp"
#include "any.hpp"

class Task;

class Result {
public:
    Result(std::shared_ptr<Task>, bool isValid = true);
    ~Result() = default;

    void setVal(Any any);
    Any get();
private:
    Any any_;
    Semaphore sem_;
    std::shared_ptr<Task> task_;
    std::atomic_bool isValid_; //返回值是否有效
};