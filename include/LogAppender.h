#pragma once

#include <memory>

#include "LogLevel.h"
#include "LogMessage.h"
#include "LogFormatter.h"
#include "Policy.h"

namespace lim_webserver
{

    struct LogAppenderDefine
    {
        int type = 0; // 1 File, 0 Stdout
        std::string name;
        LogLevel level = LogLevel_UNKNOWN;
        std::string formatter;
        std::string file;
        bool append;

        bool operator==(const LogAppenderDefine &oth) const
        {
            return type == oth.type && level == oth.level && formatter == oth.formatter && file == oth.file && append == oth.append && name == oth.name;
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

        void setName(const std::string &name) { m_name = name; }

        virtual const char* accept(LogVisitor& visitor)=0;

        const std::string &getName() { return m_name; }
        /**
         * @brief 输出日志，必须重构
         */
        virtual void log(LogLevel level, LogMessage::ptr message) = 0;

        /**
         * @brief 输出日志，必须重构
         */
        virtual void log(LogLevel level, LogStream &stream) = 0;

        /**
         * @brief 设置格式器
         */
        void setFormatter(const std::string &pattern);
        void setFormatter(LogFormatter::ptr formatter);
        /**
         * @brief 获得格式器
         */
        const LogFormatter::ptr &getFormatter();

        /**
         * @brief 删除格式器
         */
        void delFormatter();

        /**
         * @brief 设置输出地级别
         */
        void setLevel(LogLevel level) { m_level = level; }

        /**
         * @brief 获得输出地级别
         */
        LogLevel getLevel() const { return m_level; }

    protected:
        std::string m_name;                      // 名字
        LogLevel m_level;                        // 级别
        MutexType m_mutex;                       // 锁
        LogFormatter::ptr m_formatter;           // 格式器
    };

    class OutputAppender : public LogAppender
    {
    public:
        using ptr = std::shared_ptr<OutputAppender>;

    public:
        /**
         * @brief 输出日志，必须重构
         */
        void log(LogLevel level, LogMessage::ptr message) override;

        /**
         * @brief 输出日志，必须重构
         */
        void log(LogLevel level, LogStream &stream) override;

    protected:
        FILE *m_ptr = nullptr; // 文件流
    };

    /**
     * @brief 输出到控制台Appender
     */
    class ConsoleAppender : public OutputAppender
    {
        friend YamlVisitor;
    public:
        using ptr = std::shared_ptr<ConsoleAppender>;
        static ptr Create(FILE *target = stdout)
        {
            return std::make_shared<ConsoleAppender>(target);
        }

    public:
        ConsoleAppender(FILE *target = stdout);
        void setTarget(FILE *target);

        const char* accept(LogVisitor& visitor) override;
    };

    /**
     * @brief 输出到文件的Appender
     */
    class FileAppender : public OutputAppender
    {
        friend YamlVisitor;
    public:
        using ptr = std::shared_ptr<FileAppender>;
        static ptr Create(const std::string &filename, bool append = true)
        {
            return std::make_shared<FileAppender>(filename, append);
        }

    public:
        FileAppender(const std::string &filename, bool append = true);

        /**
         * @brief 重新打开文件，打开成功返回true
         */
        bool reopen();

        const char* accept(LogVisitor& visitor) override;

    protected:
        std::string m_filename; // 文件名
        bool m_append;          // 追加模式
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
} // namespace lim_webserver
