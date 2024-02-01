# 开发更新日志

## 2024/02/01

优化唤醒机制，不再用哈希表存储等待中的协程任务，而是由任务自身重新加入调度队列的方式；
使用无锁队列实现调度队列，提高并发量，经过apache压力测试，并发性能提高

```bash
Server Software:        http
Server Hostname:        192.168.144.135
Server Port:            8020

Document Path:          /http
Document Length:        11 bytes

Concurrency Level:      200
Time taken for tests:   0.923 seconds
Complete requests:      10000
Failed requests:        0
Keep-Alive requests:    10000
Total transferred:      880000 bytes
HTML transferred:       110000 bytes
Requests per second:    10838.46 [#/sec] (mean)
Time per request:       18.453 [ms] (mean)
Time per request:       0.092 [ms] (mean, across all concurrent requests)
Transfer rate:          931.43 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   0.2      0       9
Processing:     2   17  12.1     16     142
Waiting:        0   17  12.1     16     141
Total:          2   17  12.2     16     143

Percentage of the requests served within a certain time (ms)
  50%     16
  66%     18
  75%     20
  80%     20
  90%     22
  95%     24
  98%     31
  99%     95
 100%    143 (longest request)
```

> 第二版测试数据：单epoll线程单协程线程
> 短连接 1100 PRS   (800->1100)
> 长连接 11000 RPS (5000->11000)
> 可以发现，对唤醒机制的优化效果明显（在长连接中唤醒是频繁操作），而对短连接优化提升较少
> 同时，实验发现存在意外退出程序的风险，待进一步检测

## 2024/01/30

经过apache压力测试发现，使用epoll独立线程的方法存在问题（由于其不直接处理任何函数，每一个到来的请求都会唤醒epoll，没有达到io多路复用的效果），决定使用强继承的方式实现基于协程的reactor.(RPS为800)。

```bash
Server Software:        http
Server Hostname:        192.168.144.131
Server Port:            8020

Document Path:          /http
Document Length:        11 bytes

Concurrency Level:      20
Time taken for tests:   1.326 seconds
Complete requests:      1000
Failed requests:        0
Total transferred:      83000 bytes
HTML transferred:       11000 bytes
Requests per second:    754.05 [#/sec] (mean)
Time per request:       26.523 [ms] (mean)
Time per request:       1.326 [ms] (mean, across all concurrent requests)
Transfer rate:          61.12 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    1   1.1      1      17
Processing:     2   25  10.8     28      52
Waiting:        1   17  10.1     15      48
Total:          4   26  11.2     29      54

Percentage of the requests served within a certain time (ms)
  50%     29
  66%     33
  75%     34
  80%     36
  90%     39
  95%     42
  98%     48
  99%     49
 100%     54 (longest request)
```

测试发现存在丢包问题，调试发现为意外关闭socket所导致，问题日志如下

``` bash
2024-01-30 22:50:41 56917 Proc_0 [system] [TRACE] HttpServer.cpp:16 handleClient 192.168.144.1:65519
2024-01-30 22:50:41 56917 Proc_0 [system] [TRACE] Hook.cpp:91 task(5445) hook recv(fd = 5345) in coroutine.
2024-01-30 22:50:41 56917 Proc_0 [system] [TRACE] Hook.cpp:107 task(5445) try hook recv(fd = 5345). timeout = 120000
2024-01-30 22:50:41 56917 Proc_0 [system] [TRACE] Poller.cpp:168 epoll_ctl op = MOD fd = 5345 event = { IN  }
2024-01-30 22:50:41 56917 Proc_0 [system] [FATAL] Poller.cpp:177 epoll_ctl op =MOD fd =5345
```

> 上述日志中的5445号协程一直未被调度器重新唤醒，它在尝试resv报文后挂起，但是紧接着该socket的epoll_ctl为MOD而不是ADD，经排查为删除fd时没有正确设置状态，修改后短连接可以正常处理高并发。

TODO: 发现在长连接时会丢失报文，原因不详。（apr_pollset_poll: 在一个非套接字上尝试了一个操作。   (730038)）

> 第一版测试数据：单epoll线程单协程线程
> 短连接 700 RPS
> 长连接 5000 RPS

## 2024/01/29

使用双指针优化bytearray，并为socket添加相关接口，实现了echo server的测试

```bash
book@100ask:~$ telnet 127.0.0.1 8020
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
d
d
fdfd
fdfd
fdsfadfas
fdsfadfas


fsdfasd
fsdfasd
dddddddddddddddddddddd
dddddddddddddddddddddd
sdafsdfsad
sdafsdfsad
fsadfsadfasd
fsadfsadfasd
^]
telnet> q
Connection closed.

```

> 使用telnet测试echo服务器，可以看到服务器可以正确的接受数据并返回

## 2024/01/27

epoll的event没有设置边沿触发，设置后依然存在问题

