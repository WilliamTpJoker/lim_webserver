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
    class LogManager;
    class LogVisitor;
    class YamlVisitor;


    /**
     * @brief Logger构造单
     * 
     * @param name 
     * @param level
     * @param appender_refs
    */
    struct LoggerDefine
    {
        std::string name;
        std::vector<std::string> appender_refs;
        LogLevel level = LogLevel_UNKNOWN;

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

    /**
     * @brief 日志器
     */
    class Logger
    {
        friend YamlVisitor;

    public:
        using ptr = std::shared_ptr<Logger>;

    public:
        using MutexType = Spinlock;
        Logger() {}
        Logger(const std::string &name);
        Logger(const std::string &name, LogLevel level);

        /**
         * @brief 输出日志
         */
        void log(LogLevel level, const LogMessage::ptr &message);

        const char *accept(LogVisitor &visitor);

        /**
         * @brief 添加日志输出地
         */
        void addAppender(LogAppender::ptr& appender);
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

        void setName(const std::string &name) { m_name = name; }

    private:
        std::string m_name = "root";             // 日志名称
        LogLevel m_level = LogLevel_DEBUG;       // 日志级别
        std::list<LogAppender::ptr> m_appenders; // Appender集合
        MutexType m_mutex;
    };

}