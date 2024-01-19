#include "coroutine/Timer.h"
#include "base/TimeStamp.h"

namespace lim_webserver
{
    Timer::Timer(uint64_t time, std::function<void()> callback, bool recurring, TimerManager *manager)
        : m_recurring(recurring), m_ms(time), m_callback(callback), m_manager(manager)
    {
        m_next = TimeStamp::now()->ms() + m_ms;
    }

    Timer::Timer(uint64_t next)
        : m_next(next)
    {
    }

    bool Timer::cancel()
    {
        TimerManager::MutexType::Lock lock(m_manager->m_mutex);
        if (m_callback)
        {
            m_callback = nullptr;
            auto it = m_manager->m_timer_set.find(shared_from_this());
            m_manager->m_timer_set.erase(it);
            return true;
        }
        return false;
    }

    bool Timer::refresh()
    {
        TimerManager::MutexType::Lock lock(m_manager->m_mutex);
        if (!m_callback)
        {
            return false;
        }
        auto it = m_manager->m_timer_set.find(shared_from_this());
        if (it != m_manager->m_timer_set.end())
        {
            m_next = TimeStamp::now()->ms() + m_ms;
            m_manager->m_timer_set.erase(it);
            m_manager->m_timer_set.insert(shared_from_this());
            return true;
        }
        return false;
    }

    bool Timer::reset(uint64_t ms, bool from_now)
    {
        if (ms == m_ms && !from_now)
        {
            return true;
        }
        TimerManager::MutexType::Lock lock(m_manager->m_mutex);
        if (!m_callback)
        {
            return false;
        }
        auto it = m_manager->m_timer_set.find(shared_from_this());
        if (it != m_manager->m_timer_set.end())
        {
            m_manager->m_timer_set.erase(it);

            uint64_t start = 0;
            if (from_now)
            {
                start = TimeStamp::now()->ms();
            }
            else
            {
                start = m_next - m_ms;
            }
            m_ms = ms;
            m_next = start + m_ms;
            m_manager->addTimer(shared_from_this(), lock);
            return true;
        }
        return false;
    }

    bool Timer::Comparator::operator()(const Timer::ptr &lhs, const Timer::ptr &rhs) const
    {
        if (!lhs)
        {
            return false;
        }
        if (!rhs)
        {
            return true;
        }
        return lhs->m_next < rhs->m_next || (rhs->m_next == lhs->m_next && lhs.get() < rhs.get());
    }

    TimerManager::TimerManager()
        : m_cond(m_mutex)
    {
        m_thread = Thread::Create([this]
                                  { this->run(); },
                                  "Timer");
    }

    TimerManager::~TimerManager()
    {
        stop();
    }

    Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> callback, bool recurring)
    {
        Timer::ptr timer(new Timer(ms, callback, recurring, this));
        MutexType::Lock lock(m_mutex);
        addTimer(timer, lock);
        return timer;
    }

    Timer::ptr TimerManager::addConditionTimer(uint64_t ms, std::function<void()> callback, std::weak_ptr<void> weak_cond, bool recurring)
    {
        return addTimer(
            ms,
            [weak_cond, callback]()
            {
                std::shared_ptr<void> tmp = weak_cond.lock();
                if (tmp)
                {
                    callback();
                }
            },
            recurring);
    }

    uint64_t TimerManager::getNextExpire()
    {
        // 没有超时任务，则一秒后再次查询
        if (m_timer_set.empty())
        {
            return 1000;
        }

        // 获取最近超时点
        const Timer::ptr &next = *m_timer_set.begin();
        uint64_t now_ms = TimeStamp::now()->ms();

        // 已超时则返回0
        if (now_ms >= next->m_next)
        {
            return 0;
        }
        else // 未超时则返回剩余时间
        {
            return next->m_next - now_ms;
        }
    }

    void TimerManager::stop()
    {
        if (!m_started)
        {
            return;
        }
        {
            MutexType::Lock lock(m_mutex);
            m_started = false;
        }
        m_thread->join();
    }

    void TimerManager::listExpiredCallback(std::vector<std::function<void()>> &callback_list)
    {
        MutexType::Lock lock(m_mutex);
        // 若没有定时器任务则返回
        if (m_timer_set.empty())
        {
            return;
        }

        // 检测系统时间
        uint64_t now_ms = TimeStamp::now()->ms();
        bool rollover = detectClockRollover(now_ms);
        if (!rollover && ((*m_timer_set.begin())->m_next > now_ms))
        {
            return;
        }

        Timer::ptr now_timer(new Timer(now_ms));
        // 如果系统超时则指针指向尾部，未超时则指向第一个大于当前时间的位置
        auto it = rollover ? m_timer_set.end() : m_timer_set.lower_bound(now_timer);
        while (it != m_timer_set.end() && (*it)->m_next == now_ms)
        {
            ++it;
        }

        // 将超时任务加入超时队列并移出定时器任务队列
        std::vector<Timer::ptr> expired_timer_vec;
        expired_timer_vec.insert(expired_timer_vec.begin(), m_timer_set.begin(), it);
        m_timer_set.erase(m_timer_set.begin(), it);

        // 将回调函数提取
        callback_list.reserve(expired_timer_vec.size());
        for (auto &timer : expired_timer_vec)
        {
            callback_list.push_back(timer->m_callback);
            // 若为循环定时器任务则重新添加到红黑树中
            if (timer->m_recurring)
            {
                timer->m_next = now_ms + timer->m_ms;
                m_timer_set.insert(timer);
            }
            // 不为则释放回调
            else
            {
                timer->m_callback = nullptr;
            }
        }
    }

    void TimerManager::addTimer(Timer::ptr timer, MutexType::Lock &lock)
    {
        // Timer入队
        auto it = m_timer_set.insert(timer).first;

        // 如果加入了队首则需要重新设置Cond等待时间
        if (it == m_timer_set.begin())
        {
            lock.unlock();
            tickle();
        }
    }

    void TimerManager::doFunc(std::function<void()> &callback)
    {
        try
        {
            // 执行用户指定的回调函数
            callback();
        }
        catch (...)
        {
        }
    }

    bool TimerManager::detectClockRollover(uint64_t now_ms)
    {
        bool rollover = false;
        if (now_ms < m_previousTime && now_ms < (m_previousTime - 60 * 60 * 1000))
        {
            rollover = true;
        }
        m_previousTime = now_ms;
        return rollover;
    }
    void TimerManager::tickle()
    {
        MutexType::Lock lock(m_mutex);

        // 唤醒
        m_cond.notify_one();
    }

    void TimerManager::run()
    {
        while (m_started)
        {
            {
                // 获取下一个超时点
                MutexType::Lock lock(m_mutex);
                int newTimeOut = getNextExpire();
                m_cond.waitTime(newTimeOut);
            }

            // 获取超时回调
            std::vector<std::function<void()>> callback_list;
            listExpiredCallback(callback_list);
            if (!callback_list.empty())
            {
                for (auto &callback : callback_list)
                {
                    doFunc(callback);
                }
                callback_list.clear();
            }
        }
    }
} // namespace lim_webserver
