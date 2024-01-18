#include "LogManager.h"

namespace lim_webserver
{
    LogManager::LogManager()
    {
        init();
    }

    Logger::ptr LogManager::getLogger(const std::string &name)
    {
        MutexType::Lock lock(m_mutex);
        if(m_root->getName()==name)
        {
            return m_root;
        }

        auto it = m_loggers.find(name);
        if (it != m_loggers.end())
        {
            return it->second;
        }
        else
        {
            Logger::ptr logger = Logger::Create();
            logger->setName(name);
            m_loggers[logger->getName()] = logger;
            return logger;
        }
    }

    Logger::ptr LogManager::getRoot()
    {
        MutexType::Lock lock(m_mutex);
        return m_root;
    }

    void LogManager::delLogger(const std::string &name)
    {
        MutexType::Lock lock(m_mutex);
        auto it = m_loggers.find(name);
        if (it != m_loggers.end())
        {
            Logger::ptr logger = it->second;
            logger->clearAppender();
            logger->setLevel(LogLevel_OFF);
        }
    }

    void LogManager::createOrUpdateLogger(const LoggerDefine &ld)
    {
        MutexType::Lock lock(m_mutex);
        Logger::ptr logger;
        auto it = m_loggers.find(ld.name);

        // 找到了，进行Update
        if (it != m_loggers.end())
        {
            logger = it->second;
            updateLogger(logger, ld);
            return;
        }
        // 没找到，新建
        logger = createLogger(ld);
    }

    Logger::ptr LogManager::createLogger(const LoggerDefine &ld)
    {
        Logger::ptr logger = Logger::ptr(new Logger);
        m_loggers[ld.name] = logger;
        updateLogger(logger, ld);
        return logger;
    }

    void LogManager::updateLogger(Logger::ptr logger, const LoggerDefine &ld)
    {
        logger->setName(ld.name);
        logger->setLevel(ld.level);
        logger->clearAppender();
        for (auto appender_name : ld.appender_refs)
        {
            logger->addAppender(m_appenders[appender_name]);
        }
    }

    LogAppender::ptr LogManager::getAppender(const std::string &name)
    {
        MutexType::Lock lock(m_mutex);
        auto it = m_appenders.find(name);
        if (it != m_appenders.end())
        {
            LogAppender::ptr appender = m_appenders[name];
            return m_appenders[name];
        }
        else
        {
            return m_appenders["console"];
        }
    }

    void LogManager::delAppender(const std::string &name)
    {
        MutexType::Lock lock(m_mutex);
        auto it = m_appenders.find(name);
        if (it != m_appenders.end())
        {
            m_appenders.erase(it);
        }
        for (auto &it : m_loggers)
        {
            it.second->detachAppender(name);
        }
    }

    void LogManager::createOrUpdateAppender(const LogAppenderDefine &lad)
    {
        MutexType::Lock lock(m_mutex);
        auto it = m_appenders.find(lad.name);
        // 若输出地存在
        if (it != m_appenders.end())
        {
            LogAppender::ptr appender = it->second;
            // 输出地类型变化,则忽略
            if (appender->getType() != lad.type)
            {
                return;
            }
            else // 输出地没有变化，修改
            {
                updateAppender(appender, lad);
            }
        }
        else // 若输出地不存在
        {
            createAppender(lad);
        }
    }

    LogAppender::ptr LogManager::createAppender(const LogAppenderDefine &lad)
    {
        LogAppender::ptr appender;
        if (lad.type == 0)
        {
            appender = ConsoleAppender::ptr(new ConsoleAppender(lad));
        }
        else if (lad.type == 1)
        {
            if (lad.file.empty())
            {
                appender = nullptr;
            }
            appender = FileAppender::ptr(new FileAppender(lad));
        }
        else
        {
            appender = nullptr;
        }
        m_appenders[lad.name] = appender;
        return appender;
    }

    void LogManager::updateAppender(LogAppender::ptr appender, const LogAppenderDefine &lad)
    {
        appender->setName(lad.name);
    }

    void LogManager::init()
    {
        ConsoleAppender::ptr appender = AppenderFactory::newConsoleAppender();
        appender->setName("console");
        appender->setLevel(LogLevel_DEBUG);
        appender->setFormatter(DEFAULT_PATTERN);
        appender->start();

        m_appenders[appender->getName()]=appender;

        m_root = Logger::Create();
        m_root->setName("root");
        m_root->addAppender(m_appenders["console"]);

        Logger::ptr sys_logger = Logger::Create();
        sys_logger->setName("system");
        sys_logger->addAppender(m_appenders["console"]);
        
        m_loggers[sys_logger->getName()]=sys_logger;

    }

} // namespace lim_webserver
