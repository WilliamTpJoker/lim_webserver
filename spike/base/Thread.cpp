#include "Thread.h"

#include <sys/syscall.h>
#include <unistd.h>


namespace lim_webserver
{
    
    static thread_local Thread *t_thread = nullptr;
    static thread_local std::string t_thread_name = "unknown";
    static thread_local pid_t t_thread_id = 0;

    void setThreadID()
    {
        t_thread_id = syscall(SYS_gettid);
    }

    struct ThreadInitializer
    {
        ThreadInitializer()
        {
            t_thread_name = "main";
            setThreadID();
        }
    };

    static ThreadInitializer g_thread_init;

    Thread::Thread(std::function<void()> callback, const std::string &name)
        : m_name(name), m_callback(callback)
    {
        int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
        if (rt)
        {
            throw std::logic_error("pthread_create error");
        }
        m_semaphore.wait(); // 等待初始化 确保各线程的执行顺序
    }

    Thread::~Thread()
    {
        if (m_thread)
        {
            pthread_join(m_thread, nullptr);
        }
    }

    Thread *Thread::GetThis()
    {
        return t_thread;
    }

    const std::string &Thread::GetThreadName()
    {
        return t_thread_name;
    }

    const pid_t Thread::GetThreadId()
    {
        return t_thread_id;
    }

    void Thread::SetName(const std::string &name)
    {
        if (t_thread)
        {
            t_thread->m_name = name;
        }
        t_thread_name = name;
    }

    void *Thread::run(void *args)
    {
        Thread *thread = (Thread *)args;
        t_thread = thread;
        t_thread_name = thread->m_name;
        setThreadID();
        thread->m_id = t_thread_id;
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

        std::function<void()> callback;
        callback.swap(thread->m_callback);

        thread->m_semaphore.notify(); // 初始化完毕，唤醒
        callback();
        return 0;
    }

    void Thread::join()
    {
        if (m_thread)
        {
            int rt = pthread_join(m_thread, nullptr);
            if (rt)
            {
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }
}