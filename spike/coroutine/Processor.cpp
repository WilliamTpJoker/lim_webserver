#include "Processor.h"
#include "Hook.h"
#include "Scheduler.h"

#include <iostream>

namespace lim_webserver
{

    Processor *&Processor::GetCurrentProcessor()
    {
        static thread_local Processor *proc = nullptr;
        return proc;
    }

    Scheduler *Processor::GetCurrentScheduler()
    {
        auto proc = GetCurrentProcessor();
        if (proc)
        {
            return proc->m_scheduler;
        }
        return nullptr;
    }

    Task *Processor::GetCurrentTask()
    {
        auto proc = GetCurrentProcessor();
        return proc ? proc->m_curTask : nullptr;
    }

    void Processor::CoYield()
    {
        auto proc = GetCurrentProcessor();
        proc->coYield();
    }

    void Processor::CoHold()
    {
        auto proc = GetCurrentProcessor();
        proc->coHold();
    }

    void Processor::coYield()
    {
        Task *task = GetCurrentTask();
        assert(task);
        task->yield();
    }

    void Processor::coHold()
    {
        Task *task = GetCurrentTask();
        assert(task);
        task->hold();
    }

    Processor::Processor(Scheduler *scheduler, int id) : m_scheduler(scheduler), m_id(id), m_cond(m_mutex) {}

    void Processor::addTask(Task *&task)
    {
        m_newQueue.enqueue(task);
        tickle();
    }

    void Processor::getNextTask(bool flag)
    {
        // 没任务则尝试添加新任务
        if (!m_runableQueue.dequeue(m_curTask))
        {
            if (flag || (m_addNewRemain > 0))
            {
                m_runableQueue.concatenate(m_newQueue);

                if (!flag)
                {
                    --m_addNewRemain;
                }
            }

            // 失败则有其他操作 TODO:
            if (!m_runableQueue.dequeue(m_curTask))
            {
            }
        }
    }

    void Processor::wakeupTask(Task *task)
    {
        m_scheduler->addTask(task);
        tickle();
    }

    inline void Processor::garbageCollection() { m_garbageList.clear(); }

    void Processor::start()
    {
        MutexType::Lock lock(m_mutex);
        m_thread = Thread::Create([this]() { this->run(); }, "Proc_" + std::to_string(m_id));
    }

    void Processor::tickle()
    {
        MutexType::Lock lock(m_mutex);
        // 若处于休眠态，则唤醒
        if (m_idled)
        {
            m_cond.notify_one();
        }
        // 若处于工作态，则尝试通知
        else
        {
            // 若已被通知，则不通知
            if (!m_notified)
            {
                m_notified = true;
            }
        }
    }

    void Processor::idle()
    {
        garbageCollection();

        MutexType::Lock lock(m_mutex);

        // 若在企图休眠前接受到了调度器的通知，则不休眠
        if (m_notified)
        {
            m_notified = false;
            return;
        }

        // 进入等待状态
        m_idled = true;
        m_cond.wait();
        m_idled = false;
    }

    void Processor::run()
    {
        GetCurrentProcessor() = this;
        while (m_scheduler->m_started)
        {
            getNextTask(true);
            if (m_curTask == nullptr) // 无任务 闲置
            {
                // 执行空闲任务
                idle();
                // 回到了工作状态，说明调度了新任务到队列中
                m_runableQueue.concatenate(m_newQueue);
                continue;
            }
            m_addNewRemain = 1;
            // 每轮环形协程调度后都重置
            while (m_curTask && m_scheduler->m_started)
            {
                m_curTask->setProcessor(this);
                ++m_switchCount;
                m_curTask->swapIn();

                // 此时回到了Processor中
                // 若为ready态则重新加入调度队列
                switch (m_curTask->state())
                {
                case TaskState::READY:
                    m_runableQueue.enqueue(m_curTask);
                    break;
                case TaskState::HOLD:
                    break;
                case TaskState::TERM:
                default:
                    if (m_garbageList.size() > 16)
                    {
                        garbageCollection();
                    }
                    m_garbageList.push_back(m_curTask);
                }
                m_curTask = nullptr;
                getNextTask();
            }
        }
    }

} // namespace lim_webserver
