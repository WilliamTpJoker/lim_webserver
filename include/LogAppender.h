#pragma once

#include <memory>

#include "LogLevel.h"
#include "LogMessage.h"
#include "LogFormatter.h"

namespace lim_webserver
{
    class Logger;
    /**
     * @brief 日志输出地
     */
    class LogAppender
    {
        friend class Logger;

    public:
        using ptr = std::shared_ptr<LogAppender>;
        using MutexType = Spinlock;

    public:
        virtual ~LogAppender(){};

        /**
         * @brief 输出日志，必须重构
         */
        virtual void log(LogLevel level, LogMessage::ptr message) = 0;

        /**
         * @brief 输出日志，必须重构
        */
        virtual void log(LogLevel level, LogStream &stream)=0;

        /**
         * @brief 输出为Yaml字符串格式，必须重构
         */
        virtual std::string toYamlString() = 0;

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
         * @brief 设置输出地级别
         */
        void setLevel(LogLevel level) { m_level = level; }

        /**
         * @brief 获得输出地级别
         */
        LogLevel getLevel() const { return m_level; }

    protected:
        LogLevel m_level; // 级别
        MutexType m_mutex;
        LogFormatter::ptr m_formatter; // 格式器
    };

    class DecoratorLogAppender : public LogAppender
    {
    protected:
        LogAppender::ptr m_appender;
        DecoratorLogAppender(LogAppender::ptr appender)
            : m_appender(appender)
        {
        }
    };

    /**
     * @brief 输出到控制台Appender
     */
    class StdoutLogAppender : public LogAppender
    {
    public:
        using ptr = std::shared_ptr<StdoutLogAppender>;
        static ptr Create()
        {
            return std::make_shared<StdoutLogAppender>();
        }

    public:
        /**
         * @brief 输出日志到控制台中
         */
        void log(LogLevel level, LogMessage::ptr message) override;

        /**
         * @brief 输出日志到控制台中
        */
        void log(LogLevel level, LogStream &stream) override;

        /**
         * @brief 打印成Yaml格式字符串
         */
        std::string toYamlString();
    };
    /**
     * @brief 输出到文件的Appender
     */
    class FileLogAppender : public LogAppender
    {
    public:
        using ptr = std::shared_ptr<FileLogAppender>;
        static ptr Create(const std::string &filename)
        {
            return std::make_shared<FileLogAppender>(filename);
        }

    public:
        FileLogAppender(const std::string &filename);

        /**
         * @brief 输出日志到文件中
         */
        void log(LogLevel level, LogMessage::ptr message) override;

        /**
         * @brief 输出日志到文件中
         */
        void log(LogLevel level, LogStream &stream) override;

        /**
         * @brief 重新打开文件，打开成功返回true
         */
        bool reopen();
        /**
         * @brief 检测文件是否存在
         */
        bool fileExist();

        /**
         * @brief 打印成Yaml格式字符串
         */
        std::string toYamlString();

    private:
        std::string m_filename; // 文件名
        FILE *m_ptr;            // 文件流
    };
} // namespace lim_webserver
