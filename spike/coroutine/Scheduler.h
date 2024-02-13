#pragma once

#include "base/Mutex.h"
#include "base/Noncopyable.h"
#include "base/Singleton.h"
#include "base/Thread.h"
#include "coroutine/Processor.h"
#include "coroutine/Task.h"

#include <vector>

namespace lim_webserver
{
    class Scheduler : public Noncopyable, public Singleton<Scheduler>
    {
        friend Processor;
        friend Singleton<Scheduler>;

    public:
        using MutexType = Mutex;

        static Scheduler *Create();

        static Scheduler *CreateNetScheduler();

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
        void start(int num_threads = 1);

        /**
         * @brief 在新线程开始调度任务
         *
         * @param num_threads
         */
        void startInNewThread(int num_threads = 1);

        /**
         * @brief 关闭调度
         *
         */
        void stop();

        /**
         * @brief 设置名字
         *
         * @param name
         */
        inline void setName(const std::string &name) { m_name = name; }

        inline const std::string &name() const { return m_name; }

    private:
        Scheduler();
        ~Scheduler();

        inline void setProcessor(Processor *processor) { m_mainProcessor = processor; }

        /**
         * @brief 添加任务
         *
         * @param task 任务
         */
        void addTask(Task *&task);

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
        Processor *m_mainProcessor = nullptr;  // 主处理器
        size_t m_lastActiveIdx;                // 最后一次调度的处理器
        int m_threadId;                        // Scheduler所在线程
        bool m_started;                        // 开始标志位
        std::string m_name;                    // 名字
        Thread::ptr m_thread = nullptr;        // 绑定线程
        MutexType m_mutex;                     // 锁
        ConditionVariable m_cond;              // 条件变量
    };

} // namespace lim_webserver
