#pragma once

#include <memory>
#include <stdio.h>

#include "Mutex.h"
#include "Thread.h"
#include "LogLevel.h"
#include "LogMessage.h"
#include "LogFormatter.h"
#include "Policy.h"
#include "LogSink.h"

namespace lim_webserver
{
    enum class AppenderType
    {
        Console,
        File,
        RollingFile,
        Async
    };

    struct LogAppenderDefine
    {
        int type = 0; // 1 File, 0 Stdout
        std::string name;
        std::string file;
        std::string formatter = DEFAULT_PATTERN;
        LogLevel level = LogLevel_UNKNOWN;
        bool append = true;

        bool operator<(const LogAppenderDefine &oth) const
        {
            return name < oth.name;
        }

        bool operator==(const LogAppenderDefine &oth) const
        {
            return name == oth.name && type == oth.type && file == oth.file && formatter == oth.formatter && level == oth.level && append == oth.append;
        }

        bool isValid() const
        {
            return !name.empty();
        }
    };

    class LogVisitor;
    class YamlVisitor;
    /**
     * @brief 日志输出地
     */
    class LogAppender
    {
    public:
        using ptr = std::shared_ptr<LogAppender>;
        using MutexType = Spinlock;

    public:
        virtual ~LogAppender(){};

        virtual const char *accept(LogVisitor &visitor) = 0;

        void setName(const std::string &name) { m_name = name; }

        const std::string &getName() { return m_name; }
        /**
         * @brief 输出日志，必须重构
         */
        void doAppend(LogMessage::ptr message);

        /**
         * @brief 输出日志，必须重构
         */
        virtual void format(LogStream &logstream, LogMessage::ptr message) = 0;

        virtual void append(const char *logline, int len) = 0;

        virtual int getType() = 0;

        /**
         * @brief 设置输出地级别
         */
        void setLevel(LogLevel level) { m_level = level; }

        /**
         * @brief 获得输出地级别
         */
        LogLevel getLevel() const { return m_level; }

        virtual void start() { m_started = true; }

        virtual void stop() { m_started = false; }

        bool isStarted() { return m_started; }

    protected:
        std::string m_name; // 名字
        LogLevel m_level;   // 级别
        MutexType m_mutex;  // 锁

        bool m_started; // 启动标志位
    };

    class OutputAppender : public LogAppender
    {
    public:
        using ptr = std::shared_ptr<OutputAppender>;

    public:
        OutputAppender(){};
        OutputAppender(const LogAppenderDefine &lad);

        virtual ~OutputAppender(){};
        /**
         * @brief 输出日志，必须重构
         */
        void format(LogStream &logstream, LogMessage::ptr message) override;

        void append(const char *logline, int len) override;

        void flush();

        /**
         * @brief 设置格式器
         */
        void setFormatter(const std::string &pattern);
        void setFormatter(LogFormatter::ptr formatter);
        /**
         * @brief 获得格式器
         */
        const LogFormatter::ptr &getFormatter();

        void setSink(LogSink::ptr sink);

        /**
         * @brief 删除格式器
         */
        void delFormatter();

        void start() override;

        void stop() override;

    protected:
        LogFormatter::ptr m_formatter; // 格式器
        LogSink::ptr m_sink;           // 落地器
        LogStream::Buffer m_buffer;    // 4k缓存
    };

    /**
     * @brief 输出到控制台Appender
     */
    class ConsoleAppender : public OutputAppender
    {
        friend YamlVisitor;

    public:
        using ptr = std::shared_ptr<ConsoleAppender>;

    public:
        ConsoleAppender();
        ConsoleAppender(const LogAppenderDefine &lad);

        int getType() override;

        const char *accept(LogVisitor &visitor) override;
    };

    /**
     * @brief 输出到文件的Appender
     */
    class FileAppender : public OutputAppender
    {
        friend YamlVisitor;

    public:
        using ptr = std::shared_ptr<FileAppender>;

    public:
        FileAppender(const std::string &filename, bool append = true);
        FileAppender(const LogAppenderDefine &lad);

        void openFile();

        void setFile(const std::string &filename);

        void setAppend(bool append);

        bool isAppend();

        int getType() override;

        const char *accept(LogVisitor &visitor) override;

        void start() override;

    protected:
        std::string m_filename; // 文件名
        bool m_append = true;   // 追加模式
    };

    class RollingFileAppender : public FileAppender
    {
    public:
        using ptr = std::shared_ptr<RollingFileAppender>;

    public:
        RollingFileAppender(const std::string &filename, RollingPolicy::ptr rollingPolicy);

    private:
        RollingPolicy::ptr m_rollingPolicy;
    };

    /**
     * @brief 异步Appender
     */
    class AsyncAppender : public LogAppender
    {
    public:
        using ptr = std::shared_ptr<AsyncAppender>;
        using MutexType = Mutex;

    public:
        AsyncAppender();
        AsyncAppender(OutputAppender::ptr appender);

        void setInterval(int interval);

        void format(LogStream &logstream, LogMessage::ptr message) override;

        void append(const char *logline, int len) override;

        int getType() override;

        const char *accept(LogVisitor &visitor) override { return ""; }

        void start() override;

        void stop() override;

        void bindAppender(OutputAppender::ptr appender);

    private:
        using Buffer = FixedBuffer<kLargeBuffer>;
        using BufferPtr = std::shared_ptr<Buffer>;
        using BufferVec = std::vector<BufferPtr>;

        /**
         * @brief 异步日志落地操作，工作在后端线程，生产者-消费者模型中的消费者
         */
        void run();

        void submitBuffer();

        struct DoubleBuffer
        {
            BufferPtr buffer1;    // 当前缓存
            BufferPtr buffer2;    // 备用缓存
            BufferVec buffer_vec; // 满缓存存储区

            DoubleBuffer()
                : buffer1(new Buffer), buffer2(new Buffer), buffer_vec()
            {
                buffer1->bzero();
                buffer2->bzero();
                buffer_vec.reserve(16);
            }

            void reset()
            {
                if (buffer_vec.size() > 2)
                    buffer_vec.resize(2);
                if (!buffer1)
                {
                    buffer1 = buffer_vec.back();
                    buffer_vec.pop_back();
                    buffer1->reset();
                }
                if (!buffer2)
                {
                    buffer2 = buffer_vec.back();
                    buffer_vec.pop_back();
                    buffer2->reset();
                }
                buffer_vec.clear();
            }
        };

        DoubleBuffer m_buffer;          // 双缓冲
        OutputAppender::ptr m_appender; // 工作输出地
        int m_flushInterval;            // 写入间隔
        Thread::ptr m_thread;           // 工作线程
        MutexType m_mutex;              // 互斥锁
        ConditionVariable m_cond;       // 条件变量
    };

    class AppenderFactory
    {
    public:
        using ptr = std::shared_ptr<AppenderFactory>;

    public:
        virtual ~AppenderFactory(){};
        virtual LogAppender::ptr create(const LogAppenderDefine &lad) = 0;
    };
} // namespace lim_webserver
