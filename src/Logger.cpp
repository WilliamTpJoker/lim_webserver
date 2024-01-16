#include "Logger.h"
#include "LogManager.h"
#include "LogAppender.h"

#include <algorithm>

namespace lim_webserver
{

    /**
     * @brief Logger成员函数
     */

    Logger::Logger(const std::string &name)
        : m_name(name) {}

    void Logger::log(const LogMessage::ptr &message)
    {
        if (message->getLevel() >= m_level)
        {
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
        if (!appender->isStarted())
        {
            appender->start();
        }
        m_appenders.emplace_back(appender);
    }

    bool Logger::detachAppender(const std::string &name)
    {
        // 查找并将目标移动到链表尾部
        auto it = std::remove_if(m_appenders.begin(), m_appenders.end(), [name](LogAppender::ptr appender)
                                 { return appender->getName() == name; });
        // 删除链表尾部
        m_appenders.erase(it, m_appenders.end());
        return it != m_appenders.end();
    }

    LogAppender::ptr Logger::getAppender(const std::string &name)
    {
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it)
        {
            if (it->get()->getName() == name)
            {
                return *it;
            }
        }
        return nullptr;
    }

    void Logger::clearAppender()
    {
        m_appenders.clear();
    }
}
