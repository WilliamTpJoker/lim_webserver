#include "scheduler.h"
#include "hook.h"

namespace lim_webserver
{
    static Logger::ptr g_logger = LIM_LOG_NAME("system");

    static thread_local Scheduler *t_scheduler = nullptr;
    static thread_local Fiber *t_fiber = nullptr;

    Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name)
        : m_name(name)
    {
        LIM_ASSERT(threads > 0);
        if (use_caller)
        {
            Fiber::GetThis();
            --threads;
            LIM_ASSERT(GetThis() == nullptr);
            t_scheduler = this;
            m_rootFiber = Fiber::Create([this]()
                                        { this->run(); },
                                        0, true);
            Thread::SetName(m_name);

            t_fiber = m_rootFiber.get();
            m_rootThread = GetThreadId();
            m_threadIds.emplace_back(m_rootThread);
        }
        else
        {
            m_rootThread = -1;
        }
        m_threadCount = threads;
    }

    Scheduler::~Scheduler()
    {
        LIM_ASSERT(m_stopping);
        if (GetThis() == this)
        {
            t_scheduler = nullptr;
        }
    }

    void Scheduler::start()
    {
        MutexType::Lock lock(m_mutex);
        if (!m_stopping)
        {
            return;
        }
        m_stopping = false;
        LIM_ASSERT(m_thread_list.empty());

        m_thread_list.resize(m_threadCount);
        for (size_t i = 0; i < m_threadCount; ++i)
        {
            m_thread_list[i] = Thread::Create([this]()
                                              { this->run(); },
                                              m_name + "_" + std::to_string(i));
            m_threadIds.emplace_back(m_thread_list[i]->getId());
        }
    }

    void Scheduler::stop()
    {
        LIM_LOG_INFO(g_logger) << this << " stopping";
        {
            MutexType::Lock lock(m_mutex);
            m_autoStop = true;
            m_stopping = true;
        }

        // 实例化调度器时的参数 use_caller 为 true, 并且指定线程数量为 1 时,说明只有当前一条主线程在执行，简单等待执行结束即可
        if (m_rootFiber && m_threadCount == 0 && (m_rootFiber->getState() == FiberState::TERM || m_rootFiber->getState() == FiberState::INIT))
        {
            if (onStop())
            {
                return;
            }
        }
        if (m_rootThread != -1)
        {
            LIM_ASSERT(GetThis() == this);
        }
        else
        {
            LIM_ASSERT(GetThis() != this);
        }

        for (size_t i = 0; i < m_threadCount; ++i)
        {
            tickle();
        }
        // 调用者线程析构
        if (m_rootFiber)
        {
            tickle();
            if (!onStop())
            {
                m_rootFiber->call();
            }
        }
        // 非调用者线程析构
        std::vector<Thread::ptr> thread_list;
        {
            MutexType::Lock lock(m_mutex);
            thread_list.swap(m_thread_list);
        }
        for (auto &i : thread_list)
        {
            i->join();
        }
        LIM_LOG_INFO(g_logger) << this << " stopped";
    }

    void Scheduler::tickle()
    {
        if (!hasIdleThreads())
        {
            return;
        }
        LIM_LOG_DEBUG(g_logger) << "on tickle";
    }

    void Scheduler::onIdle()
    {
        LIM_LOG_DEBUG(g_logger) << "on idle";
        while (!onStop())
        {
            Fiber::YieldToHold();
        }
    }

    void Scheduler::run()
    {
        LIM_LOG_DEBUG(g_logger) << m_name << " run";
        set_hook_enable(true);
        t_scheduler = this;
        if (GetThreadId() != m_rootThread)
        {
            t_fiber = Fiber::GetThis().get();
        }
        Fiber::ptr idle_fiber = Fiber::Create([this]()
                                              { this->onIdle(); });
        Fiber::ptr cb_fiber;

        FiberAndThread ft;
        while (true)
        {
            ft.reset();
            bool tickle_me = false;
            {
                MutexType::Lock lock(m_mutex);
                if (!m_task_queue.empty())
                {
                    ft = m_task_queue.front();
                    m_task_queue.pop();
                    LIM_ASSERT(ft.fiber || ft.callback);
                    tickle_me = true;
                    ++m_activeThreadCount;
                }
            }
            if (tickle_me)
            {
                tickle();
            }
            // 如果是 callback 任务，为则在专门的callback_fiber运行
            if (ft.fiber && (ft.fiber->getState() != FiberState::TERM && ft.fiber->getState() != FiberState::EXCEPT))
            {
                ft.fiber->swapIn();
                --m_activeThreadCount;

                if (ft.fiber->getState() == FiberState::READY)
                {
                    schedule(ft.fiber);
                }
                else if (ft.fiber->getState() != FiberState::TERM && ft.fiber->getState() != FiberState::EXCEPT)
                {
                    ft.fiber->setState(FiberState::HOLD);
                }
                ft.reset();
            }
            else if (ft.callback)
            {
                if (cb_fiber)
                {
                    cb_fiber->reset(ft.callback);
                }
                else
                {
                    cb_fiber = Fiber::Create(ft.callback);
                }
                ft.reset();
                cb_fiber->swapIn();
                --m_activeThreadCount;
                if (cb_fiber->getState() == FiberState::READY)
                {
                    // 如果状态为READY，表明该回调协程通过YeildToReady返回scheduler，将其重新规划入队列，回调协程则指定为新的协程
                    schedule(cb_fiber);
                    cb_fiber.reset();
                }
                else if (cb_fiber->getState() == FiberState::EXCEPT || cb_fiber->getState() == FiberState::TERM)
                {
                    // 如果状态为结束或者异常，表明该回调已经结束，保留该协程的指针，释放该协程的资源
                    cb_fiber->reset(nullptr);
                }
                else
                {
                    // 如果状态为其他，则表明该回调运行中断，设置状态为挂起
                    cb_fiber->setState(FiberState::HOLD);
                    cb_fiber.reset();
                }
            }
            else
            {
                if (idle_fiber->getState() == FiberState::TERM)
                {
                    LIM_LOG_INFO(g_logger) << "idle fiber term";
                    break;
                }
                ++m_idleThreadCount;
                idle_fiber->swapIn();
                --m_idleThreadCount;
                if (idle_fiber->getState() != FiberState::TERM && idle_fiber->getState() != FiberState::EXCEPT)
                {
                    idle_fiber->setState(FiberState::HOLD);
                }
            }
        }
    }

    bool Scheduler::onStop()
    {
        MutexType::Lock lock(m_mutex);
        return m_autoStop && m_stopping && m_task_queue.empty() && m_activeThreadCount == 0;
    }

    Scheduler *Scheduler::GetThis()
    {
        return t_scheduler;
    }

    Fiber *Scheduler::GetMainFiber()
    {
        return t_fiber;
    }
}
