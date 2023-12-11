#include <dlfcn.h>

#include "hook.h"
#include "fiber.h"
#include "thread.h"
#include "io_manager.h"
#include "fd_manager.h"
#include "log.h"
#include "macro.h"
#include "config.h"

namespace lim_webserver
{
    static thread_local bool t_hook_enable = false;
    static Logger::ptr g_logger = LIM_LOG_NAME("system");
    static ConfigVar<int>::ptr g_tcp_connect_timeout = Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");

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
                    LIM_LOG_INFO(g_logger) << "tcp connect timeout changed from " << old_value << " to " << new_value;
                    s_connect_timeout = new_value;
                });
        }
    };

    static _HookIniter s_hookIniter;

    bool is_hook_enable()
    {
        return t_hook_enable;
    }

    void set_hook_enable(bool flag)
    {
        t_hook_enable = flag;
    }
}

struct timer_info
{
    int cancelled = 0;
};

template <typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char *hook_fun_name, uint32_t event, int timeout_so, Args &&...args)
{
    // 如果全局变量 t_hook_enable 为否，不启用钩子，则直接调用系统原生函数
    if (!t_hook_enable)
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    // 获取文件描述符的上下文信息，如果无法获取上下文信息，则说明文件句柄不存在，直接调用原生函数
    FdCtx::ptr ctx = FdMgr::GetInstance()->get(fd);
    if (!ctx)
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    // 文件描述符若关闭，则设置错误码并返回 -1
    if (ctx->isClosed())
    {
        errno = EBADF; // 文件描述符无效或不可用。
        return -1;
    }

    // 文件描述符若不为套接字或用户设置了非阻塞模式，直接调用原生函数
    if (!ctx->isSocket() || ctx->getUserNonblock())
    {
        return fun(fd, std::forward<Args>(args)...);
    }

    // 后续则为自定义的socket操作函数

    // 获取超时时间
    uint64_t to = ctx->getTimeout(timeout_so);
    // 创建一个用于管理定时器的信息结构
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);
    ssize_t n;
    IoManager *iom;
    Timer::ptr timer;
    // 进入无限循环，用于重试IO 操作
    while (true)
    {
        // 调用传入的原生函数执行操作
        n = fun(fd, std::forward<Args>(args)...);
        // n==-1表示函数调用失败；
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

        // 后续则为函数调用成功但资源不可用,则表明需要进行异步操作

        iom = IoManager::GetThis();

        // 若设置了超时时间，则创建一个条件定时器来处理超时事件
        if (to != (uint64_t)-1)
        {
            // 超时则触发回调，若条件弱指针的对象析构了或条件为取消，则说明事件结束，直接退出；否则就设定事件类型为取消并取消事件
            timer = iom->addConditionTimer(
                to, [winfo, fd, iom, event]()
                {
                        auto t = winfo.lock();
                        if (!t || t->cancelled) {
                            return;
                        }
                        t->cancelled = ETIMEDOUT;
                        iom->cancelEvent(fd, (IoManager::IoEvent)event); },
                winfo);
        }

        // 添加该协程事件，即后续内容
        int rt = iom->addEvent(fd, (IoManager::IoEvent)event);

        // 处理添加事件失败的情况，设置错误码并返回 -1
        if (rt)
        {
            LIM_LOG_ERROR(g_logger) << hook_fun_name << " addEvent(" << fd << ", " << event << ")";
            if (timer)
            {
                timer->cancel();
            }
            return -1;
        }
        // 添加成功了则让出协程执行权，等待读到事件的唤醒或者定时器的唤醒
        else
        {
            Fiber::YieldToHold();
            if (timer)
            {
                timer->cancel();
            }
            // 如果定时器信息为超时，则表明事件超时，设置错误码为超时并返回 -1
            if (tinfo->cancelled)
            {
                errno = tinfo->cancelled;
                return -1;
            }
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
        if (!lim_webserver::t_hook_enable)
        {
            return sleep_f(seconds);
        }

        lim_webserver::Fiber::ptr fiber = lim_webserver::Fiber::GetThis();
        lim_webserver::IoManager *iom = lim_webserver::IoManager::GetThis();
        iom->addTimer(seconds * 1000,
                      [iom, fiber]()
                      { iom->schedule(fiber); });
        lim_webserver::Fiber::YieldToHold();
        return 0;
    }

    int usleep(useconds_t usec)
    {
        if (!lim_webserver::t_hook_enable)
        {
            return usleep_f(usec);
        }

        lim_webserver::Fiber::ptr fiber = lim_webserver::Fiber::GetThis();
        lim_webserver::IoManager *iom = lim_webserver::IoManager::GetThis();
        iom->addTimer(usec / 1000,
                      [iom, fiber]()
                      { iom->schedule(fiber); });
        lim_webserver::Fiber::YieldToHold();
        return 0;
    }

    int nanosleep(const struct timespec *req, struct timespec *rem)
    {
        if (!lim_webserver::t_hook_enable)
        {
            return nanosleep_f(req, rem);
        }

        int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
        lim_webserver::Fiber::ptr fiber = lim_webserver::Fiber::GetThis();
        lim_webserver::IoManager *iom = lim_webserver::IoManager::GetThis();
        iom->addTimer(timeout_ms, [iom, fiber]()
                      { iom->schedule(fiber); });
        lim_webserver::Fiber::YieldToHold();
        return 0;
    }

    int socket(int domain, int type, int protocol)
    {
        int fd = socket_f(domain, type, protocol);
        if (lim_webserver::t_hook_enable)
        {
            if (fd >= 0)
            {
                lim_webserver::FdMgr::GetInstance()->get(fd, true);
            }
        }
        return fd;
    }

    int connect_with_timeout(int fd, const sockaddr *addr, socklen_t addrlen, uint64_t timeout_ms)
    {
        if (!lim_webserver::t_hook_enable)
        {
            return connect_f(fd, addr, addrlen);
        }

        // 获取与文件描述符关联的 FdCtx 对象，用于跟踪文件描述符的状态
        lim_webserver::FdCtx::ptr ctx = lim_webserver::FdMgr::GetInstance()->get(fd);
        // 如果找不到关联的 FdCtx 对象或者文件描述符已关闭，设置 errno 为 EBADF（无效的文件描述符）并返回 -1
        if (!ctx || ctx->isClosed())
        {
            errno = EBADF;
            return -1;
        }

        // 如果文件描述符不是套接字，直接调用原始的 connect 函数并返回其结果
        if (!ctx->isSocket())
        {
            return connect_f(fd, addr, addrlen);
        }

        // 如果用户设置了非阻塞模式，直接调用原始的 connect 函数并返回其结果
        if (ctx->getUserNonblock())
        {
            return connect_f(fd, addr, addrlen);
        }

        // 调用原始的 connect 函数，尝试建立连接
        int n = connect_f(fd, addr, addrlen);

        // 如果连接成功，返回 0
        if (n == 0)
        {
            return 0;
        }
        // 如果连接没有立即成功且 errno 不是 EINPROGRESS，返回 n（通常为 -1）
        else if (n != -1 || errno != EINPROGRESS)
        {
            return n;
        }

        lim_webserver::IoManager *iom = lim_webserver::IoManager::GetThis();
        lim_webserver::Timer::ptr timer;
        std::shared_ptr<timer_info> tinfo(new timer_info);
        std::weak_ptr<timer_info> winfo(tinfo);

        // 如果设置了连接超时时间 timeout_ms  不等于 (uint64_t)-1
        if (timeout_ms != (uint64_t)-1)
        {
            // 创建一个定时器，当超时时取消连接
            timer = iom->addConditionTimer(
                timeout_ms,
                [winfo, fd, iom]()
                {
                    auto t = winfo.lock();
                    if (!t || t->cancelled)
                    {
                        return;
                    }
                    t->cancelled = ETIMEDOUT;
                    iom->cancelEvent(fd, lim_webserver::IoManager::WRITE);
                },
                winfo);
        }

        int rt = iom->addEvent(fd, lim_webserver::IoManager::WRITE);
        if (rt == 0)
        {
            lim_webserver::Fiber::YieldToHold();
            if (timer)
            {
                timer->cancel();
            }
            if (tinfo->cancelled)
            {
                errno = tinfo->cancelled;
                return -1;
            }
        }
        else
        {
            if (timer)
            {
                timer->cancel();
            }
            LIM_LOG_ERROR(lim_webserver::g_logger) << "connect addEvent(" << fd << ", WRITE) error";
        }

        int error = 0;
        socklen_t len = sizeof(int);
        if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len))
        {
            return -1;
        }
        if (!error)
        {
            return 0;
        }
        else
        {
            errno = error;
            return -1;
        }
    }

    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
    {
        return connect_with_timeout(sockfd, addr, addrlen, lim_webserver::s_connect_timeout);
    }

    int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
    {
        int fd = do_io(s, accept_f, "accept", lim_webserver::IoManager::READ, SO_RCVTIMEO, addr, addrlen);
        if (fd >= 0)
        {
            lim_webserver::FdMgr::GetInstance()->get(fd, true);
        }
        return fd;
    }

    ssize_t readv(int fd, const struct iovec *iov, int iovcnt)
    {
        return do_io(fd, readv_f, "readv", lim_webserver::IoManager::READ, SO_RCVTIMEO, iov, iovcnt);
    }

    ssize_t recv(int sockfd, void *buf, size_t len, int flags)
    {
        return do_io(sockfd, recv_f, "recv", lim_webserver::IoManager::READ, SO_RCVTIMEO, buf, len, flags);
    }

    ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen)
    {
        return do_io(sockfd, recvfrom_f, "recvfrom", lim_webserver::IoManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr, addrlen);
    }

    ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)
    {
        return do_io(sockfd, recvmsg_f, "recvmsg", lim_webserver::IoManager::READ, SO_RCVTIMEO, msg, flags);
    }

    ssize_t write(int fd, const void *buf, size_t count)
    {
        return do_io(fd, write_f, "write", lim_webserver::IoManager::WRITE, SO_SNDTIMEO, buf, count);
    }

    ssize_t writev(int fd, const struct iovec *iov, int iovcnt)
    {
        return do_io(fd, writev_f, "writev", lim_webserver::IoManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
    }

    ssize_t send(int s, const void *msg, size_t len, int flags)
    {
        return do_io(s, send_f, "send", lim_webserver::IoManager::WRITE, SO_SNDTIMEO, msg, len, flags);
    }

    ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
    {
        return do_io(s, sendto_f, "sendto", lim_webserver::IoManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
    }

    ssize_t sendmsg(int s, const struct msghdr *msg, int flags)
    {
        return do_io(s, sendmsg_f, "sendmsg", lim_webserver::IoManager::WRITE, SO_SNDTIMEO, msg, flags);
    }

    int close(int fd)
    {
        if (lim_webserver::t_hook_enable)
        {
            lim_webserver::FdCtx::ptr ctx = lim_webserver::FdMgr::GetInstance()->get(fd);
            if (ctx)
            {
                auto iom = lim_webserver::IoManager::GetThis();
                if (iom)
                {
                    iom->cancelAll(fd);
                }
                lim_webserver::FdMgr::GetInstance()->del(fd);
            }
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
            lim_webserver::FdCtx::ptr ctx = lim_webserver::FdMgr::GetInstance()->get(fd);
            if (!ctx || ctx->isClosed() || !ctx->isSocket())
            {
                return fcntl_f(fd, cmd, arg);
            }
            // 更新用户非阻塞标识。
            ctx->setUserNonblock(arg & O_NONBLOCK);
            // 根据系统非阻塞表示更新arg
            if (ctx->getSysNonblock())
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
            lim_webserver::FdCtx::ptr ctx = lim_webserver::FdMgr::GetInstance()->get(fd);
            if (!ctx || ctx->isClosed() || !ctx->isSocket())
            {
                return arg;
            }
            if (ctx->getUserNonblock())
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
            // 获取与文件描述符关联的 FdCtx 对象，用于跟踪文件描述符的状态
            lim_webserver::FdCtx::ptr ctx = lim_webserver::FdMgr::GetInstance()->get(d);
            // 如果文件描述符不存在、已关闭或者不是套接字，直接调用底层的 ioctl 函数并返回结果
            if (!ctx || ctx->isClosed() || !ctx->isSocket())
            {
                return ioctl_f(d, request, arg);
            }
            // 否则，更新 FdCtx 对象的用户非阻塞标志
            ctx->setUserNonblock(user_nonblock);
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
        if (!lim_webserver::t_hook_enable)
        {
            return setsockopt_f(sockfd, level, optname, optval, optlen);
        }
        // 如果选项级别是 SOL_SOCKET（套接字选项级别）
        if (level == SOL_SOCKET)
        {
            // 如果选项名称是 SO_RCVTIMEO（接收超时）或 SO_SNDTIMEO（发送超时）
            if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO)
            {
                lim_webserver::FdCtx::ptr ctx = lim_webserver::FdMgr::GetInstance()->get(sockfd);
                if (ctx)
                {
                    // 将 optval 转换为 timeval 结构体指针
                    const timeval *v = (const timeval *)optval;
                    // 计算超时值（以毫秒为单位）并设置到 FdCtx 对象中
                    ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
                }
            }
        }
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
}
