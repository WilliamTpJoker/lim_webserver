#include "EventLoop.h"

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>

namespace lim_webserver
{
    EventLoop::EventLoop()
        : m_poller(new EpollPoller())
    {
    }

    EventLoop::~EventLoop()
    {
        stop();
    }

    void EventLoop::stop()
    {
        if (!m_started)
        {
            return;
        }
        m_started = false;
    }

    bool EventLoop::addEvent(int fd, IoEvent event)
    {
        return m_poller->addEvent(fd, event);
    }

    bool EventLoop::cancelEvent(int fd, IoEvent event)
    {
        return m_poller->cancelEvent(fd, event);
    }

    bool EventLoop::clearEvent(int fd)
    {
        return m_poller->clearEvent(fd);
    }

    void EventLoop::run()
    {
        while (m_started)
        {
            m_poller->poll(10000);
        }
    }

} // namespace lim_webserver
