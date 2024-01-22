#pragma once

#include "base/Mutex.h"
#include "base/Noncopyable.h"
#include "base/Singleton.h"
#include "net/IoChannel.h"
#include "net/Poller.h"

#include <memory>

namespace lim_webserver
{
    class EventLoop : public Noncopyable, public Singleton<EventLoop>
    {
    public:
        using MutexType = Mutex;

    public:
        EventLoop();
        ~EventLoop();
        void stop();

        inline IoChannel * getChannel(int fd){return m_poller->getChannel(fd);}

        /**
         * @brief 更新Channel
         *
         * @param channel
         */
        inline void updateChannel(IoChannel *channel) { m_poller->updateChannel(channel); }

        /**
         * @brief 移除Channel
         *
         * @param channel
         */
        inline void removeChannel(IoChannel *channel) { m_poller->removeChannel(channel); }

        /**
         * @brief 运行
         *
         */
        void run();

    private:
        bool m_started = true;          // 开始标志符
        Poller::ptr m_poller;           // IO模块
        MutexType m_mutex;
    };
} // namespace lim_webserver
