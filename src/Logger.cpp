#include "Logger.h"
#include "Config.h"
#include "Fiber.h"
#include "AsyncLog.h"
#include "LogVisitor.h"

namespace lim_webserver
{

    /**
     * @brief Logger成员函数
     */

    Logger::Logger(const std::string &name)
        : m_name(name) {}

    Logger::Logger(const std::string &name, LogLevel level)
        : m_name(name), m_level(level) {}

    const char* Logger::accept(LogVisitor& visitor)
    {
        return visitor.visitLogger(*this);
    }

    void Logger::log(LogLevel level, const LogMessage::ptr &message)
    {
        if (level >= m_level)
        {
            MutexType::Lock lock(m_mutex);
            // 若该日志没有指定输出地，则默认在root的输出地中进行输出
            if (!m_appenders.empty())
            {
                for (auto &appender : m_appenders)
                {
                    appender->log(level, message);
                }
            }
            else
            {
                LOG_ROOT()->log(level, message);
            }
        }
    }

    void Logger::addAppender(LogAppender::ptr appender)
    {
        MutexType::Lock lock(m_mutex);
        m_appenders.emplace_back(appender);
    }

    void Logger::delAppender(std::string &name)
    {
        MutexType::Lock lock(m_mutex);
        auto it = std::remove_if(m_appenders.begin(), m_appenders.end(), [name](LogAppender::ptr appender)
                                 { return appender->getName() == name; });
        m_appenders.erase(it, m_appenders.end());
    }

    void Logger::clearAppender()
    {
        MutexType::Lock lock(m_mutex);
        m_appenders.clear();
    }

    LoggerManager::LoggerManager()
    {
        m_root = Logger::Create();
        LogAppender::ptr default_appender = ConsoleAppender::Create();
        default_appender->setFormatter(DEFAULT_PATTERN);
        m_root->addAppender(default_appender);
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
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    
}
