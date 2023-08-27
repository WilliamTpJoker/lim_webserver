#include <atomic>

#include "fiber.h"
#include "config.h"
#include "macro.h"
#include "scheduler.h"

namespace lim_webserver
{
    static Shared_ptr<Logger> g_logger = LIM_LOG_NAME("system");

    static std::atomic<uint64_t> s_fiber_id{0};
    static std::atomic<uint64_t> s_fiber_count{0};

    static thread_local Fiber *t_fiber = nullptr;
    static thread_local Shared_ptr<Fiber> t_threadFiber = nullptr;

    static Shared_ptr<ConfigVar<uint32_t>> s_fiber_stack_size = Config::Lookup<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");

    class MallocStackAllocator
    {
    public:
        static void *Alloc(size_t size)
        {
            return malloc(size);
        }

        static void Dealloc(void *vp, size_t size)
        {
            return free(vp);
        }
    };

    using StackAllocator = MallocStackAllocator;

    Fiber::Fiber()
    {
        m_state = FiberState::EXEC;
        SetThis(this);
        LIM_ASSERT(!getcontext(&m_context), "getcontext");
        ++s_fiber_count;

        LIM_LOG_DEBUG(g_logger) << "Fiber::Fiber main";
    }

    Fiber::Fiber(std::function<void()> callback, size_t stacksize, bool use_caller)
        : m_id(++s_fiber_id), m_callback(callback)
    {
        ++s_fiber_count;
        // 设置协程栈大小，若指定为空，则调用主协程获取大内存空间
        m_stacksize = stacksize ? stacksize : s_fiber_stack_size->getValue();

        // 分配协程栈内存
        m_stack = StackAllocator::Alloc(m_stacksize);

        // 初始化上下文
        LIM_ASSERT(!getcontext(&m_context), "getcontext");
        m_context.uc_link = nullptr;
        m_context.uc_stack.ss_sp = m_stack;
        m_context.uc_stack.ss_size = m_stacksize;

        // 设置上下文入口函数
        if (use_caller)
        {
            makecontext(&m_context, &Fiber::CallerMainFunc, 0);
        }
        else
        {
            makecontext(&m_context, &Fiber::MainFunc, 0);
        }

        LIM_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
    }

    Fiber::~Fiber()
    {
        --s_fiber_count;
        if (m_stack)
        // 有栈则为运行协程，确认不处于运行状态并释放内存
        {
            LIM_ASSERT(m_state == FiberState::TERM || m_state == FiberState::INIT || m_state == FiberState::EXCEPT);
            StackAllocator::Dealloc(m_stack, m_stacksize);
        }
        else
        // 无栈则为主协程，确认在运行，无回调函数， 将自身指针释放
        {
            LIM_ASSERT(!m_callback);
            LIM_ASSERT(m_state == FiberState::EXEC);

            Fiber *cur = t_fiber;
            if (cur == this)
            {
                SetThis(nullptr);
            }
        }
        LIM_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id << " total=" << s_fiber_count;
    }

    void Fiber::reset(std::function<void()> callback)
    {
        LIM_ASSERT(m_stack);
        LIM_ASSERT(m_state == FiberState::TERM || m_state == FiberState::INIT || m_state == FiberState::EXCEPT);
        m_callback = callback;

        // 初始化上下文
        LIM_ASSERT(!getcontext(&m_context), "getcontext");
        m_context.uc_link = nullptr;
        m_context.uc_stack.ss_sp = m_stack;
        m_context.uc_stack.ss_size = m_stacksize;

        // 设置上下文入口函数
        makecontext(&m_context, &Fiber::MainFunc, 0);
        m_state = FiberState::INIT;
    }

    void Fiber::swapIn()
    {
        // 将当前协程设置为本次执行的协程
        SetThis(this);

        // 断言当前协程不处于执行状态
        LIM_ASSERT(m_state != FiberState::EXEC);
        // 将当前协程状态设置为执行状态
        m_state = FiberState::EXEC;

        // 使用 swapcontext 切换到新协程的上下文
        if (swapcontext(&Scheduler::GetMainFiber()->m_context, &m_context))
        {
            LIM_ASSERT(false, "swapcontext");
        }
    }

    void Fiber::swapOut()
    {
        // 将当前协程设置为主协程
        SetThis(Scheduler::GetMainFiber());

        // 使用 swapcontext 切换回主协程的上下文
        if (swapcontext(&m_context, &Scheduler::GetMainFiber()->m_context))
        {
            LIM_ASSERT(false, "swapcontext");
        }
    }

