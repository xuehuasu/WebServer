#ifndef LOG_H
#define LOG_H

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <mutex>
#include <string>
#include <thread>

#include "../buffer/buffer.h"
#include "blockqueue.h"

class Log {
public:
    /**
     * @brief 初始化日志系统
    */
    void Init(int level, const char* path = "../log", const char* suffix = ".log",
 int maxQueueCapacity = 1024);
    
    /**
     * @brief 获取日志系统的单例
    */
    static Log* Instance();

    /**
     * @brief 刷新日志系统的线程
    */
    static void FlushLogThread();

    /**
     * @brief 写日志
    */
    void Write(int level, const char* format, ...);

    /**
     * @brief 刷新日志
    */
    void Flush();

    /**
     * @brief 获取日志等级
    */
    int GetLevel();

    /**
     * @brief 设置日志等级
    */
    void SetLevel(int level);

    /**
     * @brief 判断日志系统是否开启
    */
    bool IsOpen() ;

private:
    /**
     * @brief 构造函数
    */
    Log();

    /**
     * @brief 添加日志等级标题
    */
    void AppendLogLevelTitle_(int level);

    /**
     * @brief 析构函数
    */
    virtual ~Log();

    /**
     * @brief 异步写日志
    */
    void AsyncWrite_();

private:
    static const int LOG_PATH_LEN = 256; // 日志路径长度
    static const int LOG_NAME_LEN = 256; // 日志名称长度
    static const int MAX_LINES = 50000;  // 最大行数

    const char* path_;   // 日志路径
    const char* suffix_; // 日志后缀

    int MAX_LINES_; // 最大行数

    int lineCount_; // 行数
    int toDay_;     // 今天

    bool isOpen_; // 是否开启

    Buffer buff_; // 缓冲区
    int level_;   // 日志等级
    bool isAsync_; // 是否异步

    FILE* fp_; // 文件指针
    std::unique_ptr<BlockDeque<std::string>> deque_; // 阻塞队列
    std::unique_ptr<std::thread> writeThread_; // 写日志的线程
    std::mutex mtx_; // 互斥锁
};

/**
 * @brief 日志宏定义
*/
#define LOG_BASE(level, format, ...)                 \
    do {                                             \
        Log* log = Log::Instance();                  \
        if (log->IsOpen() && log->GetLevel() <= level) { \
            log->Write(level, format, ##__VA_ARGS__); \
            log->Flush();                            \
        }                                            \
    } while (0);

#define LOG_DEBUG(format, ...)              \
    do {                                    \
        LOG_BASE(0, format, ##__VA_ARGS__)  \
    } while (0);
#define LOG_INFO(format, ...)               \
    do {                                    \
        LOG_BASE(1, format, ##__VA_ARGS__)  \
    } while (0);
#define LOG_WARN(format, ...)               \
    do {                                    \
        LOG_BASE(2, format, ##__VA_ARGS__)  \
    } while (0);
#define LOG_ERROR(format, ...)              \
    do {                                    \
        LOG_BASE(3, format, ##__VA_ARGS__)  \
    } while (0);


#endif