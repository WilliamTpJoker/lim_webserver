#include "EventLoop.h"

#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>

namespace lim_webserver
{
    EventLoop::EventLoop()
    : m_poller(new EpollPoller(this)), m_funcHandler(new CoHandler(g_Scheduler))
    {
        m_thread = Thread::Create([this]
                                  { this->run(); },
                                  "EventLoop");
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
        m_thread->join();
    }



    void EventLoop::run()
    {
        while (true)
        {

        }
    }

} // namespace lim_webserver
