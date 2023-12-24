#include "IoManager.h"
#include "Logger.h"

#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_NAME("system");

    IoManager::IoManager(size_t threads, bool use_caller, const std::string &name)
        : Scheduler(threads, use_caller, name)
    {
        m_epollFd = epoll_create(5000);
        ASSERT(m_epollFd > 0);

        int rt = pipe(m_ticlefds);
        ASSERT(!rt);

        // 创建 epoll_event 对象
        epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        // 读事件 与 开启边缘触发
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = m_ticlefds[0];

        // 设置管道的文件描述符为非阻塞
        rt = fcntl(m_ticlefds[0], F_SETFL, O_NONBLOCK);
        ASSERT(!rt);

        // 将管道文件描述符添加到 epoll 中监听可读事件
        rt = epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_ticlefds[0], &event);
        ASSERT(!rt);

        // 初始化 FdContext 列表的大小
        contextResize(32);

        start();
    }

    IoManager::~IoManager()
    {
        stop();
        close(m_epollFd);
        close(m_ticlefds[0]);
        close(m_ticlefds[1]);

        for (size_t i = 0; i < m_fdContext_list.size(); ++i)
        {
            if (m_fdContext_list[i])
            {
                delete m_fdContext_list[i];
            }
        }
    }

    int IoManager::addEvent(int fd, IoEvent event, std::function<void()> callback)
    {
        FdContext *fd_context = nullptr;
        {
            RWMutexType::ReadLock lock(m_mutex);
            // 若句柄超出句柄列表容量，则扩容列表
            if ((int)m_fdContext_list.size() <= fd)
            {
                lock.unlock();
                RWMutexType::WriteLock lock2(m_mutex);
                contextResize(fd * 1.5);
            }
            fd_context = m_fdContext_list[fd];
        }

        FdContext::MutexType::Lock lock3(fd_context->mutex);
        // 检查给定的事件是否已经存在于fd_context 的事件中,存在则报错
        if (fd_context->events & event)
        {
            LOG_ERROR(g_logger) << "addEvent assert fd=" << fd << " event=" << (EPOLL_EVENTS)event << " fd_ctx.event=" << (EPOLL_EVENTS)fd_context->events;
            ASSERT(!(fd_context->events & event));
        }

        IoEvent new_event = (IoEvent)(fd_context->events | event);
        // 根据原来的事件类型确定要执行的操作类型（ EPOLL_CTL_MOD：修改事件，EPOLL_CTL_ADD：添加事件）
        int op = fd_context->events ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;
        // 创建epoll事件对象并设置事件类型和关联数据
        epoll_event epEvent;
        // 保持边缘触发
        epEvent.events = EPOLLET | new_event;
        epEvent.data.ptr = fd_context;

        // 执行 epoll_ctl 操作，添加/修改事件
        int rt = epoll_ctl(m_epollFd, op, fd, &epEvent);
        if (rt == -1)
        {
            // 操作失败，返回错误码
            LOG_ERROR(g_logger) << "epoll_ctl(" << m_epollFd << ", " << op << ", " << fd << ", " << (EPOLL_EVENTS)epEvent.events << "):"
                                    << rt << " (" << errno << ") (" << strerror(errno) << ") fd_ctx->events=" << (EPOLL_EVENTS)fd_context->events;
            return -1;
        }

        ++m_pendingEventCount;
        // 更新 fd_context 中的事件
        fd_context->events = new_event;
        // 获取事件上下文
        FdContext::EventContext &ev_context = fd_context->getContext(event);
        ASSERT(!ev_context.scheduler && !ev_context.fiber && !ev_context.callback);

        ev_context.scheduler = Scheduler::GetThis();
        // 更新事件所要执行的任务，若指定回调，则直接交换；若未指定，则将该协程添加进去
        if (callback)
        {
            ev_context.callback.swap(callback);
        }
        else
        {
            ev_context.fiber = Fiber::GetThis();
            ASSERT(ev_context.fiber->getState() == FiberState::EXEC, "state=" << ev_context.fiber->getState());
        }
        return 0;
    }

    bool IoManager::delEvent(int fd, IoEvent event)
    {
        FdContext *fd_context = nullptr;
        {
            RWMutexType::ReadLock lock(m_mutex);
            if ((int)m_fdContext_list.size() <= fd)
            {
                return false;
            }
            fd_context = m_fdContext_list[fd];
        }
        FdContext::MutexType::Lock lock2(fd_context->mutex);
        // 检查给定的事件是否已经存在于fd_context 的事件中，不存在则返回
        if (!(fd_context->events & event))
        {
            return false;
        }

        IoEvent new_events = (IoEvent)(fd_context->events & ~event);
        // 根据新的事件类型确定要执行的操作类型（ EPOLL_CTL_MOD：修改事件，EPOLL_CTL_DEL：删除事件）
        int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        // 创建epoll事件对象并设置事件类型和关联数据
        epoll_event epEvent;
        epEvent.events = EPOLLET | new_events;
        epEvent.data.ptr = fd_context;

        // 执行 epoll_ctl 操作，删除/修改事件
        int rt = epoll_ctl(m_epollFd, op, fd, &epEvent);
        if (rt == -1)
        {
            // 操作失败，返回错误码
            LOG_ERROR(g_logger) << "epoll_ctl(" << m_epollFd << ", " << op << ", " << fd << ", " << (EPOLL_EVENTS)epEvent.events << "):"
                                    << rt << " (" << errno << ") (" << strerror(errno) << ")";
            return false;
        }

        --m_pendingEventCount;
        // 更新 fd_context 中的事件
        fd_context->events = new_events;
        // 重置事件上下文
        fd_context->resetContext(event);
        return true;
    }

    bool IoManager::cancelEvent(int fd, IoEvent event)
    {
        FdContext *fd_context = nullptr;
        {
            RWMutexType::ReadLock lock(m_mutex);
            if ((int)m_fdContext_list.size() <= fd)
            {
                return false;
            }
            fd_context = m_fdContext_list[fd];
        }
        FdContext::MutexType::Lock lock2(fd_context->mutex);
        // 检查给定的事件是否已经存在于fd_context 的事件中，不存在则返回
        if (!(fd_context->events & event))
        {
            return false;
        }

        IoEvent new_events = (IoEvent)(fd_context->events & ~event);
        // 根据新的事件类型确定要执行的操作类型（ EPOLL_CTL_MOD：修改事件，EPOLL_CTL_DEL：删除事件）
        int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        // 创建epoll事件对象并设置事件类型和关联数据
        epoll_event epEvent;
        epEvent.events = EPOLLET | new_events;
        epEvent.data.ptr = fd_context;

        // 执行 epoll_ctl 操作，删除/修改事件
        int rt = epoll_ctl(m_epollFd, op, fd, &epEvent);
        if (rt == -1)
        {
            // 操作失败，返回错误码
            LOG_ERROR(g_logger) << "epoll_ctl(" << m_epollFd << ", " << op << ", " << fd << ", " << (EPOLL_EVENTS)epEvent.events << "):"
                                    << rt << " (" << errno << ") (" << strerror(errno) << ")";
            return false;
        }

        // 触发该事件
        fd_context->triggerEvent(event);
        --m_pendingEventCount;
        return true;
    }

    bool IoManager::cancelAll(int fd)
    {
        FdContext *fd_context = nullptr;
        {
            RWMutexType::ReadLock lock(m_mutex);
            if ((int)m_fdContext_list.size() <= fd)
            {
                return false;
            }
            fd_context = m_fdContext_list[fd];
        }
        FdContext::MutexType::Lock lock2(fd_context->mutex);
        // 检查给定的句柄上下文中是否存在事件，不存在则返回
        if (!fd_context->events)
        {
            return false;
        }

        // EPOLL_CTL_DEL：删除事件
        int op = EPOLL_CTL_DEL;
        // 创建epoll事件对象并设置事件类型和关联数据
        epoll_event epEvent;
        epEvent.events = 0;
        epEvent.data.ptr = fd_context;

        // 执行 epoll_ctl 操作，删除/修改事件
        int rt = epoll_ctl(m_epollFd, op, fd, &epEvent);
        if (rt == -1)
        {
            // 操作失败，返回错误码
            LOG_ERROR(g_logger) << "epoll_ctl(" << m_epollFd << ", " << op << ", " << fd << ", " << (EPOLL_EVENTS)epEvent.events << "):"
                                    << rt << " (" << errno << ") (" << strerror(errno) << ")";
            return false;
        }

        // 触发所有事件
        if (fd_context->events & IoEvent::READ)
        {
            fd_context->triggerEvent(IoEvent::READ);
            --m_pendingEventCount;
        }
        if (fd_context->events & IoEvent::WRITE)
        {
            fd_context->triggerEvent(IoEvent::WRITE);
            --m_pendingEventCount;
        }
        ASSERT(fd_context->events == 0);
        return true;
    }

    IoManager *IoManager::GetThis()
    {
        return dynamic_cast<IoManager *>(Scheduler::GetThis());
    }

    IoManager::FdContext::EventContext &IoManager::FdContext::getContext(IoManager::IoEvent event)
    {
        if (event == IoEvent::READ)
        {
            return read;
        }
        else if (event == IoEvent::WRITE)
        {
            return write;
        }
        ASSERT(false, "getContext");
        throw std::invalid_argument("getContext invalid event");
    }

    void IoManager::FdContext::resetContext(IoEvent event)
    {
        EventContext &ev_context = getContext(event);
        ev_context.scheduler = nullptr;
        ev_context.fiber.reset();
        ev_context.callback = nullptr;
    }

    void IoManager::FdContext::triggerEvent(IoEvent event)
    {
        ASSERT(events & event);
        events = (IoEvent)(events & ~event);
        EventContext &ev_context = getContext(event);
        if (ev_context.callback)
        {
            ev_context.scheduler->schedule(&ev_context.callback);
        }
        else
        {
            ev_context.scheduler->schedule(&ev_context.fiber);
        }
        ev_context.scheduler = nullptr;
        return;
    }

    void IoManager::tickle()
    {
        if (!hasIdleThreads())
        {
            return;
        }
        int rt = write(m_ticlefds[1], "T", 1);
        ASSERT(rt == 1);
    }

    bool IoManager::onStop()
    {
        uint64_t timeout = 0;
        return onStop(timeout);
    }

    bool IoManager::onStop(uint64_t &timeout)
    {
        timeout = getNextTimer();
        return timeout == ~0ull && m_pendingEventCount == 0 && Scheduler::onStop();
    }

    void IoManager::onIdle()
    {
        const uint64_t MAX_EVENTS = 256;
        epoll_event *events = new epoll_event[MAX_EVENTS]();
        std::shared_ptr<epoll_event> shared_events(events, [](epoll_event *ptr)
                                                   { delete[] ptr; });

        while (true)
        {
            uint64_t next_timeout = 0;
            if (onStop(next_timeout))
            {
                LOG_INFO(g_logger) << "name=" << getName() << " idle stopping exit";
                break;
            }

            int rt = 0;
            do
            {
                static const int MAX_TIMEOUT = 1000;
                if (next_timeout != ~0ull)
                {
                    next_timeout = (int)next_timeout > MAX_TIMEOUT ? MAX_TIMEOUT : next_timeout;
                }
                else
                {
                    next_timeout = MAX_TIMEOUT;
                }
                // LOG_DEBUG(g_logger) << "next_timeout=" << next_timeout;
                rt = epoll_wait(m_epollFd, events, MAX_EVENTS, (int)next_timeout);
                // LOG_DEBUG(g_logger) << "epoll_wait rt=" << rt;
                if (rt < 0 && errno == EINTR)
                {
                }
                else
                {
                    break;
                }
            } while (true);

            // 处理定时器
            std::vector<std::function<void()>> callback_list;
            listExpiredCallback(callback_list);
            if (!callback_list.empty())
            {
                schedule(callback_list.begin(), callback_list.end());
                callback_list.clear();
            }

            // 处理io消息
            for (int i = 0; i < rt; ++i)
            {
                epoll_event &event = events[i];
                if (event.data.fd == m_ticlefds[0])
                {
                    uint8_t dummy[256];
                    while (read(m_ticlefds[0], dummy, sizeof(dummy)) > 0)
                    {
                    };
                    continue;
                }

                FdContext *fd_context = (FdContext *)event.data.ptr;
                FdContext::MutexType::Lock lock(fd_context->mutex);
                if (event.events & (EPOLLERR | EPOLLHUP))
                {
                    event.events |= (EPOLLIN | EPOLLOUT) & fd_context->events;
                }
                int real_events = IoEvent::NONE;
                if (event.events & EPOLLIN)
                {
                    real_events |= IoEvent::READ;
                }
                if (event.events & EPOLLOUT)
                {
                    real_events |= IoEvent::WRITE;
                }

                if ((fd_context->events & real_events) == IoEvent::NONE)
                {
                    continue;
                }
                int left_events = (fd_context->events & ~real_events);
                int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
                event.events = EPOLLET | left_events;

                int rt2 = epoll_ctl(m_epollFd, op, fd_context->fd, &event);
                if (rt2)
                {
                    LOG_ERROR(g_logger) << "epoll_ctl(" << m_epollFd << ", " << op << ", " << fd_context->fd << ", " << (EPOLL_EVENTS)event.events << "):"
                                            << rt2 << " (" << errno << ") (" << strerror(errno) << ")";
                    continue;
                }
                if (real_events & READ)
                {
                    fd_context->triggerEvent(READ);
                    --m_pendingEventCount;
                }
                if (real_events & WRITE)
                {
                    fd_context->triggerEvent(WRITE);
                    --m_pendingEventCount;
                }
            }
            Fiber::ptr cur = Fiber::GetThis();
            auto raw_ptr = cur.get();
            cur.reset();

            raw_ptr->swapOut();
        }
    }

    void IoManager::onTimerInsertedAtFront()
    {
        tickle();
    }

    void IoManager::contextResize(size_t size)
    {
        m_fdContext_list.resize(size);

        for (size_t i = 0; i < m_fdContext_list.size(); ++i)
        {
            if (!m_fdContext_list[i])
            {
                m_fdContext_list[i] = new FdContext;
                m_fdContext_list[i]->fd = i;
            }
        }
    }
}