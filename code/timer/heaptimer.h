#ifndef HEAP_TIMER_H
#define HEAP_TIMER_H

#include <arpa/inet.h>
#include <assert.h>
#include <time.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <queue>
#include <unordered_map>

#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;      // 超时回调函数
typedef std::chrono::high_resolution_clock Clock;   // 高精度时钟
typedef std::chrono::milliseconds MS;              // 毫秒
typedef Clock::time_point TimeStamp;               // 时间戳

struct TimerNode {      // 定时器节点
  int id;               // 定时器id
  TimeStamp expires;    // 超时时间
  TimeoutCallBack cb;   // 超时回调函数
  bool operator<(const TimerNode& t) { return expires < t.expires; }
};

class HeapTimer {
public:
    HeapTimer() { heap_.reserve(64); }

    ~HeapTimer() { clear(); }

    void adjust(int id, int newExpires);

    void add(int id, int timeOut, const TimeoutCallBack& cb);

    void doWork(int id);

    void clear();

    void tick();

    void pop();

    int GetNextTick();

    private:
    void del(size_t i);

    void up(size_t i);

    bool down(size_t index);

    void swapNode(size_t i, size_t j);

    std::vector<TimerNode> heap_;       // 堆数组

    std::unordered_map<int, int> ref_;  // fd到定时器节点的映射
};

#endif