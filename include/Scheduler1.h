#pragma once

#include "Task.h"
#include "Processor.h"
#include "Noncopyable.h"
#include "Thread.h"
#include "Mutex.h"

#include <vector>

namespace lim_webserver
{
    class Scheduler1 : public Noncopyable
    {
        friend Processor;

    public:
        using MutexType = Spinlock;

    public:
        void start(int num_threads);
        void stop();

    private:
        Scheduler1(){}
        ~Scheduler1();
        /**
         * @brief 线程工作函数，负责为处理器线程分发协程任务
         *
         */
        void run();

        /**
         * @brief 创建新的处理器
         *
         */
        void newPoccessorThread();

    private:
        size_t m_threadCounts;                 // 工作线程数
        std::vector<Processor *> m_processors; // 处理器池
        Processor *m_mainProcessor;            // 主处理器
        int m_threadId;                        // Scheduler所在线程
        bool m_started;                        // 开始标志位
        Thread::ptr m_thread;                  // 绑定线程
        MutexType m_mutex;                     // 锁
    };
} // namespace lim_webserver
