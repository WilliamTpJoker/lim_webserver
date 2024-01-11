#pragma once

#include <ucontext.h>

#include "Thread.h"

#define FiberState_INIT lim_webserver::FiberState::INIT
#define FiberState_HOLD lim_webserver::FiberState::HOLD
#define FiberState_EXEC lim_webserver::FiberState::EXEC
#define FiberState_TERM lim_webserver::FiberState::TERM
#define FiberState_READY lim_webserver::FiberState::READY
#define FiberState_EXCEPT lim_webserver::FiberState::EXCEPT

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
    enum class FiberState
    {
        INIT,   // 初始状态，协程对象已创建但未开始执行
        READY,  // 就绪状态，协程已准备好被调度执行
        EXEC,   // 执行状态，协程正在执行中
        HOLD,   // 暂停状态，协程执行被暂时挂起
        EXCEPT, // 异常状态，协程执行中产生异常
        TERM    // 终止状态，协程已执行完成并结束
    };

    class FiberStateHandler
    {
    public:
        static std::string ToString(FiberState state);
    };

    class Fiber : public std::enable_shared_from_this<Fiber>
    {
    public:
        using ptr = std::shared_ptr<Fiber>;
        /**
         * @brief 创建Fiber对象智能指针。
         * @param callback  协程执行函数。
         * @param stacksize 协程栈大小，默认为0，表示使用系统默认大小。
         * @param use_caller  是否使用 Sched 实例化所在的线程（Sched 所在线程也工作）
         */
        static ptr Create(std::function<void()> callback, size_t stacksize = 0, bool use_caller = false)
        {
            return std::make_shared<Fiber>(callback, stacksize, use_caller);
        }

    private:
        /**
         * @brief 私有构造函数，用于创建主Fiber对象。
         */
        Fiber();

    public:
        /**
         * @brief 构造函数，用于创建Fiber对象。
         * @param callback  协程执行函数。
         * @param stacksize 协程栈大小，默认为0，表示使用系统默认大小。
         * @param use_caller  是否使用 Sched 实例化所在的线程（Sched 所在线程也工作）
         */
        Fiber(std::function<void()> callback, size_t stacksize = 0, bool use_caller = false);
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
         * @brief 将协程切换到调度器线程的主协程
         */
        void swapOut();
        /**
         * @brief 将当前线程切换到执行状态
         * @pre 执行的为当前线程的主协程
         */
        void call();
        /**
         * @brief 将协程切换到对应线程的主协程
         * @pre 执行的为当前线程的主协程
         */
        void back();
        /**
         * @brief 获取当前协程ID。
         * @return 协程ID。
         */
        uint64_t getId() { return m_id; }
        /**
         * @brief 获取当前协程状态。
         * @return 协程状态。
         */
        FiberState getState() { return m_state; }
        /**
         * @brief 获取当前协程状态字符串。
         * @return 协程状态字符串。
         */
        std::string getStateString() { return FiberStateHandler::ToString(m_state); }
        /**
         * @brief 设置协程状态。
         */
        void setState(FiberState state) { m_state = state; }

    public:
        /**
         * @brief 获取当前协程ID。
         * @return 协程ID。
         */
        static uint64_t GetFiberId();
        /**
         * @brief 获取当前线程关联的协程对象。
         * @return 协程对象的共享指针。
         */
        static ptr GetThis();
        /**
         * @brief 设置当前线程关联的协程对象。
         * @param f 协程对象指针
         */
        static void SetThis(Fiber *f);
        /**
         * @brief 将当前协程切换至就绪状态并让出 CPU
         */
        static void YieldToReady();
        /**
         * @brief 将当前协程切换至保持状态并让出 CPU
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

        /**
         * @brief 调用者模式主要工作函数
         */
        static void CallerMainFunc();

    private:
        uint64_t m_id = 0;                     // 协程ID
        uint32_t m_stacksize = 0;              // 协程栈大小
        FiberState m_state = FiberState::INIT; // 协程状态
        ucontext_t m_context;                  // 协程上下文
        void *m_stack = nullptr;               // 协程栈指针
        std::function<void()> m_callback;      // 协程执行的回调函数
    };
}