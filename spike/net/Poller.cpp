#include "Poller.h"
#include "splog/splog.h"

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_SYS();

    Poller::Poller(EventLoop *loop)
        : m_loop(loop)
    {
        
    }

    IoChannel *Poller::getChannel(int fd)
    {
        auto it = m_channels_map.find(fd);
        return it->second;
    }

    EpollPoller::EpollPoller(EventLoop *loop)
        : Poller(loop), m_epfd(epoll_create(5000))
    {
        if (m_epfd < 0)
        {
            LOG_FATAL(g_logger) << "EPollPoller::EPollPoller";
        }
    }

    EpollPoller::~EpollPoller()
    {
        ::close(m_epfd);
    }

    void EpollPoller::updateChannel(IoChannel *channel)
    {
        // 获取Channel状态与句柄
        IoChannelState state = channel->state();
        LOG_TRACE(g_logger) << "fd = " << channel->fd()
                            << " events = {" << channel->eventsToString() << "} state = " << channel->stateToString();
        int fd = channel->fd();
        // 若为新Channel或为空Channel
        if (state == IoChannelState::NEW || state == IoChannelState::DISCARD)
        {
            // 新Channel则存入红黑树
            if (state == IoChannelState::NEW)
            {
                assert(m_channels_map.find(fd) == m_channels_map.end());
                m_channels_map[fd] = channel;
            }
            else // 老Channel则确保存在
            {
                assert(m_channels_map.find(fd) != m_channels_map.end());
                assert(m_channels_map[fd] == channel);
            }
            // 设置Channel状态为常规
            channel->setState(IoChannelState::EXIST);
            // 添加句柄
            update(EPOLL_CTL_ADD, channel);
        }
        else // 已经存在的Channel则确保其状态
        {
            assert(m_channels_map.find(fd) != m_channels_map.end());
            assert(m_channels_map[fd] == channel);
            assert(state == IoChannelState::EXIST);
            // 若Channel没有事件则删除
            if (channel->isNoneEvent())
            {
                update(EPOLL_CTL_DEL, channel);
                channel->setState(IoChannelState::DISCARD);
            }
            else // 有事件则修改
            {
                update(EPOLL_CTL_MOD, channel);
            }
        }
    }

    void EpollPoller::removeChannel(IoChannel *channel)
    {
        // 获得文件句柄
        int fd = channel->fd();
        LOG_TRACE(g_logger) << "fd = " << fd;
        assert(m_channels_map.find(fd) != m_channels_map.end());
        assert(m_channels_map[fd] == channel);
        assert(channel->isNoneEvent());

        // 获得状态
        IoChannelState state = channel->state();
        assert(state == IoChannelState::EXIST || state == IoChannelState::DISCARD);

        // 移除Channel
        size_t n = m_channels_map.erase(fd);
        (void)n;
        assert(n == 1);

        // 若为活动句柄，则废弃
        if (state == IoChannelState::EXIST)
        {
            update(EPOLL_CTL_DEL, channel);
        }

        // 重置句柄状态
        channel->setState(IoChannelState::NEW);
    }

    void EpollPoller::poll(int ms)
    {
        LOG_TRACE(g_logger) << "fd total count " << m_channels_map.size();
        int n = ::epoll_wait(m_epfd, &*m_event_vec.begin(), m_event_vec.size(), ms);
        int savedErrno = errno;
        // 若有则触发
        if (n > 0)
        {
            LOG_TRACE(g_logger) << n << " events happened";
            for (auto i = 0; i < n; ++i)
            {
                epoll_event &event = m_event_vec[i];
                IoChannel *channel = static_cast<IoChannel *>(event.data.ptr);
                // 根据得到的events触发
                channel->trigger(event.events);
            }
        }
        else if (n == 0)
        {
            LOG_TRACE(g_logger) << "nothing happened";
        }
        else
        {
            // error happens, log uncommon ones
            if (savedErrno != EINTR)
            {
                errno = savedErrno;
                LOG_ERROR(g_logger) << "EpollPoller::poll()";
            }
        }
    }

    void EpollPoller::update(int op, IoChannel *channel)
    {
        struct epoll_event ev;
        ev.events = channel->event();
        ev.data.ptr = channel;
        int fd = channel->fd();
        LOG_TRACE(g_logger) << "epoll_ctl op = " << opToString(op)
                            << " fd = " << fd << " event = { " << channel->eventsToString() << " }";
        if (::epoll_ctl(m_epfd, op, fd, &ev) < 0)
        {
            if (op == EPOLL_CTL_DEL)
            {
                LOG_ERROR(g_logger) << "epoll_ctl op =" << opToString(op) << " fd =" << fd;
            }
            else
            {
                LOG_FATAL(g_logger) << "epoll_ctl op =" << opToString(op) << " fd =" << fd;
            }
        }
    }

    const char *EpollPoller::opToString(int op)
    {
        switch (op)
        {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR op");
            return "Unknown Operation";
        }
    }

} // namespace lim_webserver
