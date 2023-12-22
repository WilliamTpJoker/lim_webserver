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

/**
 * @brief 使用流式方式将设定的日志级别的日志事件写入到logger
 *
 * @param logger 目标日志器
 * @param level  事件级别
 */
#define LIM_LOG_LEVEL(logger, level) lim_webserver::LogMessageWrap(lim_webserver::LogMessage::Create(logger, __FILE__, __LINE__, time(0), level, logger->getName())).getStream()

/**
 * @brief 使用流式方式将日志级别debug的日志事件写入到logger
 *
 * @param logger 目标日志器
 */
#define LIM_LOG_DEBUG(logger) LIM_LOG_LEVEL(logger, LogLevel_DEBUG)

/**
 * @brief 使用流式方式将日志级别info的日志事件写入到logger
 *
 * @param logger 目标日志器
 */
#define LIM_LOG_INFO(logger) LIM_LOG_LEVEL(logger, LogLevel_INFO)

/**
 * @brief 使用流式方式将日志级别warn的日志事件写入到logger
 *
 * @param logger 目标日志器
 */
#define LIM_LOG_WARN(logger) LIM_LOG_LEVEL(logger, LogLevel_WARN)

/**
 * @brief 使用流式方式将日志级别error的日志事件写入到logger
 *
 * @param logger 目标日志器
 */
#define LIM_LOG_ERROR(logger) LIM_LOG_LEVEL(logger, LogLevel_ERROR)

/**
 * @brief 使用流式方式将日志级别fatal的日志事件写入到logger
 *
 * @param logger 目标日志器
 */
#define LIM_LOG_FATAL(logger) LIM_LOG_LEVEL(logger, LogLevel_FATAL)

/**
 * @brief 获取根日志器
 */
#define LIM_LOG_ROOT() lim_webserver::LoggerMgr::GetInstance()->getRoot()

/**
 * @brief 获取对应名字的日志器
 *
 * @param name 日志名称
 */
#define LIM_LOG_NAME(name) lim_webserver::LoggerMgr::GetInstance()->getLogger(name)

namespace lim_webserver
{
    class LogAppender;
    /**
     * @brief 日志器
     */
    class Logger : public std::enable_shared_from_this<Logger>
    {
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
        static ptr Create(const std::string &name, LogLevel level, const std::string &pattern)
        {
            return std::make_shared<Logger>(name, level, pattern);
        }

    public:
        using MutexType = Spinlock;
        Logger() {}
        Logger(const std::string &name);
        Logger(const std::string &name, LogLevel level, const std::string &pattern);
        /**
         * @brief 输出日志
         */
        void log(LogLevel level, const LogMessage::ptr &message);

        /**
         * @brief 添加日志输出地
         */
        void addAppender(LogAppender::ptr appender);
        /**
         * @brief 删除日志输出地
         */
        void delAppender(LogAppender::ptr appender);
        /**
         * @brief 清空日志输出地
         */
        void clearAppender();

        /**
         * @brief 设置日志的默认格式
         */
        void setPattern(const std::string &pattern);

        /**
         * @brief 获取日志的输出格式
         */
        const std::string &getPattern();

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

        /**
         * @brief 打印成Yaml格式字符串
         */
        std::string toYamlString();

    private:
        /**
         * @brief 确认所有输出地是否统一格式
         */
        bool isUnifiedFormatter();

    private:
        std::string m_name = "root";                 // 日志名称
        LogLevel m_level = LogLevel_DEBUG;           // 日志级别
        std::list<LogAppender::ptr> m_appenders;     // Appender集合
        std::string m_pattern = LIM_DEFAULT_PATTERN; // 日志格式
        LogFormatter::ptr m_formatter = nullptr;     // 日志格式器
        int f_unifiedFormatter = 0;                  // 统一格式标志位,0:统一格式 >0:不统一格式
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

    class LoggerBuilder
    {
    public:
    private:
    };

    class LoggerManager
    {
    public:
        using MutexType = Spinlock;

    public:
        LoggerManager();

        /**
         * @brief 使用指定名称日志器，若不存在，则创建默认格式的该名日志器
         */
        Logger::ptr getLogger(const std::string &name);

        /**
         * @brief 使用全局日志器
         */
        Logger::ptr getRoot() const { return m_root; }

        /**
         * @brief 打印成Yaml格式字符串
         */
        std::string toYamlString();

    private:
        std::unordered_map<std::string, Logger::ptr> m_logger_map; // 日志器容器
        Logger::ptr m_root;                                        // 全局日志器
        MutexType m_mutex;
    };
    using LoggerMgr = Singleton<LoggerManager>;
}
