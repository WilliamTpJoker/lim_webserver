#include "Timer.h"
#include "Util.h"

namespace lim_webserver
{
    Timer::Timer(uint64_t time, std::function<void()> callback, bool recurring, TimerManager *manager)
        : m_recurring(recurring), m_ms(time), m_callback(callback), m_manager(manager)
    {
        m_next = GetCurrentMS() + m_ms;
    }

    Timer::Timer(uint64_t next)
        : m_next(next)
    {
    }

    bool Timer::cancel()
    {
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
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
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
        if (!m_callback)
        {
            return false;
        }
        auto it = m_manager->m_timer_set.find(shared_from_this());
        if (it != m_manager->m_timer_set.end())
        {
            m_next = GetCurrentMS() + m_ms;
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
        TimerManager::RWMutexType::WriteLock lock(m_manager->m_mutex);
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
                start = GetCurrentMS();
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
    {
    }

    TimerManager::~TimerManager()
    {
    }

    Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> callback, bool recurring)
    {
        Timer::ptr timer(new Timer(ms, callback, recurring, this));
        RWMutexType::WriteLock lock(m_mutex);
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

    uint64_t TimerManager::getNextTimer()
    {
        RWMutexType::ReadLock lock(m_mutex);
        m_tickled = false;
        if (m_timer_set.empty())
        {
            return ~0ull;
        }

        const Timer::ptr &next = *m_timer_set.begin();
        uint64_t now_ms = GetCurrentMS();
        if (now_ms >= next->m_next)
        {
            return 0;
        }
        else
        {
            return next->m_next - now_ms;
        }
    }

    void TimerManager::listExpiredCallback(std::vector<std::function<void()>> &callback_list)
    {
        uint64_t now_ms = GetCurrentMS();
        std::vector<Timer::ptr> expired_timer_list;
        {
            RWMutexType::ReadLock lock(m_mutex);
            if (m_timer_set.empty())
            {
                return;
            }
        }
        RWMutexType::WriteLock lock(m_mutex);
        if (m_timer_set.empty())
        {
            return;
        }
        bool rollover = detectClockRollover(now_ms);
        if (!rollover && ((*m_timer_set.begin())->m_next > now_ms))
        {
            return;
        }
        Timer::ptr now_timer(new Timer(now_ms));
        // 若超时则将全部timer重置
        auto it = rollover ? m_timer_set.end() : m_timer_set.lower_bound(now_timer);
        while (it != m_timer_set.end() && (*it)->m_next == now_ms)
        {
            ++it;
        }
        expired_timer_list.insert(expired_timer_list.begin(), m_timer_set.begin(), it);
        m_timer_set.erase(m_timer_set.begin(), it);
        callback_list.reserve(expired_timer_list.size());

        for (auto &timer : expired_timer_list)
        {
            callback_list.push_back(timer->m_callback);
            if (timer->m_recurring)
            {
                timer->m_next = now_ms + timer->m_ms;
                m_timer_set.insert(timer);
            }
            else
            {
                timer->m_callback = nullptr;
            }
        }
    }

    void TimerManager::addTimer(Timer::ptr timer, RWMutexType::WriteLock& lock)
    {
        auto it = m_timer_set.insert(timer).first;
        bool at_front = (it == m_timer_set.begin()) && !m_tickled;
        if (at_front)
        {
            m_tickled = true;
        }
        lock.unlock();
        if (at_front)
        {
            onTimerInsertedAtFront();
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
}