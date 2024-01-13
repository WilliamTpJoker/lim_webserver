#pragma once

#include <memory>

#include "Noncopyable.h"
#include "Context.h"

namespace lim_webserver
{
    /**
     * INIT:   初始状态，协程对象已创建但未开始执行
     * READY:  就绪状态，协程已准备好被调度执行
     * EXEC:   执行状态，协程正在执行中
     * HOLD:   暂停状态，协程执行被暂时挂起
     * EXCEPT:  异常状态，协程执行中产生异常
     * TERM:   终止状态，协程已执行完成并结束
     */
    enum class TaskState
    {
        INIT,   // 初始状态，协程对象已创建但未开始执行
        READY,  // 就绪状态，协程已准备好被调度执行
        EXEC,   // 执行状态，协程正在执行中
        HOLD,   // 暂停状态，协程执行被暂时挂起
        EXCEPT, // 异常状态，协程执行中产生异常
        TERM    // 终止状态，协程已执行完成并结束
    };

    using TaskFunc = std::function<void()>;

    class Processor;
    class Scheduler;

    class Task : public std::enable_shared_from_this<Task>, public Noncopyable
    {
        friend Processor;
        friend Scheduler;

    public:
        using ptr = std::unique_ptr<Task>;
        static ptr Create(TaskFunc func, size_t size)
        {
            return std::make_unique<Task>(func, size);
        }

    public:
        Task(TaskFunc func, size_t size);
        ~Task();

        inline uint64_t getId() const { return m_id; }

        inline void swapIn()
        {
            m_context.swapIn();
        }

        inline void swapOut()
        {
            m_context.swapOut();
        }

    private:
        /**
         * @brief 工作函数
         *
         */
        void run();

        static void StaticRun(uint32_t low32, uint32_t high32);

    private:
        const uint64_t m_id;                 // 协程ID
        TaskState m_state = TaskState::INIT; // 协程状态
        Context m_context;                   // 协程上下文
        Processor *m_processor = nullptr;    // 对应的执行器
        TaskFunc m_callback;                 // 回调函数
    };
} // namespace lim_webserver
