#ifndef __LIM_FIBER_H__
#define __LIM_FIBER_H__

#include <ucontext.h>

#include "common.h"
#include "thread.h"

namespace lim_webserver
{
    enum class FiberState
    {
        INIT, // 初始状态，协程对象已创建但未开始执行
        HOLD, // 暂停状态，协程执行被暂时挂起
        EXEC, // 执行状态，协程正在执行中
        TERM, // 终止状态，协程已执行完成并结束
        READY // 就绪状态，协程已准备好被调度执行
    };

    class Fiber : public std::enable_shared_from_this<Fiber>
    {

    public:
        Fiber() = delete;
        /**
         * @brief 构造函数，用于创建Fiber对象。
         * @param callback  协程执行函数。
         * @param stacksize 协程栈大小，默认为0，表示使用系统默认大小。
         */
        Fiber(std::function<void()> callback, size_t stacksize = 0);
        /**
         * @brief 析构函数，释放Fiber对象资源。
         */
        ~Fiber();

        /**
         * @brief 重置协程的执行函数。
         * @param callback 新的协程执行函数。
         */
        void reset(std::function<void()> callback);
        /**
         * @brief 将当前线程切换到协程执行。
         */
        void swapIn();
        /**
         * @brief 将协程切换到后台。
         */
        void swapOut();

    public:
        /**
         * @brief 获取当前线程关联的协程对象。
         * @return 协程对象的共享指针。
         */
        static Shared_ptr<Fiber> GetThis();
        /**
         * @brief 切换到协程调度中的下一个可执行协程。
         */
        static void YieldToReady();
        /**
         * @brief 切换到协程调度中的等待队列，暂时放弃CPU执行权。
         */
        static void YieldToHold();

        /**
         * @brief 获取总协程数。
         * @return 当前系统中所有协程的数量。
         */
        static uint64_t TotalFibers();

        /**
         * @brief 主要工作函数
         */
        static void MainFunc();

    private:
        uint64_t m_id;                    // 协程ID
        uint32_t m_stacksize;             // 协程栈大小
        FiberState m_state;               // 协程状态
        ucontext_t m_context;             // 协程上下文
        void *m_stack = nullptr;          // 协程栈指针
        std::function<void()> m_callback; // 协程执行的回调函数
    };
}

#endif