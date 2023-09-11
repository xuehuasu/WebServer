#ifndef HTTP_CONN_H
#define HTTP_CONN_H

#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "../pool/sqlconnRAII.h"
#include "httprequest.h"
#include "httpresponse.h"


class HttpConn {
public:
    HttpConn();
    
    ~HttpConn();
    
    void init(int sockFd, const sockaddr_in& addr);
    
    ssize_t read(int* saveErrno);
    
    ssize_t write(int* saveErrno);
    
    void Close();
    
    int GetFd() const;
    
    int GetPort() const;
    
    const char* GetIP() const;
    
    sockaddr_in GetAddr() const;
    
    bool process();
    
    int ToWriteBytes() { return iov_[0].iov_len + iov_[1].iov_len; }
    
    bool IsKeepAlive() const { return request_.IsKeepAlive(); }
    
    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;

private:
    int fd_;                    // socket文件描述符
    struct sockaddr_in addr_;   // 客户端地址
    bool isClose_;              // 是否关闭连接
    
    int iovCnt_;                // 写缓冲区中有多少个iovec
    struct iovec iov_[2];       // 两个iovec，一个是Buffer缓冲区，一个是mmfile
    
    Buffer readBuff_;           // 读缓冲区
    Buffer writeBuff_;          // 写缓冲区
    
    HttpRequest request_;       // 请求
    HttpResponse response_;     // 响应
};

#endif