#pragma once

#include "base/Mutex.h"
#include "base/Noncopyable.h"

namespace lim_webserver
{
    enum IoEvent
    {
        NONE = 0x0,
        READ = 0x1,
        WRITE = 0x4
    };

    enum class IoChannelState
    {
        NEW,    // 新句柄
        EXIST,  // 活动句柄
        DISCARD // 废弃句柄
    };

    class EventLoop;

    class Task;

    class IoChannel : public Noncopyable
    {
    public:
        using MutexType = Spinlock;

    public:
        IoChannel(int fd);
        ~IoChannel();

        /**
         * @brief 触发协程
         *
         */
        void trigger(uint32_t op);

        /**
         * @brief 获得事件行为
         *
         * @return int
         */
        inline int event() const { return m_events; }

        /**
         * @brief 获得句柄
         *
         * @return int
         */
        inline int fd() const { return m_fd; }

        /**
         * @brief 是否为空事件
         *
         * @return true
         * @return false
         */
        inline bool isNoneEvent() const { return m_events == IoEvent::NONE; }

        /**
         * @brief 获得Channel状态
         *
         * @return IoChannelState
         */
        inline IoChannelState state() const { return m_state; }

        /**
         * @brief 设置Channel状态
         *
         * @param state
         * @return int
         */
        inline int setState(IoChannelState state) { m_state = state; }

        /**
         * @brief 添加event
         *
         * @param event
         */
        bool addEvent(IoEvent event);

        /**
         * @brief 删除event
         *
         * @param event
         */
        bool cancelEvent(IoEvent event);

        /**
         * @brief 取消所有
         *
         */
        bool clearEvent();

        /**
         * @brief 可写
         *
         * @return true
         * @return false
         */
        bool isWriting() const { return m_events & IoEvent::WRITE; }

        /**
         * @brief 可读
         *
         * @return true
         * @return false
         */
        bool isReading() const { return m_events & IoEvent::READ; }

        std::string stateToString() const;

        std::string eventsToString() const;

    private:
        static std::string stateToString(IoChannelState state);

        static std::string eventsToString(int ev);

    private:
        const int m_fd;                               // 管理的fd
        IoChannelState m_state = IoChannelState::NEW; // fd状态
        int m_events = 0;                             // fd的Event
        Task *m_readTask = nullptr;                   // 读协程
        Task *m_writeTask = nullptr;                  // 写协程
        Task *m_task = nullptr;
        MutexType m_mutex;
    };
} // namespace lim_webserver
