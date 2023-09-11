#include "buffer.h"

/**
 * @brief 构造函数
 * @param initBuffSize 缓冲区大小
 */
Buffer::Buffer(int initBuffSize) : buffer_(initBuffSize), readPos_(0), writePos_(0) {}

/**
 * @brief 返回可读字节数
 */
size_t Buffer::ReadableBytes() const { return writePos_ - readPos_; }

/**
 * @brief 返回可写字节数
 */
size_t Buffer::WritableBytes() const { return buffer_.size() - writePos_; }

/**
 * @brief 返回预留空间字节数
 */
size_t Buffer::PrependableBytes() const { return readPos_; }

/**
 * @brief 返回缓冲区头指针
 */
const char* Buffer::Peek() const { return BeginPtr_() + readPos_; }

/**
 * @brief 确保缓冲区有足够的空间存放len个字节
 */
void Buffer::EnsureWriteable(size_t len) {
  if (WritableBytes() < len) {
    MakeSpace_(len);
  }
  assert(WritableBytes() >= len);
}

/**
 * @brief 返回可写指针
 */
char* Buffer::BeginWrite() { return BeginPtr_() + writePos_; }

/**
 * @brief 返回可读指针
 */
const char* Buffer::BeginWriteConst() const { return BeginPtr_() + writePos_; }

/**
 * @brief 从缓冲区中取出数据
 * @param len 数据长度
 */
void Buffer::Retrieve(size_t len) {
  assert(len <= ReadableBytes());
  readPos_ += len;
}

/**
 * @brief 从缓冲区中取出数据直到end
 * @param end 数据结束位置
 */
void Buffer::RetrieveUntil(const char* end) {
  assert(Peek() <= end);
  Retrieve(end - Peek());
}

/**
 * @brief 重置缓冲区
 */
void Buffer::RetrieveAll() {
  bzero(&buffer_[0], buffer_.size());
  readPos_ = 0;
  writePos_ = 0;
}

/**
 * @brief 重置缓冲区并返回字符串
 */
std::string Buffer::RetrieveAllToStr() {
  std::string str(Peek(), ReadableBytes());
  RetrieveAll();
  return str;
}

/**
 * @brief 添加数据
 * @param str 字符串
 */
void Buffer::Append(const std::string& str) { Append(str.data(), str.length()); }

/**
 * @brief 添加数据
 * @param str 字符串
 * @param len 字符串长度
 */
void Buffer::Append(const char* str, size_t len) {
  assert(str);
  EnsureWriteable(len);
  std::copy(str, str + len, BeginWrite());
  HasWritten(len);
}

/**
 * @brief 添加数据
 * @param data 数据
 * @param len 数据长度
 */
void Buffer::Append(const void* data, size_t len) {
  assert(data);
  Append(static_cast<const char*>(data), len);
}

/**
 * @brief 添加数据
 * @param buff 缓冲区
 */
void Buffer::Append(const Buffer& buff) { Append(buff.Peek(), buff.ReadableBytes()); }

/**
 * @brief 更新写指针
 * @param len 更新长度
 */
void Buffer::HasWritten(size_t len) { writePos_ += len; }

/**
 * @brief 返回缓冲区头指针
 */
char* Buffer::BeginPtr_() { return &*buffer_.begin(); }

/**
 * @brief 返回缓冲区头指针
 */
const char* Buffer::BeginPtr_() const { return &*buffer_.begin(); }

/**
 * @brief 扩大缓冲区
 * @param len 扩大的长度
 */
void Buffer::MakeSpace_(size_t len) {
  if (WritableBytes() + PrependableBytes() < len) {
    buffer_.resize(writePos_ + len + 1);
  } else {
    size_t readable = ReadableBytes();
    std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
    readPos_ = 0;
    writePos_ = readPos_ + readable;
    assert(readable == ReadableBytes());
  }
}

/**
 * @brief 从fd中读取数据
 * @param fd 文件描述符
 * @param saveErrno 错误码
*/
ssize_t Buffer::ReadFd(int fd, int* saveErrno) {
  char buff[65535];
  struct iovec iov[2];
  const size_t writable = WritableBytes();
  /* 分散读， 保证数据全部读完 */
  iov[0].iov_base = BeginPtr_() + writePos_;
  iov[0].iov_len = writable;
  iov[1].iov_base = buff;
  iov[1].iov_len = sizeof(buff);

  const ssize_t len = readv(fd, iov, 2);
  if (len < 0) {
    *saveErrno = errno;
  } else if (static_cast<size_t>(len) <= writable) {
    writePos_ += len;  // 直接读
  } else {
    writePos_ = buffer_.size();
    Append(buff, len - writable);  // 将buff的数据添加到末尾
  }
  return len;
}

/**
 * @brief 向fd中写入数据
 * @param fd 文件描述符
 * @param saveErrno 错误码
*/
ssize_t Buffer::WriteFd(int fd, int* saveErrno) {
  size_t readSize = ReadableBytes();
  ssize_t len = write(fd, Peek(), readSize);
  if (len < 0) {
    *saveErrno = errno;
    return len;
  }
  readPos_ += len;
  return len;
}


