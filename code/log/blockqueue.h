#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <sys/time.h>

#include <condition_variable>
#include <deque>
#include <mutex>

template <class T>
class BlockDeque {
public:
    /**
     * 构造函数
    */
    explicit BlockDeque(size_t MaxCapacity = 1024);

    /**
     * 析构函数
    */
    ~BlockDeque();

    /**
     * 清空队列
    */
    void clear();

    /**
     * 判断队列是否为空
    */
    bool empty();
    
    /**
     * 判断队列是否已满
    */
    bool full();

    /**
     * 关闭队列
    */
    void Close();

    /**
     * 返回队列大小
    */
    size_t size();

    /**
     * 返回队列容量
    */
    size_t capacity();

    /**
     * 返回队首元素
    */
    T front();

    /**
     * 返回队尾元素
    */
    T back();

    /**
     * 在队尾添加元素
    */
    void push_back(const T &item);

    /**
     * 在队首添加元素
    */
    void push_front(const T &item);

    /**
     * 弹出队首元素
    */
    bool pop(T &item);

    /**
     * 弹出队首元素
     * @param timeout 超时时间
    */
    bool pop(T &item, int timeout);

    /**
     * 刷新队列
    */
    void flush(); 

private:
    std::deque<T> deq_; // 双端队列
    size_t capacity_; // 队列容量
    std::mutex mtx_; // 互斥锁
    bool isClose_; // 队列是否关闭
    std::condition_variable condConsumer_; // 消费者条件变量
    std::condition_variable condProducer_; // 生产者条件变量
};

/**
 * 构造函数
*/
template <class T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity) : capacity_(MaxCapacity) {
    assert(MaxCapacity > 0);
    isClose_ = false;
}

/**
 * 析构函数
*/
template <class T>
BlockDeque<T>::~BlockDeque() {
    Close();
};

/**
 * 清空队列
*/
template <class T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> locker(mtx_);
    deq_.clear();
}

/**
 * 判断队列是否为空
*/
template <class T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.empty();
}

/**
 * 判断队列是否已满
*/
template <class T>
bool BlockDeque<T>::full() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size() >= capacity_;
}

/**
 * 关闭队列
*/
template <class T>
void BlockDeque<T>::Close() {
    {
        std::lock_guard<std::mutex> locker(mtx_);
        deq_.clear();
        isClose_ = true;
    }
    condProducer_.notify_all();
    condConsumer_.notify_all();
};

/**
 * 返回队列大小
*/
template <class T>
size_t BlockDeque<T>::size() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.size();
}

/**
 * 返回队列容量
*/
template <class T>
size_t BlockDeque<T>::capacity() {
    std::lock_guard<std::mutex> locker(mtx_);
    return capacity_;
}

/**
 * 返回队首元素
*/
template <class T>
T BlockDeque<T>::front() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.front();
}

/**
 * 返回队尾元素
*/
template <class T>
T BlockDeque<T>::back() {
    std::lock_guard<std::mutex> locker(mtx_);
    return deq_.back();
}

/**
 * 在队尾添加元素
*/
template <class T>
void BlockDeque<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while (deq_.size() >= capacity_) {
        condProducer_.wait(locker);
    }
    deq_.push_back(item);
    condConsumer_.notify_one();
}

/**
 * 在队首添加元素
*/
template <class T>
void BlockDeque<T>::push_front(const T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while (deq_.size() >= capacity_) {
        condProducer_.wait(locker);
    }
    deq_.push_front(item);
    condConsumer_.notify_one();
}

/**
 * 弹出队首元素
*/
template <class T>
bool BlockDeque<T>::pop(T &item) {
    std::unique_lock<std::mutex> locker(mtx_);
    while (deq_.empty()) {
        condConsumer_.wait(locker);
        if (isClose_) {
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

/**
 * 弹出队首元素
*/
template <class T>
bool BlockDeque<T>::pop(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(mtx_);
    while (deq_.empty()) {
        if (condConsumer_.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout) {
            return false;
        }
        if (isClose_) {
            return false;
        }
    }
    item = deq_.front();
    deq_.pop_front();
    condProducer_.notify_one();
    return true;
}

/**
 * 刷新队列
*/
template <class T>
void BlockDeque<T>::flush() {
    condConsumer_.notify_one();
}


#endif