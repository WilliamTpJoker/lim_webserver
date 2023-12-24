#include "LogManager.h"
#include "Config.h"


namespace lim_webserver
{
    LogManager::LogManager()
    {
        m_root_logger = Logger::Create();
        LogAppender::ptr default_appender = ConsoleAppender::Create();
        default_appender->setFormatter(DEFAULT_PATTERN);
        m_root_logger->addAppender(default_appender);
        m_loggers[m_root_logger->getName()] = m_root_logger;
    }

    Logger::ptr LogManager::getLogger(const std::string &name)
    {
        MutexType::Lock lock(m_mutex);
        auto it = m_loggers.find(name);
        if (it != m_loggers.end())
        {
            return it->second;
        }
        else
        {
            Logger::ptr logger = Logger::Create(name);
            m_loggers[name] = logger;
            return logger;
        }
    }

    
} // namespace lim_webserver
