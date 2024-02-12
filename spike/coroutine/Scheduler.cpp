#include "Scheduler.h"
#include "Hook.h"
#include "splog.h"
#include "net/EventLoop.h"

#include <iostream>
#include <memory>

namespace lim_webserver
{
    Logger::ptr g_logger = LOG_SYS();

    Scheduler *Scheduler::Create()
    {
        Scheduler *sched = new Scheduler();
        Processor *proc = new Processor(sched, 0);
        sched->setProcessor(proc);
        return sched;
    }

    Scheduler *Scheduler::CreateNetScheduler()
    {
        Scheduler *sched = new Scheduler();
        EventLoop *loop = new EventLoop(sched, 0);
        sched->setProcessor(loop);
        return sched;
    }

    void Scheduler::createTask(TaskFunc const &func)
    {
        Task *tk = new Task(func, 128 * 1024);
        this->addTask(tk);
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
        // m_thread = Thread::Create([this]() { this->run(); }, "Sched");

        // 在当前线程运行
        m_mainProcessor->run();
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
        if (m_thread)
        {
            m_thread->join();
        }
    }

    Scheduler::Scheduler() : m_cond(m_mutex) {}

    Scheduler::~Scheduler() { stop(); }

    void Scheduler::addTask(Task *&task)
    {
        // 如果任务指定了处理器则直接调度
        Processor *processor = task->getProcessor();
        // 若处在激活态则可以调度
        if (processor && processor->m_activated)
        {
            processor->addTask(task);
            return;
        }

        // 没有则为当前指定的处理器分发任务
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

        if (processor && processor->m_activated && processor->getScheduler() == this)
        {
            processor->addTask(task);
            return;
        }

        // 都没有，则直接向主处理器添加
        m_mainProcessor->addTask(task);
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