    void Fiber::call()
    {
        // 将当前协程设置为本次执行的协程
        SetThis(this);
        m_state = FiberState::EXEC;
        if (swapcontext(&t_threadFiber->m_context, &m_context))
        {
            LIM_ASSERT(false, "swapcontext");
        }
    }

    void Fiber::back()
    {
        SetThis(t_threadFiber.get());
        if (swapcontext(&m_context, &t_threadFiber->m_context))
        {
            LIM_ASSERT(false, "swapcontext");
        }
    }

    uint64_t Fiber::GetFiberId()
    {
        if (t_fiber)
        {
            return t_fiber->getId();
        }
        return 0;
    }

    Shared_ptr<Fiber> Fiber::GetThis()
    {
        // 如果当前协程不存在，则创建一个主协程并设置为当前协程
        if (!t_fiber)
        {
            Shared_ptr<Fiber> main_fiber(new Fiber);
            // 断言协程创建成功
            LIM_ASSERT(t_fiber == main_fiber.get());
            // 将主协程设置为当前协程
            t_threadFiber = main_fiber;
        }
        // 返回当前协程的 shared_ptr 对象
        return t_fiber->shared_from_this();
    }

    void Fiber::SetThis(Fiber *f)
    {
        t_fiber = f;
    }

    void Fiber::YieldToReady()
    {
        Shared_ptr<Fiber> cur = GetThis();
        // 断言当前协程状态为执行状态
        LIM_ASSERT(cur->m_state == FiberState::EXEC);
        // 将当前协程状态切换至就绪状态
        cur->m_state = FiberState::READY;
        // 执行协程切换操作以让出 CPU 执行权
        cur->swapOut();
    }
    void Fiber::YieldToHold()
    {
        Shared_ptr<Fiber> cur = GetThis();
        // 断言当前协程状态为执行状态
        LIM_ASSERT(cur->m_state == FiberState::EXEC);
        // 将当前协程状态切换至保持状态
        cur->m_state = FiberState::HOLD;
        // 执行协程切换操作以让出 CPU 执行权
        cur->swapOut();
    }

    uint64_t Fiber::TotalFibers()
    {
        return s_fiber_count;
    }

    void Fiber::MainFunc()
    {
        Shared_ptr<Fiber> cur = GetThis();
        // 断言当前协程对象不为空
        LIM_ASSERT(cur);
        try
        {
            // 执行用户指定的回调函数
            cur->m_callback();
            cur->m_callback = nullptr;
            cur->m_state = FiberState::TERM;
        }
        catch (const std::exception &e)
        {
            cur->m_state = FiberState::EXCEPT;
            LIM_LOG_ERROR(g_logger) << "Fiber Except: " << e.what() << " fiber_id=" << cur->getId()
                                    << std::endl
                                    << BackTraceToString();
        }
        catch (...)
        {
            cur->m_state = FiberState::EXCEPT;
            LIM_LOG_ERROR(g_logger) << "Fiber Except: fiber_id=" << cur->getId()
                                    << std::endl
                                    << BackTraceToString();
        }

        auto raw_ptr = cur.get();
        // 切换协程状态并执行协程切换操作
        cur.reset();
        raw_ptr->swapOut();
        LIM_ASSERT(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
    }
    void Fiber::CallerMainFunc()
    {
        Shared_ptr<Fiber> cur = GetThis();
        // 断言当前协程对象不为空
        LIM_ASSERT(cur);
        try
        {
            // 执行用户指定的回调函数
            cur->m_callback();
            cur->m_callback = nullptr;
            cur->m_state = FiberState::TERM;
        }
        catch (const std::exception &e)
        {
            cur->m_state = FiberState::EXCEPT;
            LIM_LOG_ERROR(g_logger) << "Fiber Except: " << e.what() << " fiber_id=" << cur->getId()
                                    << std::endl
                                    << BackTraceToString();
        }
        catch (...)
        {
            cur->m_state = FiberState::EXCEPT;
            LIM_LOG_ERROR(g_logger) << "Fiber Except: fiber_id=" << cur->getId()
                                    << std::endl
                                    << BackTraceToString();
        }

        auto raw_ptr = cur.get();
        // 切换协程状态并执行协程切换操作
        cur.reset();
        raw_ptr->back();
        LIM_ASSERT(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
    }
}