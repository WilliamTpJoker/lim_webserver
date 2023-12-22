#pragma once

#include <memory>
#include <vector>

#include "LogStream.h"
#include "LogMessage.h"
#include "Thread.h"
#include "Fiber.h"

#define LIM_DEFAULT_PATTERN "%d%T%t %N%T%F%T[%c] [%p] %f:%l%T%m%n"

namespace lim_webserver
{
    /**
     * @brief 日志格式器
     */
    class LogFormatter
    {
    public:
        using ptr = std::shared_ptr<LogFormatter>;
        static ptr Create(const std::string &pattern)
        {
            return std::make_shared<LogFormatter>(pattern);
        }

    public:
        LogFormatter(const std::string &pattern);

        /**
         * @brief 构造字符流
         */
        void format(LogStream &stream, LogMessage::ptr event);
        /**
         * @brief 获得格式
         */
        const std::string &getPattern() const { return m_pattern; }
        /**
         * @brief 判断格式是否异常
         * @return true：异常 false: 正常
         */
        bool isError() { return m_error; }

        /**
         * @brief 格式器初始化
         */
        void init();

    public:
        /**
         * @brief 格式体虚父类
         */
        class FormatItem
        {
        public:
            virtual ~FormatItem() {}

            /**
             * @brief 构造字符流
             */
            virtual void format(LogStream &stream, LogMessage::ptr event) = 0;
        };

    private:
        std::string m_pattern;                            // 格式
        std::vector<std::shared_ptr<FormatItem>> m_items; // 格式体容器
        bool m_error = false;                             // 异常标志符
    };

    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << event->getStream();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << event->getLevelString();
        }
    };

    // TODO:未实现
    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        ElapseFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << 0;
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem
    {
    public:
        NameFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {

            stream << event->getName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << Thread::GetThreadId();
        }
    };

    class ThreadNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadNameFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << Thread::GetThreadName();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberIdFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << Fiber::GetFiberId();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem
    {
    public:
        DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
            : m_format(format) {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            struct tm time_struct;              // 定义存储时间的结构体
            time_t time_l = event->getTime();   // 获取时间
            localtime_r(&time_l, &time_struct); // 将时间数转换成当地时间格式
            char buf[64]{0};
            strftime(buf, sizeof(buf), m_format.c_str(), &time_struct); // 将时间输出成文本
            stream << buf;
        }

    private:
        std::string m_format;
    };

    class FileNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FileNameFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &str = "") {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << '\n';
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str)
            : m_string(str) {}
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << m_string;
        }

    private:
        std ::string m_string;
    };

    class PercentSignFormatItem : public LogFormatter::FormatItem
    {
    public:
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << '%';
        }
    };

    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        void format(LogStream &stream, LogMessage::ptr event) override
        {
            stream << '\t';
        }
    };

} // namespace lim_webserver
