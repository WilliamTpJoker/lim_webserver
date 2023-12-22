#pragma once

#include <memory>
#include <set>
#include <vector>

#include "Thread.h"

namespace lim_webserver
{

    class TimerManager;
    /**
     * @brief 定时器
     */
    class Timer : public std::enable_shared_from_this<Timer>
    {
        friend TimerManager;
        
    public:
        using ptr = std::shared_ptr<Timer>;

    public:
        /**
         * @brief 取消定时器
         */
        bool cancel();
        /**
         * @brief 刷新设置定时器的执行时间
         */
        bool refresh();
        /**
         * @brief 重置定时器时间
         * @param ms 定时器执行间隔时间(毫秒)
         * @param from_now 是否从当前时间开始计算
         */
        bool reset(uint64_t ms, bool from_now=false);

    private:
        Timer(uint64_t time, std::function<void()> callback, bool recurring, TimerManager *manager);
        Timer(uint64_t next);

        /**
         * @brief 定时器比较仿函数
         */
        struct Comparator
        {
            /**
             * @brief 比较定时器的智能指针的大小(按执行时间排序)
             * @param lhs 定时器智能指针
             * @param rhs 定时器智能指针
             */
            bool operator()(const Timer::ptr &lhs, const Timer::ptr &rhs) const;
        };

    private:
        bool m_recurring = false;                   // 是否循环
        u_int64_t m_ms = 0;                         // 执行周期
        u_int64_t m_next = 0;                       // 精确的执行事件
        std::function<void()> m_callback = nullptr; // 召回函数
        TimerManager *m_manager;                    // 管理器
    };

    /**
     * @brief 定时器管理器
     */
    class TimerManager
    {
        friend Timer;

    public:
        using RWMutexType = RWMutex;

    public:
        TimerManager();
        virtual ~TimerManager();

        /**
         * @brief 添加定时器
         * @param ms 定时器执行间隔时间
         * @param callback 定时器回调函数
         * @param recurring 是否循环定时器
         */
        Timer::ptr addTimer(uint64_t ms, std::function<void()> callback, bool recurring = false);
        /**
         * @brief 添加条件定时器
         * @param ms 定时器执行间隔时间
         * @param callback 定时器回调函数
         * @param weak_cond 条件
         * @param recurring 是否循环
         */
        Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> callback, std::weak_ptr<void> weak_cond, bool recurring = false);

        /**
         * @brief 到最近一个定时器执行的时间间隔(毫秒)
         */
        uint64_t getNextTimer();

        /**
         * @brief 获取需要执行的定时器的回调函数列表
         * @param callback_list 回调函数数组
         */
        void listExpiredCallback(std::vector<std::function<void()>> &callback_list);

    protected:
        /**
         * @brief 当有新的定时器插入到定时器的首部,执行该函数
         */
        virtual void onTimerInsertedAtFront() = 0;

        /**
         * @brief 将定时器添加到管理器中
         */
        void addTimer(Timer::ptr timer, RWMutexType::WriteLock& lock);

        /**
         * @brief 检测服务器时间是否被调后了
         */
        bool detectClockRollover(uint64_t now_ms);

    private:
        std::set<Timer::ptr, Timer::Comparator> m_timer_set; // 定时器集合
        bool m_tickled = false;                              // 是否触发onTimerInsertedAtFront
        uint64_t m_previousTime = 0;                         // 上次执行时间
        RWMutexType m_mutex;
    };
}