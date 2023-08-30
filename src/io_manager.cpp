#include "io_manager.h"
#include <sys/epoll.h>
#include <fcntl.h>

namespace lim_webserver
{
    static Shared_ptr<Logger> g_logger = LIM_LOG_NAME("system");

    IoManager::IoManager(size_t threads, bool use_caller, const std::string &name)
        : Scheduler(threads, use_caller, name)
    {
        m_epfd = epoll_create(5000);
        LIM_ASSERT(m_epfd > 0);

        int rt = pipe(m_ticlefds);
        LIM_ASSERT(rt);

        epoll_event event{};
        event.data.fd = m_ticlefds[0];
        // 监听可读事件 与 开启边缘触发
        event.events = EPOLLIN | EPOLLET;

        rt = fcntl(m_ticlefds[0], F_SETFL, O_NONBLOCK);
        LIM_ASSERT(rt);

        rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_ticlefds[0], &event);
        LIM_ASSERT(rt);

        m_fdContext_list.resize(64);

        start();
    }

    IoManager::~IoManager()
    {
        stop();
        close(m_epfd);
        close(m_ticlefds[0]);
        close(m_ticlefds[1]);

        for (size_t i = 0; i < m_fdContext_list.size(); ++i)
        {
            if (m_fdContext_list[i])
            {
                delete m_fdContext_list[i];
            }
        }
    }

    int IoManager::addEvent(int fd, IoEvent event, std::function<void()> callback)
    {
        
        return 0;
    }

    bool IoManager::delEvent(int fd, IoEvent event)
    {
        return false;
    }

    bool IoManager::cancelEvent(int fd, IoEvent event)
    {
        return false;
    }

    bool IoManager::cancelAll(int fd)
    {
        return false;
    }

    IoManager *IoManager::GetThis()
    {
        return nullptr;
    }

    void IoManager::tickle()
    {
    }

    bool IoManager::onStop()
    {
        return false;
    }
}