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

        /**
         * @brief 添加event
         *
         * @param fd
         * @param event
         * @return true
         * @return false
         */
        bool addEvent(int fd, IoEvent event);

        /**
         * @brief 取消event
         *
         * @param fd
         * @param event
         * @return true
         * @return false
         */
        bool cancelEvent(int fd, IoEvent event);

        /**
         * @brief 清空event
         *
         * @param fd
         * @return true
         * @return false
         */
        bool clearEvent(int fd);

        /**
         * @brief 运行
         *
         */
        void run();

    private:
        bool m_started = true; // 开始标志符
        Poller::ptr m_poller;  // IO模块
    };
} // namespace lim_webserver
