#include "Poller.h"
#include "splog.h"

#include <assert.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_SYS();

    Poller::Poller() {}

    EpollPoller::EpollPoller() : m_epfd(epoll_create(1)), m_event_vec(16) {}

    EpollPoller::~EpollPoller() { ::close(m_epfd); }

    void EpollPoller::updateChannel(IoChannel::ptr channel)
    {
        IoChannelState state = channel->state();
        LOG_TRACE(g_logger) << "fd = " << channel->fd() << " events = " << channel->eventsToString() << " state = " << channel->stateToString();
        if (state == IoChannelState::NEW || state == IoChannelState::DISCARD)
        {
            // a new one, add with EPOLL_CTL_ADD
            int fd = channel->fd();
            if (state == IoChannelState::NEW)
            {
                ASSERT(m_channel_map.find(fd) == m_channel_map.end());
                m_channel_map[fd] = channel;
            }
            else // index == kDeleted
            {
                ASSERT(m_channel_map.find(fd) != m_channel_map.end());
                ASSERT(m_channel_map[fd] == channel);
            }

            channel->setState(IoChannelState::EXIST);
            update(EPOLL_CTL_ADD, channel);
        }
        else
        {
            // update existing one with EPOLL_CTL_MOD/DEL
            int fd = channel->fd();
            (void)fd;
            ASSERT(m_channel_map.find(fd) != m_channel_map.end());
            ASSERT(m_channel_map[fd] == channel);
            ASSERT(state == IoChannelState::EXIST);
            if (channel->isNoneEvent())
            {
                update(EPOLL_CTL_DEL, channel);
                channel->setState(IoChannelState::DISCARD);
            }
            else
            {
                update(EPOLL_CTL_MOD, channel);
            }
        }
    }

    void EpollPoller::removeChannel(IoChannel::ptr channel)
    {
        int fd = channel->fd();
        LOG_TRACE(g_logger) << "fd = " << fd << "removeChannel";
        ASSERT(m_channel_map.find(fd) != m_channel_map.end());
        ASSERT(m_channel_map[fd] == channel);
        ASSERT(channel->isNoneEvent());
        IoChannelState state = channel->state();
        ASSERT(state == IoChannelState::EXIST || state == IoChannelState::DISCARD);
        size_t n = m_channel_map.erase(fd);
        (void)n;
        ASSERT(n == 1);

        if (state == IoChannelState::EXIST)
        {
            update(EPOLL_CTL_DEL, channel);
        }
        channel->setState(IoChannelState::NEW);
    }

    void EpollPoller::poll(int ms)
    {
        LOG_TRACE(g_logger) << "current fd total count " << m_channel_map.size();
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
            if ((size_t)n == m_event_vec.size())
            {
                m_event_vec.resize(m_event_vec.size() * 2);
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
                LOG_ERROR(g_logger) << "EpollPoller::poll() errono = " << strerror(errno);
            }
        }
    }

    void EpollPoller::update(int op, IoChannel::ptr channel)
    {
        struct epoll_event ev;
        ev.events = channel->event() | EPOLLET;
        ev.data.ptr = channel.get();
        int fd = channel->fd();
        LOG_TRACE(g_logger) << "epoll_ctl op = " << opToString(op) << " fd = " << fd << " event = { " << channel->eventsToString() << " }";
        if (::epoll_ctl(m_epfd, op, fd, &ev) < 0)
        {
            if (op == EPOLL_CTL_DEL)
            {
                LOG_ERROR(g_logger) << "epoll_ctl op = " << opToString(op) << " fd =" << fd;
            }
            else
            {
                LOG_FATAL(g_logger) << "epoll_ctl op = " << opToString(op) << " fd =" << fd;
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
