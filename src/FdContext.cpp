#include "FdContext.h"
#include "EventLoop.h"

#include <sys/epoll.h>

namespace lim_webserver
{
    FdContext::FdContext(EventLoop *loop, int fd)
        : m_loop(loop), m_fd(fd)
    {
    }

    FdContext::~FdContext()
    {
    }

    void FdContext::update()
    {
        m_loop->updateContext(this);
    }

    void FdContext::remove()
    {
        m_loop->removeContext(this);
    }

    void FdContext::trigger(uint32_t op)
    {
        // 发生挂起事件
        if ((op & EPOLLHUP) && !(op & EPOLLIN))
        {
            if (m_closeCallback)
            {
                m_loop->handleFunction(m_closeCallback);
            }
        }
        // 发生错误事件
        if (op & EPOLLERR)
        {
            if (m_errorCallback)
            {
                m_loop->handleFunction(m_errorCallback);
            }
        }
        // 读操作   紧急读操作  关闭写半部分
        if (op & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
        {
            if (m_readCallback)
            {
                m_loop->handleFunction(m_readCallback);
            }
        }
        // 写操作
        if (op & EPOLLOUT)
        {
            if (m_writeCallback)
            {
                m_loop->handleFunction(m_writeCallback);
            }
        }
    }

    FdContext::EventCallback &FdContext::getCallback(IoEvent event)
    {
        if (event == IoEvent::READ)
        {
            return m_readCallback;
        }
        else if (event == IoEvent::WRITE)
        {
            return m_writeCallback;
        }
        throw std::invalid_argument("getCallback invalid event");
    }
} // namespace lim_webserver
