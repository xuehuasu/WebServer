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
  bool InitSocket_();
  void InitEventMode_(int trigMode);
  void AddClient_(int fd, sockaddr_in addr);

  void DealListen_();
  void DealWrite_(HttpConn* client);
  void DealRead_(HttpConn* client);

  void SendError_(int fd, const char* info);
  void ExtentTime_(HttpConn* client);
  void CloseConn_(HttpConn* client);

  void OnRead_(HttpConn* client);
  void OnWrite_(HttpConn* client);
  void OnProcess(HttpConn* client);

  static const int MAX_FD = 65536;

  static int SetFdNonblock(int fd);

  int port_;
  bool openLinger_;
  int timeoutMS_; /* 毫秒MS */
  bool isClose_;
  int listenFd_;
  char* srcDir_;  //

  uint32_t listenEvent_;  // 监听的文件描述符的事件
  uint32_t connEvent_;    // 连接的文件描述符的事件

  std::unique_ptr<HeapTimer> timer_;         // 定时器
  std::unique_ptr<ThreadPool> threadpool_;   // 线程池对象
  std::unique_ptr<Epoller> epoller_;         // 事件处理对象
  std::unordered_map<int, HttpConn> users_;  // 客户端信息
};

#endif  // WEBSERVER_H