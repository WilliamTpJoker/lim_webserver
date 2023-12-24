#include "Logger.h"
#include "Config.h"
#include "Fiber.h"
#include "AsyncLog.h"
#include "LogVisitor.h"
#include "LogManager.h"

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
    
}
