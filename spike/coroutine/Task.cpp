#include "base/BackTrace.h"
#include "coroutine/Task.h"
#include "coroutine/Processor.h"

#include <atomic>
#include <iostream>

namespace lim_webserver
{
    static std::atomic<uint64_t> s_task_id{0};

    Task::Task(TaskFunc func, size_t size)
        : m_context((ContextFunc)&Task::StaticRun, (uintptr_t)this, size), m_callback(func), m_id(++s_task_id)
    {
    }

    Task::~Task()
    {
    }

    void Task::run()
    {
        auto callback = [this]()
        {
            m_state = TaskState::EXEC;
            m_callback();
            m_callback = TaskFunc();
            m_state = TaskState::TERM;
        };

        try
        {
            // 执行用户指定的回调函数
            callback();
        }
        catch (const std::exception &e)
        {
            m_callback = TaskFunc();
            m_state = TaskState::EXCEPT;
            std::cout << "Task Except: " << e.what() << " task_id=" << getId()
                      << "\n"
                      << BackTraceToString() << std::endl;
        }
        catch (...)
        {
            m_callback = TaskFunc();
            m_state = TaskState::EXCEPT;
            std::cout << "Task Except: task_id=" << getId()
                      << "\n"
                      << BackTraceToString() << std::endl;
        }
        Processor::CoYield();
    }

    void Task::StaticRun(uint32_t low32, uint32_t high32)
    {
        uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)high32 << 32);
        Task *tk = (Task *)ptr;
        tk->run();
    }

} // namespace lim_webserver
