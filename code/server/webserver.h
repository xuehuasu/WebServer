#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <unordered_map>

#include "../http/httpconn.h"
#include "../log/log.h"
#include "../pool/sqlconnRAII.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../timer/heaptimer.h"
#include "epoller.h"

class WebServer {
public:
    WebServer(int port, int trigMode, int timeoutMS, bool OptLinger, int sqlPort,
              const char* sqlUser, const char* sqlPwd, const char* dbName, int connPoolNum,
              int threadNum, bool openLog, int logLevel, int logQueSize);

    ~WebServer();
    void Start();

private:
    bool InitSocket_();                     // 初始化socket
    void InitEventMode_(int trigMode);      // 初始化事件模式
    void AddClient_(int fd, sockaddr_in addr);  // 添加客户端

    void DealListen_();                     // 处理监听事件
    void DealWrite_(HttpConn* client);      // 处理写事件
    void DealRead_(HttpConn* client);       // 处理读事件

    void SendError_(int fd, const char* info);  // 发送错误信息
    void ExtentTime_(HttpConn* client);         // 延长超时时间
    void CloseConn_(HttpConn* client);          // 关闭连接

    void OnRead_(HttpConn* client);             // 读事件处理
    void OnWrite_(HttpConn* client);            // 写事件处理
    void OnProcess_(HttpConn* client);          // 处理请求

    static int SetFdNonblock(int fd);           // 设置文件描述符非阻塞

    static const int MAX_FD = 65536;            // 最大文件描述符数量
    int port_;                                  // 端口号
    bool openLinger_;                           // 是否开启优雅关闭
    int timeoutMS_;                             // 超时时间
    bool isClose_;                              // 是否关闭
    int listenFd_;                              // 监听的文件描述符
    char* srcDir_;                              // 资源目录

    uint32_t listenEvent_;                      // 监听的文件描述符的事件
    uint32_t connEvent_;                        // 连接的文件描述符的事件

    std::unique_ptr<HeapTimer> timer_;          // 堆定时器
    std::unique_ptr<ThreadPool> threadpool_;    // 线程池
    std::unique_ptr<Epoller> epoller_;          // 事件处理对象
    std::unordered_map<int, HttpConn> users_;   // 用户信息

};

#endif