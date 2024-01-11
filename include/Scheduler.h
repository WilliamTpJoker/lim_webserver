#pragma once

#include "Task.h"
#include "Processor.h"
#include "Noncopyable.h"
#include "Thread.h"
#include "Mutex.h"
#include "Singleton.h"

#include <vector>

namespace lim_webserver
{
    class Scheduler : public Noncopyable
    {
        friend Processor;
        friend Singleton<Scheduler>;

    public:
        using MutexType = Spinlock;

        static Scheduler* Create();

    public:
        /**
         * @brief 创建新协程任务
         * 
         * @param func 
         */
        void createTask(TaskFunc const &func);

        /**
         * @brief 开始调度任务
         * 
         * @param num_threads 工作线程数
         */
        void start(int num_threads);

        /**
         * @brief 关闭调度
         * 
         */
        void stop();

    private:
        Scheduler() {}
        ~Scheduler();

        /**
         * @brief 添加任务
         * 
         * @param task 任务
         */
        void addTask(Task::ptr &task);
        
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
        size_t m_lastActiveIdx;                // 最后一次调度的处理器
        int m_threadId;                        // Scheduler所在线程
        bool m_started;                        // 开始标志位
        Thread::ptr m_thread;                  // 绑定线程
        MutexType m_mutex;                     // 锁
    };

#define g_Scheduler Singleton<Scheduler>::GetInstance()

} // namespace lim_webserver
