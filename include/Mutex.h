#pragma once

#include <pthread.h>
#include <thread>
#include <functional>
#include <semaphore.h>

#include "Noncopyable.h"

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
        Semaphore(uint32_t count = 0)
        {
            if (sem_init(&m_semaphore, 0, count))
            {
                throw std::logic_error("sem_init error");
            }
        }
        /**
         * @brief 析构函数
         */
        ~Semaphore()
        {
            sem_destroy(&m_semaphore);
        }

        /**
         * @brief 等待信号量
         */
        void wait()
        {
            if (sem_wait(&m_semaphore))
            {
                throw std::logic_error("sem_wait error");
            }
        }

        /**
         * @brief 释放信号量
         */
        void notify()
        {
            if (sem_post(&m_semaphore))
            {
                throw std::logic_error("sem_post error");
            }
        }

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
            m_mutex.lock();
        }

        /**
         * @brief 解锁
         */
        void unlock()
        {

            m_mutex.unlock();
        }

    private:
        T &m_mutex; // 互斥锁实例
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

            m_mutex.rdlock();
        }

        /**
         * @brief 解锁
         */
        void unlock()
        {

            m_mutex.unlock();
        }

    private:
        T &m_mutex; // 读写锁实例
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

            m_mutex.wrlock();
        }

        /**
         * @brief 解锁
         */
        void unlock()
        {
            m_mutex.unlock();
        }

    private:
        T &m_mutex; // 读写锁实例
    };

    /**
     * @brief 互斥锁
     */
    class Mutex : Noncopyable
    {
        friend class ConditionVariable;

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
        void lock()
        {

            pthread_mutex_lock(&m_mutex);
        }
        /**
         * @brief 解锁操作
         */
        void unlock()
        {

            pthread_mutex_unlock(&m_mutex);
        }

    private:
        pthread_mutex_t m_mutex; // 互斥锁
    };

    /**
     * @brief 读写锁
     */
    class RWMutex : Noncopyable
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

    class ConditionVariable : Noncopyable
    {
    public:
        ConditionVariable(Mutex &mutex)
            : m_mutex(mutex)
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
        void wait(const std::function<bool()> condition = []
                  { return false; })
        {
            while (!condition())
            {
                pthread_cond_wait(&cond, &m_mutex.m_mutex);
            }
        }

        void wait()
        {
            pthread_cond_wait(&cond, &m_mutex.m_mutex);
        }

        bool waitForSeconds(int seconds)
        {
            struct timespec abstime;
            clock_gettime(CLOCK_REALTIME, &abstime);
            abstime.tv_sec += static_cast<time_t>(seconds);
            // m_mutex.m_locked=false;
            return ETIMEDOUT == pthread_cond_timedwait(&cond, &m_mutex.m_mutex, &abstime);
        }

        void notify_one()
        {
            pthread_cond_signal(&cond);
        }

        void notify_all()
        {
            pthread_cond_broadcast(&cond);
        }

    private:
        pthread_cond_t cond;
        Mutex &m_mutex;
    };

}