#include "Poller.h"

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

namespace lim_webserver
{
    bool Poller::hasContext(FdContext *context)
    {
        auto it = m_contexts_map.find(context->fd());
        return it != m_contexts_map.end() && it->second == context;
    }

    EpollPoller::EpollPoller(EventLoop *loop)
        : Poller(loop), m_epfd(epoll_create(5000))
    {
    }

    EpollPoller::~EpollPoller()
    {
        ::close(m_epfd);
    }

    void EpollPoller::updateContext(FdContext *context)
    {
        // 获取Context状态与句柄
        FdContextState state = context->state();
        int fd = context->fd();
        // 若为新Context或为空Context
        if (state == FdContextState::NEW || state == FdContextState::DISCARD)
        {
            // 新Context则存入红黑树
            if (state == FdContextState::NEW)
            {
                assert(m_contexts_map.find(fd) == m_contexts_map.end());
                m_contexts_map[fd] = context;
            }
            else // 老Context则确保存在
            {
                assert(m_contexts_map.find(fd) != m_contexts_map.end());
                assert(m_contexts_map[fd] == context);
            }
            // 设置Context状态为常规
            context->setState(FdContextState::EXIST);
            // 添加句柄
            update(EPOLL_CTL_ADD, context);
        }
        else // 已经存在的Context则确保其状态
        {
            assert(m_contexts_map.find(fd) != m_contexts_map.end());
            assert(m_contexts_map[fd] == context);
            assert(state == FdContextState::EXIST);
            // 若Context没有事件则删除
            if (context->isNoneEvent())
            {
                update(EPOLL_CTL_DEL, context);
                context->setState(FdContextState::DISCARD);
            }
            else // 有事件则修改
            {
                update(EPOLL_CTL_MOD, context);
            }
        }
    }

    void EpollPoller::removeContext(FdContext *context)
    {
        // 获得文件句柄
        int fd = context->fd();
        assert(m_contexts_map.find(fd) != m_contexts_map.end());
        assert(m_contexts_map[fd] == context);
        assert(context->isNoneEvent());

        // 获得状态
        FdContextState state = context->state();
        assert(state == FdContextState::EXIST || state == FdContextState::DISCARD);

        // 移除Context
        size_t n = m_contexts_map.erase(fd);
        (void)n;
        assert(n == 1);

        // 若为活动句柄，则废弃
        if (state == FdContextState::EXIST)
        {
            update(EPOLL_CTL_DEL, context);
        }

        // 重置句柄状态
        context->setState(FdContextState::NEW);
    }

    void EpollPoller::poll(int ms)
    {
        // 等待文件描述符
        int n = epoll_wait(m_epfd, &*m_event_vec.begin(), m_event_vec.size(), ms);

        // 若有则触发回调
        if (n > 0)
        {
            for (auto i = 0; i < n; ++i)
            {
                epoll_event &event = m_event_vec[i];
                FdContext *context = static_cast<FdContext *>(event.data.ptr);
                // 根据得到的events触发回调
                context->trigger(event.events);
            }
        }
    }

    void EpollPoller::update(int op, FdContext *context)
    {
        struct epoll_event ev;
        ev.events = context->event();
        ev.data.ptr = context;
        int fd = context->fd();

        int rt = epoll_ctl(m_epfd, op, fd, &ev);
    }

} // namespace lim_webserver
