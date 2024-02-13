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
        m_poller->bindEventFd(m_wakeFd);
    }

    EventLoop::~EventLoop()
    {
        stop();
        ::close(m_wakeFd);
    }

    void EventLoop::stop() { tickle(); }

    void EventLoop::tickle()
    {
        if (m_idled)
        {
            LOG_TRACE(g_logger) << "EventLoop of " << m_scheduler->name() << " tickle()";
            uint64_t one = 1;
            ssize_t n = write(m_wakeFd, &one, sizeof one);
            if (n != sizeof one)
            {
                LOG_ERROR(g_logger) << "EventLoop::tickle() writes " << n << " bytes instead of 8";
            }
        }
        // 若处于工作态，则尝试通知
        else
        {
            // 若已被通知，则不通知
            if (!m_notified)
            {
                m_notified = true;
            }
        }
    }

    void EventLoop::idle()
    {
        // 若在企图休眠前接受到了调度器的通知，则不休眠
        if (m_notified)
        {
            m_notified = false;
            return;
        }

        // 进入等待状态
        m_idled = true;
        m_poller->poll(10000);
        m_idled = false;
    }

    bool EventLoop::stopping() { return Processor::stopping(); }

} // namespace lim_webserver
