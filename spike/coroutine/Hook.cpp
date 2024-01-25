#include <dlfcn.h>

#include "base/Configer.h"
#include "Hook.h"
#include "base/Thread.h"
#include "splog.h"
#include "coroutine/FdInfo.h"
#include "coroutine/Timer.h"
#include "coroutine/Scheduler.h"
#include "net/EventLoop.h"

static lim_webserver::Logger::ptr g_logger = LOG_SYS();

namespace lim_webserver
{
    static ConfigerVar<int>::ptr g_tcp_connect_timeout = Configer::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

#define HOOK_FUN(F) \
    F(sleep)        \
    F(usleep)       \
    F(nanosleep)    \
    F(socket)       \
    F(connect)      \
    F(accept)       \
    F(read)         \
    F(readv)        \
    F(recv)         \
    F(recvfrom)     \
    F(recvmsg)      \
    F(write)        \
    F(writev)       \
    F(send)         \
    F(sendto)       \
    F(sendmsg)      \
    F(close)        \
    F(fcntl)        \
    F(ioctl)        \
    F(getsockopt)   \
    F(setsockopt)

    void hook_init()
    {
        static bool is_inited = false;
        if (is_inited)
        {
            return;
        }
        /**
         * 最终宏拓展为
         * sleep_f = (sleep_fun)dlsym(((void *) -1l), "sleep");
         *
         * 其中 dlsym 函数用于在动态链接库中查找函数的地址，
         * ((void *) -1l) 表示在当前进程的地址空间中查找函数，
         * "sleep" 是要查找的函数的名称。
         */
#define LOAD_HOOK_FUN(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
        HOOK_FUN(LOAD_HOOK_FUN);
#undef LOAD_HOOK_FUN
        is_inited = true;
    }
    static uint64_t s_connect_timeout = -1;
    struct _HookIniter
    {
        _HookIniter()
        {
            hook_init();
            s_connect_timeout = g_tcp_connect_timeout->getValue();

            g_tcp_connect_timeout->addListener(
                [](const int &old_value, const int &new_value)
                {
                    LOG_INFO(g_logger) << "tcp connect timeout changed from " << old_value << " to " << new_value;
                    s_connect_timeout = new_value;
                });
        }
    };

    static _HookIniter s_hookIniter;
}

template <typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char *hook_fun_name, uint32_t event, int timeout_so, Args &&...args)
{
    // 如果不在协程中，则直接调用系统原生函数
    lim_webserver::Task *task = lim_webserver::Processor::GetCurrentTask();
    LOG_TRACE(g_logger) << "task(" << task->id() << ") hook " << hook_fun_name << "(fd = " << fd << ") " << (!!task ? "in" : "not in") << " coroutine.";
    if (!task)
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    lim_webserver::FdInfo::ptr fdInfo = lim_webserver::FdManager::GetInstance()->get(fd);
    if (!fdInfo || !fdInfo->isSocket() || fdInfo->getUserNonblock())
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    // 获取超时时间
    uint64_t to = fdInfo->getTimeout(timeout_so);
    lim_webserver::Timer::ptr timer;

    // 重试IO 操作(一般循环一次)
    while (true)
    {
        LOG_TRACE(g_logger) << "task(" << task->id() << ") try hook " << hook_fun_name << "(fd = " << fd << ").";

        // 调用传入的原生函数执行操作
        ssize_t n = fun(fd, std::forward<Args>(args)...);

        // n==-1表示函数调用失败;
        // 若错误码为 EINTR 即系统调用被中断的情况，继续调用原生函数。
        while (n == -1 && errno == EINTR)
        {
            n = fun(fd, std::forward<Args>(args)...);
        }

        // 若函数调用成功或者错误码不为 EAGAIN 即资源暂时不可用，表示操作成功或出错，直接返回结果。
        if (n != -1 || errno != EAGAIN)
        {
            return n;
        }

        // 后续则为函数因为资源不可用的调用失败,阻塞等待
        lim_webserver::EventLoop *eventloop = lim_webserver::EventLoop::GetInstance();
        bool expired = false;

        // 若设置了超时时间，则创建一个条件定时器来处理超时事件
        if (to != (uint64_t)-1)
        {
            // 超时则触发回调
            timer = lim_webserver::TimerManager::GetInstance()->addTimer(
                to,
                [eventloop, fd, event, &expired]()
                {
                    eventloop->cancelEvent(fd, (lim_webserver::IoEvent)event);
                    expired = true;
                });
        }

        // 添加该协程事件，即后续内容
        lim_webserver::EventLoop::GetInstance()->addEvent(fd, (lim_webserver::IoEvent)event);

        lim_webserver::Processor::CoHold();
        if (timer)
        {
            timer->cancel();
        }
        // 如果定时器信息为超时，则表明事件超时，设置错误码为超时并返回 -1
        if (expired)
        {
            errno = ETIMEDOUT;
            return -1;
        }
    }
}

