#include "thread.h"

namespace lim_webserver
{
    static thread_local Thread *t_thread = nullptr;
    static thread_local std::string t_thread_name = "UNKNOWN";
    static Shared_ptr<Logger> thread_logger = LIM_LOG_NAME("system");

    Thread::Thread(std::function<void()> callback, const std::string &name)
        : m_name(name), m_callback(callback)
    {
        int rt = pthread_create(&m_thread, nullptr, &Thread::run, this);
        if (rt)
        {
            LIM_LOG_ERROR(thread_logger) << "pthread_create thread fail, rt=" << rt << " name=" << name;
            throw std::logic_error("pthread_create error");
        }
        m_semaphore.wait(); //等待初始化 确保各线程的执行顺序
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

    std::string Thread::GetThisThreadName()
    {
        return t_thread_name;
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
        thread->m_id = lim_webserver::GetThreadId();
        pthread_setname_np(pthread_self(), thread->m_name.substr(0, 15).c_str());

        std::function<void()> callback;
        callback.swap(thread->m_callback);

        thread->m_semaphore.notify(); //初始化完毕，唤醒
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
                LIM_LOG_ERROR(thread_logger) << "pthread_join thread fail, rt=" << rt << " name=" << m_name;
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }
}