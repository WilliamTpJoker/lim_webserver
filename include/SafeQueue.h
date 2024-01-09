#pragma once

#include "Mutex.h"

#include <queue>

namespace lim_webserver
{
    template<class T>
    class SafeQueue
    {
        public:
            using MutexType = Spinlock;
        public:
            inline T& front()
            {
                MutexType::Lock lock(m_mutex);
                return m_queue.front();
            }

            inline void pop()
            {
                MutexType::Lock lock(m_mutex);
                m_queue.pop();
            }
            
            inline void push(T& item)
            {
                MutexType::Lock lock(m_mutex);
                m_queue.push(std::move(item));
            }
            
            inline bool empty()
            {
                MutexType::Lock lock(m_mutex);
                return m_queue.empty();
            }

            inline void swap(SafeQueue<T> &other)
            {
                MutexType::Lock lock(m_mutex);
                MutexType::Lock lock2(other.m_mutex);
                m_queue.swap(other.m_queue);
            }

            inline void clear()
            {
                MutexType::Lock lock(m_mutex);
                std::queue<T> newQueue;
                m_queue.swap(newQueue);
            }

            inline size_t size()
            {
                MutexType::Lock lock(m_mutex);
                return m_queue.size();
            }

        private:
            std::queue<T> m_queue;
            MutexType m_mutex;
    };
} // namespace lim_webserver
