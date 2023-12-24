#include <atomic>

#include "Fiber.h"
#include "Config.h"
#include "Macro.h"
#include "Scheduler.h"
#include "Mutex.h"

namespace lim_webserver
{
    static Logger::ptr g_logger = LOG_NAME("system");

    static std::atomic<uint64_t> s_fiber_count{0};
    static std::atomic<uint64_t> s_fiber_id{0};

    static thread_local Fiber *t_fiber = nullptr;
    static thread_local Fiber::ptr t_threadFiber = nullptr;

    static ConfigVar<uint32_t>::ptr s_fiber_stack_size = Config::Lookup<uint32_t>("fiber.stack_size", 128 * 1024, "fiber stack size");

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

    std::string FiberStateHandler::ToString(FiberState state)
    {
        static const std::map<FiberState, std::string> stateMap = {
            {FiberState::INIT, "INIT"},
            {FiberState::HOLD, "HOLD"},
            {FiberState::EXEC, "EXEC"},
            {FiberState::TERM, "TERM"},
            {FiberState::READY, "READY"},
            {FiberState::EXCEPT, "EXCEPT"}};

        auto it = stateMap.find(state);
        if (it != stateMap.end())
        {
            return it->second;
        }
        else
        {
            return "UNKNOWN";
        }
    }

    Fiber::Fiber()
    {
        m_state = FiberState::EXEC;
        SetThis(this);
        ASSERT(!getcontext(&m_context), "getcontext");
        ++s_fiber_count;

        LOG_DEBUG(g_logger) << "Fiber::Fiber main";
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
        ASSERT(!getcontext(&m_context), "getcontext");
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

        LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
    }

    Fiber::~Fiber()
    {
        if (m_stack)
        // 有栈则为运行协程，确认不处于运行状态并释放内存
        {
            ASSERT(m_state == FiberState::TERM || m_state == FiberState::INIT || m_state == FiberState::EXCEPT);
            StackAllocator::Dealloc(m_stack, m_stacksize);
        }
        else
        // 无栈则为主协程，确认在运行，无回调函数， 将自身指针释放
        {
            ASSERT(!m_callback);
            ASSERT(m_state == FiberState::EXEC);

            Fiber *cur = t_fiber;
            if (cur == this)
            {
                SetThis(nullptr);
            }
        }
        LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id << " total_remain=" << --s_fiber_count;
    }

    void Fiber::reset(std::function<void()> callback)
    {
        ASSERT(m_stack);
        ASSERT(m_state == FiberState::TERM || m_state == FiberState::INIT || m_state == FiberState::EXCEPT);
        m_callback = callback;

        // 初始化上下文
        ASSERT(!getcontext(&m_context), "getcontext");
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
        ASSERT(m_state != FiberState::EXEC);
        // 将当前协程状态设置为执行状态
        m_state = FiberState::EXEC;

        // 使用 swapcontext 切换到新协程的上下文
        if (swapcontext(&Scheduler::GetMainFiber()->m_context, &m_context))
        {
            ASSERT(false, "swapcontext");
        }
    }

    void Fiber::swapOut()
    {
        // 切换到主协程
        SetThis(Scheduler::GetMainFiber());

        // 使用 swapcontext 切换回主协程的上下文
        if (swapcontext(&m_context, &Scheduler::GetMainFiber()->m_context))
        {
            ASSERT(false, "swapcontext");
        }
    }

    void Fiber::call()
    {
        // 切换到本次执行的协程
        SetThis(this);
        m_state = FiberState::EXEC;
        if (swapcontext(&t_threadFiber->m_context, &m_context))
        {
            ASSERT(false, "swapcontext");
        }
    }

    void Fiber::back()
    {
        SetThis(t_threadFiber.get());
        if (swapcontext(&m_context, &t_threadFiber->m_context))
        {
            ASSERT(false, "swapcontext");
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

    Fiber::ptr Fiber::GetThis()
    {
        // 如果当前协程不存在，则创建一个主协程并设置为当前协程
        if (!t_fiber)
        {
            ptr main_fiber(new Fiber);
            // 断言协程创建成功
            ASSERT(t_fiber == main_fiber.get());
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
        ptr cur = GetThis();
        // 断言当前协程状态为执行状态
        ASSERT(cur->m_state == FiberState::EXEC);
        // 将当前协程状态切换至就绪状态
        cur->m_state = FiberState::READY;
        // 执行协程切换操作以让出 CPU 执行权
        cur->swapOut();
    }
    void Fiber::YieldToHold()
    {
        ptr cur = GetThis();
        // 断言当前协程状态为执行状态
        ASSERT(cur->m_state == FiberState::EXEC);
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
        ptr cur = GetThis();
        // 断言当前协程对象不为空
        ASSERT(cur);
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
            LOG_ERROR(g_logger) << "Fiber Except: " << e.what() << " fiber_id=" << cur->getId()
                                    << "\n"
                                    << BackTraceToString();
        }
        catch (...)
        {
            cur->m_state = FiberState::EXCEPT;
            LOG_ERROR(g_logger) << "Fiber Except: fiber_id=" << cur->getId()
                                    << "\n"
                                    << BackTraceToString();
        }

        auto raw_ptr = cur.get();
        // 切换协程状态并执行协程切换操作
        cur.reset();
        raw_ptr->swapOut();
        ASSERT(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
    }
    void Fiber::CallerMainFunc()
    {
        ptr cur = GetThis();
        // 断言当前协程对象不为空
        ASSERT(cur);
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
            LOG_ERROR(g_logger) << "Fiber Except: " << e.what() << " fiber_id=" << cur->getId()
                                    << "\n"
                                    << BackTraceToString();
        }
        catch (...)
        {
            cur->m_state = FiberState::EXCEPT;
            LOG_ERROR(g_logger) << "Fiber Except: fiber_id=" << cur->getId()
                                    << "\n"
                                    << BackTraceToString();
        }

        auto raw_ptr = cur.get();
        // 切换协程状态并执行协程切换操作
        cur.reset();
        raw_ptr->back();
        ASSERT(false, "never reach fiber_id=" + std::to_string(raw_ptr->getId()));
    }

}