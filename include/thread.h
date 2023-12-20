#pragma once

#include "mutex.h"

namespace lim_webserver
{
    // 线程类，用于封装线程的操作
    class Thread : Noncopyable
    {
    public:
        using ptr = std::shared_ptr<Thread>;
        /**
         * 创建线程对象智能指针，并指定线程函数和名称。
         *
         * @param callback 线程要执行的函数
         * @param name     线程的名称，默认为"UNKNOWN"
         */
        static ptr Create(std::function<void()> callback, const std::string &name = "UNKNOWN")
        {
            return std::make_shared<Thread>(callback, name);
        }

    public:
        /**
         * 构造函数，创建线程对象，并指定线程函数和名称。
         *
         * @param callback 线程要执行的函数
         * @param name     线程的名称，默认为"UNKNOWN"
         */
        Thread(std::function<void()> callback, const std::string &name = "UNKNOWN");

        /** 析构函数，释放线程资源 */
        ~Thread();

        /** 获取线程的ID */
        pid_t getId() const { return m_id; }

        /** 获取线程的名称 */
        const std::string &getName() const { return m_name; }

        /** 阻塞等待线程的结束 */
        void join();

    public:
        /** 获取当前线程的Thread对象 */
        static Thread *GetThis();

        /** 获取当前线程的名称 */
        static const std::string& GetThreadName();

        /** 设置当前线程的名称 */
        static void SetName(const std::string &name);

    private:
        /** 线程的执行函数 */
        static void *run(void *args);

    private:
        pid_t m_id = -1;                  // 线程ID
        pthread_t m_thread = 0;           // 线程结构
        std::function<void()> m_callback; // 线程执行函数
        std::string m_name;               // 线程名称
        Semaphore m_semaphore;            // 信号量，用于同步线程的初始化
    };
}