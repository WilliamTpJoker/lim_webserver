#pragma once

#include "Logger.h"
#include "Singleton.h"
#include "Mutex.h"

/**
 * @brief 使用流式方式将设定的日志级别的日志事件写入到logger
 *
 * @param logger 目标日志器
 * @param level  事件级别
 */
#define LOG_LEVEL(logger, level) lim_webserver::LogMessageWrap(lim_webserver::LogMessage::Create(logger, __FILE__, __LINE__, time(0), level, logger->getName())).getStream()
#define LOG_DEBUG(logger) LOG_LEVEL(logger, LogLevel_DEBUG)
#define LOG_INFO(logger) LOG_LEVEL(logger, LogLevel_INFO)
#define LOG_WARN(logger) LOG_LEVEL(logger, LogLevel_WARN)
#define LOG_ERROR(logger) LOG_LEVEL(logger, LogLevel_ERROR)
#define LOG_FATAL(logger) LOG_LEVEL(logger, LogLevel_FATAL)



#define LOG_ROOT() lim_webserver::LoggerMgr::GetInstance()->getRoot()
#define LOG_NAME(name) lim_webserver::LoggerMgr::GetInstance()->getLogger(name)

namespace lim_webserver
{
    

    class LogManager
    {
    public:
        using MutexType = Spinlock;
    public:
        LogManager();
        /**
         * @brief 使用指定名称日志器，若不存在，则创建默认格式的该名日志器
         */
        Logger::ptr getLogger(const std::string &name);

        /**
         * @brief 使用全局日志器
         */
        Logger::ptr getRoot() const { return m_root_logger; }



    private:
        std::unordered_map<std::string, LogAppender::ptr> m_appenders; // 系统全部输出地
        std::unordered_map<std::string, Logger::ptr> m_loggers;        // 系统全部日志器
        Logger::ptr m_root_logger;                                // 根日志
        MutexType m_mutex;                                             // 锁
    };
    using LoggerMgr = Singleton<LogManager>;
} // namespace lim_webserver
