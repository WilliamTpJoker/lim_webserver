#include "IoChannel.h"
#include "EventLoop.h"
#include "coroutine.h"
#include "splog.h"

#include <sstream>
#include <sys/epoll.h>

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_SYS();

    IoChannel::IoChannel(int fd) : m_fd(fd) {}

    IoChannel::~IoChannel() {}

    void IoChannel::trigger(uint32_t op)
    {
        LOG_TRACE(g_logger) << "fd = " << m_fd << " { " << eventsToString(op) << "}";
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

        // 读操作   紧急读操作  关闭写半部分 写操作
        if (op & (EPOLLIN | EPOLLPRI | EPOLLRDHUP | EPOLLOUT))
        {
            if (m_task)
            {
                m_task->wake();
            }
        }
    }

    void IoChannel::close() { trigger(EPOLLERR); }

    bool IoChannel::addEvent(IoEvent event)
    {
        MutexType::Lock lock(m_mutex);
        // 存在则跳过
        if (m_events & event)
        {
            return false;
        }

        // 若在协程中则记录
        Task *task = Processor::GetCurrentTask();
        if (task)
        {
            m_task = task;
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
            LOG_ERROR(g_logger) << "cancelEvent assert fd = " << m_fd << " event = {" << eventsToString(event) << "} channel.event = {"
                                << eventsToString(m_events) << "}";
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

    std::string IoChannel::stateToString() const { return stateToString(m_state); }

    std::string IoChannel::eventsToString() const { return eventsToString(m_events); }

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
