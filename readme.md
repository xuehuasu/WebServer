# WebServer

* 利用vector实现自动增长的缓冲区
* 利用缓冲区和队列实现异步日志系统
* 基于小根堆实现定时器，关闭超时的连接
* 利用正则与有限状态机解析HTTP请求报文
* 使用线程池+非阻塞socket+epoll(ET)实现Reactor模式的高并发处理请求
* 利用实现数据库连接池，减少数据库连接建立与关闭的开销，实现了用户注册登录功能


## 环境
* Linux
* C++14
* MySql

## 项目启动

```bash
make
cd bin 
./server
```

## 压力测试
![image](https://github.com/xuehuasu/WebServer/assets/81012806/deeae76b-c3e4-46eb-8b7e-3d4f254e67df)

```bash
./webbench-1.5/webbench -c clientNum -t Time http://ip:port/
```
## 项目目录
```bash
.
├── code          // 项目源码
│   ├── buffer
│   ├── build
│   ├── http
│   ├── log
│   ├── main.cpp
│   ├── makefile
│   ├── pool
│   ├── server
│   └── timer
|—— bin          // 项目可执行文件
|—— log          // 项目日志
|—— objs         // 项目编译文件
├── Makefile     // 项目makefile
├── readme.md    // 项目说明
├── resources    // 项目资源
└── webbench-1.5 // 压力测试工具
```

# 感谢:  [markparticle](https://github.com/markparticle/WebServer)
