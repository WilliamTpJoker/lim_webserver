#pragma once

#include "Mutex.h"
#include "Noncopyable.h"
#include "LogStream.h"
#include "Thread.h"

#include <vector>

namespace lim_webserver
{
    class AppendFile : Noncopyable
    {
    public:
        explicit AppendFile(const char *filename);
        ~AppendFile();
        void append(const char *logline, const size_t len);
        void flush();

    private:
        size_t write(const char *logline, size_t len);
        FILE *m_file_ptr;
        char m_buffer[64 * 1024]; // 64k 缓存
    };

    class LogFile : Noncopyable
    {
    public:
        using MutexType = Mutex;

    public:
        LogFile(const std::string &basename, int flushInterval = 1024);
        ~LogFile();

        void append(const char *logline, int len);

        void flush();
        bool roolFile();

    private:
        void append_unlocked(const char *logline, int len);

        const std::string m_basename;
        const int m_flushInterval;
        int m_count;
        MutexType m_mutex;
        std::unique_ptr<AppendFile> m_file;
    };

    class AsyncLog : Noncopyable
    {
    public:
        using MutexType = Mutex;
        using Buffer = FixedBuffer<kLargeBuffer>;
        using BufferPtr = std::shared_ptr<Buffer>;
        using BufferVec = std::vector<BufferPtr>;

    public:
        AsyncLog(const std::string &basename, int flushInterval = 2)
            : m_basename(basename), m_flushInterval(flushInterval), m_mutex(), m_cond(m_mutex)
        {

            BufferPtr m_current_buffer(new Buffer);
            BufferPtr m_next_buffer(new Buffer);

            m_current_buffer->bzero();
            m_next_buffer->bzero();

            m_buffer_vec.reserve(16);
        }
        ~AsyncLog()
        {
            if (m_running)
            {
                stop();
            }
        }

        /**
         * @brief 异步日志写入操作，工作在前端线程，生产者-消费者模型中的生产者
         * @param[in] logline 日志行
         * @param[in] len 日志长度
         *
         */
        void append(const char *logline, int len);

        void start()
        {
            m_running = true;
            m_thread = Thread::Create([this]()
                                      { this->run(); },
                                      "log");
        }

        void stop()
        {
            m_running = false;
            m_thread->join();
        }

    private:
        /**
         * @brief 异步日志落地操作，工作在后端线程，生产者-消费者模型中的消费者
         */
        void run();

        bool m_running = false;     // 工作状态
        std::string m_basename;     //
        int m_flushInterval;        // 写入间隔
        MutexType m_mutex;          // 互斥锁
        Thread::ptr m_thread;       // 工作线程
        ConditionVariable m_cond;   // 条件变量
        BufferPtr m_current_buffer; // 当前缓存
        BufferPtr m_next_buffer;    // 备用缓存
        BufferVec m_buffer_vec;     // 满缓存存储区
    };

} // namespace lim_webserver
