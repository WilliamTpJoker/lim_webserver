#pragma once

#include "coroutine/Scheduler.h"

#include <functional>

namespace lim_webserver
{
    using FuncType = std::function<void()>;

    class FuncHandler
    {
    public:
        using ptr = std::unique_ptr<FuncHandler>;

    public:
        FuncHandler() {}
        virtual ~FuncHandler(){}

        virtual void handle(FuncType &func) = 0;
    };

    /**
     * @brief 系统原生处理：直接处理
     *
     */
    class SysHandler : public FuncHandler
    {
    public:
        using ptr = std::unique_ptr<SysHandler>;

    public:
        SysHandler() {}
        ~SysHandler() {}

        void handle(FuncType &func) override
        {
            try
            {
                func();
            }
            catch (...)
            {
            }
        }
    };

    /**
     * @brief 协程处理方式：加入调度队列
     *
     */
    class CoHandler : public FuncHandler
    {
    public:
        using ptr = std::unique_ptr<CoHandler>;

    public:
        CoHandler(Scheduler *scheduler) : m_scheduler(scheduler) {}
        ~CoHandler() {}

        void handle(FuncType &func) override
        {
            m_scheduler->createTask(func);
        }

    private:
        Scheduler *m_scheduler;
    };
} // namespace lim_webserver
