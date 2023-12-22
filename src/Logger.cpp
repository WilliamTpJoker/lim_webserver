#include "Logger.h"
#include "Config.h"
#include "Fiber.h"
#include "AsyncLog.h"

namespace lim_webserver
{
    

    /**
     * @brief Logger成员函数
     */

    Logger::Logger(const std::string &name)
        : m_name(name) {}

    Logger::Logger(const std::string &name, LogLevel level, const std::string &pattern)
        : m_name(name), m_level(level), m_pattern(pattern) {}

    std::string Logger::toYamlString()
    {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["name"] = m_name;
        if (m_level != LogLevel::UNKNOWN)
        {
            node["level"] = LogLevelHandler::ToString(m_level);
        }
        if (m_pattern != "")
        {
            node["formatter"] = getPattern();
        }

        for (auto &i : m_appenders)
        {
            node["appenders"].push_back(YAML::Load(i->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    bool Logger::isUnifiedFormatter()
    {
        return f_unifiedFormatter == 0;
    }

    void Logger::log(LogLevel level, const LogMessage::ptr &message)
    {
        if (level >= m_level)
        {
            MutexType::Lock lock(m_mutex);
            // 若该日志没有指定输出地，则默认在root的输出地中进行输出
            if (!m_appenders.empty())
            {
                // 若存在格式器则说明存在同格输出地，直接构造输出流
                if (m_formatter)
                {
                    LogStream stream;
                    m_formatter->format(stream, message);
                    // 遍历输出地，若没有格式器或格式相同，则直接将日志流输出
                    for (auto &appender : m_appenders)
                    {
                        if (appender->getFormatter() && appender->getFormatter()->getPattern() == m_pattern)
                        {
                            appender->log(level, message);
                        }
                        else
                        {
                            appender->log(level, stream);
                        }
                    }
                }
                else // 若没有同格输出地，直接进入输出地构造
                {
                    for (auto &appender : m_appenders)
                    {
                        appender->log(level, message);
                    }
                }
            }
            else
            {
                LIM_LOG_ROOT()->log(level, message);
            }
        }
    }

    void Logger::addAppender(LogAppender::ptr appender)
    {
        MutexType::Lock lock(m_mutex);
        // 若输出地存在格式器且格式与日志不一致，则标志加一，即异化格式输出地数量
        if (appender->getFormatter() && appender->getFormatter()->getPattern() != m_pattern)
        {
            ++f_unifiedFormatter;
        }
        m_appenders.emplace_back(appender);
        // 若输出地数量大于异化格式输出地数量并且不存在格式器，表明存在同化格式输出地，可以提前生成消息，需要创建格式器
        if (!m_formatter && m_appenders.size() > f_unifiedFormatter)
        {
            m_formatter = LogFormatter::Create(m_pattern);
        }
    }

    void Logger::delAppender(LogAppender::ptr appender)
    {
        MutexType::Lock lock(m_mutex);
        // 遍历查找目标输出地
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
        {
            // 找到则删除
            if (*it == appender)
            {
                m_appenders.erase(it);
                // 若输出地存在格式器且格式与日志不一致，则标志减一，即异化格式输出地被删除了
                if (appender->getFormatter() && appender->getFormatter()->getPattern() != m_pattern)
                {
                    --f_unifiedFormatter;
                    assert(f_unifiedFormatter >= 0);
                    // 若输出地数量等于异化格式输出地数量并且存在格式器，表明不存在同化格式输出地，不需要提前生成消息，删除格式器
                    if (m_formatter && m_appenders.size() == f_unifiedFormatter)
                    {
                        m_formatter = nullptr;
                    }
                }
                break;
            }
        }
    }

    void Logger::clearAppender()
    {
        MutexType::Lock lock(m_mutex);
        // 清空
        m_appenders.clear();
        f_unifiedFormatter = 0;
        m_formatter = nullptr;
    }

    void Logger::setPattern(const std::string &pattern)
    {
        MutexType::Lock lock(m_mutex);
        m_pattern = pattern;
    }

    const std::string &Logger::getPattern()
    {
        MutexType::Lock lock(m_mutex);
        return m_pattern;
    }

    

    LoggerManager::LoggerManager()
    {
        m_root = Logger::Create();
        m_root->addAppender(StdoutLogAppender::Create());
        m_logger_map[m_root->getName()] = m_root;
    }

    Logger::ptr LoggerManager::getLogger(const std::string &name)
    {
        MutexType::Lock lock(m_mutex);
        auto it = m_logger_map.find(name);
        if (it != m_logger_map.end())
        {
            return it->second;
        }
        else
        {
            Logger::ptr logger = Logger::Create(name);
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

        bool operator!=(const LogDefine &oth) const
        {
            return !(*this == oth);
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
    ConfigVar<std::set<LogDefine>>::ptr g_log_defines = Config::Lookup("logs", std::set<LogDefine>(), "logs config");

    struct LogInitializer
    {
        LogInitializer()
        {
            auto log_listener_func = [](const std::set<LogDefine> &old_val, const std::set<LogDefine> &new_val)
            {
                for (auto &i : new_val)
                {
                    Logger::ptr logger;
                    auto it = old_val.find(i);
                    if (it == old_val.end() || (i != *it))
                    {
                        // 新增或修改
                        logger = LIM_LOG_NAME(i.name);
                    }
                    logger->setLevel(i.level);
                    if (!i.formatter.empty())
                    {
                        logger->setPattern(i.formatter);
                    }
                    logger->clearAppender();
                    for (auto &a : i.appenders)
                    {
                        LogAppender::ptr ap;
                        if (a.type == 1)
                        {
                            ap = FileLogAppender::Create(a.file);
                        }
                        else if (a.type == 0)
                        {
                            ap = StdoutLogAppender::Create();
                        }
                        ap->setLevel(a.level);
                        if (!a.formatter.empty())
                        {
                            LogFormatter::ptr formatter = LogFormatter::Create(a.formatter);
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

    static LogInitializer __log__init;
}
