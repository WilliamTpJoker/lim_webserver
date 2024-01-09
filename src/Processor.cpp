#include "Processor.h"
#include "Scheduler1.h"

#include <assert.h>

namespace lim_webserver
{
    Processor *&Processor::GetCurrentProcessor()
    {
        static thread_local Processor *proc = nullptr;
        return proc;
    }

    Processor::Processor(Scheduler1 *scheduler, int id)
        : m_scheduler(scheduler), m_id(id),m_cond(m_mutex)
    {
    }

    inline void Processor::addNewTask()
    {
        if (m_addNewRemain>0)
        {
            m_runableQueue.swap(m_newQueue);
            --m_addNewRemain;
            assert(m_newQueue.empty());
        }
    }

    bool Processor::getNextTask()
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
            addNewTask();
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

    inline void Processor::garbageCollection()
    {
        m_garbageQueue.clear();
    }

    void Processor::start()
    {
        if (m_started)
        {
            return;
        }
        m_started = true;
        m_thread = Thread::Create([this]()
                                  { this->run(); },
                                  "Processor" + m_id);
    }

    void Processor::run()
    {
        GetCurrentProcessor() = this;
        while (m_scheduler->m_started)
        {
            m_addNewRemain = 2;
            if (!getNextTask()) // 获取任务失败则等待新任务的分配
            {
                // TODO:
            }

            // 每轮环形协程调度后都重置
            while (m_curTask && m_scheduler->m_started)
            {
                m_curTask->m_state = TaskState::EXEC;
                m_curTask->m_processor = this;

                ++m_switchCount;
                m_curTask->swapIn();

                // 此时回到了Processor中
                // 若为ready态则重新加入调度队列
                if (m_curTask->m_state == TaskState::READY)
                {
                    m_runableQueue.push(m_curTask);
                }
                // 若为hold态则加入等待队列
                else if (m_curTask->m_state == TaskState::HOLD)
                {
                    m_waitQueue.push(m_curTask);
                }
                // 若为term态则加入垃圾队列，等待析构
                else if (m_curTask->m_state == TaskState::TERM)
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
