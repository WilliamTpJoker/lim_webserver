#include "LogManager.h"
#include "Config.h"

namespace lim_webserver
{
    LogManager::LogManager()
    {
        LogAppenderDefine console_lad;
        console_lad.name = "console";
        console_lad.level = LogLevel_DEBUG;
        createAppender(console_lad);

        LoggerDefine root_ld;
        root_ld.name = "root";
        root_ld.appender_refs.push_back("console");
        m_root_logger = createLogger(root_ld);

        LoggerDefine sys_ld;
        sys_ld.name = "system";
        sys_ld.appender_refs.push_back("console");
        createLogger(sys_ld);
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
            Logger::ptr logger = Logger::ptr(new Logger);
            m_loggers["name"]=logger;
            return logger;
        }
    }

    Logger::ptr LogManager::getRoot()
    {
        MutexType::Lock lock(m_mutex);
        return m_root_logger;
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

    LogAppender::ptr &LogManager::getAppender(const std::string &name)
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

    void LogManager::createOrUpdateAppender(const LogAppenderDefine &lad)
    {
        MutexType::Lock lock(m_mutex);
        auto it = m_appenders.find(lad.name);
        // 若输出地存在
        if (it != m_appenders.end())
        {
            LogAppender::ptr appender = it->second;
            // 输出地类型变化
            if (appender->getType() != lad.type)
            {
                createAppender(lad);
            }
            else // 输出地没有变化，修改
            {
                updateAppender(appender,lad);
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
        appender->setLevel(lad.level);
        appender->setFormatter(lad.formatter);
    }

} // namespace lim_webserver
