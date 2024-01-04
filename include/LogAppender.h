#pragma once

#include <memory>
#include <stdio.h>
#include <string>
#include <unordered_map>

#include "Mutex.h"
#include "Thread.h"
#include "LogLevel.h"
#include "LogMessage.h"
#include "LogFormatter.h"
#include "RollingPolicy.h"
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

    /**
     * @brief 日志输出地
     */
    class LogAppender
    {
    public:
        using ptr = std::shared_ptr<LogAppender>;

    public:
        virtual ~LogAppender(){};

        /**
         * @brief 落地日志接口，将message解包为字符串并进行落地
         */
        void doAppend(LogMessage::ptr message);

        virtual int getType() = 0;

        void setName(const std::string &name) { m_name = name; }
        const std::string &getName() { return m_name; }

        void setLevel(LogLevel level) { m_level = level; }
        LogLevel getLevel() const { return m_level; }

        virtual void start() { m_started = true; }
        virtual void stop() { m_started = false; }
        bool isStarted() { return m_started; }

    protected:
        /**
         * @brief 将message解包为字符串
         */
        virtual void format(LogStream &logstream, LogMessage::ptr message) = 0;
        /**
         * @brief 将字符串进行落地
         */
        virtual void append(const char *logline, int len) = 0;

        std::string m_name; // 名字
        LogLevel m_level;   // 级别

        bool m_started; // 启动标志位
    };

    class AsyncAppender;

    class OutputAppender : public LogAppender
    {
        friend AsyncAppender;

    public:
        using ptr = std::shared_ptr<OutputAppender>;

    public:
        OutputAppender(){};
        OutputAppender(const LogAppenderDefine &lad);
        virtual ~OutputAppender(){};

        void flush();

        /**
         * @brief 设置格式器
         */
        void setFormatter(const std::string &pattern);
        void setFormatter(LogFormatter::ptr formatter);

        const LogFormatter::ptr &getFormatter();

        void start() override;
        void stop() override;

    protected:
        void format(LogStream &logstream, LogMessage::ptr message) override;
        void append(const char *logline, int len) override;

        LogFormatter::ptr m_formatter; // 格式器
        LogSink::ptr m_sink;           // 落地器
        LogStream::Buffer m_buffer;    // 4k缓存
    };

    /**
     * @brief 输出到控制台Appender
     */
    class ConsoleAppender : public OutputAppender
    {
        friend AsyncAppender;

    public:
        using ptr = std::shared_ptr<ConsoleAppender>;

    public:
        ConsoleAppender();
        ConsoleAppender(const LogAppenderDefine &lad);

        int getType() override;
    };

    /**
     * @brief 输出到文件的Appender
     */
    class FileAppender : public OutputAppender
    {
        friend AsyncAppender;

    public:
        using ptr = std::shared_ptr<FileAppender>;

    public:
        FileAppender(){};
        FileAppender(const std::string &filename, bool append = true);
        FileAppender(const LogAppenderDefine &lad);

        void openFile();
        void setFile(const std::string &filename);
        const std::string &rawFileProperty();

        void setAppend(bool append);
        bool isAppend();

        int getType() override;

        void start() override;

    protected:
        std::string m_filename; // 文件名
        bool m_append = true;   // 追加模式
    };

    class RollingFileAppender : public FileAppender
    {
        friend AsyncAppender;

    public:
        using ptr = std::shared_ptr<RollingFileAppender>;
        using MutexType = Spinlock;

    public:
        RollingFileAppender(){};
        RollingFileAppender(const std::string &filename, RollingPolicy::ptr rollingPolicy, TriggeringPolicy::ptr triggeringPolicy);

        void setRollingPolicy(RollingPolicy::ptr rollingPolicy);

        void setTriggeringPolicy(TriggeringPolicy::ptr triggeringPolicy);

        void start() override;

    private:
        void format(LogStream &logstream, LogMessage::ptr message) override;

        void rollover();

        RollingPolicy::ptr m_rollingPolicy;       // 滚动策略
        TriggeringPolicy::ptr m_triggeringPolicy; // 触发策略
        MutexType m_trigger_mutex;                // 触发策略锁
    };

    /**
     * @brief 异步Appender
     */
    class AsyncAppender final: public LogAppender
    {
    public:
        using ptr = std::shared_ptr<AsyncAppender>;

    public:
        AsyncAppender();
        AsyncAppender(OutputAppender::ptr appender);

        void setInterval(int interval);

        void bindAppender(OutputAppender::ptr appender);

        int getType() override;

        void start() override;
        void stop() override;

    private:
        void format(LogStream &logstream, LogMessage::ptr message) override;
        void append(const char *logline, int len) override;

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

            bool empty()
            {
                return buffer_vec.empty();
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
        Mutex m_append_mutex;           // 后台交换缓存锁
        ConditionVariable m_cond;       // 条件变量
    };

    class AppenderFactory
    {
    public:
        static ConsoleAppender::ptr newConsoleAppender()
        {
            return std::make_shared<ConsoleAppender>();
        }

        static FileAppender::ptr newFileAppender()
        {
            return std::make_shared<FileAppender>();
        }

        static RollingFileAppender::ptr newRollingFileAppender()
        {
            return std::make_shared<RollingFileAppender>();
        }

        static AsyncAppender::ptr newAsyncAppender()
        {
            return std::make_shared<AsyncAppender>();
        }
    public:
        
    private:
        std::unordered_map<std::string, LogAppender::ptr> m_appenders; // 系统全部输出地
    };
} // namespace lim_webserver
