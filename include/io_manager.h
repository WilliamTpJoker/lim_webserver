#ifndef __LIM_IO_H__
#define __LIM_IO_H__

#include "scheduler.h"

namespace lim_webserver
{
    class IoManager : public Scheduler
    {
    public:
        using RWMutexType = RWMutex;
        using MutexType = Mutex;

        enum class IoEvent
        {
            NONE = 0x0,
            READ = 0x1,
            WRITE = 0x2,
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
            struct EventContext
            {
                Scheduler *scheduler = nullptr; // 目标调度器
                Shared_ptr<Fiber> fiber;        // 事件协程
                std::function<void()> callback; // 事件回调函数
            };
            EventContext read;               // 读事件
            EventContext write;              // 写事件
            int fd;                          // 事件关联句柄
            IoEvent m_event = IoEvent::NONE; // 已经注册的事件
            MutexType mutex;
        };

    protected:
        virtual void tickle() override;
        virtual bool onStop() override;

    private:
        int m_epfd = 0;
        int m_ticlefds[2];
        std::atomic<size_t> m_pendingEventCount = {0};
        std::vector<FdContext*> m_fdContext_list;
        MutexType m_mutex;
    };
}

#endif