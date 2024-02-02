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
        m_wakeChannel = IoChannel::Create(m_wakeFd);
        m_wakeChannel->addEvent(IoEvent::READ);
    }

    EventLoop::~EventLoop()
    {
        stop();
        m_wakeChannel->clearEvent();
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

    void EventLoop::tickle()
    {
        LOG_TRACE(g_logger) << "EventLoop:tickle()";
        uint64_t one = 1;
        ssize_t n = write(m_wakeFd, &one, sizeof one);
        if (n != sizeof one)
        {
            LOG_ERROR(g_logger) << "EventLoop::tickle() writes " << n << " bytes instead of 8";
        }
    }

    void EventLoop::run()
    {
        while (m_started)
        {
            m_poller->poll(10000);
        }
    }

} // namespace lim_webserver
