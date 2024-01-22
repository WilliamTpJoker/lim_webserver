#include "EventLoop.h"

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>

namespace lim_webserver
{
    EventLoop::EventLoop()
        : m_poller(new EpollPoller(this))
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
        MutexType::Lock lock(m_mutex);
        m_started = false;
    }

    void EventLoop::run()
    {
        while (m_started)
        {
            m_poller->poll(10000);
        }
    }

} // namespace lim_webserver
