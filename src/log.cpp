#include "log.h"

namespace lim_webserver
{

    std::string LogLevelHandler::ToString(LogLevel level)
    {
        static const std::unordered_map<LogLevel, std::string> levelStrings = {
            {LogLevel::DEBUG, "DEBUG"},
            {LogLevel::INFO, "INFO"},
            {LogLevel::WARN, "WARN"},
            {LogLevel::ERROR, "ERROR"},
            {LogLevel::FATAL, "FATAL"}};
        auto it = levelStrings.find(level);
        if (it != levelStrings.end())
        {
            return it->second;
        }
        else
        {
            return std::string("UNKNOWN");
        }
    }
    LogEventWrap::~LogEventWrap()
    {
        m_event->getLogger()->log(m_event->getLevel(), m_event);
    }
    class MessageFormatItem : public LogFormatter::FormatItem
    {
    public:
        MessageFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Shared_ptr<LogEvent> event) override
        {
            os << event->getContent();
        }
    };

    class LevelFormatItem : public LogFormatter::FormatItem
    {
    public:
        LevelFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Shared_ptr<LogEvent> event) override
        {
            os << LogLevelHandler::ToString(event->getLevel());
        }
    };

    class ElapseFormatItem : public LogFormatter::FormatItem
    {
    public:
        ElapseFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Shared_ptr<LogEvent> event) override
        {
            os << event->getElapse();
        }
    };

    class NameFormatItem : public LogFormatter::FormatItem
    {
    public:
        NameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Shared_ptr<LogEvent> event) override
        {
            os << event->getLogger()->getName();
        }
    };

    class ThreadIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        ThreadIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Shared_ptr<LogEvent> event) override
        {
            os << event->getThreadId();
        }
    };

    class FiberIdFormatItem : public LogFormatter::FormatItem
    {
    public:
        FiberIdFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Shared_ptr<LogEvent> event) override
        {
            os << event->getFiberId();
        }
    };

    class DateTimeFormatItem : public LogFormatter::FormatItem
    {
    public:
        DateTimeFormatItem(const std::string &format = "%Y-%m-%d %H:%M:%S")
            : m_format(format) {}
        void format(std::ostream &os, Shared_ptr<LogEvent> event) override
        {
            struct tm time_struct;              // 定义存储时间的结构体
            time_t time_l = event->getTime();   // 获取时间
            localtime_r(&time_l, &time_struct); // 将时间数转换成当地时间格式
            char buf[64]{0};
            strftime(buf, sizeof(buf), m_format.c_str(), &time_struct); // 将时间输出成文本
            os << buf;
        }

    private:
        std::string m_format;
    };

    class FileNameFormatItem : public LogFormatter::FormatItem
    {
    public:
        FileNameFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Shared_ptr<LogEvent> event) override
        {
            os << event->getFile();
        }
    };

    class LineFormatItem : public LogFormatter::FormatItem
    {
    public:
        LineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Shared_ptr<LogEvent> event) override
        {
            os << event->getLine();
        }
    };

    class NewLineFormatItem : public LogFormatter::FormatItem
    {
    public:
        NewLineFormatItem(const std::string &str = "") {}
        void format(std::ostream &os, Shared_ptr<LogEvent> event) override
        {
            os << std::endl;
        }
    };

    class StringFormatItem : public LogFormatter::FormatItem
    {
    public:
        StringFormatItem(const std::string &str)
            : m_string(str) {}
        void format(std::ostream &os, Shared_ptr<LogEvent> event) override
        {
            os << m_string;
        }

    private:
        std ::string m_string;
    };

    class PercentSignFormatItem : public LogFormatter::FormatItem
    {
    public:
        void format(std::ostream &os, Shared_ptr<LogEvent> event) override
        {
            os << '%';
        }
    };

    class TabFormatItem : public LogFormatter::FormatItem
    {
    public:
        void format(std::ostream &os, Shared_ptr<LogEvent> event) override
        {
            os << '\t';
        }
    };

    /**
     * %p 输出日志等级
     * %f 输出文件名
     * %l 输出行号
     * %d 输出日志时间
     * %t 输出线程号
     * %F 输出协程号
     * %m 输出日志消息
     * %n 输出换行
     * %% 输出百分号
     * %T 输出制表符
     * %r 输出自启动到现在的时间
     * %c 输出日志信息所属的类目
     * */
    thread_local static const std::unordered_map<char, Shared_ptr<LogFormatter::FormatItem>> format_item_map{
#define FN(CH, ITEM_NAME) \
    {                     \
        CH, std::make_shared<ITEM_NAME>()}
        FN('p', LevelFormatItem),
        FN('f', FileNameFormatItem),
        FN('l', LineFormatItem),
        FN('d', DateTimeFormatItem),
        FN('t', ThreadIdFormatItem),
        FN('F', FiberIdFormatItem),
        FN('m', MessageFormatItem),
        FN('n', NewLineFormatItem),
        FN('%', PercentSignFormatItem),
        FN('T', TabFormatItem),
        FN('r', ElapseFormatItem),
        FN('c', NameFormatItem)
#undef FN
    };

    /**
     * @brief Logger成员函数
     */

    Logger::Logger()
        : m_name("root"), m_level(LogLevel::DEBUG)
    {
        setFormatter(LIM_DEFAULT_PATTERN);
    }

    Logger::Logger(const std::string &name, LogLevel level, const std::string &pattern)
        : m_name(name), m_level(level)
    {
        setFormatter(pattern);
    }

    void Logger::log(LogLevel level, const Shared_ptr<LogEvent> &event)
    {
        if (level >= m_level)
        {
            for (auto &i : m_appenders)
            {
                i->log(level, event);
            }
        }
    }

    void Logger::debug(Shared_ptr<LogEvent> event)
    {
        log(LogLevel::DEBUG, event);
    }
    void Logger::info(Shared_ptr<LogEvent> event)
    {
        log(LogLevel::INFO, event);
    }
    void Logger::warn(Shared_ptr<LogEvent> event)
    {
        log(LogLevel::WARN, event);
    }
    void Logger::error(Shared_ptr<LogEvent> event)
    {
        log(LogLevel::ERROR, event);
    }
    void Logger::fatal(Shared_ptr<LogEvent> event)
    {
        log(LogLevel::FATAL, event);
    }

    void Logger::addAppender(Shared_ptr<LogAppender> appender)
    {
        if (!appender->getFormatter())
        {
            appender->setFormatter(m_formatter);
        }
        m_appenders.emplace_back(appender);
    }
    void Logger::delAppender(Shared_ptr<LogAppender> appender)
    {
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
        {
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }

    /**
     * @brief LogAppender 子类的成员函数实现
     */
    FileLogAppender::FileLogAppender(const std::string &filename)
        : m_filename(filename)
    {
        reopen();
    }

    void FileLogAppender::log(LogLevel level, Shared_ptr<LogEvent> event)
    {
        if (level >= m_Level)
        {
            m_filestream << m_formatter->format(event);
        }
    }

    bool FileLogAppender::reopen()
    {
        if (m_filestream)
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename);
        return !!m_filestream;
    }

    void StdoutLogAppender::log(LogLevel level, Shared_ptr<LogEvent> event)
    {
        if (level >= m_Level)
        {
            std::cout << m_formatter->format(event);
        }
    }

    /**
     * @brief LogFormatter实现
     */
    LogFormatter::LogFormatter(const std::string &pattern)
        : m_pattern(pattern)
    {
        init();
    }

    std::string LogFormatter::format(Shared_ptr<LogEvent> event)
    {
        std::stringstream ss;
        for (auto &i : m_items)
        {
            i->format(ss, event);
        }

        return ss.str();
    }

    void LogFormatter::init()
    {
        enum class PARSE_STATUS
        {
            SCAN_STATUS,   // 扫描普通字符
            CREATE_STATUS, // 扫描到 %，处理占位符
        };
        PARSE_STATUS STATUS = PARSE_STATUS::SCAN_STATUS;
        size_t str_begin = 0, str_end = 0;
        for (size_t i = 0; i < m_pattern.length(); i++)
        {
            switch (STATUS)
            {
            case PARSE_STATUS::SCAN_STATUS: // 普通扫描分支，将扫描到普通字符串创建对应的普通字符处理对象后填入 m_item 中
                // 扫描记录普通字符的开始结束位置
                str_begin = i;
                for (str_end = i; str_end < m_pattern.length(); str_end++)
                {
                    // 扫描到 % 结束普通字符串查找，将 STATUS 赋值为占位符处理状态，等待后续处理后进入占位符处理状态
                    if (m_pattern[str_end] == '%')
                    {
                        STATUS = PARSE_STATUS::CREATE_STATUS;
                        break;
                    }
                }
                i = str_end;
                m_items.push_back(
                    std::make_shared<StringFormatItem>(
                        m_pattern.substr(str_begin, str_end - str_begin)));
                break;

            case PARSE_STATUS::CREATE_STATUS: // 处理占位符
                assert(!format_item_map.empty() && "format_item_map 没有被正确的初始化");
                auto itor = format_item_map.find(m_pattern[i]);
                if (itor == format_item_map.end())
                {
                    m_items.push_back(std::make_shared<StringFormatItem>("<error format>"));
                }
                else
                {
                    m_items.push_back(itor->second);
                }
                STATUS = PARSE_STATUS::SCAN_STATUS;
                break;
            }
        }
    }

    LoggerManager::LoggerManager()
    {
        m_root = MakeShared<Logger>();
        m_root->addAppender(MakeShared<StdoutLogAppender>());
    }

    Shared_ptr<Logger> LoggerManager::getLogger(const std::string &name)
    {
        auto it = m_logger_map.find(name);
        return it == m_logger_map.end() ? m_root : it->second;
    }
}
