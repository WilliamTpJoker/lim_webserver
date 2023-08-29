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

        m_condition.notify_all();
        // 调用者线程析构
        if (m_rootFiber)
        {
            if (!onStop())
            {
                m_rootFiber->call();
            }
        }
        // 非调用者线程析构
        std::vector<Shared_ptr<Thread>> thread_list;
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
        FiberAndThread ft;
        while (true)
        {
            ft.reset();
            {
                MutexType::Lock lock(m_mutex);
                m_condition.wait(m_mutex, [this]
                                 { return !m_task_queue.empty() || m_stopping; });
                if (m_stopping && m_task_queue.empty())
                {
                    break;
                }
                auto it = m_task_queue.begin();
                LIM_ASSERT(it->fiber || it->callback);

                ft = *it;
                m_task_queue.erase(it);
                ++m_activeThreadCount;
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
