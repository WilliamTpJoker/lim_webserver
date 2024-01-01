#include "Logger.h"
#include "Config.h"
#include "Fiber.h"
#include "AsyncLog.h"
#include "LogVisitor.h"
#include "LogManager.h"
#include "LogAppender.h"

namespace lim_webserver
{

    /**
     * @brief Logger成员函数
     */

    Logger::Logger(const std::string &name)
        : m_name(name) {}

    const char *Logger::accept(LogVisitor &visitor)
    {
        return visitor.visitLogger(*this);
    }

    void Logger::log(const LogMessage::ptr &message)
    {
        if (message->getLevel() >= m_level)
        {
            MutexType::Lock lock(m_mutex);
            // 若该日志没有指定输出地，则默认在root的输出地中进行输出
            if (!m_appenders.empty())
            {
                for (auto &appender : m_appenders)
                {
                    appender->doAppend(message);
                }
            }
            else
            {
                LOG_ROOT()->log(message);
            }
        }
    }

    void Logger::addAppender(LogAppender::ptr appender)
    {
        MutexType::Lock lock(m_mutex);

        m_appenders.emplace_back(appender);
    }

    bool Logger::detachAppender(const std::string &name)
    {
        MutexType::Lock lock(m_mutex);
        // 查找并将目标移动到链表尾部
        auto it = std::remove_if(m_appenders.begin(), m_appenders.end(), [name](LogAppender::ptr appender)
                                 { return appender->getName() == name; });
        // 删除链表尾部
        m_appenders.erase(it, m_appenders.end());
        return it != m_appenders.end();
    }

    void Logger::clearAppender()
    {
        MutexType::Lock lock(m_mutex);
        m_appenders.clear();
    }
}
