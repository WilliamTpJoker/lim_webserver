#include "Poller.h"
#include "splog.h"

#include <assert.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_SYS();

    Poller::Poller() { channelVecResize(32); }

    bool Poller::addEvent(int fd, IoEvent event)
    {
        MutexType::Lock lock(m_mutex);
        if ((int)m_channel_vec.size() <= fd)
        {
            lock.unlock();
            channelVecResize(fd * 2);
        }
        lock.lock();
        IoChannel *channel = m_channel_vec[fd];
        return channel->addEvent(event);
    }

    bool Poller::cancelEvent(int fd, IoEvent event)
    {
        MutexType::Lock lock(m_mutex);
        if ((int)m_channel_vec.size() <= fd)
        {
            return false;
        }
        IoChannel *channel = m_channel_vec[fd];
        return channel->cancelEvent(event);
    }

    bool Poller::clearEvent(int fd)
    {
        MutexType::Lock lock(m_mutex);
        if ((int)m_channel_vec.size() <= fd)
        {
            return false;
        }
        IoChannel *channel = m_channel_vec[fd];
        return channel->clearEvent();
    }

    void Poller::channelVecResize(size_t size)
    {
        MutexType::Lock lock(m_mutex);
        m_channel_vec.resize(size);
        for (size_t i = 0; i < m_channel_vec.size(); ++i)
        {
            if (!m_channel_vec[i])
            {
                m_channel_vec[i] = new IoChannel(i);
            }
        }
    }

    EpollPoller::EpollPoller() : m_epfd(epoll_create(1)), m_event_vec(16) {}

    EpollPoller::~EpollPoller() { ::close(m_epfd); }

    bool EpollPoller::addEvent(int fd, IoEvent event)
    {
        if (!Poller::addEvent(fd, event))
        {
            return false;
        }
        IoChannel *channel = m_channel_vec[fd];
        IoChannelState state = channel->state();

        // (老event)无event加，有event改
        if (state == IoChannelState::NEW || state == IoChannelState::DISCARD)
        {
            update(EPOLL_CTL_ADD, channel);
            channel->setState(IoChannelState::EXIST);
        }
        else
        {
            ASSERT(state == IoChannelState::EXIST);
            update(EPOLL_CTL_MOD, channel);
        }
        return true;
    }

    bool EpollPoller::cancelEvent(int fd, IoEvent event)
    {
        if (!Poller::cancelEvent(fd, event))
        {
            return false;
        }
        IoChannel *channel = m_channel_vec[fd];
        IoChannelState state = channel->state();

        // (新event)无event删，有event改
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->setState(IoChannelState::DISCARD);
        }
        else
        {
            ASSERT(state == IoChannelState::EXIST);
            update(EPOLL_CTL_MOD, channel);
        }
        return true;
    }

    bool EpollPoller::clearEvent(int fd)
    {
        if (!Poller::clearEvent(fd))
        {
            return false;
        }
        IoChannel *channel = m_channel_vec[fd];
        IoChannelState state = channel->state();

        if (state == IoChannelState::EXIST)
        {
            update(EPOLL_CTL_DEL, channel);
            channel->setState(IoChannelState::DISCARD);
        }
    }

    void EpollPoller::poll(int ms)
    {
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

    void EpollPoller::update(int op, IoChannel *channel)
    {
        struct epoll_event ev;
        ev.events = channel->event() | EPOLLET;
        ev.data.ptr = channel;
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
