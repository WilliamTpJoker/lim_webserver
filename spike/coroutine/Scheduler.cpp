#include "Scheduler.h"

#include <memory>
#include <iostream>

namespace lim_webserver
{
    Scheduler *Scheduler::Create()
    {
        return new Scheduler();
    }

    void Scheduler::createTask(TaskFunc const &func)
    {
        if (!m_started)
        {
            std::cout << "Scheduler not started" << std::endl;
            return;
        }
        Task::ptr tk = Task::Create(func, 128 * 1024);
        addTask(tk);
    }

    void Scheduler::start(int num_threads)
    {
        if (m_started)
        {
            return;
        }
        MutexType::Lock lock(m_mutex);
        m_threadCounts = num_threads;

        // 创建主处理器

        m_processors.push_back(m_mainProcessor);

        // 创建从处理器
        for (int i = 0; i < num_threads - 1; ++i)
        {
            newPoccessorThread();
        }
        m_started = true;

        // 创建调度线程
        m_thread = Thread::Create([this]()
                                  { this->run(); },
                                  "Sched");

        m_mainProcessor->start();
    }

    void Scheduler::stop()
    {
        if (!m_started)
        {
            return;
        }
        {
            MutexType::Lock lock(m_mutex);
            m_started = false;
        }
        // 关闭所有处理器
        for (auto &processor : m_processors)
        {
            processor->tickle();
        }
        // 关闭调度线程
        m_thread->join();
    }

    Scheduler::Scheduler()
        : m_cond(m_mutex)
    {
        m_mainProcessor = new Processor(this, 0);
    }

    Scheduler::~Scheduler()
    {
        stop();
    }

    void Scheduler::addTask(Task::ptr &task)
    {
        // 如果任务指定了处理器则直接调度
        Processor *processor = task->m_processor;
        // 若处在激活态则可以调度
        if (processor && processor->m_activated)
        {
            processor->addTask(task);
            return;
        }

        // 没有则为当前协程分发任务
        processor = Processor::GetCurrentProcessor();
        if (processor && processor->m_activated && processor->getScheduler() == this)
        {
            processor->addTask(task);
            return;
        }

        std::size_t num_processors = m_processors.size();
        std::size_t idx = m_lastActiveIdx;

        for (std::size_t i = 0; i < num_processors; ++i, ++idx)
        {
            idx = idx % num_processors;
            processor = m_processors[idx];

            // 找到第一个处于活动状态的进程后，将任务添加到该进程
            if (processor && processor->m_activated)
            {
                break;
            }
        }
        processor->addTask(task);
    }

    void Scheduler::run()
    {
        while (m_started)
        {
            // 每一秒调度一次
            m_cond.waitTime(1000);

            for (auto &processor : m_processors)
            {
                if (!processor->m_activated)
                {
                    if (processor->isIdled())
                    {
                        processor->m_activated = true;
                    }
                }
                if (processor->isIdled())
                {
                    processor->tickle();
                }
            }
        }
    }

    void Scheduler::newPoccessorThread()
    {
        auto p = new Processor(this, m_processors.size());
        p->start();
        m_processors.push_back(p);
    }

} // namespace lim_webserver