``` bash
2024-01-28 01:03:26     125537 main     [system] [TRACE] EventLoop.cpp:19       EventLoop create wakeFd = 4
2024-01-28 01:03:26     125537 main     [system] [TRACE] Poller.cpp:174 epoll_ctl op = ADD fd = 4 event = { IN  }
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Address.cpp:53 family = 2 type = 0 protocol = 0
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:255   task(1) hook socket, domain = 2 type = 526338 protocal = 0
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:264   task(1) hook socket, returns (fd = 5).
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:276   task(1) hook connect(fd = 5) in coroutine.
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:296   task(1) hook connect(fd = 5) completed immediately.
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:91    task(1) hook send(fd = 5) in coroutine.
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:107   task(1) try hook send(fd = 5). timeout = NULL
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:91    task(1) hook recvfrom(fd = 5) in coroutine.
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:107   task(1) try hook recvfrom(fd = 5). timeout = NULL
2024-01-28 01:03:26     125568 Proc_0   [root] [INFO] test_socket.cpp:12        get address: 180.101.50.188:0
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:255   task(1) hook socket, domain = 2 type = 1 protocal = 0
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:264   task(1) hook socket, returns (fd = 5).
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:276   task(1) hook connect(fd = 5) in coroutine.
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Poller.cpp:174 epoll_ctl op = ADD fd = 5 event = { OUT  }
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:173   task(2) hook sleep(seconds = 3) in coroutine.
2024-01-28 01:03:26     125537 main     [system] [TRACE] Poller.cpp:140 1 events happened
2024-01-28 01:03:26     125537 main     [system] [TRACE] IoChannel.cpp:24       fd = 5 { OUT }
2024-01-28 01:03:26     125537 main     [system] [TRACE] Task.cpp:27    try wake task(1).
2024-01-28 01:03:26     125568 Proc_0   [root] [INFO] test_socket.cpp:29        connnect 180.101.50.188:80 success.
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:91    task(1) hook send(fd = 5) in coroutine.
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:107   task(1) try hook send(fd = 5). timeout = NULL
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:91    task(1) hook recv(fd = 5) in coroutine.
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:107   task(1) try hook recv(fd = 5). timeout = NULL
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Poller.cpp:174 epoll_ctl op = MOD fd = 5 event = { IN OUT  }
2024-01-28 01:03:26     125537 main     [system] [TRACE] Poller.cpp:140 1 events happened
2024-01-28 01:03:26     125537 main     [system] [TRACE] IoChannel.cpp:24       fd = 5 { OUT }
2024-01-28 01:03:26     125537 main     [system] [TRACE] Task.cpp:27    try wake task(1).
2024-01-28 01:03:26     125537 main     [system] [TRACE] Poller.cpp:140 1 events happened
2024-01-28 01:03:26     125537 main     [system] [TRACE] IoChannel.cpp:24       fd = 5 { IN OUT }
2024-01-28 01:03:26     125537 main     [system] [TRACE] Task.cpp:27    try wake task(1).
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:107   task(1) try hook recv(fd = 5). timeout = NULL
2024-01-28 01:03:26     125568 Proc_0   [root] [INFO] test_socket.cpp:50        HTTP/1.0 200 OK
Accept-Ranges: bytes
Cache-Control: no-cache
Content-Length: 9508
Content-Type: text/html
Date: Sat, 27 Jan 2024 17:03:26 GMT
P3p: CP=" OTI DSP COR IVA OUR IND COM "
P3p: CP=" OTI DSP COR IVA OUR IND COM "
Pragma: no-cache
Server: BWS/1.1
Set-Cookie: BAIDUID=14F557E4F8B503C839895969ABDB0E33:FG=1; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com
Set-Cookie: BIDUPSID=14F557E4F8B503C839895969ABDB0E33; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com
Set-Cookie: PSTM=1706375006; expires=Thu, 31-Dec-37 23:55:55 GMT; max-age=2147483647; path=/; domain=.baidu.com
Set-Cookie: BAIDUID=14F557E4F8B503C846053F7A7204F296:FG=1; max-age=31536000; expires=Sun, 26-Jan-25 17:03:26 GMT; domain=.baidu.com; path=/; version=1; comment=bd
Traceid: 170637500606236856428292563864224621885
Vary: Accept-Encoding
X-Ua-Compatible: IE=Edge,chrome=1

<!DOCTYPE html><html><head><meta http-equiv="Content-Type" content="text/html; charset=UTF-8"><meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1"><meta content="always" name="referrer"><meta name="description" content="全球领先的中文搜索引擎、致力于让网民更便捷地获取信息，找到所求。百度超过千亿的中文网页数据库，可以瞬间找到相关的搜索结果。"><link rel="shortcut icon" href="//www.baidu.com/favicon.ico" type="image/x-icon">
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Poller.cpp:174 epoll_ctl op = DEL fd = 5 event = {  }
2024-01-28 01:03:26     125568 Proc_0   [system] [TRACE] Hook.cpp:422   task(1) hook close(fd = 5)
2024-01-28 01:03:31     125537 main     [system] [TRACE] Poller.cpp:174 epoll_ctl op = DEL fd = 4 event = {  }
28 01:03:31     125568 Proc_0   [system] [TRACE] EventLoop.cpp:57       EventLoop:tickle()
28 01:03:31     125568 Proc_0   [system] [TRACE] Hook.cpp:91    task(2) hook write(fd = 4) in coroutine.
28 01:03:31     125568 Proc_0   [system] [TRACE] Hook.cpp:96    return unblocked.
28 01:03:31     125568 Proc_0   [system] [ERROR] EventLoop.cpp:62       EventLoop::tickle() writes -1 bytes instead of 8
```

> 在退出阶段，日志的打印也出现了问题，~~疑似内存溢出导致~~，发现在日志打印中fd=4事件在EventLoop:tickle前被删除(wakeFd的删除只在eventloop的析构中发生，也就是说eventloop在tickle前析构)，因此导致了内存的错误。

以下是正确的退出顺序

```bash
2024-01-28 01:16:23     4146 Proc_0     [system] [TRACE] Poller.cpp:174 epoll_ctl op = DEL fd = 5 event = {  }
2024-01-28 01:16:23     4146 Proc_0     [system] [TRACE] Hook.cpp:422   task(1) hook close(fd = 5)
2024-01-28 01:16:26     4146 Proc_0     [system] [TRACE] EventLoop.cpp:57       EventLoop:tickle()
2024-01-28 01:16:26     4146 Proc_0     [system] [TRACE] Hook.cpp:91    task(2) hook write(fd = 4) in coroutine.
2024-01-28 01:16:26     4146 Proc_0     [system] [TRACE] Hook.cpp:96    return unblocked.
2024-01-28 01:16:26     4144 main       [system] [TRACE] Poller.cpp:140 1 events happened
2024-01-28 01:16:26     4144 main       [system] [TRACE] IoChannel.cpp:24       fd = 4 { IN }
2024-01-28 01:16:27     4144 main       [system] [TRACE] Poller.cpp:174 epoll_ctl op = DEL fd = 4 event = {  }
```

TODO: 优雅退出

## 2024/01/26

构建完成TcpServer，使用EchoServer做简单测试，低并发情况下运行正常，高并发时存在问题，日志如下

