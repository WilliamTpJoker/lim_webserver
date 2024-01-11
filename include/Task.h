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

    class Processor;

    class Task : public std::enable_shared_from_this<Task>, public Noncopyable
    {
        friend Processor;

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
        // inline ptrdiff_t getCap() const { return m_cap; }
        // inline ptrdiff_t getSize() const { return m_size; }
        inline TaskState getState() const { return m_state; }
        inline Context *getContext() { return &m_context; }
        inline Processor *getProcessor() const { return m_processor; }

        // inline void setCap(ptrdiff_t cap) { m_cap = cap; }
        // inline void setSize(ptrdiff_t size) { m_size = size; }
        inline void setState(TaskState state) { m_state = state; }
        inline void setProcessor(Processor *processor) { m_processor = processor; }

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
        // ptrdiff_t m_cap;                     // 协程已用内存空间
        // ptrdiff_t m_size;                    // 协程总内存空间
        uint64_t m_id;                       // 协程ID
        TaskState m_state = TaskState::INIT; // 协程状态
        Context m_context;                   // 协程上下文
        Processor *m_processor;              // 对应的执行器
        TaskFunc m_callback;                 // 回调函数
    };
} // namespace lim_webserver
