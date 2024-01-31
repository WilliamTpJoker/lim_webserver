#pragma once

#include <memory>

#include "base/Noncopyable.h"
#include "coroutine/Context.h"

namespace lim_webserver
{
    /**
     * READY:  就绪状态，协程已准备好被调度执行
     * EXEC:   执行状态，协程正在执行中
     * HOLD:   暂停状态，协程执行被暂时挂起
     * EXCEPT:  异常状态，协程执行中产生异常
     * TERM:   终止状态，协程已执行完成并结束
     */
    enum class TaskState
    {
        READY,  // 就绪状态，协程已准备好被调度执行
        EXEC,   // 执行状态，协程正在执行中
        HOLD,   // 暂停状态，协程执行被暂时挂起
        EXCEPT, // 异常状态，协程执行中产生异常
        TERM    // 终止状态，协程已执行完成并结束
    };

    using TaskFunc = std::function<void()>;

    class Processor;
    class TaskQueue;

    class Task : public Noncopyable
    {
        friend Processor;
        friend TaskQueue;

    public:
        using ptr = std::shared_ptr<Task>;
        static ptr Create(TaskFunc func, size_t size) { return std::make_shared<Task>(func, size); }

    public:
        Task(TaskFunc func, size_t size);
        ~Task();

        inline uint64_t id() const { return m_id; }

        /**
         * @brief 让出线程执行权
         *
         */
        inline void yield()
        {
            m_state = TaskState::READY;
            swapOut();
        }

        /**
         * @brief 进入阻塞态
         *
         */
        inline void hold()
        {
            m_state = TaskState::HOLD;
            swapOut();
        }

        /**
         * @brief 从阻塞唤醒
         *
         */
        void wake();

        /**
         * @brief 设定Processor
         *
         */
        inline void setProcessor(Processor *processor) { m_processor = processor; }

        /**
         * @brief 获得Processor
         *
         * @return Processor*
         */
        inline Processor *getProcessor() const { return m_processor; }

        /**
         * @brief 获得任务状态
         *
         * @return TaskState
         */
        inline TaskState state() const { return m_state; }

    private:
        /**
         * @brief 上下文切入
         *
         */
        inline void swapIn() { m_context.swapIn(); }

        /**
         * @brief 上下文切出
         *
         */
        inline void swapOut() { m_context.swapOut(); }

        /**
         * @brief 工作函数
         *
         */
        void run();

        static void StaticRun(uint32_t low32, uint32_t high32);

    private:
        const uint64_t m_id;                  // 协程ID
        TaskState m_state = TaskState::READY; // 协程状态
        Context m_context;                    // 协程上下文
        Processor *m_processor = nullptr;     // 对应的执行器
        TaskFunc m_callback;                  // 回调函数
        Task *m_next = nullptr;               // 下一个任务
    };
} // namespace lim_webserver
