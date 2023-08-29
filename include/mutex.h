#ifndef __LIM_MUTEX_H__
#define __LIM_MUTEX_H__

#include <pthread.h>
#include <thread>
#include <functional>
#include <semaphore.h>

#include "noncopyable.h"

namespace lim_webserver
{
    /**
     * @brief 信号量类
     */
    class Semaphore : Noncopyable
    {
    public:
        /**
         * @brief 构造函数
         *
         * @param count 信号量初始计数
         */
        Semaphore(uint32_t count = 0);
        /**
         * @brief 析构函数
         */
        ~Semaphore();

        /**
         * @brief 等待信号量
         */
        void wait();

        /**
         * @brief 释放信号量
         */
        void notify();

    private:
        sem_t m_semaphore; // 信号量实例
    };

    /**
     * @brief 局部互斥锁模板
     *
     * @tparam T 互斥锁类型
     */
    template <class T>
    class ScopedLock
    {
    public:
        /**
         * @brief 构造函数，加锁
         *
         * @param mutex 互斥锁实例
         */
        ScopedLock(T &mutex) : m_mutex(mutex) { lock(); }

        /**
         * @brief 析构函数，解锁
         */
        ~ScopedLock() { unlock(); }

        /**
         * @brief 加锁
         */
        void lock()
        {
            if (!m_locked)
            {
                m_mutex.lock();
                m_locked = true;
            }
        }

        /**
         * @brief 解锁
         */
        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;    // 互斥锁实例
        bool m_locked; // 锁标志符
    };

    /**
     * @brief 读锁模板，用于加读锁
     *
     * @tparam T 互斥锁类型
     */
    template <class T>
    class ReadScopedLock
    {
    public:
        /**
         * @brief 构造函数，加读锁
         *
         * @param mutex 读写锁实例
         */
        ReadScopedLock(T &mutex) : m_mutex(mutex) { lock(); }
        /**
         * @brief 析构函数，解锁
         */
        ~ReadScopedLock() { unlock(); }

        /**
         * @brief 加读锁
         */
        void lock()
        {
            if (!m_locked)
            {
                m_mutex.rdlock();
                m_locked = true;
            }
        }

        /**
         * @brief 解锁
         */
        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;    // 读写锁实例
        bool m_locked; // 锁标志符
    };

    /**
     * @brief 写锁模板，用于加写锁
     *
     * @tparam T 互斥锁类型
     */
    template <class T>
    class WriteScopedLock
    {
    public:
        /**
         * @brief 构造函数，加写锁
         *
         * @param mutex 读写锁实例
         */
        WriteScopedLock(T &mutex) : m_mutex(mutex) { lock(); }
        /**
         * @brief 析构函数，解锁
         */
        ~WriteScopedLock() { unlock(); }

        /**
         * @brief 加写锁
         */
        void lock()
        {
            if (!m_locked)
            {
                m_mutex.wrlock();
                m_locked = true;
            }
        }

        /**
         * @brief 解锁
         */
        void unlock()
        {
            if (m_locked)
            {
                m_mutex.unlock();
                m_locked = false;
            }
        }

    private:
        T &m_mutex;    // 读写锁实例
        bool m_locked; // 锁标志符
    };

    /**
     * @brief 互斥锁
     */
    class Mutex
    {
    public:
        using Lock = ScopedLock<Mutex>;

        /**
         * @brief 构造函数，初始化互斥锁
         */
        Mutex() { pthread_mutex_init(&m_mutex, nullptr); }
        /**
         * @brief 析构函数，销毁互斥锁
         */
        ~Mutex() { pthread_mutex_destroy(&m_mutex); }

        /**
         * @brief 加锁操作
         */
        void lock() { pthread_mutex_lock(&m_mutex); }
        /**
         * @brief 解锁操作
         */
        void unlock() { pthread_mutex_unlock(&m_mutex); }

        pthread_mutex_t* getMutex(){return &m_mutex;}

    private:
        pthread_mutex_t m_mutex; // 互斥锁
    };

    /**
     * @brief 读写锁
     */
    class RWMutex
    {
    public:
        using ReadLock = ReadScopedLock<RWMutex>;
        using WriteLock = WriteScopedLock<RWMutex>;

        /**
         * @brief 构造函数，初始化读写锁
         */
        RWMutex() { pthread_rwlock_init(&m_mutex, nullptr); }
        /**
         * @brief 析构函数，销毁读写锁
         */
        ~RWMutex() { pthread_rwlock_destroy(&m_mutex); }

        /**
         * @brief 获取读锁
         */
        void rdlock() { pthread_rwlock_rdlock(&m_mutex); }
        /**
         * @brief 获取写锁
         */
        void wrlock() { pthread_rwlock_wrlock(&m_mutex); }
        /**
         * @brief 解锁操作
         */
        void unlock() { pthread_rwlock_unlock(&m_mutex); }

    private:
        pthread_rwlock_t m_mutex; // 读写锁
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
        using Lock = ScopedLock<Spinlock>;

        /**
         * @brief 构造函数，初始化自旋锁
         */
        Spinlock() { pthread_spin_init(&m_mutex, 0); }
        /**
         * @brief 析构函数，销毁自旋锁
         */
        ~Spinlock() { pthread_spin_destroy(&m_mutex); }

        /**
         * @brief 加锁操作
         */
        void lock() { pthread_spin_lock(&m_mutex); }
        /**
         * @brief 解锁操作
         */
        void unlock() { pthread_spin_unlock(&m_mutex); }

    private:
        pthread_spinlock_t m_mutex; // 自旋锁
    };

    template <class T=Mutex>
    class ConditionVariable
    {
    public:
        ConditionVariable()
        {
            pthread_cond_init(&cond, nullptr);
        }

        ~ConditionVariable()
        {
            pthread_cond_destroy(&cond);
        }
        /**
         * @brief 当条件不成立时，则会将该线程置于等待状态
        */
        void wait(T &mutex, std::function<bool()> condition=[]{return false;})
        {
            ++waitersCount;
            while (!condition())
            {
                pthread_cond_wait(&cond, mutex.getMutex());
            }
            --waitersCount;
        }

        void notify_one()
        {
            if (waitersCount > 0)
            {
                pthread_cond_signal(&cond);
            }
        }

        void notify_all()
        {
            if (waitersCount > 0)
            {
                pthread_cond_broadcast(&cond);
            }
        }

    private:
        pthread_cond_t cond;
        int waitersCount = 0;
    };

}

#endif