#include "IoChannel.h"
#include "EventLoop.h"
#include "coroutine.h"
#include "splog.h"

#include <sys/epoll.h>
#include <sstream>

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_SYS();

    IoChannel::IoChannel(int fd)
        : m_fd(fd)
    {
    }

    IoChannel::~IoChannel()
    {
    }

    void IoChannel::trigger(uint32_t op)
    {
        LOG_TRACE(g_logger) << eventsToString(op);
        // 发生挂起事件则强制通知所有存在的协程
        if ((op & EPOLLHUP) && !(op & EPOLLIN))
        {
            op |= (EPOLLIN | EPOLLOUT) & m_events;
        }

        // 发生错误事件
        if (op & EPOLLERR)
        {
            op |= (EPOLLIN | EPOLLOUT) & m_events;
        }

        // 读操作   紧急读操作  关闭写半部分
        if (op & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
        {
            if (m_readTask)
            {
                m_readTask->getProcessor()->wakeupTask(m_readTask->id());
                // 协程操作完就置空，防止野指针
                m_readTask = nullptr;
            }
        }

        // 写操作
        if (op & EPOLLOUT)
        {
            if (m_writeTask)
            {
                m_writeTask->getProcessor()->wakeupTask(m_writeTask->id());
                m_writeTask = nullptr;
            }
        }
    }

    bool IoChannel::addEvent(IoEvent event)
    {
        MutexType::Lock lock(m_mutex);

        // 若在协程中则记录
        Task *task = Processor::GetCurrentTask();
        if (task)
        {
            if (event == IoEvent::READ)
            {
                m_readTask = task;
            }
            else if (event == IoEvent::WRITE)
            {
                m_writeTask = task;
            }
        }
        m_events = m_events | event;
        return true;
    }

    bool IoChannel::cancelEvent(IoEvent event)
    {
        MutexType::Lock lock(m_mutex);
        // 不存在则报错
        if (!(m_events & event))
        {
            LOG_ERROR(g_logger) << "cancelEvent assert fd = " << m_fd << " event = {" << eventsToString(event) << "} channel.event = {" << eventsToString(m_events) << "}";
            return false;
        }

        m_events = m_events & ~event;
        return true;
    }

    bool IoChannel::clearEvent()
    {
        MutexType::Lock lock(m_mutex);
        // 已空
        if (m_events == IoEvent::NONE)
        {
            return false;
        }
        m_events = IoEvent::NONE;
        return true;
    }

    std::string IoChannel::stateToString() const
    {
        return stateToString(m_state);
    }

    std::string IoChannel::eventsToString() const
    {
        return eventsToString(m_events);
    }

    std::string IoChannel::stateToString(IoChannelState state)
    {
        switch (state)
        {
        case IoChannelState::DISCARD:
            return "DISCARD";
        case IoChannelState::EXIST:
            return "EXIST";
        case IoChannelState::NEW:
            return "NEW";
        default:
            assert(false && "ERROR state");
            return "Unknown channel state";
        }
    }

    std::string IoChannel::eventsToString(int ev)
    {
        std::ostringstream oss;
        if (ev & EPOLLIN)
            oss << "IN ";
        if (ev & EPOLLPRI)
            oss << "PRI ";
        if (ev & EPOLLOUT)
            oss << "OUT ";
        if (ev & EPOLLHUP)
            oss << "HUP ";
        if (ev & EPOLLRDHUP)
            oss << "RDHUP ";
        if (ev & EPOLLERR)
            oss << "ERR ";

        return oss.str();
    }

} // namespace lim_webserver