```bash
2024-01-26 17:07:38 13996 Proc_0 [system] [TRACE] Address.cpp:53 family = 2 type = 0 protocol = 0
2024-01-26 17:07:38 13996 Proc_0 [system] [TRACE] Hook.cpp:236 task(1) hook socket, domain = 2 type = 1 protocal = 0
2024-01-26 17:07:38 13996 Proc_0 [system] [TRACE] Hook.cpp:245 task(1) hook socket, returns (fd = 5).
2024-01-26 17:07:38 13996 Proc_0 [system] [INFO] Server.cpp:60  name=echo ssl=0 server bind success: 1
2024-01-26 17:07:38 13996 Proc_0 [system] [TRACE] Hook.cpp:236 task(2) hook socket, domain = 2 type = 1 protocal = 0
2024-01-26 17:07:38 13996 Proc_0 [system] [TRACE] Hook.cpp:245 task(2) hook socket, returns (fd = 6).
2024-01-26 17:07:38 13996 Proc_0 [system] [TRACE] Hook.cpp:90 task(2) hook accept(fd = 5) in coroutine.
2024-01-26 17:07:38 13996 Proc_0 [system] [TRACE] Hook.cpp:105 task(2) try hook accept(fd = 5). timeout = NULL
2024-01-26 17:07:38 13996 Proc_0 [system] [TRACE] Poller.cpp:175 epoll_ctl op = ADD fd = 5 event = { IN  }
2024-01-26 17:07:40 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:40 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:40 13993 main [system] [TRACE] IoChannel.cpp:42 read task(2) trigger.
2024-01-26 17:07:40 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:40 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:40 13996 Proc_0 [system] [TRACE] Hook.cpp:105 task(2) try hook accept(fd = 5). timeout = NULL
2024-01-26 17:07:40 13996 Proc_0 [system] [TRACE] Hook.cpp:236 task(2) hook socket, domain = 2 type = 1 protocal = 0
2024-01-26 17:07:40 13996 Proc_0 [system] [TRACE] Hook.cpp:245 task(2) hook socket, returns (fd = 8).
2024-01-26 17:07:40 13996 Proc_0 [system] [TRACE] Hook.cpp:90 task(2) hook accept(fd = 5) in coroutine.
2024-01-26 17:07:40 13996 Proc_0 [system] [TRACE] Hook.cpp:105 task(2) try hook accept(fd = 5). timeout = NULL
2024-01-26 17:07:40 13996 Proc_0 [system] [TRACE] Poller.cpp:175 epoll_ctl op = MOD fd = 5 event = { IN  }
2024-01-26 17:07:40 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:40 13996 Proc_0 [system] [TRACE] Hook.cpp:90 task(3) hook recvmsg(fd = 7) in coroutine.
2024-01-26 17:07:40 13996 Proc_0 [system] [TRACE] Hook.cpp:105 task(3) try hook recvmsg(fd = 7). timeout = 120000
2024-01-26 17:07:40 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:40 13993 main [system] [TRACE] IoChannel.cpp:42 read task(2) trigger.
2024-01-26 17:07:40 13996 Proc_0 [system] [TRACE] Poller.cpp:175 epoll_ctl op = ADD fd = 7 event = { IN  }
2024-01-26 17:07:40 13996 Proc_0 [system] [TRACE] Hook.cpp:105 task(2) try hook accept(fd = 5). timeout = NULL
2024-01-26 17:07:40 13996 Proc_0 [system] [TRACE] Poller.cpp:175 epoll_ctl op = MOD fd = 5 event = { IN  }
2024-01-26 17:07:42 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:42 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:42 13993 main [system] [TRACE] IoChannel.cpp:42 read task(3) trigger.
2024-01-26 17:07:42 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:42 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:42 13996 Proc_0 [system] [TRACE] Hook.cpp:146 cancel timer
2024-01-26 17:07:42 13996 Proc_0 [system] [TRACE] Hook.cpp:105 task(3) try hook recvmsg(fd = 7). timeout = 120000
2024-01-26 17:07:42 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:42 13996 Proc_0 [system] [TRACE] Hook.cpp:90 task(3) hook recvmsg(fd = 7) in coroutine.
2024-01-26 17:07:42 13996 Proc_0 [system] [TRACE] Hook.cpp:105 task(3) try hook recvmsg(fd = 7). timeout = 120000
2024-01-26 17:07:42 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:42 13996 Proc_0 [system] [TRACE] Poller.cpp:175 epoll_ctl op = MOD fd = 7 event = { IN  }
2024-01-26 17:07:44 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:44 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:44 13993 main [system] [TRACE] IoChannel.cpp:42 read task(3) trigger.
2024-01-26 17:07:44 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:44 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:44 13996 Proc_0 [system] [TRACE] Hook.cpp:146 cancel timer
2024-01-26 17:07:44 13996 Proc_0 [system] [TRACE] Hook.cpp:105 task(3) try hook recvmsg(fd = 7). timeout = 120000
2024-01-26 17:07:44 13996 Proc_0 [system] [TRACE] Hook.cpp:90 task(3) hook recvmsg(fd = 7) in coroutine.
2024-01-26 17:07:44 13996 Proc_0 [system] [TRACE] Hook.cpp:105 task(3) try hook recvmsg(fd = 7). timeout = 120000
2024-01-26 17:07:44 13996 Proc_0 [system] [TRACE] Poller.cpp:175 epoll_ctl op = MOD fd = 7 event = { IN  }
2024-01-26 17:07:44 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:44 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:44 13993 main [system] [TRACE] IoChannel.cpp:42 read task(3) trigger.
2024-01-26 17:07:44 13996 Proc_0 [system] [TRACE] Hook.cpp:146 cancel timer
2024-01-26 17:07:44 13996 Proc_0 [system] [TRACE] Hook.cpp:105 task(3) try hook recvmsg(fd = 7). timeout = 120000
2024-01-26 17:07:44 13996 Proc_0 [system] [TRACE] Poller.cpp:175 epoll_ctl op = MOD fd = 7 event = { IN  }
2024-01-26 17:07:45 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:45 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:45 13993 main [system] [TRACE] IoChannel.cpp:42 read task(3) trigger.
2024-01-26 17:07:45 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:45 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:45 13996 Proc_0 [system] [TRACE] Hook.cpp:146 cancel timer
2024-01-26 17:07:45 13996 Proc_0 [system] [TRACE] Hook.cpp:105 task(3) try hook recvmsg(fd = 7). timeout = 120000
2024-01-26 17:07:45 13996 Proc_0 [system] [TRACE] Hook.cpp:90 task(3) hook recvmsg(fd = 7) in coroutine.
2024-01-26 17:07:45 13996 Proc_0 [system] [TRACE] Hook.cpp:105 task(3) try hook recvmsg(fd = 7). timeout = 120000
2024-01-26 17:07:45 13996 Proc_0 [system] [TRACE] Poller.cpp:175 epoll_ctl op = MOD fd = 7 event = { IN  }
2024-01-26 17:07:45 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:45 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:45 13993 main [system] [TRACE] IoChannel.cpp:42 read task(3) trigger.
2024-01-26 17:07:45 13996 Proc_0 [system] [TRACE] Hook.cpp:146 cancel timer
2024-01-26 17:07:45 13996 Proc_0 [system] [TRACE] Hook.cpp:105 task(3) try hook recvmsg(fd = 7). timeout = 120000
2024-01-26 17:07:45 13996 Proc_0 [system] [TRACE] Poller.cpp:175 epoll_ctl op = MOD fd = 7 event = { IN  }
2024-01-26 17:07:46 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:46 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:46 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:46 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:46 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:46 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
2024-01-26 17:07:46 13993 main [system] [TRACE] Poller.cpp:141 1 events happened
2024-01-26 17:07:46 13993 main [system] [TRACE] IoChannel.cpp:24 IN 
.
.
.
.
.
.
```

> 上图的报错信息为，后面一直循环在事件触发，但是没有协程任务被唤醒（channel中没有绑定协程）:原因是每次tigger后都置空了task指针，不再置空指针则解决该问题。修改置空的原因是在之前考虑到可能一个channel绑定的task可能会变动，但在编写好server后并实际运行后发现，在该协程设计中，**一个socket对应一个fd对应一个channel同时也对应一个协程任务**，在这4个对应关系的约束下需要考虑线程安全问题，TODO:

TODO: 发现close存在问题

