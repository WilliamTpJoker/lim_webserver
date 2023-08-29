#include "scheduler.h"

namespace lim_webserver
{
    static Shared_ptr<Logger> g_logger = LIM_LOG_NAME("system");

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
            m_rootFiber = MakeShared<Fiber>([this]()
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
            m_thread_list[i] = MakeShared<Thread>([this]()
                                                  { this->run(); },
                                                  m_name + "_" + std::to_string(i));
            m_threadIds.emplace_back(m_thread_list[i]->getId());
        }
    }

    void Scheduler::stop()
    {
        m_autoStop = true;
        if (m_rootFiber && m_threadCount == 0 && (m_rootFiber->getState() == FiberState::TERM || m_rootFiber->getState() == FiberState::INIT))
        {
            LIM_LOG_INFO(g_logger) << this << " stopped";
            m_stopping = true;

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
        m_stopping = true;
        for (size_t i = 0; i < m_threadCount; ++i)
        {
            tickle();
        }
        if (m_rootFiber)
        {
            tickle();
            if (!onStop())
            {
                m_rootFiber->call();
            }
        }
        std::vector<Shared_ptr<Thread>> thread_list;
        {
            MutexType::Lock lock(m_mutex);
            thread_list.swap(m_thread_list);
        }

        for (auto &i : thread_list)
        {
            i->join();
        }
    }

    void Scheduler::tickle()
    {
        LIM_LOG_DEBUG(g_logger) << "on tickle";
    }

    void Scheduler::run()
    {
        LIM_LOG_DEBUG(g_logger) << m_name << " run";
        t_scheduler = this;
        if (GetThreadId() != m_rootThread)
        {
            t_fiber = Fiber::GetThis().get();
        }
        Shared_ptr<Fiber> idle_fiber = MakeShared<Fiber>([this]()
                                                         { this->onIdle(); });
        FiberAndThread ft;
        while (true)
        {
            ft.reset();
            bool tickle_me = false;
            {
                MutexType::Lock lock(m_mutex);
                if (!m_task_queue.empty())
                {
                    auto it = m_task_queue.begin();
                    LIM_ASSERT(it->fiber || it->callback);

                    ft = *it;
                    tickle_me = true;
                    m_task_queue.erase(it);
                    ++m_activeThreadCount;
                }
            }
            if (tickle_me)
            {
                tickle();
            }
            // 如果是 callback 任务，为其创建 fiber
            if (ft.callback)
            {
                ft.fiber = MakeShared<Fiber>(ft.callback);
                ft.callback = nullptr;
            }
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

    void Scheduler::onIdle()
    {
        LIM_LOG_INFO(g_logger) << "idle";
        while (!onStop())
        {
            Fiber::YieldToHold();
        }
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
