#include "log.h"
#include "config.h"

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
    LogLevel LogLevelHandler::FromString(const std::string &val)
    {
        static const std::unordered_map<std::string, LogLevel> stringToLevel = {
            {"DEBUG", LogLevel::DEBUG},
            {"INFO", LogLevel::INFO},
            {"WARN", LogLevel::WARN},
            {"ERROR", LogLevel::ERROR},
            {"FATAL", LogLevel::FATAL}};

        auto it = stringToLevel.find(val);
        if (it != stringToLevel.end())
        {
            return it->second;
        }
        else
        {
            return LogLevel::UNKNOWN;
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
        CH, MakeShared<ITEM_NAME>()}
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

    void LogAppender::setFormatter(const std::string &pattern)
    {
        MutexType::Lock lock(m_mutex);
        m_formatter = MakeShared<LogFormatter>(pattern);
        m_custom_pattern = true;
    }

    void LogAppender::setFormatter(Shared_ptr<LogFormatter> formatter)
    {
        MutexType::Lock lock(m_mutex);
        m_formatter = formatter;
        m_custom_pattern = true;
    }

    const Shared_ptr<LogFormatter> &LogAppender::getFormatter()
    {
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }

    /**
     * @brief Logger成员函数
     */

    Logger::Logger(const std::string &name)
        : m_name(name), m_level(LogLevel::DEBUG)
    {
        m_formatter = MakeShared<LogFormatter>(LIM_DEFAULT_PATTERN);
    }

    std::string Logger::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["name"] = m_name;
        if (m_level != LogLevel::UNKNOWN)
        {
            node["level"] = LogLevelHandler::ToString(m_level);
        }
        if (m_formatter)
        {
            node["formatter"] = m_formatter->getPattern();
        }

        for (auto &i : m_appenders)
        {
            node["appenders"].push_back(YAML::Load(i->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    void Logger::log(LogLevel level, const Shared_ptr<LogEvent> &event)
    {
        if (level >= m_level)
        {
            MutexType::Lock lock(m_mutex);
            // 若该日志没有指定输出地，则默认在root的输出地中进行输出
            if (!m_appenders.empty())
            {
                for (auto &i : m_appenders)
                {
                    i->log(level, event);
                }
            }
            else
            {
                LIM_LOG_ROOT()->log(level, event);
            }
        }
    }
    /**
     * @brief 创建appender时将自身的指针传入，当不指定formatter则默认使用logger的formatter
     */
    void Logger::addAppender(Shared_ptr<LogAppender> appender)
    {
        MutexType::Lock lock(m_mutex);
        if (!appender->getFormatter())
        {
            MutexType::Lock ll(appender->m_mutex);
            appender->m_formatter = m_formatter;
        }
        m_appenders.emplace_back(appender);
    }

    void Logger::delAppender(Shared_ptr<LogAppender> appender)
    {
        MutexType::Lock lock(m_mutex);
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
        {
            if (*it == appender)
            {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::clearAppender()
    {
        MutexType::Lock lock(m_mutex);
        m_appenders.clear();
    }

    void Logger::setFormatter(const std::string &pattern)
    {
        MutexType::Lock lock(m_mutex);
        m_formatter = MakeShared<LogFormatter>(pattern);
        if (m_formatter->isError())
        {
            std::cout << "log.name=" << m_name << " formatter=" << pattern << " is invalid" << std::endl;
        }
    }

    void Logger::setFormatter(Shared_ptr<LogFormatter> formatter)
    {
        MutexType::Lock lock(m_mutex);
        m_formatter = formatter;
        if (m_formatter->isError())
        {
            std::cout << "log.name=" << m_name << " formatter=" << formatter->getPattern() << " is invalid" << std::endl;
        }
    }

    const Shared_ptr<LogFormatter> &Logger::getFormatter()
    {
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }

    const std::string &Logger::getPattern()
    {
        MutexType::Lock lock(m_mutex);
        return m_formatter->getPattern();
    }

    /**
     * @brief LogAppender 子类的成员函数实现
     */
    std::string StdoutLogAppender::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"] = "StdoutLogAppender";
        if (m_level != LogLevel::UNKNOWN)
        {
            node["level"] = LogLevelHandler::ToString(m_level);
        }
        if (m_custom_pattern)
        {
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    std::string FileLogAppender::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"] = "FileLogAppender";
        node["file"] = m_filename;
        if (m_level != LogLevel::UNKNOWN)
        {
            node["level"] = LogLevelHandler::ToString(m_level);
        }
        if (m_custom_pattern)
        {
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    FileLogAppender::FileLogAppender(const std::string &filename)
        : m_filename(filename)
    {
        reopen();
    }

    void FileLogAppender::log(LogLevel level, Shared_ptr<LogEvent> event)
    {
        if (level >= m_level)
        {
            if (!fileExist())
            {
                std::cout << "file: " << m_filename << " deleted, create new one" << std::endl;
                reopen();
            }
            MutexType::Lock lock(m_mutex);
            std::string s;
            s = m_formatter->format(event);
            m_filestream << s;
            m_filestream.flush();
        }
    }

    bool FileLogAppender::reopen()
    {
        MutexType::Lock lock(m_mutex);
        if (m_filestream)
        {
            m_filestream.close();
        }
        m_filestream.open(m_filename);
        return !!m_filestream;
    }

    bool FileLogAppender::fileExist()
    {
        std::ifstream file(m_filename);
        return file.good();
    }

    void StdoutLogAppender::log(LogLevel level, Shared_ptr<LogEvent> event)
    {
        if (level >= m_level)
        {
            MutexType::Lock lock(m_mutex);
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
                    MakeShared<StringFormatItem>(
                        m_pattern.substr(str_begin, str_end - str_begin)));
                break;

            case PARSE_STATUS::CREATE_STATUS: // 处理占位符
                assert(!format_item_map.empty() && "format_item_map 没有被正确的初始化");
                auto itor = format_item_map.find(m_pattern[i]);
                if (itor == format_item_map.end())
                {
                    m_error = true;
                    m_items.push_back(MakeShared<StringFormatItem>("<error format>"));
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
        m_logger_map[m_root->getName()] = m_root;
    }

    Shared_ptr<Logger> LoggerManager::getLogger(const std::string &name)
    {
        MutexType::Lock lock(m_mutex);
        auto it = m_logger_map.find(name);
        if (it != m_logger_map.end())
        {
            return it->second;
        }
        else
        {
            Shared_ptr<Logger> logger = MakeShared<Logger>(name);
            m_logger_map[name] = logger;
            return logger;
        }
    }

    std::string LoggerManager::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        for (auto &i : m_logger_map)
        {
            node.push_back(YAML::Load(i.second->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    struct LogAppenderDefine
    {
        int type = 0; // 1 File, 0 Stdout
        LogLevel level = LogLevel::UNKNOWN;
        std::string formatter;
        std::string file;

        bool operator==(const LogAppenderDefine &oth) const
        {
            return type == oth.type && level == oth.level && formatter == oth.formatter && file == oth.file;
        }
    };

    struct LogDefine
    {
        std::string name;
        LogLevel level = LogLevel::UNKNOWN;
        std::string formatter;
        std::vector<LogAppenderDefine> appenders;

        bool operator==(const LogDefine &oth) const
        {
            return name == oth.name && level == oth.level && formatter == oth.formatter && appenders == appenders;
        }

        bool operator<(const LogDefine &oth) const
        {
            return name < oth.name;
        }

        bool isValid() const
        {
            return !name.empty();
        }
    };

    template <>
    class LexicalCast<std::string, LogDefine>
    {
    public:
        LogDefine operator()(const std::string &v)
        {
            YAML::Node n = YAML::Load(v);
            LogDefine ld;
            if (!n["name"].IsDefined())
            {
                std::cout << "log config error: name is null, " << n << std::endl;
                throw std::logic_error("log config name is null");
            }
            ld.name = n["name"].as<std::string>();
            ld.level = LogLevelHandler::FromString(n["level"].IsDefined() ? n["level"].as<std::string>() : "");
            if (n["formatter"].IsDefined())
            {
                ld.formatter = n["formatter"].as<std::string>();
            }

            if (n["appenders"].IsDefined())
            {
                for (size_t x = 0; x < n["appenders"].size(); ++x)
                {
                    auto a = n["appenders"][x];
                    if (!a["type"].IsDefined())
                    {
                        std::cout << "log config error: appender type is null, " << a << std::endl;
                        continue;
                    }
                    std::string type = a["type"].as<std::string>();
                    LogAppenderDefine lad;
                    if (type == "1")
                    {
                        lad.type = 1;
                        if (!a["file"].IsDefined())
                        {
                            std::cout << "log config error: fileappender file is null, " << a << std::endl;
                            continue;
                        }
                        lad.file = a["file"].as<std::string>();
                        if (a["level"].IsDefined())
                        {
                            lad.level = LogLevelHandler::FromString(a["level"].as<std::string>());
                        }
                        if (a["formatter"].IsDefined())
                        {
                            lad.formatter = a["formatter"].as<std::string>();
                        }
                    }
                    else if (type == "0")
                    {
                        lad.type = 0;
                        if (a["level"].IsDefined())
                        {
                            lad.level = LogLevelHandler::FromString(a["level"].as<std::string>());
                        }
                        if (a["formatter"].IsDefined())
                        {
                            lad.formatter = a["formatter"].as<std::string>();
                        }
                    }
                    else
                    {
                        std::cout << "log config error: appender type is invalid, " << a << std::endl;
                        continue;
                    }

                    ld.appenders.push_back(lad);
                }
            }
            return ld;
        }
    };

    template <>
    class LexicalCast<LogDefine, std::string>
    {
    public:
        std::string operator()(const LogDefine &i)
        {
            YAML::Node n;
            n["name"] = i.name;
            if (i.level != LogLevel::UNKNOWN)
            {
                n["level"] = LogLevelHandler::ToString(i.level);
            }
            if (!i.formatter.empty())
            {
                n["formatter"] = i.formatter;
            }

            for (auto &a : i.appenders)
            {
                YAML::Node na;
                if (a.type == 1)
                {
                    na["type"] = "FileLogAppender";
                    na["file"] = a.file;
                }
                else if (a.type == 0)
                {
                    na["type"] = "StdoutLogAppender";
                }
                if (a.level != LogLevel::UNKNOWN)
                {
                    na["level"] = LogLevelHandler::ToString(a.level);
                }

                if (!a.formatter.empty())
                {
                    na["formatter"] = a.formatter;
                }

                n["appenders"].push_back(na);
            }
            std::stringstream ss;
            ss << n;
            return ss.str();
        }
    };

    // 读取配置
    Shared_ptr<ConfigVar<std::set<LogDefine>>> g_log_defines = Config::Lookup("logs", std::set<LogDefine>(), "logs config");

    struct LogIniter
    {
        LogIniter()
        {
            auto log_listener_func = [](const std::set<LogDefine> &old_val, const std::set<LogDefine> &new_val)
            {
                for (auto &i : new_val)
                {

                    Shared_ptr<Logger> logger;
                    auto it = old_val.find(i);
                    if (it == old_val.end())
                    {
                        // 新增
                        logger = LIM_LOG_NAME(i.name);
                    }
                    else
                    {
                        if (!(i == *it))
                        {
                            // 修改
                            logger = LIM_LOG_NAME(i.name);
                        }
                        else
                        {
                            continue;
                        }
                    }
                    logger->setLevel(i.level);
                    if (!i.formatter.empty())
                    {
                        logger->setFormatter(i.formatter);
                    }
                    logger->clearAppender();
                    for (auto &a : i.appenders)
                    {
                        Shared_ptr<LogAppender> ap;
                        if (a.type == 1)
                        {
                            ap = MakeShared<FileLogAppender>(a.file);
                        }
                        else if (a.type == 0)
                        {
                            ap = MakeShared<StdoutLogAppender>();
                        }
                        ap->setLevel(a.level);
                        if (!a.formatter.empty())
                        {
                            Shared_ptr<LogFormatter> formatter = MakeShared<LogFormatter>(a.formatter);
                            if (formatter->isError())
                            {
                                std::cout << "log.name=" << i.name << " appender type=" << a.type << " formatter=" << a.formatter << " is invalid" << std::endl;
                            }
                            else
                            {
                                ap->setFormatter(formatter);
                            }
                        }
                        logger->addAppender(ap);
                    }
                }

                for (auto &i : old_val)
                {
                    auto it = new_val.find(i);
                    if (it == new_val.end())
                    {
                        // 删除Logger: 删除其所有Appender,并设置日志级别为最高
                        auto logger = LIM_LOG_NAME(i.name);
                        logger->clearAppender();
                        logger->setLevel(LogLevel::OFF);
                    }
                }
            };

            g_log_defines->addListener(log_listener_func);
        }
    };

    static LogIniter __log__init;
}