extern "C"
{
#define DEF_FUN_NAME(name) name##_fun name##_f = nullptr;
    HOOK_FUN(DEF_FUN_NAME);
#undef DEF_FUN_NAME

    unsigned int sleep(unsigned int seconds)
    {
        lim_webserver::Task *task = lim_webserver::Processor::GetCurrentTask();
        LOG_TRACE(g_logger) << "task(" << task->id() << ") hook sleep " << (!!task ? "in" : "not in") << " coroutine.";
        if (!task)
        {
            return sleep_f(seconds);
        }

        // 获取当前task的id并设定当前processor在一定时间后唤醒对应task
        uint64_t id = task->id();
        lim_webserver::Processor *processor = lim_webserver::Processor::GetCurrentProcessor();
        lim_webserver::TimerManager::GetInstance()->addTimer(seconds * 1000,
                                                             [processor, id]
                                                             {
                                                                 processor->wakeupTask(id);
                                                             });
        // 阻塞当前协程
        lim_webserver::Processor::CoHold();

        return 0;
    }

    int usleep(useconds_t usec)
    {
        lim_webserver::Task *task = lim_webserver::Processor::GetCurrentTask();
        LOG_TRACE(g_logger) << "task(" << task->id() << ") hook usleep " << (!!task ? "in" : "not in") << " coroutine.";
        if (!task)
        {
            return usleep_f(usec);
        }

        uint64_t id = task->id();
        lim_webserver::Processor *processor = lim_webserver::Processor::GetCurrentProcessor();
        lim_webserver::TimerManager::GetInstance()->addTimer(usec / 1000,
                                                             [processor, id]
                                                             {
                                                                 processor->wakeupTask(id);
                                                             });
        lim_webserver::Processor::CoHold();
        return 0;
    }

    int nanosleep(const struct timespec *req, struct timespec *rem)
    {
        lim_webserver::Task *task = lim_webserver::Processor::GetCurrentTask();
        LOG_TRACE(g_logger) << "task(" << task->id() << ") hook nanosleep " << (!!task ? "in" : "not in") << " coroutine.";
        if (!task)
        {
            return nanosleep_f(req, rem);
        }

        int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
        uint64_t id = task->id();
        lim_webserver::Processor *processor = lim_webserver::Processor::GetCurrentProcessor();
        lim_webserver::TimerManager::GetInstance()->addTimer(timeout_ms,
                                                             [processor, id]
                                                             {
                                                                 processor->wakeupTask(id);
                                                             });

        lim_webserver::Processor::CoHold();

        return 0;
    }

    int socket(int domain, int type, int protocol)
    {
        lim_webserver::Task *task = lim_webserver::Processor::GetCurrentTask();
        LOG_TRACE(g_logger) << "task(" << task->id() << ") hook socket, domain = "<<domain<<" type = "<<type<<" protocal = "<<protocol;
        int fd = socket_f(domain, type, protocol);
        if (fd > 0)
        {
            lim_webserver::FdManager::GetInstance()->insert(fd);
        }
        LOG_TRACE(g_logger) << "task(" << task->id() << ") hook socket, returns (fd = " << fd << ").";
        return fd;
    }

    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    {
        lim_webserver::Task *task = lim_webserver::Processor::GetCurrentTask();
        LOG_TRACE(g_logger) << "task(" << task->id() << ") hook connect(fd = " << sockfd << ") " << (!!task ? "in" : "not in") << " coroutine.";
        if (!task)
        {
            return connect_f(sockfd, addr, addrlen);
        }

        lim_webserver::FdInfo::ptr fdInfo = lim_webserver::FdManager::GetInstance()->get(sockfd);
        if (!fdInfo)
        {
            errno = EBADF;
            return -1;
        }

        if (!fdInfo->isSocket() || fdInfo->getUserNonblock())
        {
            return connect_f(sockfd, addr, addrlen);
        }

        // 调用原始的 connect 函数，尝试建立连接(非阻塞的，立即返回结果)
        int n = connect_f(sockfd, addr, addrlen);

        // 如果连接成功，返回 0
        if (n == 0)
        {
            LOG_TRACE(g_logger) << "task(" << task->id() << ") hook connect(fd = " << sockfd << ") completed immediately.";
            return 0;
        }
        else if (n != -1 || errno != EINPROGRESS) // 如果连接不在进行中，返回
        {
            return n;
        }

        // 此时错误为EINPROGRESS，表明连接正在进行中,hold住协程等待epoll反馈

        // 若定义了超时，则创建超时定时器
        lim_webserver::Timer::ptr timer;
        uint64_t timeout_ms = fdInfo->getConnectTimeout();
        lim_webserver::EventLoop *eventloop = lim_webserver::EventLoop::GetInstance();
        bool expired = false;
        // 如果设置了连接超时时间 timeout_ms  不等于 (uint64_t)-1
        if (timeout_ms != (uint64_t)-1)
        {
            // 创建一个定时器，当超时时取消连接
            timer = lim_webserver::TimerManager::GetInstance()->addTimer(
                timeout_ms,
                [eventloop, &sockfd, &expired]()
                {
                    eventloop->cancelEvent(sockfd, lim_webserver::WRITE);
                    expired = true;
                });
        }

        // 添加读时间监听
        lim_webserver::EventLoop::GetInstance()->addEvent(sockfd, lim_webserver::WRITE);
        lim_webserver::Processor::CoHold();

        // 协程被唤醒
        // 若未超时,则取消定时器
        if (timer)
        {
            timer->cancel();
        }

        // 若超时,则返回超时
        if (expired)
        {
            errno = ETIMEDOUT;
            return -1;
        }

        int error = 0;
        socklen_t len = sizeof(int);
        if (-1 == getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len))
        {
            return -1;
        }
        if (!error)
        {
            return 0;
        }
        errno = error;
        return -1;
    }

    int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
    {
        int fd = do_io(s, accept_f, "accept", lim_webserver::READ, SO_RCVTIMEO, addr, addrlen);
        if (fd >= 0)
        {
            lim_webserver::FdManager::GetInstance()->insert(fd);
        }
        return fd;
    }

    ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
    {
        return do_io(fd, readv_f, "readv", lim_webserver::READ, SO_RCVTIMEO, iov, iovcnt);
    }

    ssize_t recv(int sockfd, void *buf, size_t len, int flags)
    {
        return do_io(sockfd, recv_f, "recv", lim_webserver::READ, SO_RCVTIMEO, buf, len, flags);
    }

    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
    {
        return do_io(sockfd, recvfrom_f, "recvfrom", lim_webserver::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
    }

    ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
    {
        return do_io(sockfd, recvmsg_f, "recvmsg", lim_webserver::READ, SO_RCVTIMEO, msg, flags);
    }

    ssize_t write(int fd, const void *buf, size_t count)
    {
        return do_io(fd, write_f, "write", lim_webserver::WRITE, SO_SNDTIMEO, buf, count);
    }

    ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
    {
        return do_io(fd, writev_f, "writev", lim_webserver::WRITE, SO_SNDTIMEO, iov, iovcnt);
    }

    ssize_t send(int s, const void *msg, size_t len, int flags)
    {
        return do_io(s, send_f, "send", lim_webserver::WRITE, SO_SNDTIMEO, msg, len, flags);
    }

    ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
    {
        return do_io(s, sendto_f, "sendto", lim_webserver::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
    }

    ssize_t sendmsg(int s, const struct msghdr *msg, int flags)
    {
        return do_io(s, sendmsg_f, "sendmsg", lim_webserver::WRITE, SO_SNDTIMEO, msg, flags);
    }

    int close(int fd)
    {
        lim_webserver::FdInfo::ptr fdInfo = lim_webserver::FdManager::GetInstance()->get(fd);
        if (fdInfo)
        {
            lim_webserver::EventLoop::GetInstance()->clearEvent(fd);
            lim_webserver::FdManager::GetInstance()->del(fd);
        }
        return close_f(fd);
    }

    int fcntl(int fd, int cmd, ... /* arg */)
    {
        // 声明 va_list 对象，并使用 va_start(va, cmd) 初始化。允许函数访问在 cmd 参数之后传递的可变参数。
        va_list va;
        va_start(va, cmd);
        // 在每个 case 块的结尾，调用 va_end(va) 来清理可变参数列表。
        switch (cmd)
        {
        // 设置文件状态标志
        case F_SETFL:
        {
            int arg = va_arg(va, int);
            va_end(va);
            lim_webserver::FdInfo::ptr fdInfo = lim_webserver::FdManager::GetInstance()->get(fd);
            if (!fdInfo || !fdInfo->isSocket())
            {
                return fcntl_f(fd, cmd, arg);
            }
            // 更新用户非阻塞标识。
            fdInfo->setUserNonblock(arg & O_NONBLOCK);
            // 根据系统非阻塞表示更新arg
            if (fdInfo->getSysNonblock())
            {
                arg |= O_NONBLOCK;
            }
            else
            {
                arg &= ~O_NONBLOCK;
            }
            return fcntl_f(fd, cmd, arg);
        }
        break;
        // 获取文件状态标志
        case F_GETFL:
        {
            va_end(va);
            int arg = fcntl_f(fd, cmd);
            lim_webserver::FdInfo::ptr fdInfo = lim_webserver::FdManager::GetInstance()->get(fd);
            if (!fdInfo || !fdInfo->isSocket())
            {
                return arg;
            }
            if (fdInfo->getUserNonblock())
            {
                return arg | O_NONBLOCK;
            }
            else
            {
                return arg & ~O_NONBLOCK;
            }
        }
        break;
        // 从可变参数列表中获取整数参数，并将其传递给 fcntl_f 函数。
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
        {
            int arg = va_arg(va, int);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        break;
        // 直接调用
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
        {
            va_end(va);
            return fcntl_f(fd, cmd);
        }
        break;
        // 从可变参数列表中获取一个 struct flock* 参数，并将其传递给 fcntl_f 函数。
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK:
        {
            struct flock *arg = va_arg(va, struct flock *);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        break;
        // 从可变参数列表中获取一个 struct f_owner_exlock* 参数，并将其传递给 fcntl_f 函数。
        case F_GETOWN_EX:
        case F_SETOWN_EX:
        {
            struct f_owner_exlock *arg = va_arg(va, struct f_owner_exlock *);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
        break;
        // 默认情况将直接调用 fcntl_f(fd, cmd) 来执行底层的 fcntl 操作并返回结果。
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
        }
    }

    int ioctl(int d, unsigned long int request, ...)
    {
        va_list va;
        va_start(va, request);
        void *arg = va_arg(va, void *);
        va_end(va);

        // 如果请求码是 FIONBIO，表示设置或获取非阻塞模式
        if (FIONBIO == request)
        {
            // 解析 void* 参数，将其转换为 int 指针，然后取其值，将其转换为布尔值
            bool user_nonblock = !!*(int *)arg;
            // 获取与文件描述符关联的 FdInfo 对象，用于跟踪文件描述符的状态
            lim_webserver::FdInfo::ptr fdInfo = lim_webserver::FdManager::GetInstance()->get(d);

            // 更新 FdInfo 对象的用户非阻塞标志
            if (fdInfo)
            {
                fdInfo->setUserNonblock(user_nonblock);
            }
        }
        // 最后，调用底层的 ioctl 函数并返回其结果
        return ioctl_f(d, request, arg);
    }

    int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
    {
        return getsockopt_f(sockfd, level, optname, optval, optlen);
    }

    int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen)
    {
        int res = setsockopt_f(sockfd, level, optname, optval, optlen);

        // 如果选项级别是 SOL_SOCKET（套接字选项级别）
        if (res == 0 && level == SOL_SOCKET)
        {
            // 如果选项名称是 SO_RCVTIMEO（接收超时）或 SO_SNDTIMEO（发送超时）
            if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO)
            {
                lim_webserver::FdInfo::ptr fdInfo = lim_webserver::FdManager::GetInstance()->get(sockfd);
                if (fdInfo)
                {
                    // 将 optval 转换为 timeval 结构体指针
                    const timeval *v = (const timeval *)optval;
                    // 计算超时值（以毫秒为单位）并设置到 FdInfo 对象中
                    fdInfo->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
                }
            }
        }
        return res;
    }
}
