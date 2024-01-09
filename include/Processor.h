#pragma once

#include "Task.h"
#include "Noncopyable.h"
#include "Thread.h"
#include "SafeQueue.h"
#include "Mutex.h"

namespace lim_webserver
{
    class Scheduler1;

    class Processor : public Noncopyable
    {
        friend Scheduler1;

    public:
        using MutexType = Mutex;
        static Processor *&GetCurrentProcessor();

    private:
        Processor();
        explicit Processor(Scheduler1 *scheduler, int id);

        inline void addNewTask();

        bool getNextTask();

        inline void garbageCollection();

        void start();

        void run();

        using TaskQueue = SafeQueue<Task::ptr>;
        Task::ptr m_curTask = nullptr;       // 当前运行的任务
        TaskQueue m_newQueue;                // 新任务队列
        TaskQueue m_waitQueue;               // 等待队列
        TaskQueue m_runableQueue;            // 可工作队列
        TaskQueue m_garbageQueue;            // 待回收队列
        Scheduler1 *m_scheduler;             // 调度者
        int m_id;                            // 编号
        bool m_started;                      // 开始标志位
        Thread::ptr m_thread;                // 绑定线程
        volatile uint64_t m_switchCount = 0; // 调度次数
        int m_addNewRemain;                  // 每轮调度可添加新任务次数

        MutexType m_mutex;
        ConditionVariable m_cond;
    };
} // namespace lim_webserver
