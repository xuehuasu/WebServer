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

            std::thread([pool = pool_] {  // 复制线程池的指针，使得线程可以访问线程池
                std::unique_lock<std::mutex> locker(pool->mtx); 
                while (true) {
                    if (!pool->tasks.empty()) {
                        auto task = std::move(pool->tasks.front());
                        pool->tasks.pop();
                        locker.unlock(); 
                        task();           // 业务，具体是什么看调用方
                        locker.lock(); 
                    } else if (pool->isClosed)
                        break;
                    else {
                        pool->cond.wait(locker);  // 没有任务，等待唤醒
                    }
                }
            }).detach();
        }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool&&) = default;

    ~ThreadPool() {
        if (static_cast<bool>(pool_)) {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx); 
                pool_->isClosed = true;
            }
            pool_->cond.notify_all(); 
        }
    }

    template <class F>
    void AddTask(F&& task) {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task));
        }
        pool_->cond.notify_one();
    }

private:
    struct Pool {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> pool_;
};

#endif