#ifndef __THREAD_H__
#define __THREAD_H__


#include "mutex.h"
#include "log.h"

namespace lim_webserver
{

    // 线程
    class Thread : Noncopyable
    {
    public:
        Thread(std::function<void()> callback, const std::string &name = "UNKNOWN");
        ~Thread();

        pid_t getId() const { return m_id; }
        const std::string &getName() const { return m_name; }

        void join();

    public:
        static Thread *GetThis();
        static std::string GetThisThreadName();
        static void SetName(const std::string &name);

    private:
        static void *run(void *args);

    private:
        // 线程id
        pid_t m_id = -1;
        // 线程结构
        pthread_t m_thread = 0;
        // 线程执行函数
        std::function<void()> m_callback;
        // 线程名称
        std::string m_name;
        // 信号量
        Semaphore m_semaphore;
    };

}

#endif