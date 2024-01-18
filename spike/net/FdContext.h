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

    enum class FdContextState
    {
        NEW,    // 新句柄
        EXIST,  // 活动句柄
        DISCARD // 废弃句柄
    };

    class EventLoop;

    class FdContext : public Noncopyable
    {
        friend EventLoop;

    public:
        using MutexType = Mutex;
        using EventCallback = std::function<void()>;

    public:
        FdContext(EventLoop *loop, int fd);
        ~FdContext();

        /**
         * @brief 更新
         *
         */
        void update();

        /**
         * @brief 移除
         *
         */
        void remove();

        /**
         * @brief 触发回调(加入到协程调度)
         *
         */
        void trigger(uint32_t op);

        /**
         * @brief 设置读事件回调
         *
         * @param cb
         */
        inline void setReadCallback(EventCallback cb) { m_readCallback = std::move(cb); }

        /**
         * @brief 设置写事件回调
         *
         * @param cb
         */
        inline void setWriteCallback(EventCallback cb) { m_writeCallback = std::move(cb); }

        /**
         * @brief 设置关闭事件回调
         *
         * @param cb
         */
        inline void setCloseCallback(EventCallback cb) { m_closeCallback = std::move(cb); }

        /**
         * @brief 设置错误事件回调
         *
         * @param cb
         */
        inline void setErrorCallback(EventCallback cb) { m_errorCallback = std::move(cb); }

        /**
         * @brief 获得回调函数
         *
         * @param event
         * @return EventCallback&
         */
        EventCallback &getCallback(IoEvent event);

        /**
         * @brief 获得事件行为
         *
         * @return IoEvent
         */
        inline IoEvent event() const { return m_events; }

        /**
         * @brief 获得句柄
         *
         * @return int
         */
        inline int fd() const { return m_fd; }

        /**
         * @brief 是否为空时间
         *
         * @return true
         * @return false
         */
        inline bool isNoneEvent() { return m_events == IoEvent::NONE; }

        /**
         * @brief 获得Context状态
         *
         * @return FdContextState
         */
        inline FdContextState state() const { return m_state; }

        /**
         * @brief 设置Context状态
         *
         * @param state
         * @return int
         */
        inline int setState(FdContextState state) { m_state = state; }

    private:
        const int m_fd;                               // 管理的fd
        FdContextState m_state = FdContextState::NEW; // fd状态
        IoEvent m_events = IoEvent::NONE;             // fd的Event
        EventLoop *m_loop;                            // 所属的EventLoop
        EventCallback m_readCallback;                 // 读回调
        EventCallback m_writeCallback;                // 写回调
        EventCallback m_closeCallback;                // 关闭回调
        EventCallback m_errorCallback;                  // 错误回调
        MutexType m_mutex;
    };
} // namespace lim_webserver
