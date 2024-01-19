#pragma once

#include "base/Mutex.h"
#include "base/Noncopyable.h"
#include "net/IoChannel.h"
#include "net/Poller.h"
#include "net/FuncHandler.h"

#include <memory>

namespace lim_webserver
{
    class EventLoop : public Noncopyable
    {
    public:
        using MutexType = Mutex;

    public:
        EventLoop();
        ~EventLoop();
        void stop();

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
         * @brief 确认是否存在Channel
         *
         * @param channel
         * @return true
         * @return false
         */
        inline bool hasChannel(IoChannel *channel) { return m_poller->hasChannel(channel); }

        /**
         * @brief 处理回调
         *
         * @param cb 回调函数
         */
        inline void handleFunction(FuncType &cb) { m_funcHandler->handle(cb); }

        /**
         * @brief 运行
         *
         */
        void run();

    private:
        bool m_started = true;          // 开始标志符
        Poller::ptr m_poller;           // IO模块
        FuncHandler::ptr m_funcHandler; // 函数处理器
        MutexType m_mutex;
    };
} // namespace lim_webserver