## 2024/01/25

底层的协程已经简单的封装完毕，现在需要封装Server模块，一个Server可以监听多个socket，每个socket对应一条连接，在我的设计中每条连接对应一个协程，因此，也需要将连接封装成类并维护成员Task.

## 2024/01/24

针对之前的connect问题，采取无视重复设置，使用connect方法的是TCP连接，其需要读取三次握手中最后一次握手的常规报文，而后该fd还是会持续读取事件的，因此也不考虑将读事件关闭；但是需要在cannel的trigger方法中触发协程唤醒后将task指向空指针，以防在后续的意外读事件发生时栈溢出。

## 2024/01/23

仿照libgo实现语法糖

``` c++
// 化简后的协程创建
co[]
{
    sleep(2);
}

// 原版的协程创建
co_sched->createTask(
    []
    {
        sleep(2);
    }
)
```

目前针对channel的设计仍然有问题——没有主动创建channel。虽然channel被管理于poller中，但channel的创建不在poller中。
通过修改数据结构为数组，在构造函数中创建32个channel，当不够时再增加。

****

目前hook也可以正常运行，但仍然存在错误

```c++
2024-01-23 17:22:15     36372 Proc_0    [system] [TRACE] Hook.cpp:286       fd = 4 create socket
2024-01-23 17:22:15     36372 Proc_0    [root] [INFO] test_hook.cpp:40      begin connect
2024-01-23 17:22:15     36372 Proc_0    [system] [TRACE] Hook.cpp:300       fd = 4 connect
2024-01-23 17:22:15     36372 Proc_0    [system] [TRACE] Hook.cpp:335       run in coroutine
2024-01-23 17:22:15     36372 Proc_0    [system] [TRACE] Poller.cpp:175     epoll_ctl op = ADD fd = 4 event = { OUT  }
2024-01-23 17:22:15     36370 main      [system] [TRACE] Poller.cpp:141     1 events happened
2024-01-23 17:22:15     36370 main      [system] [TRACE] IoChannel.cpp:24   OUT 
2024-01-23 17:22:15     36370 main      [system] [TRACE] Poller.cpp:141     1 events happened
2024-01-23 17:22:15     36372 Proc_0    [system] [TRACE] Hook.cpp:360       coroutine waked
2024-01-23 17:22:15     36370 main      [system] [TRACE] IoChannel.cpp:24   OUT 
2024-01-23 17:22:15     36372 Proc_0    [root] [INFO] test_hook.cpp:42      connect rt=0 errno = 115 strerror = Operation now in progress
2024-01-23 17:22:15     36372 Proc_0    [system] [TRACE] Hook.cpp:134       fd = 4 try function : send
2024-01-23 17:22:15     36370 main      [system] [TRACE] IoChannel.cpp:24   OUT 
2024-01-23 17:22:15     36370 main      [system] [TRACE] Poller.cpp:141     1 events happened
2024-01-23 17:22:15     36370 main      [system] [TRACE] IoChannel.cpp:24   OUT 
2024-01-23 17:22:15     36370 main      [system] [TRACE] Poller.cpp:141     1 events happened
2024-01-23 17:22:15     36370 main      [system] [TRACE] IoChannel.cpp:24   OUT 
2024-01-23 17:22:15     36372 Proc_0    [system] [TRACE] Hook.cpp:134       fd = 4 try function : recv
2024-01-23 17:22:15     36370 main      [system] [TRACE] Poller.cpp:141     1 events happened
2024-01-23 17:22:15     36372 Proc_0    [system] [ERROR] IoChannel.cpp:62   addEvent assert fd = 4 event = {IN } channel.event = {IN OUT }
2024-01-23 17:22:15     36372 Proc_0    [root] [ERROR] IoChannel.cpp:63
ASSERTION: !(m_events & event)
backtrace:
output/test_hook(+0x43f9b) [0x55a295001f9b]
output/test_hook(+0x44896) [0x55a295002896]
output/test_hook(+0x4af5e) [0x55a295008f5e]
output/test_hook(+0x3cc47) [0x55a294ffac47]
output/test_hook(+0x176b8) [0x55a294fd56b8]
output/test_hook(recv+0x2d) [0x55a294fd5c6d]
output/test_hook(+0xd112) [0x55a294fcb112]
output/test_hook(+0x32012) [0x55a294ff0012]
/lib/x86_64-linux-gnu/libc.so.6(+0x58680) [0x7f97483b4680]

test_hook: IoChannel.cpp:63: bool lim_webserver::IoChannel::addEvent(lim_webserver::IoEvent): Assertion `!(m_events & event)' failed.
Aborted (core dumped)
```

> 从以上调试信息可以发现，是由于我在addEvent中添加的重复事件报错所导致，将其注释则问题解决
> 原因是在hook中connect本身也是一个读操作，后续的recv又是一个读操作，导致了操作的重复注册。
> **TODO:** 所以目前需要考虑的问题是是否需要对这部分工作进行优化（换方式实现connect）
**已2024/01/24解决**

## 2024/01/22

原本的思路是协程相关的线程工作于独立的线程,即GMP模型在其自己工作线程;网络IO的IO多路复用模型运行与独立的线程。这种设计模式线程分工明确,但是存在一大设计难点就是在设计非阻塞IO时,传统的同步表现异步的方式是采用忙询法,即重复调用io操作，直到查询到结果；而采用了协程的方法，那么利用好协程的特性是非常必要的，即在io无果后hold协程，在合适的时机即io多路复用接受到了信息后唤醒协程。
这一机制对分离线程模式的设计造成了非常大的挑战，主要原因在于协程调度的线程同步方式为条件变量，对io信息无感知；而对io信息有感知的IO多路复用模型运行与别的线程。因此想要达到协程的唤醒，需要绑定回调到IO多路复用中,通过IO线程唤醒协程。
同时这种设计方法将IO检测到的行为添加到协程调度后，会调度到多个Processor中，可能导致行为处理的无序。

****

最理想状态下有

+ 主从Reactor模型和GMP模型对应，即主Reactor与Scheduler对应，从Reactor与Processor对应。
+ 一切行为基于协程，服务器的逻辑符合同步从而不使用回调
+ 协程的调度和Reactor对用户不可见，只需要创建服务并编写运行逻辑。
+ 回调只在协程task中

总结来说，当程序运行时，至少运行三条线程：1条timer,1条scheduler,1条processor.

此时的一个重要问题是，IO多路复用该如何嵌入

+ 独立线程
+ 替换协程中的条件变量

> 需要注意的是除了IO多路复用的epoll_wait或poll_wait部分，其他所有任务都应当在协程中运行

****

为了消除回调，则需要知道各个回调的运行时机

+ OnConnection: accept后执行
+ OnRead: read后执行
+ OnWrite: write后执行
+ OnClose: close后执行

理想状态下的echo-server代码

```c++
class Echo: public TcpServer
{
    void handleClient(Socket::ptr client) override
    {
        // 业务逻辑
    }
}

