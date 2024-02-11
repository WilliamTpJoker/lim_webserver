#pragma once

#include "base/Mutex.h"
#include "base/Singleton.h"
#include "net/IoChannel.h"
#include "net/Poller.h"
#include "coroutine/Processor.h"

#include <memory>

namespace lim_webserver
{
    class EventLoop : public Processor, public Singleton<EventLoop>
    {
    public:
        using MutexType = Mutex;

    public:
        EventLoop(Scheduler *scheduler, int id);
        ~EventLoop();
        void stop();

        inline void updateChannel(IoChannel::ptr channel) { m_poller->updateChannel(channel); }

        inline void removeChannel(IoChannel::ptr channel) { m_poller->removeChannel(channel); }

        inline bool hasChannel(IoChannel::ptr channel) const { return m_poller->hasChannel(channel); }

    protected:
        /**
         * @brief 唤醒
         *
         */
        void tickle() override;

        /**
         * @brief 运行
         *
         */
        void idle() override;

    private:
        bool m_started = true; // 开始标志符
        Poller::ptr m_poller;  // IO模块
        int m_wakeFd;
        IoChannel::ptr m_wakeChannel;
    };
} // namespace lim_webserver
