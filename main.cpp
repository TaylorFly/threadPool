#include "threadPool.hpp"
#include <chrono>
#include <cstdio>
#include <memory>
#include <thread>
#include <iostream>

class MyTask: public Task {
public:
  void run() override {
    std::cout << "tid: " << std::this_thread::get_id() << " begin\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cerr << "tid: " << std::this_thread::get_id() << " end\n";
  }
};

int main() {
  ThreadPool pool;
  pool.start();

  pool.submitTask(std::make_shared<MyTask>());
  pool.submitTask(std::make_shared<MyTask>());
  pool.submitTask(std::make_shared<MyTask>());
  pool.submitTask(std::make_shared<MyTask>());
  
  pool.submitTask(std::make_shared<MyTask>());
  pool.submitTask(std::make_shared<MyTask>());
  pool.submitTask(std::make_shared<MyTask>());
  pool.submitTask(std::make_shared<MyTask>());

  pool.submitTask(std::make_shared<MyTask>());
  getchar();
  return 0;
}
