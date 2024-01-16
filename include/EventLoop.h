#pragma once

#include "Thread.h"
#include "Mutex.h"
#include "Noncopyable.h"
#include "FdContext.h"
#include "Poller.h"
#include "FuncHandler.h"

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
         * @brief 更新Context
         *
         * @param context
         */
        inline void updateContext(FdContext *context) { m_poller->updateContext(context); }

        /**
         * @brief 移除Context
         *
         * @param context
         */
        inline void removeContext(FdContext *context) { m_poller->removeContext(context); }

        /**
         * @brief 确认是否存在Context
         *
         * @param context
         * @return true
         * @return false
         */
        inline bool hasContext(FdContext *context) { return m_poller->hasContext(context); }

        /**
         * @brief 处理回调
         *
         * @param cb 回调函数
         */
        inline void handleFunction(FuncType &cb) { m_funcHandler->handle(cb); }

    private:
        void run();

    private:
        bool m_started = true;          // 开始标志符
        Poller::ptr m_poller;           // IO模块
        FuncHandler::ptr m_funcHandler; // 函数处理器
        Thread::ptr m_thread;           // 工作线程
        MutexType m_mutex;
    };
} // namespace lim_webserver
