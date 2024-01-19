#include "Processor.h"
#include "Scheduler.h"

#include <iostream>

namespace lim_webserver
{
    Processor *&Processor::GetCurrentProcessor()
    {
        static thread_local Processor *proc = nullptr;
        return proc;
    }

    Task *Processor::GetCurrentTask()
    {
        auto proc = GetCurrentProcessor();
        return proc ? proc->m_curTask.get() : nullptr;
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

    Processor::Processor(Scheduler *scheduler, int id)
        : m_scheduler(scheduler), m_id(id), m_cond(m_mutex)
    {
    }

    void Processor::addTask(Task::ptr &task)
    {
        m_newQueue.push(task);
        tickle();
    }

    bool Processor::getNextTask(bool flag)
    {
        // 有任务则取任务
        if (!m_runableQueue.empty())
        {
            m_curTask = std::move(m_runableQueue.front());
            m_runableQueue.pop();
        }
        else
        {
            // 没任务则尝试添加新任务
            if (flag || (m_addNewRemain > 0))
            {
                addNewTask();

                if (!flag)
                {
                    --m_addNewRemain;
                }
            }

            // 添加成功则取任务
            if (!m_runableQueue.empty())
            {
                m_curTask = std::move(m_runableQueue.front());
                m_runableQueue.pop();
            }
            // 失败则返回失败
            else
            {
                return false;
            }
        }
        return true;
    }

    void Processor::wakeupTask(uint64_t id)
    {
        auto it = m_waitMap.find(id);
        if (it == m_waitMap.end())
        {
            return;
        }
        m_runableQueue.push(it->second);
        m_waitMap.erase(it);

        tickle();
    }

    inline void Processor::garbageCollection()
    {
        m_garbageQueue.clear();
    }

    void Processor::start()
    {
        MutexType::Lock lock(m_mutex);
        m_thread = Thread::Create([this]()
                                  { this->run(); },
                                  "Proc_" + std::to_string(m_id));
    }

    void Processor::tickle()
    {
        MutexType::Lock lock(m_mutex);
        // 若处于休眠态，则唤醒
        if (m_idled)
        {
            std::cout << "tickle" << std::endl;
            m_cond.notify_one();
        }
        // 若处于工作态，则尝试通知
        else
        {
            // 若已被通知，则不通知
            if (!m_notified)
            {
                std::cout << "notify" << std::endl;
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
        std::cout << "condition wait" << std::endl;
        m_cond.wait();
        std::cout << "condition back" << std::endl;
        m_idled = false;
    }

    void Processor::run()
    {
        GetCurrentProcessor() = this;
        while (m_scheduler->m_started)
        {
            if (!getNextTask(true)) // 获取任务失败则表示当前处理器空闲
            {
                // 执行空闲任务
                idle();
                // 回到了工作状态，说明调度了新任务到队列中
                std::cout << "back" << std::endl;
                addNewTask();
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
                if (m_curTask->state() == TaskState::READY)
                {
                    m_runableQueue.push(m_curTask);
                }
                // 若为hold态则加入等待队列
                else if (m_curTask->state() == TaskState::HOLD)
                {
                    m_waitMap[m_curTask->id()] = std::move(m_curTask);
                }
                // 若为term态则加入垃圾队列，等待析构
                else if (m_curTask->state() == TaskState::TERM)
                {
                    if (m_garbageQueue.size() > 16)
                    {
                        garbageCollection();
                    }
                    m_garbageQueue.push(m_curTask);
                }
                getNextTask();
            }
        }
    }

} // namespace lim_webserver
