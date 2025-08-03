#ifndef THREADPOLL_H
#define THREADPOLL_H

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#include "Result.hpp"

// abstract class for task
class Task {
public:
  //virtual 不能与模板一起使用
  virtual void run() = 0;
};

enum class PoolMode { MODE_FIXED, MODE_CACHED };

class Thread {
  using ThreadFunc = std::function<void()>;

public:
  Thread(ThreadFunc);
  ~Thread();
  // 启动线程
  void start();

private:
  ThreadFunc func_;
};

class ThreadPool {
public:
  ThreadPool();
  ~ThreadPool();

  void setMode(PoolMode mode);
  void taskQueSizeMaxThreshHold(size_t threshold);
  Result submitTask(std::shared_ptr<Task> sp);
  void start(int initThreadSize = 4);
  
  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;
  
private:
  void threadFunc();

private:
  std::vector<std::unique_ptr<Thread>> threads_;
  size_t initThreadSize_; // initial thread number

  std::queue<std::shared_ptr<Task>> taskQue_;
  std::atomic_uint taskCnt_;
  int taskQueSizeMaxThreshHold_;

  std::mutex taskQueMtx_;
  // 两个的好处是可以避免伪唤醒
  std::condition_variable cv_noempty_, cv_nofull_;

  PoolMode mode_;
};

#endif