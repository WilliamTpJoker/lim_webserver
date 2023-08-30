#ifndef __LIM_SCHEDULER_H__
#define __LIM_SCHEDULER_H__

#include <atomic>

#include "macro.h"
#include "fiber.h"
#include "mutex.h"
#include "thread.h"

namespace lim_webserver
{
    class Scheduler : public Noncopyable
    {
    public:
        using MutexType = Mutex;

        /**
         * @brief 构造函数，创建调度器对象
         * @param threads 线程数量
         * @param use_caller 是否使用 Scheduler 实例化所在的线程（Scheduler 所在线程也工作）
         * @param name 调度器名称
         */
        Scheduler(size_t threads = 1, bool use_caller = true, const std::string &name = "UNKNOWN");
        /**
         * @brief 析构函数，销毁调度器对象
         */
        virtual ~Scheduler();
        /**
         * @brief 获取调度器名称
         * @return 调度器名称
         */
        const std::string &getName() const { return m_name; }

        /**
         * @brief 启动调度器
         */
        void start();
        /**
         * @brief 停止调度器
         */
        void stop();

        /**
         * @brief 调度执行任务或协程
         * @tparam FiberOrCb 协程或回调类型
         * @param fc 协程或回调对象
         * @param thread 指定线程
         */
        template <class FiberOrCb>
        void schedule(FiberOrCb fc, int thread = -1)
        {
            bool need_tickle = false;
            {
                MutexType::Lock lock(m_mutex);
                need_tickle = localSchedule(fc, thread);
            }
            if (need_tickle)
            {
                tickle();
            }
        }

        /**
         * @brief 调度执行一组任务或协程
         * @tparam InputIterator 输入迭代器类型
         * @param begin 起始迭代器
         * @param end 结束迭代器
         */
        template <class InputIterator>
        void schedule(InputIterator begin, InputIterator end)
        {
            bool need_tickle = false;
            {
                MutexType::Lock lock(m_mutex);
                while (begin != end)
                {
                    need_tickle = localSchedule(&*begin, -1) || need_tickle;
                    ++begin;
                }
            }
            if (need_tickle)
            {
                tickle();
            }
        }

    protected:
        /**
         * @brief 触发调度
         */
        virtual void tickle();
        virtual void onIdle();
        void run();
        virtual bool onStop();

    private:
        /**
         * @brief 本地调度
         * @tparam FiberOrCb 协程或回调类型
         * @param fc 协程或回调对象
         * @param thread 指定线程
         * @return 是否需要触发调度
         */
        template <class FiberOrCb>
        bool localSchedule(FiberOrCb fc, int thread)
        {
            bool need_tickle = m_task_queue.empty();
            FiberAndThread ft(fc, thread);
            if (ft.fiber || ft.callback)
            {
                m_task_queue.emplace_back(ft);
            }
            return need_tickle;
        }

    public:
        /**
         * @brief 获取当前线程的调度器对象
         * @return 当前线程的调度器对象
         */
        static Scheduler *GetThis();
        /**
         * @brief 获取主协程
         * @return 主协程指针
         */
        static Fiber *GetMainFiber();

    private:
        struct FiberAndThread
        {
            Shared_ptr<Fiber> fiber;        // 协程
            std::function<void()> callback; // 回调
            int thread;                     // 线程编号

            FiberAndThread(Shared_ptr<Fiber> fbr, int trd)
                : fiber(fbr), thread(trd) {}
            // 传入指针则使用swap切换。
            FiberAndThread(Shared_ptr<Fiber> *fbr, int trd)
                : thread(trd) { fiber.swap(*fbr); }

            FiberAndThread(std::function<void()> cb, int trd)
                : callback(cb), thread(trd) {}
            FiberAndThread(std::function<void()> *cb, int trd)
                : thread(trd) { callback.swap(*cb); }

            FiberAndThread() : thread(-1){};

            /**
             * @brief 重置协程和线程结构体
             */
            void reset()
            {
                fiber = nullptr;
                callback = nullptr;
                thread = -1;
            }
        };

    protected:
        std::vector<int> m_threadIds;                  // 存储已创建的工作线程的线程ID
        size_t m_threadCount = 0;                      // 工作线程数量
        std::atomic<size_t> m_activeThreadCount = {0}; // 原子计数器，表示当前活动中的工作线程数量
        std::atomic<size_t> m_idleThreadCount = {0};   // 原子计数器，表示当前空闲的工作线程数量
        bool m_stopping = true;                        // 标志变量，表示调度器是否处于停止状态
        bool m_autoStop = false;                       // 标志变量，表示是否在所有协程执行完毕后自动停止调度器
        int m_rootThread = 0;                          // 根协程所在的线程ID

    private:
        MutexType m_mutex;                             // 互斥锁
        std::vector<Shared_ptr<Thread>> m_thread_list; // 线程池
        std::list<FiberAndThread> m_task_queue;        // 任务列表
        std::string m_name;                            // 调度器名称
        Shared_ptr<Fiber> m_rootFiber;                 // 主协程
    };
}

#endif