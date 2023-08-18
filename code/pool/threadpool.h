#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <assert.h>

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class ThreadPool {
 public:
  explicit ThreadPool(size_t threadCount = 8) : pool_(std::make_shared<Pool>()) {
    assert(threadCount > 0);
    for (size_t i = 0; i < threadCount; i++) {
      // std::thread([pool = pool_] {...}).detach();
      // 这是用lambda表达式，将pool_复制一份，使得表达式内部可以修改pool值

      std::thread([pool = pool_] {  // 注意这是一个线程池中的一个线程
        std::unique_lock<std::mutex> locker(pool->mtx);  // 创建并获取锁
        while (true) {
          if (!pool->tasks.empty()) {
            auto task = std::move(pool->tasks.front());
            pool->tasks.pop();
            locker.unlock();  // 解锁，让线程池中其它线程执行
            task();           // 业务，具体是什么看调用方
            locker.lock();    // 这个锁会在析构的时候自动释放
          } else if (pool->isClosed)
            break;
          else
            pool->cond.wait(locker);  // 没有任务，等待唤醒
        }

      })
          .detach();
    }
  }

  ThreadPool() = default;

  ThreadPool(ThreadPool&&) = default;

  ~ThreadPool() {
    if (static_cast<bool>(pool_)) {
      {
        std::lock_guard<std::mutex> locker(pool_->mtx);  // 加锁，阻塞其它线程读写工作队列
        pool_->isClosed = true;
      }
      // 线程池结束之前会执行工作队列中剩余的工作
      pool_->cond.notify_all();  // 唤醒阻塞等待的线程
    }
  }

  template <class F>
  void AddTask(F&& task) {
    {
      std::lock_guard<std::mutex> locker(pool_->mtx);  // 独占工作队列，离开{}自动释放
      pool_->tasks.emplace(std::forward<F>(task));
    }
    pool_->cond.notify_one();  // 通知一个线程
  }

 private:
  struct Pool {
    std::mutex mtx;  // 工作队列锁
    std::condition_variable cond;
    bool isClosed;
    std::queue<std::function<void()>> tasks;  // 工作队列
  };
  std::shared_ptr<Pool> pool_;
};

#endif
