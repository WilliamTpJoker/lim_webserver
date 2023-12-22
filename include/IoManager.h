#pragma once

#include "Scheduler.h"
#include "Timer.h"

namespace lim_webserver
{

    class IoManager : public Scheduler, public TimerManager
    {
    public:
        using RWMutexType = RWMutex;
        
        enum IoEvent
        {
            NONE = 0x0,
            READ = 0x1,
            WRITE = 0x4
        };

    public:
        IoManager(size_t threads = 1, bool use_caller = true, const std::string &name = "UNKNOWN");
        ~IoManager();

        int addEvent(int fd, IoEvent event, std::function<void()> callback = nullptr);
        bool delEvent(int fd, IoEvent event);
        bool cancelEvent(int fd, IoEvent event);

        bool cancelAll(int fd);

    public:
        static IoManager *GetThis();

    private:
        struct FdContext
        {
            using MutexType = Mutex;
            struct EventContext
            {
                Scheduler *scheduler = nullptr; // 目标调度器
                Fiber::ptr fiber;        // 事件协程
                std::function<void()> callback; // 事件回调函数
            };
            EventContext &getContext(IoEvent event);
            void resetContext(IoEvent event);

            void triggerEvent(IoEvent event);
            EventContext read;              // 读事件
            EventContext write;             // 写事件
            int fd = 0;                     // 事件关联句柄
            IoEvent events = IoEvent::NONE; // 已经注册的事件
            MutexType mutex;
        };

    protected:
        void tickle() override;
        bool onStop() override;
        bool onStop(uint64_t& timeout);
        void onIdle() override;

        void onTimerInsertedAtFront() override;

        void contextResize(size_t size);

    private:
        int m_epollFd = 0;                             // epoll 文件句柄
        int m_ticlefds[2];                             // pipe 文件句柄
        std::atomic<size_t> m_pendingEventCount = {0}; // 当前等待执行的事件数量
        std::vector<FdContext *> m_fdContext_list;     // socket事件上下文的容器
        RWMutexType m_mutex;
    };
}