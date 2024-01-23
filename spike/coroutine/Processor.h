#pragma once

#include "base/Noncopyable.h"
#include "base/Thread.h"
#include "base/Mutex.h"
#include "coroutine/Task.h"
#include "coroutine/SafeQueue.h"

#include <assert.h>
#include <atomic>
#include <unordered_map>

namespace lim_webserver
{
    class Scheduler;

    class Processor : public Noncopyable
    {
        friend Scheduler;

    public:
        using MutexType = Mutex;

        /**
         * @brief 获得当前Processor对象
         *
         * @return Processor*&
         */
        static Processor *&GetCurrentProcessor();

        static Scheduler *GetCurrentScheduler();

        /**
         * @brief 获得当前Task对象
         *
         * @return Task*
         */
        static Task *GetCurrentTask();

        /**
         * @brief 当前Task主动让出线程并重新入队
         *
         */
        static void CoYield();

        /**
         * @brief 当前Task主动阻塞并等待唤醒
         *
         */
        static void CoHold();

    public:
        /**
         * @brief 获得该Processor的调度器
         *
         * @return Scheduler*
         */
        inline Scheduler *getScheduler() { return m_scheduler; }

        /**
         * @brief 唤醒等待队列中的任务
         *
         * @param uint64_t 协程id
         */
        void wakeupTask(uint64_t id);

    private:
        Processor();
        explicit Processor(Scheduler *scheduler, int id);

        /**
         * @brief 当前Processor的task主动让出线程并重新入队
         *
         */
        void coYield();

        /**
         * @brief 当前Processor的Task主动阻塞并等待唤醒
         *
         */
        static void coHold();

        /**
         * @brief 确认是否处在空闲态
         *
         * @return true 空闲
         * @return false 工作
         */
        inline bool isIdled() { return m_idled; }

        /**
         * @brief 从新任务队列获取任务
         *
         */
        inline void addNewTask()
        {
            while (!m_newQueue.empty())
            {
                m_runableQueue.push(m_newQueue.front());
                m_newQueue.pop();
            }
            assert(m_newQueue.empty());
        }

        /**
         * @brief 添加任务到newQueue
         *
         * @param task
         */
        void addTask(Task::ptr &task);

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
        using TaskMap = std::unordered_map<uint64_t, Task::ptr>;
        Task::ptr m_curTask = nullptr;       // 当前运行的任务
        TaskQueue m_newQueue;                // 新任务队列
        TaskMap m_waitMap;                   // 等待队列
        TaskQueue m_runableQueue;            // 可工作队列
        TaskQueue m_garbageQueue;            // 待回收队列
        Scheduler *m_scheduler;              // 调度者
        int m_id;                            // 编号
        volatile bool m_activated = true;    // 激活标志位
        volatile bool m_notified = false;    // 通知标志位
        std::atomic_bool m_idled{false};     // 空闲标志位
        Thread::ptr m_thread;                // 绑定线程
        volatile uint64_t m_switchCount = 0; // 调度次数
        int m_addNewRemain;                  // 每轮调度可添加新任务次数

        MutexType m_mutex;
        ConditionVariable m_cond;
    };
} // namespace lim_webserver
