#include "EventLoop.h"

#include "coroutine.h"
#include "splog.h"

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/eventfd.h>

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_SYS();

    EventLoop::EventLoop()
        : m_poller(new EpollPoller())
    {
        m_wakeFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        LOG_TRACE(g_logger) << "EventLoop create wakeFd = " << m_wakeFd;
        addEvent(m_wakeFd, IoEvent::READ);
    }

    EventLoop::~EventLoop()
    {
        stop();
        clearEvent(m_wakeFd);
        ::close(m_wakeFd);
    }

    void EventLoop::stop()
    {
        if (!m_started)
        {
            return;
        }
        m_started = false;
        tickle();
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

    void EventLoop::tickle()
    {
        co[&]
        {
            LOG_TRACE(g_logger) << "EventLoop:tickle()";
            uint64_t one = 1;
            ssize_t n = write(m_wakeFd, &one, sizeof one);
            if (n != sizeof one)
            {
                LOG_ERROR(g_logger) << "EventLoop::tickle() writes " << n << " bytes instead of 8";
            }
            n = read(m_wakeFd, &one, sizeof one);
            LOG_INFO(g_logger) << n;
            if (n != sizeof one)
            {
                LOG_ERROR(g_logger) << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
            }
        };
    }

    void EventLoop::run()
    {
        while (m_started)
        {
            m_poller->poll(10000);
        }
    }

} // namespace lim_webserver
