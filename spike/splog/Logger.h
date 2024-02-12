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

#include "splog/LogAppender.h"
#include "splog/LogMessage.h"

namespace lim_webserver
{
    class LogManager;

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
    public:
        using ptr = std::shared_ptr<Logger>;

        static ptr Create()
        {
            return std::make_shared<Logger>();
        }

    public:
        Logger() {}
        Logger(const std::string &name);

        /**
         * @brief 输出日志
         * 
         * @param message 日志消息
         */
        void log(const LogMessage::ptr &message);

        /**
         * @brief 添加日志输出地
         * 
         * @param appender Appender共享指针
         */
        void addAppender(LogAppender::ptr appender);
        
        /**
         * @brief 删除日志输出地
         * 
         * @param name Appender名
         * @return true 删除成功
         * @return false 删除失败
         */
        bool detachAppender(const std::string &name);

        /**
         * @brief 获取日志输出地
         *
         * @param name Appender名
         * @return LogAppender::ptr Appender共享指针
         */
        LogAppender::ptr getAppender(const std::string &name);

        /**
         * @brief 清空日志输出地
         */
        void clearAppender();

        /**
         * @brief 获取日志的默认级别
         * 
         * @return LogLevel 日志级别
         */
        inline LogLevel getLevel() const { return m_level; }

        /**
         * @brief 设置日志的默认级别
         *
         * @param level 日志级别
         */
        inline void setLevel(LogLevel level) { m_level = level; }

        /**
         * @brief 获取日志的名称
         * 
         * @return const std::string& 日志名
         */
        inline const std::string &getName() const { return m_name; }

        /**
         * @brief 设置日志的名称
         *
         * @param name 日志名
         */
        inline void setName(const std::string &name) { m_name = name; }

    private:
        std::string m_name;                      // 日志名称
        LogLevel m_level = LogLevel_DEBUG;       // 日志级别
        std::list<LogAppender::ptr> m_appenders; // Appender集合
    };

}