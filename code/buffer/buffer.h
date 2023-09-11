#ifndef BUFFER_H
#define BUFFER_H
#include <assert.h>
#include <sys/uio.h>
#include <unistd.h>

#include <atomic>
#include <cstring>
#include <iostream>
#include <vector>

class Buffer {
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    /**
     * @brief 可读字节数
    */
    size_t WritableBytes() const;

    /**
     * @brief 可写字节数
    */
    size_t ReadableBytes() const;

    /**
     * @brief 前置字节数
    */
    size_t PrependableBytes() const;

    /**
     * @brief 返回缓冲区头指针
    */
    const char* Peek() const;

    /**
     * @brief 确保缓冲区有足够的空间存放len个字节
    */
    void EnsureWriteable(size_t len);

    /**
     * @brief 更新写指针
    */
    void HasWritten(size_t len);

    /**
     * @brief 读取len个字节
    */
    void Retrieve(size_t len);

    /**
     * @brief 读取缓冲区中[begin, end)之间的数据
    */
    void RetrieveUntil(const char* end);

    /**
     * @brief 重置缓冲区
    */
    void RetrieveAll();

    /**
     * @brief 重置缓冲区并返回字符串
    */
    std::string RetrieveAllToStr();

    /**
     * @brief 返回缓冲区头指针
    */
    const char* BeginWriteConst() const;

    /**
     * @brief 返回缓冲区头指针
    */
    char* BeginWrite();

    /**
     * @brief 添加字符串
    */
    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);

    /**
     * @brief 添加缓冲区
    */
    void Append(const Buffer& buff);

    /**
     * @brief 从fd中读取数据
    */
    ssize_t ReadFd(int fd, int* Errno);

    /**
     * @brief 向fd中写入数据
    */
    ssize_t WriteFd(int fd, int* Errno);

private:
    /**
     * @brief 返回缓冲区头指针
    */
    char* BeginPtr_();

    /**
     * @brief 返回缓冲区头指针
    */
    const char* BeginPtr_() const;

    /**
     * @brief 扩充缓冲区
    */
    void MakeSpace_(size_t len);

private:
    std::vector<char> buffer_; // 缓冲区
    std::atomic<std::size_t> readPos_; // 读指针
    std::atomic<std::size_t> writePos_; // 写指针
};

#endif