int main()
{
    auto server = Echo::Create(8080,"myserver");
    server->start();
}
```

最理想状态下的客户端

```c++
void func()
{

    auto client = TCPClient::Create(Address("localhost",8080),"myclient");

    client->connect();
    // 原OnConnect 回调
    LOG<<"connect!";

    client->send("hello world!");
    auto res = client->read();
    // 原OnMessage 回调
    LOG<<res;

    client->close();
}

int main()
{
    co func();  // 在协程中运行逻辑
}
```

****

IoChannel的机制提供一层抽象，所有修改event的操作都需要先操作channel后实际更新到epoll，这种方法提供了很好的安全；在设计net协程时，需要几个准则：

+ 一个fd只可由一个eventloop监听，一个eventloop可以监听多个fd
+ 一个channel对应一个fd，存储读写操作的协程
+ 一个socket对应一个fd，是对网络io行为的封装

同时，也需要理清哪些组件是底层的，哪些组件是上层的（底层与上层的界限就是是否可被hook的函数调用）

+ 底层: eventloop, poller, iochannel, fdinfo
+ 上层: socket, server, client

其中，底层的组件poller是eventloop的一部分，无法被hook直接调用，那么在hook函数中如何将当前的工作协程记录到iochannel中则尤为重要。因为在hook中需要先获得channel，然后修改channel的event属性，调用loop的update方法，则可以把协程记录到io复用线程中。在原本的muduo设计中，channel由connection建立并进行动态管理，而channel是server的操作单元。在该设计中，由于不需要绑定回调，所以并没有设计connection类，而是使用socket直接进行封装，换句话说，在本系统的socket负责了muduo中connection的操作，而之前提到过socket是上层行为不可被hook的函数使用，因此对channel的操作需要独立出来，考虑的方案如下：

1. 添加channel manager单例管理
2. 修改loop和poller的函数形参，将接受channel改为接受fd

使用哪种方案需要考虑channel对原系统和现系统的意义

+ 原系统: 记录回调与状态，通过状态的不同调用不同的回调，由于回调的设定需要从内部的接口一直暴露到最外层，因此channel作为一个桥梁在各个组件层次传递（编译时绑定）
+ 现系统: 记录状态，通过状态的不同调用不同的协程，由于回调已不存在（以同步的方式存在于协程中），所以不需要再有这种将channel实体暴露在外面的设计；
而现要考虑的是如何将协程设置到channel中，当程序在协程中运行时，协程的信息对自己是已知的，也就是说要考虑的是如何获取net的信息，至此又回到了上面的议题——“如何设计channel的操作”的两种方案，由于在poller中已经管理了其连接的channel，添加全局的单例管理类会占用额外的内存开销，并且由于回调需求的消失，该方案则取消，使用上述的方案二：修改函数传参。

至此，对于修改event行为的设计则又成为设计要点——之前提到过，channel是对epoll的event属性的记录。原本的channel操作接口为

+ updateChannel: 将传入的Channel更新到poller中并修改ep行为，根据Channel的状态修改ep行为
+ removeChannel: 将传入的Channel从poller中移出，根据Channel的状态删除ep行为
+ hasChannel: 查询是否有channel，由于channel不再存在于外部，因此该方法可废弃

Channel的进一步

## 2024/01/19

完成coroutine与timer的结合，为hook做准备

```bash

notify
back
2024-01-19 21:26:17     108105 Proc_0   [test_co] [INFO] test_coroutine.cpp:41        Hello world
2024-01-19 21:26:17     108105 Proc_0   [test_co] [INFO] test_coroutine.cpp:42        before hold
condition wait
tickle
condition back
back
condition wait
tickle
condition back
back
condition wait
tickle
condition back
back
2024-01-19 21:26:19     108105 Proc_0   [test_co] [INFO] test_coroutine.cpp:44        after hold
condition wait
tickle
condition back
back
condition wait
tickle
condition back
back

