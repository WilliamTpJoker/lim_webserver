#pragma once

#include "splog/LogManager.h"
#include "splog/LogInitializer.h"
#include "base/BackTrace.h"

/**
 * @brief 使用流式方式将设定的日志级别的日志事件写入到logger
 *
 * @param logger 目标日志器
 * @param level  事件级别
 */
#define LOG_LEVEL(logger, level) lim_webserver::LogMessageWrap(lim_webserver::LogMessage::Create(logger, __FILE__, __LINE__, time(0), level, logger->getName())).getStream()
#define LOG_TRACE(logger) LOG_LEVEL(logger, LogLevel_TRACE)
#define LOG_DEBUG(logger) LOG_LEVEL(logger, LogLevel_DEBUG)
#define LOG_INFO(logger) LOG_LEVEL(logger, LogLevel_INFO)
#define LOG_WARN(logger) LOG_LEVEL(logger, LogLevel_WARN)
#define LOG_ERROR(logger) LOG_LEVEL(logger, LogLevel_ERROR)
#define LOG_FATAL(logger) LOG_LEVEL(logger, LogLevel_FATAL)

#define LOG_ROOT() lim_webserver::LogManager::GetInstance()->getRoot()
#define LOG_SYS() lim_webserver::LogManager::GetInstance()->getLogger("system")
#define LOG_NAME(name) lim_webserver::LogManager::GetInstance()->getLogger(name)

#if defined __GNUC__ || defined __llvm__
#   define LIKELY(x) __builtin_expect(!!(x), 1)
#   define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#   define LIKELY(x) (x)
#   define UNLIKELY(x) (x)
#endif

/**
 * @brief 断言宏封装
 */
#define ASSERT(condition, ...)                                                  \
    do                                                                              \
    {                                                                               \
        if (!(condition))                                                           \
        {                                                                           \
            LOG_ERROR(LOG_ROOT()) << "\nASSERTION: " << #condition << "\t"  \
                                          << #__VA_ARGS__ << "\nbacktrace:"         \
                                          << lim_webserver::BackTraceToString(100); \
            assert(condition);                                                      \
        }                                                                           \
    } while (false)

// extern lim_webserver::LogInitializer __log__init;
