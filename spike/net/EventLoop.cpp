#include "EventLoop.h"

#include "coroutine.h"
#include "splog.h"

#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_SYS();

    EventLoop::EventLoop(Scheduler *scheduler, int id) : m_poller(new EpollPoller()), Processor(scheduler, id)
    {
        m_wakeFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        m_wakeChannel = IoChannel::Create(m_wakeFd);
        m_wakeChannel->addEvent(IoEvent::READ);
    }

    EventLoop::~EventLoop()
    {
        stop();
        m_wakeChannel->clearEvent();
        ::close(m_wakeFd);
    }

    void EventLoop::stop() { tickle(); }

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

    void EventLoop::idle() { m_poller->poll(10000); }

    bool EventLoop::stopping() { return Processor::stopping(); }

} // namespace lim_webserver