```

## 2024/01/17

完成EventLoop与协程的配合，待Accepter和connector的完成后联调

## 2024/01/14

完成定时器模块

```bash
2024-01-14 21:38:33     34236 main      [test] [INFO] test_timer.cpp:20       start
2024-01-14 21:38:34     34237 Timer     [test] [INFO] test_timer.cpp:18       hello world
2024-01-14 21:38:34     34237 Timer     [test] [INFO] test_timer.cpp:18       hello world
2024-01-14 21:38:34     34237 Timer     [test] [INFO] test_timer.cpp:18       hello world
2024-01-14 21:38:34     34237 Timer     [test] [INFO] test_timer.cpp:18       hello world
2024-01-14 21:38:34     34237 Timer     [test] [INFO] test_timer.cpp:18       hello world
2024-01-14 21:38:34     34237 Timer     [test] [INFO] test_timer.cpp:18       hello world
2024-01-14 21:38:34     34237 Timer     [test] [INFO] test_timer.cpp:18       hello world
2024-01-14 21:38:34     34237 Timer     [test] [INFO] test_timer.cpp:18       hello world
2024-01-14 21:38:34     34237 Timer     [test] [INFO] test_timer.cpp:18       hello world
2024-01-14 21:38:34     34237 Timer     [test] [INFO] test_timer.cpp:18       hello world
```

采用独立线程来处理定时任务

已知问题：对于循环打印的计时器任务，存在一定问题

```bash
2024-01-14 21:46:11     34541 Timer     [test] [INFO] test_timer.cpp:28       hello world 200
2024-01-14 21:46:11     34541 Timer     [test] [INFO] test_timer.cpp:28       hello world 200
2024-01-14 21:46:11     34541 Timer     [test] [INFO] test_timer.cpp:28       hello world 200
2024-01-14 21:46:11     34541 Timer     [test] [INFO] test_timer.cpp:28       hello world 200
2024-01-14 21:46:11     34541 Timer     [test] [INFO] test_timer.cpp:28       hello world 200
2024-01-14 21:46:11     34541 Timer     [test] [INFO] test_timer.cpp:28       hello world 200
2024-01-14 21:46:11     34541 Timer     [test] [INFO] test_timer.cpp:28       hello world 200
2024-01-14 21:46:11     34541 Timer     [test] [INFO] test_timer.cpp:28       hello world 200
2024-01-14 21:46:11     34541 Timer     [test] [INFO] test_timer.cpp:28       hello world 200
2024-01-14 21:46:11     34541 Timer     [test] [INFO] test_timer.cpp:28       hello world 200
���&=V14 21:46:11       34541 Timer     [test] [UNKNOWN] test_timer.cpp:28    hello world 200
���&=V14 21:46:11       34541 Timer     [test] [UNKNOWN] test_timer.cpp:28    hello world 200
���&=V14 21:46:11       34541 Timer     [test] [UNKNOWN] test_timer.cpp:28    hello world 200
���&=V14 21:46:11       34541 Timer     [test] [UNKNOWN] test_timer.cpp:28    hello world 200
���&=V14 21:46:11       34541 Timer     [test] [UNKNOWN] test_timer.cpp:28    hello world 200
���&=V14 21:46:11       34541 Timer     [test] [UNKNOWN] test_timer.cpp:28    hello world 200
���&=V14 21:46:11       34541 Timer     [test] [UNKNOWN] test_timer.cpp:28    hello world 200
���&=V14 21:46:11       34541 Timer     [test] [UNKNOWN] test_timer.cpp:28    hello world 200
���&=V14 21:46:11       34541 Timer     [test] [UNKNOWN] test_timer.cpp:28    hello world 200
���&=V14 21:46:11       34541 Timer     [test] [UNKNOWN] test_timer.cpp:28    hello world 200
```

> 对于以上实验现象，推测可能的原因为 程序结束时析构顺序混乱导致
> **TODO:** 需要实现优雅退出，来管理各个组件的析构顺序

## 2024/01/13

修复Text中processor指针没有正确初始化的bug

## 2024/01/11

完善context封装，测试单处理器功能完成，后续继续支持多处理器

``` bash
tickle
tickle
tickle
tickle
tickle
tickle
tickle
tickle
tickle
tickle
back
2024-01-12 00:49:22     81060 Proc      1       [test_co] [INFO] test_coroutine.cpp:14        Hello world
2024-01-12 00:49:22     81060 Proc      2       [test_co] [INFO] test_coroutine.cpp:14        Hello world
2024-01-12 00:49:22     81060 Proc      3       [test_co] [INFO] test_coroutine.cpp:14        Hello world
2024-01-12 00:49:22     81060 Proc      4       [test_co] [INFO] test_coroutine.cpp:14        Hello world
2024-01-12 00:49:22     81060 Proc      5       [test_co] [INFO] test_coroutine.cpp:14        Hello world
2024-01-12 00:49:22     81060 Proc      6       [test_co] [INFO] test_coroutine.cpp:14        Hello world
2024-01-12 00:49:22     81060 Proc      7       [test_co] [INFO] test_coroutine.cpp:14        Hello world
2024-01-12 00:49:22     81060 Proc      8       [test_co] [INFO] test_coroutine.cpp:14        Hello world
2024-01-12 00:49:22     81060 Proc      9       [test_co] [INFO] test_coroutine.cpp:14        Hello world
2024-01-12 00:49:22     81060 Proc      10      [test_co] [INFO] test_coroutine.cpp:14        Hello world
tickle
back
```

> 实验结果可以看到，协程任务可以很好的进行执行，并配置正确的协程号

## 2024/01/10

**TODO**: 完成Scheduler,Processor的简单实现，编译通过，运行不通过，问题为context封装有误，待修改
**于2024/01/11优化完成:没有使用getcontext导致上下文没有初始化**
**TODO**: 多Processor的支持，包括task-steal，负载均衡等

## 2024/01/09

完成LogSink优化
**TODO**: 重新打开新的LogSink前等待当前缓存写完

## 2024/01/08

优化bench测试
**TODO**: 优化LogSink，将新文件转化功能作为接口，而不是每次打开写文件都构造新的LogSink，并将文件流写入锁从LogAppender重构到LogSink中。\
**于2024/01/09优化完成:优化LogSink的打开与写入**

## 2024/01/07

**TODO**: 计划日志模块先暂停优化，后续继续学习网络io部分的代码实现，回顾日志模块的当前完整实现，存在着大量未完成的坑：

1. RollingFileAppender的完整实现，其依赖的时间管理模块已经完成
2. LogManager的优化实现，实现更优雅的接口隔离
3. Configer模块的完整支持，由于底层结构的不断变动，原本的config方案已经无法自动生成日志体系了
4. 日志流的底层优化，包括字符的对齐，__FILE__的性能优化
5. 同步日志的性能优化（也采用缓存的方式提高并发，如spdlog一样）
6. 思考能否将日志流的构成部分设计为异步，进一步提高异步日志的并发
7. 异步日志仍然存在偶发的内存溢出问题\
**于2024/01/09优化完成:优化LogSink的打开与写入**

## 2024/01/06

### 针对同步日志的流输出上锁

为了确保同步日志在写入时数据流的安全性，额外包装了一层**有锁写入**，实验结果如下

```bash
测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:2.37453s
每秒输出日志数量：421135条
每秒输出日志大小：41126KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程1:  输出数量333333 耗时:2.55059s
线程0:  输出数量333333 耗时:2.6499s
线程2:  输出数量333333 耗时:2.65337s
每秒输出日志数量：376878条
每秒输出日志大小：36804KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:0.330457s
每秒输出日志数量：3026110条
每秒输出日志大小：295518KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程2:  输出数量333333 耗时:0.376422s
线程1:  输出数量333333 耗时:0.377153s
线程0:  输出数量333333 耗时:0.388846s
每秒输出日志数量：2571713条
每秒输出日志大小：251143KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程1:  输出数量100000 耗时:0.339424s
线程2:  输出数量100000 耗时:0.357514s
线程8:  输出数量100000 耗时:0.359495s
线程9:  输出数量100000 耗时:0.375172s
线程5:  输出数量100000 耗时:0.389225s
线程3:  输出数量100000 耗时:0.390722s
线程6:  输出数量100000 耗时:0.394513s
线程4:  输出数量100000 耗时:0.403266s
线程0:  输出数量100000 耗时:0.393982s
线程7:  输出数量100000 耗时:0.397175s
每秒输出日志数量：2479751条
每秒输出日志大小：242163KB
```

> 实验结果表明，同步日志在多线程环境下写入性能竟得到了提高

### 异步日志的多线程高并发问题

通过实验发现，异步日志在10线程测试中存在core dumped问题

```bash
测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
bench: ../nptl/pthread_mutex_lock.c:81: __pthread_mutex_lock: Assertion `mutex->__data.__owner == 0' failed.
Aborted (core dumped)
```

经过多方论坛搜索，问题定位在条件变量（可能），报错信息普遍被认为是重复的解锁mutex所导致。**???**，通过删除了局部锁中的isLocked判断，居然解决了问题(?不知是否完全解决),并增加了并发性能。

```bash
测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程4:  输出数量100000 耗时:0.252673s
线程7:  输出数量100000 耗时:0.265764s
线程8:  输出数量100000 耗时:0.29632s
线程2:  输出数量100000 耗时:0.297939s
线程1:  输出数量100000 耗时:0.317145s
线程0:  输出数量100000 耗时:0.317909s
线程3:  输出数量100000 耗时:0.321634s
线程6:  输出数量100000 耗时:0.318993s
线程5:  输出数量100000 耗时:0.326873s
线程9:  输出数量100000 耗时:0.324883s
每秒输出日志数量：3059290条
每秒输出日志大小：298758KB
```

