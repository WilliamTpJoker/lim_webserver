#pragma once

#include "Mutex.h"

#include <queue>

namespace lim_webserver
{
    template <class T>
    class SafeQueue
    {
        using MutexType = Spinlock;

    public:
        T &front()
        {
            MutexType::Lock lock(m_mutex);
            return m_queue.front();
        }

        void pop()
        {
            MutexType::Lock lock(m_mutex);
            m_queue.pop();
        }

        void push(T &item)
        {
            MutexType::Lock lock(m_mutex);
            m_queue.push(std::move(item));
        }

        bool empty()
        {
            MutexType::Lock lock(m_mutex);
            return m_queue.empty();
        }

        void swap(SafeQueue<T> &other)
        {
            MutexType::Lock lock(m_mutex);
            MutexType::Lock lock2(other.m_mutex);
            m_queue.swap(other.m_queue);
        }

        void clear()
        {
            MutexType::Lock lock(m_mutex);
            std::queue<T> newQueue;
            m_queue.swap(newQueue);
        }

        size_t size()
        {
            MutexType::Lock lock(m_mutex);
            return m_queue.size();
        }

    private:
        std::queue<T> m_queue;
        MutexType m_mutex;
    };
} // namespace lim_webserver
