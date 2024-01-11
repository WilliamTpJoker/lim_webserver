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
        m_mainProcessor = new Processor(this, 0);
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
                                  "Scheduler");

        // 开始处理器
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

    Scheduler::~Scheduler()
    {
        stop();
    }

    void Scheduler::addTask(Task::ptr &task)
    {
        // 如果任务指定了处理器则直接调度
        Processor *processor = task->getProcessor();
        if (processor)
        {
            processor->addTask(task);
            return;
        }

        // 没有则为当前协程分发任务
        processor = Processor::GetCurrentProcessor();
        if (processor && processor->getScheduler() == this)
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
            if (processor)
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
            std::this_thread::sleep_for(std::chrono::microseconds(1000));
        }
    }

    void Scheduler::newPoccessorThread()
    {
        auto p = new Processor(this, m_processors.size());
        p->start();
        m_processors.push_back(p);
    }

} // namespace lim_webserver