> 经过实验得到，在多线程时，异步日志也能达到300m/s的吞吐量
>
> 回顾修改过程，分析可能原因并不出在条件变量，而是出在局部锁包装的isLocked判断并非原子性的，当并发量高时，可能存在多个线程进入了包装类的临界区中，导致了锁的重复解锁。
而无意间删除的isLocked判断则刚好优化了这个问题。

state-of-art:以下是新的运行标准(`pattern="%d %m%n"`)

``` bash
测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:2.47172s
每秒输出日志数量：404577条
每秒输出日志大小：39509KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程2:  输出数量333333 耗时:2.57465s
线程0:  输出数量333333 耗时:2.61034s
线程1:  输出数量333333 耗时:2.63891s
每秒输出日志数量：378944条
每秒输出日志大小：37006KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:0.4228s
每秒输出日志数量：2365184条
每秒输出日志大小：230975KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程1:  输出数量333333 耗时:0.318791s
线程2:  输出数量333333 耗时:0.32337s
线程0:  输出数量333333 耗时:0.330089s
每秒输出日志数量：3029489条
每秒输出日志大小：295848KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量100000 耗时:0.254316s
线程1:  输出数量100000 耗时:0.267359s
线程7:  输出数量100000 耗时:0.286929s
线程8:  输出数量100000 耗时:0.288872s
线程6:  输出数量100000 耗时:0.293808s
线程9:  输出数量100000 耗时:0.295414s
线程3:  输出数量100000 耗时:0.295051s
线程5:  输出数量100000 耗时:0.29894s
线程4:  输出数量100000 耗时:0.310674s
线程2:  输出数量100000 耗时:0.306811s
每秒输出日志数量：3218811条
每秒输出日志大小：314337KB
```

## 2024/01/05

### 字符串构造耗时问题（已解决）

根据陈硕老师所言，在字符串构造时，将时间构造成字符串时非常耗时的任务，针对这一问题进行优化探讨

测试打印时间字符串的输出效率

``` bash
测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:3.40098s
每秒输出日志数量：294033条
每秒输出日志大小：28714KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程2:  输出数量333333 耗时:7.20461s
线程1:  输出数量333333 耗时:7.37501s
线程0:  输出数量333333 耗时:7.37811s
每秒输出日志数量：135536条
每秒输出日志大小：13235KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:0.746579s
每秒输出日志数量：1339443条
每秒输出日志大小：130805KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量333333 耗时:1.07706s
线程1:  输出数量333333 耗时:1.36384s
线程2:  输出数量333333 耗时:1.38611s
每秒输出日志数量：721445条
每秒输出日志大小：70453KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程5:  输出数量100000 耗时:1.23487s
线程6:  输出数量100000 耗时:1.32486s
线程9:  输出数量100000 耗时:1.33905s
线程0:  输出数量100000 耗时:1.39812s
线程3:  输出数量100000 耗时:1.40304s
线程4:  输出数量100000 耗时:1.40427s
线程2:  输出数量100000 耗时:1.41166s
线程8:  输出数量100000 耗时:1.41256s
线程7:  输出数量100000 耗时:1.42238s
线程1:  输出数量100000 耗时:1.43051s
每秒输出日志数量：699050条
每秒输出日志大小：68266KB
```

> 实验结果表明，所有模式下的并发输出效率都受到了时间字符串构造的严重的影响
>
> 这是由于**重复构造字符串**导致，在同一秒内所有时间字符串应当具有相同字符串格式（以秒为最小单位粒度下），优化方案为 **设计时间管理单例**，当需要打印时间字符串时向其申请，如果时间相同则直接使用其存储的字符串缓存；若时间改变，则构造新的字符串缓存。

实验结果如下

``` bash
测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:2.47128s
每秒输出日志数量：404649条
每秒输出日志大小：39516KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程2:  输出数量333333 耗时:4.76527s
线程1:  输出数量333333 耗时:4.86739s
线程0:  输出数量333333 耗时:4.9727s
每秒输出日志数量：201097条
每秒输出日志大小：19638KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:0.415794s
每秒输出日志数量：2405039条
每秒输出日志大小：234867KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量333333 耗时:0.350894s
线程2:  输出数量333333 耗时:0.347898s
线程1:  输出数量333333 耗时:0.360088s
每秒输出日志数量：2777098条
每秒输出日志大小：271201KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程3:  输出数量100000 耗时:0.379318s
线程7:  输出数量100000 耗时:0.431363s
线程0:  输出数量100000 耗时:0.399491s
线程8:  输出数量100000 耗时:0.431745s
线程6:  输出数量100000 耗时:0.447099s
线程2:  输出数量100000 耗时:0.4504s
线程4:  输出数量100000 耗时:0.448399s
线程9:  输出数量100000 耗时:0.461774s
线程1:  输出数量100000 耗时:0.471244s
线程5:  输出数量100000 耗时:0.468347s
每秒输出日志数量：2122041条
每秒输出日志大小：207230KB
```

> 实验结果表明，缓存方案提高了日志输出性能（60m/s->200m/s）

### 无锁导致的内存dump问题（已解决）

在实验过程中也发生了非常频繁的内存dump情况（尤其是在10线程时），则一定是时间管理
单例中没有加锁导致，以上实验结果为已经添加了自旋锁的结果，考虑到每一秒改动一次时间字符串，而并发的读取操作会非常多，则同时进行读写锁的设计实现，结果如下：

```bash
测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:2.5765s
每秒输出日志数量：388123条
每秒输出日志大小：37902KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量333333 耗时:4.92998s
线程2:  输出数量333333 耗时:5.04528s
线程1:  输出数量333333 耗时:5.10949s
每秒输出日志数量：195714条
每秒输出日志大小：19112KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:0.437606s
每秒输出日志数量：2285160条
每秒输出日志大小：223160KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程2:  输出数量333333 耗时:0.382744s
线程0:  输出数量333333 耗时:0.380229s
线程1:  输出数量333333 耗时:0.397285s
每秒输出日志数量：2517084条
每秒输出日志大小：245809KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程6:  输出数量100000 耗时:0.353977s
线程4:  输出数量100000 耗时:0.373159s
线程3:  输出数量100000 耗时:0.38916s
线程0:  输出数量100000 耗时:0.391656s
线程5:  输出数量100000 耗时:0.395675s
线程1:  输出数量100000 耗时:0.399618s
线程8:  输出数量100000 耗时:0.39981s
线程2:  输出数量100000 耗时:0.409114s
线程9:  输出数量100000 耗时:0.396821s
线程7:  输出数量100000 耗时:0.412939s
每秒输出日志数量：2421665条
每秒输出日志大小：236490KB
```

