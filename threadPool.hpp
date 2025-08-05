#ifndef THREADPOLL_H
#define THREADPOLL_H

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <unordered_map>

#include "Result.hpp"

// abstract class for task
class Task {
public:
  Task();

  virtual ~Task() = default;
  // virtual 不能与模板一起使用
  virtual Any run() = 0;
  void exec();
  void setResult(Result *result);

private:
  Result* result_;
};

enum class PoolMode { MODE_FIXED, MODE_CACHED };

class Thread {
  using ThreadFunc = std::function<void(int)>;

public:
  Thread(ThreadFunc);
  ~Thread();
  // 启动线程
  void start();
  int getId() const;

private:
  ThreadFunc func_;
  static int generateId_;
  int threadId_;
};

class ThreadPool {
public:
  ThreadPool();
  ~ThreadPool();

  void setMode(PoolMode mode);
  void taskQueSizeMaxThreshHold(size_t threshold);
  Result submitTask(std::shared_ptr<Task> sp);
  void start(size_t initThreadSize = std::thread::hardware_concurrency());

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

private:
  void threadFunc(int threadId);

private:
  //虽然可以使用threads_.size()获取线程数量,但是由于不是线程安全的,因此需要一个threadSize_
  std::unordered_map<int, std::unique_ptr<Thread>> threads_;
  size_t initThreadSize_; // initial thread number
  //线程数量上限阈值
  int threadSizeThreshold_;
  void setThreadSizeThreshold(int threshold);
  //线程数量
  std::atomic_int threadSize_;
  //空闲线程数量
  std::atomic_int idleThreadSize_;

  std::queue<std::shared_ptr<Task>> taskQue_;
  std::atomic_uint taskCnt_;
  int taskQueSizeMaxThreshHold_;

  std::mutex taskQueMtx_;
  // 两个的好处是可以避免伪唤醒
  //cv_exit:等待线程资源全部回收
  std::condition_variable cv_noempty_, cv_nofull_, cv_exit_;

  PoolMode mode_;

  // 防止用户在启动了之后还setmode,增加一个bool用来判断
  std::atomic_bool isPoolRunning_;

  bool checkRunningState() const;
  //空闲线程的数量
};

#endif