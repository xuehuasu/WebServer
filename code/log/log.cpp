#include "log.h"

/**
 * @brief 构造函数
*/
Log::Log() {
    lineCount_ = 0;
    isAsync_ = false;
    writeThread_ = nullptr;
    deque_ = nullptr;
    toDay_ = 0;
    fp_ = nullptr;
}

/**
 * @brief 析构函数
*/
Log::~Log() {
    if (writeThread_ && writeThread_->joinable()) {
        while (!deque_->empty()) { // 一直等待直到队列为空
            deque_->flush();
        };
        deque_->Close();
        writeThread_->join();
    }
    if (fp_) {
        std::lock_guard<std::mutex> locker(mtx_);
        fflush(fp_);
        fclose(fp_);
    }
}

/**
 * @brief 获取日志等级
*/
int Log::GetLevel() {
    std::lock_guard<std::mutex> locker(mtx_);
    return level_;
}

/**
 * @brief 设置日志等级
*/
void Log::SetLevel(int level) {
    std::lock_guard<std::mutex> locker(mtx_);
    level_ = level;
}

/**
 * @brief 判断日志系统是否开启
*/
bool Log::IsOpen() { return isOpen_; }

/**
 * @brief 添加日志等级标题
*/
void Log::AppendLogLevelTitle_(int level) {
    switch (level) {
        case 0:
            buff_.Append("[debug]: ", 9);
            break;
        case 1:
            buff_.Append("[info]: ", 8);
            break;
        case 2:
            buff_.Append("[warn]: ", 8);
            break;
        case 3:
            buff_.Append("[error]: ", 9);
            break;
        default:
            buff_.Append("[info]: ", 8);
            break;
    }
}

/**
 * @brief 初始化
*/
void Log::Init(int level = 1, const char* path, const char* suffix, int maxQueueCapacity) {
    isOpen_ = true;
    level_ = level;
    if (maxQueueCapacity > 0) {
        isAsync_ = true;
        if (!deque_) {// 为了防止多线程同时初始化deque_，导致deque_的构造函数被调用多次
            std::unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>);
            deque_ = std::move(newDeque);

            std::unique_ptr<std::thread> NewThread(new std::thread(FlushLogThread));
            writeThread_ = std::move(NewThread);
        }
    } else {
        isAsync_ = false;
    }

    lineCount_ = 0;

    time_t timer = time(nullptr);
    struct tm* sysTime = localtime(&timer);
    struct tm t = *sysTime;
    path_ = path;
    suffix_ = suffix;
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", path_, t.tm_year + 1900,
             t.tm_mon + 1, t.tm_mday, suffix_);
    toDay_ = t.tm_mday;

    {
        std::lock_guard<std::mutex> locker(mtx_);
        buff_.RetrieveAll();
        if (fp_) {
            Flush();
            fclose(fp_);
        }
        fp_ = fopen(fileName, "a");
        if (fp_ == nullptr) {
            mkdir(path_, 0777);
            fp_ = fopen(fileName, "a");
        }
        assert(fp_ != nullptr);
    }
}

/**
 * @brief 获取日志实例
*/
Log* Log::Instance() {
    static Log inst;
    return &inst;
}

/**
 * @brief 刷新日志系统的线程
*/
void Log::FlushLogThread() {
    Log::Instance()->AsyncWrite_();
}

/**
 * @brief 异步写日志
*/
void Log::AsyncWrite_() {
    std::string str = "";
    while (deque_->pop(str)) {
        std::lock_guard<std::mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

/**
 * @brief 写日志
*/
void Log::Write(int level, const char* format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t t_sec = now.tv_sec;
    struct tm* sysTime = localtime(&t_sec);
    struct tm t = *sysTime;
    va_list vaList;
    // 创建新日志文件
    if (toDay_ != t.tm_mday || (lineCount_ && (lineCount_ % MAX_LINES == 0))) {
        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
        if (toDay_ != t.tm_mday) {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        } else {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (lineCount_ / MAX_LINES),
                     suffix_);
        }
        std::unique_lock<std::mutex> locker(mtx_);
        Flush();
        fclose(fp_);
        fp_ = fopen(newFile, "a");
        assert(fp_ != nullptr);
    }
    {
        std::unique_lock<std::mutex> locker(mtx_);
        lineCount_++;
        int n = snprintf(buff_.BeginWrite(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                         t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min,
                         t.tm_sec, now.tv_usec);
        buff_.HasWritten(n);
        AppendLogLevelTitle_(level);

        va_start(vaList, format);
        int m = vsnprintf(buff_.BeginWrite(), buff_.WritableBytes(), format, vaList);
        va_end(vaList);
        buff_.HasWritten(m);
        buff_.Append("\n\0", 2);

        if (isAsync_ && deque_ && !deque_->full()) {
            deque_->push_back(buff_.RetrieveAllToStr());
        } else {
            fputs(buff_.Peek(), fp_);
            buff_.RetrieveAll();
        }
    }
}

/**
 * @brief 刷新缓冲区
*/
void Log::Flush() {
    if (isAsync_) {
        deque_->flush();
    }
    fflush(fp_);
}