> 实验结果表明，读写锁这种基于线程阻塞的锁在少线程时的并发效果不如自旋锁，但工作线程达到10个时，其性能则更优，这是由于大量的读锁并不需要进入阻塞状态，具有更好的并发性。

### 时间字符串粒度问题（已解决）

针对时间字符串的粒度问题，即陈硕老师在muduo设计中，使用了以微妙粒度的服务器框架，但根据我的实验测试，实现微秒粒度的系统函数开销较大，对并发性是一个非常大的挑战，所以在我的日志框架中不予采用。实现结果如下：

```bash
test on 1000000 iter
Method: system_clock - Time taken: 1235448 microseconds
Method: high_resolution_clock - Time taken: 1247874 microseconds
Method: gettimeofday - Time taken: 1212794 microseconds
Method: localtime - Time taken: 21093 microseconds

Method: system_clock - 1704474525385838
Method: high_resolution_clock - 1704474525385856
Method: gettimeofday - 1704474525385873
Method: localtime - 1704474525

TimeString: 1704474525.385896
TimeFormatStringUs: 2024-01-06 01:08:45.385896

test on 100000 iter
FormatString: 2024-01-06 01:08:47
Direct construction duration: 1644808 microseconds
FormatString: 2024-01-06 01:08:47
TimeManager duration: 4186 microseconds
```

> 如上实验可以看到，chrono库的方法和sys/time的方法都可以得到us粒度，但是运行耗时是localtime_r的60倍。同时在构造字符串的任务测试中，使用了缓存法的localtime_r效率是chrono的400倍。最终，日志库采用**秒为粒度**的方案。

## 2024/01/02

### 同步日志无序性问题

在之前的设计中考虑到数据写入的有序性以及成员变量的安全性，使用了自旋锁来保护临界区。
但是处于以下两点考量

+ 在实际应用中，日志模块的设置一旦完成，很少有对其成员的改动
+ ****TODO**: 日志模块的日志写入有序性是否有必要由日志模块对其进行约束？**\
**于2024/01/05优化完成：同步日志添加了有锁的方法**

则对日志模块的锁部分进行优化，前端日志的构造采用无锁的方式进行，以此来增加并发量

``` bash
测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:2.56819s
每秒输出日志数量：389379条
每秒输出日志大小：38025KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量333333 耗时:4.71559s
线程2:  输出数量333333 耗时:4.76937s
线程1:  输出数量333333 耗时:4.85498s
每秒输出日志数量：205973条
每秒输出日志大小：20114KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:0.330609s
每秒输出日志数量：3024724条
每秒输出日志大小：295383KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程2:  输出数量333333 耗时:0.316169s
线程1:  输出数量333333 耗时:0.328701s
线程0:  输出数量333333 耗时:0.334467s
每秒输出日志数量：2989831条
每秒输出日志大小：291975KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程5:  输出数量100000 耗时:0.319204s
线程0:  输出数量100000 耗时:0.341175s
线程4:  输出数量100000 耗时:0.343998s
线程8:  输出数量100000 耗时:0.35173s
线程9:  输出数量100000 耗时:0.353207s
线程2:  输出数量100000 耗时:0.354964s
线程1:  输出数量100000 耗时:0.360241s
线程7:  输出数量100000 耗时:0.365926s
线程6:  输出数量100000 耗时:0.361651s
线程3:  输出数量100000 耗时:0.382031s
每秒输出日志数量：2617591条
每秒输出日志大小：255624KB
```

> 实验结果表明，前端线程的去锁化对异步日志的多线程运行优化明显(90M/s->250M/s)
> 但是实验过程中发现了两点异常
>
> + 当并发程度非常高时，会存在内存dump的情况
> + 根据对打印出日志的观察，确实**存在假想的乱序现象**，并且存在了**打印丢失问题**
>
> 针对以上问题，分析如下
>
> + **TODO**: 在日志message构造到字符串时在某些地方发生了临界区冲突，需要针对每个构造体设置更有细度的锁。或是在写入时发生了安全性问题。\
**于2024/01/06优化完成，优化了局部锁的设计**
> + **TODO**: 打印丢失的现象需要分析是否是因为写入时出错还是其他问题，并进一步甄别异步日志是否有同样问题（之前时同步日志中发现的问题），并重新审视锁的应用。\
**于2024/01/06优化完成，优化了局部锁的设计**

## 2024/01/01

### 多线程性能下滑问题->前端锁导致(已解决)

异步日志构造完成，进行初次压力测试（测试环境为ubuntu 4核心，100length c-string）

``` bash
测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:2.38891s
每秒输出日志数量：418601条
每秒输出日志大小：40879KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程1:  输出数量333333 耗时:2.77151s
线程0:  输出数量333333 耗时:2.92703s
线程2:  输出数量333333 耗时:2.93051s
每秒输出日志数量：341237条
每秒输出日志大小：33323KB

测试日志器:sync_logger
测试日志：1000000条，总大小：97656KB
线程3:  输出数量100000 耗时:7.02885s
线程8:  输出数量100000 耗时:7.64639s
线程2:  输出数量100000 耗时:7.69952s
线程9:  输出数量100000 耗时:7.7421s
线程4:  输出数量100000 耗时:7.92021s
线程5:  输出数量100000 耗时:8.06304s
线程1:  输出数量100000 耗时:8.06482s
线程7:  输出数量100000 耗时:8.09969s
线程6:  输出数量100000 耗时:8.11577s
线程0:  输出数量100000 耗时:8.14125s
每秒输出日志数量：122831条
每秒输出日志大小：11995KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量1000000 耗时:0.366028s
每秒输出日志数量：2732031条
每秒输出日志大小：266799KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程1:  输出数量333333 耗时:0.335103s
线程0:  输出数量333333 耗时:0.350824s
线程2:  输出数量333333 耗时:0.394806s
每秒输出日志数量：2532887条
每秒输出日志大小：247352KB

测试日志器:async_logger
测试日志：1000000条，总大小：97656KB
线程0:  输出数量100000 耗时:0.803354s
线程3:  输出数量100000 耗时:0.900471s
线程1:  输出数量100000 耗时:0.914625s
线程2:  输出数量100000 耗时:1.02606s
线程4:  输出数量100000 耗时:1.01842s
线程5:  输出数量100000 耗时:1.01299s
线程9:  输出数量100000 耗时:1.04669s
线程8:  输出数量100000 耗时:1.04988s
线程6:  输出数量100000 耗时:1.0489s
线程7:  输出数量100000 耗时:1.07094s
每秒输出日志数量：933763条
每秒输出日志大小：91187KB
```

> 实验数据显示：
>
> + 异步日志的实现方式较同步日志的实现方式吞吐量强一个量级（同步40m/s，异步280m/s）
> + **TODO**: 在多线程环境下，同步日志与异步日志的性能都有非常严重的下滑\
**于2024/01/02优化完成：移除锁**
