#include "Task.h"
#include "Util.h"

#include <atomic>
#include <iostream>

namespace lim_webserver
{
    static std::atomic<uint64_t> s_task_id{0};

    Task::Task(FuncType func, size_t size)
        : m_context(size), m_callback(func), m_id(++s_task_id)
    {
    }

    Task::~Task()
    {
    }

    void Task::run()
    {
        auto callback = [this]()
        {
            m_callback();
            m_callback = FuncType();
            m_state = TaskState::TERM;
        };

        try
        {
            // 执行用户指定的回调函数
            callback();
        }
        catch (const std::exception &e)
        {
            m_callback = FuncType();
            m_state = TaskState::EXCEPT;
            std::cout << "Task Except: " << e.what() << " task_id=" << getId()
                      << "\n"
                      << BackTraceToString() << std::endl;
        }
        catch (...)
        {
            m_callback = FuncType();
            m_state = TaskState::EXCEPT;
            std::cout << "Task Except: task_id=" << getId()
                      << "\n"
                      << BackTraceToString() << std::endl;
        }

    }

} // namespace lim_webserver
