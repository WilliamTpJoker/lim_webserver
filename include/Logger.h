#pragma once

#include <memory>
#include <string>
#include <stdint.h>
#include <list>
#include <sstream>
#include <fstream>
#include <iostream>
#include <vector>
#include <functional>
#include <assert.h>
#include <unordered_map>

#include "Util.h"
#include "LogAppender.h"
#include "LogFormatter.h"
#include "AsyncLog.h"
#include "Singleton.h"
#include "LogMessage.h"

namespace lim_webserver
{
    class LogVisitor;
    class YamlVisitor;
    /**
     * @brief 日志器
     */
    class Logger 
    {
        friend YamlVisitor;
    public:
        using ptr = std::shared_ptr<Logger>;
        static ptr Create()
        {
            return std::make_shared<Logger>();
        }
        static ptr Create(const std::string &name)
        {
            return std::make_shared<Logger>(name);
        }
        static ptr Create(const std::string &name, LogLevel level)
        {
            return std::make_shared<Logger>(name, level);
        }

    public:
        using MutexType = Spinlock;
        Logger() {}
        Logger(const std::string &name);
        Logger(const std::string &name, LogLevel level);
        /**
         * @brief 输出日志
         */
        void log(LogLevel level, const LogMessage::ptr &message);

        const char* accept(LogVisitor& visitor);

        /**
         * @brief 添加日志输出地
         */
        void addAppender(LogAppender::ptr appender);
        /**
         * @brief 删除日志输出地
         */
        void delAppender(std::string &name);
        /**
         * @brief 清空日志输出地
         */
        void clearAppender();

        /**
         * @brief 获取日志的默认级别
         */
        LogLevel getLevel() const { return m_level; }
        /**
         * @brief 设置日志的默认级别
         */
        void setLevel(LogLevel val) { m_level = val; }
        /**
         * @brief 获取日志的名称
         */
        const std::string &getName() const { return m_name; }

    private:
        std::string m_name = "root";             // 日志名称
        LogLevel m_level = LogLevel_DEBUG;       // 日志级别
        std::list<LogAppender::ptr> m_appenders; // Appender集合
        MutexType m_mutex;
    };

    /**
     * @brief 日志事件包装器
     */
    class LogMessageWrap
    {
    public:
        LogMessageWrap(LogMessage::ptr e)
            : m_message(e) {}
        /**
         * @brief 析构时输出日志
         */
        ~LogMessageWrap()
        {
            m_message->getLogger()->log(m_message->getLevel(), m_message);
        }

        /**
         * @brief 获得日志事件
         */
        LogMessage::ptr getMessage() const { return m_message; }
        /**
         * @brief 获得日志内容流，以便左移操作补充内容
         */
        LogStream &getStream() { return m_message->getStream(); }

    private:
        LogMessage::ptr m_message; // 事件
    };

    struct LoggerDefine
    {
        std::string name;
        LogLevel level = LogLevel_UNKNOWN;
        std::vector<std::string> appender_refs;

        bool operator==(const LoggerDefine &oth) const
        {
            return name == oth.name && level == oth.level && appender_refs == oth.appender_refs;
        }

        bool operator<(const LoggerDefine &oth) const
        {
            return name < oth.name;
        }

        bool operator!=(const LoggerDefine &oth) const
        {
            return !(*this == oth);
        }

        bool isValid() const
        {
            return !name.empty();
        }
    };
}