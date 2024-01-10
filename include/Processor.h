#pragma once

#include "Task.h"
#include "Noncopyable.h"
#include "Thread.h"
#include "SafeQueue.h"
#include "Mutex.h"

#include <assert.h>

namespace lim_webserver
{
    class Scheduler1;

    class Processor : public Noncopyable
    {
        friend Scheduler1;
        friend std::unique_ptr<Processor>;
    public:
        using MutexType = Mutex;
        static Processor *&GetCurrentProcessor();

        static Task *GetCurrentTask();

        static void CoYield();

    public:
        inline Scheduler1 *getScheduler(){return m_scheduler;}

        void coYield();
        
    private:
        Processor();
        explicit Processor(Scheduler1 *scheduler, int id);

        /**
         * @brief 从新任务队列获取任务
         *
         */
        inline void addNewTask()
        {
            m_runableQueue.swap(m_newQueue);
            assert(m_newQueue.empty());
        }

        void addTask(Task::ptr& task);

        /**
         * @brief 调度下一个任务
         *
         * @param flag 是否无视调度限制规则
         * @return true 调度成功
         * @return false 无下一个任务
         */
        bool getNextTask(bool flag = false);

        /**
         * @brief 回收垃圾协程栈
         *
         */
        inline void garbageCollection();

        /**
         * @brief 生命周期开始
         *
         */
        void start();

        /**
         * @brief 唤醒
         * 
         */
        void tickle();

        /**
         * @brief 空闲时执行任务
         *
         */
        void idle();

        /**
         * @brief 线程执行任务
         *
         */
        void run();

    private:
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