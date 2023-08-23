#ifndef __LIM_MUTEX_H__
#define __LIM_MUTEX_H__

#include "noncopyable.h"
#include <pthread.h>
#include <thread>
#include <functional>
#include <semaphore.h>

namespace lim_webserver
{
    /**
     * @brief 信号量
     */
    class Semaphore : Noncopyable
    {
    public:
        Semaphore(uint32_t count = 0);
        ~Semaphore();

        void wait();
        void notify();

    private:
        sem_t m_semaphore;
    };

    /**
     * @brief 局部锁模板
     */
    template <class T>
    class ScopedLock
    {
    public:
        ScopedLock(T &mutex) : m_mutex(mutex) { lock(); }
        ~ScopedLock() { unlock(); }

        void lock()
        {
            if (!m_locked)
            {
                m_mutex.lock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;
        bool m_locked;
    };

    /**
     * @brief 读锁模板
     */
    template <class T>
    class ReadScopedLock
    {
    public:
        ReadScopedLock(T &mutex) : m_mutex(mutex) { lock(); }
        ~ReadScopedLock() { unlock(); }

        void lock()
        {
            if (!m_locked)
            {
                m_mutex.rdlock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;
        bool m_locked;
    };

    /**
     * @brief 写锁模板
     */
    template <class T>
    class WriteScopedLock
    {
    public:
        WriteScopedLock(T &mutex) : m_mutex(mutex) { lock(); }
        ~WriteScopedLock() { unlock(); }

        void lock()
        {
            if (!m_locked)
            {
                m_mutex.wrlock();
                m_locked = true;
            }
        }

        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;
        bool m_locked;
    };

    /**
     * @brief 互斥锁
     */
    class Mutex
    {
    public:
        using Lock = ScopedLock<Mutex>;
        Mutex() { pthread_mutex_init(&m_mutex, nullptr); }
        ~Mutex() { pthread_mutex_destroy(&m_mutex); }

        void lock() { pthread_mutex_lock(&m_mutex); }
        void unlock() { pthread_mutex_unlock(&m_mutex); }

    private:
        pthread_mutex_t m_mutex;
    };

    /**
     * @brief 读写锁
     */
    class RWMutex
    {
    public:
        using ReadLock = ReadScopedLock<RWMutex>;
        using WriteLock = WriteScopedLock<RWMutex>;

        RWMutex() { pthread_rwlock_init(&m_mutex, nullptr); }
        ~RWMutex() { pthread_rwlock_destroy(&m_mutex); }

        // 读锁
        void rdlock() { pthread_rwlock_rdlock(&m_mutex); }
        // 写锁
        void wrlock() { pthread_rwlock_wrlock(&m_mutex); }
        // 解锁
        void unlock() { pthread_rwlock_unlock(&m_mutex); }

    private:
        pthread_rwlock_t m_mutex;
    };

    /**
     * @brief 空互斥锁(用于调试)
     */
    class NullMutex : Noncopyable
    {
    public:
        using Lock = ScopedLock<NullMutex>;

        NullMutex() {}
        ~NullMutex() {}

        void lock() {}

        void unlock() {}
    };

    /**
     * @brief 空读写锁(用于调试)
     */
    class NullRWMutex : Noncopyable
    {
    public:
        // 局部读锁
        using ReadLock = ReadScopedLock<NullRWMutex>;
        // 局部写锁
        using WriteLock = WriteScopedLock<NullRWMutex>;

        NullRWMutex() {}
        ~NullRWMutex() {}

        void rdlock() {}
        void wrlock() {}
        void unlock() {}
    };

    /**
     * @brief 自旋锁
     */
    class Spinlock : Noncopyable
    {
    public:
        // 局部锁
        using Lock = ScopedLock<Spinlock>;

        Spinlock() { pthread_spin_init(&m_mutex, 0); }
        ~Spinlock() { pthread_spin_destroy(&m_mutex); }

        void lock() { pthread_spin_lock(&m_mutex); }
        void unlock() { pthread_spin_unlock(&m_mutex); }

    private:
        pthread_spinlock_t m_mutex; // 自旋锁
    };
}

#endif