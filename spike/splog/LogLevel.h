#pragma once

#include <string>

#define LogLevel_UNKNOWN lim_webserver::LogLevel::UNKNOWN
#define LogLevel_TRACE lim_webserver::LogLevel::TRACE
#define LogLevel_DEBUG lim_webserver::LogLevel::DEBUG
#define LogLevel_INFO lim_webserver::LogLevel::INFO
#define LogLevel_WARN lim_webserver::LogLevel::WARN
#define LogLevel_ERROR lim_webserver::LogLevel::ERROR
#define LogLevel_FATAL lim_webserver::LogLevel::FATAL
#define LogLevel_OFF lim_webserver::LogLevel::OFF

namespace lim_webserver
{
    /**
     * UNKNOWN:  未知级别
     * TRACE:    TRACE级别
     * DEBUG:    DEBUG级别
     * INFO:     INFO级别
     * WARN:     WARN级别
     * ERROR:    ERROR级别
     * FATAL:    FATAL级别
     * OFF:       关闭级别
     */
    enum class LogLevel
    {
        UNKNOWN, // 未知级别
        TRACE,   // TRACE级别
        DEBUG,   // DEBUG级别
        INFO,    // INFO级别
        WARN,    // WARN级别
        ERROR,   // ERROR级别
        FATAL,   // FATAL级别
        OFF      // 关闭级别
    };

    /**
     * @brief 日志级别处理器类
     */
    class LogLevelHandler
    {
    public:
        /**
         * @brief 将日志级别转换为对应的文本表示
         */
        static const char *ToString(LogLevel level)
        {
            switch (level)
            {
            case LogLevel::TRACE:
                return "TRACE";
            case LogLevel::DEBUG:
                return "DEBUG";
            case LogLevel::INFO:
                return "INFO";
            case LogLevel::WARN:
                return "WARN";
            case LogLevel::ERROR:
                return "ERROR";
            case LogLevel::FATAL:
                return "FATAL";
            case LogLevel::OFF:
                return "OFF";
            default:
                return "UNKNOWN";
            }
        }
        /**
         * @brief 将文本表示的日志级别转换为枚举值
         */
        static LogLevel FromString(const std::string &val)
        {
            LogLevel result = LogLevel::UNKNOWN;

            if (val == "TRACE")
            {
                result = LogLevel::TRACE;
            }
            else if (val == "DEBUG")
            {
                result = LogLevel::DEBUG;
            }
            else if (val == "INFO")
            {
                result = LogLevel::INFO;
            }
            else if (val == "WARN")
            {
                result = LogLevel::WARN;
            }
            else if (val == "ERROR")
            {
                result = LogLevel::ERROR;
            }
            else if (val == "FATAL")
            {
                result = LogLevel::FATAL;
            }
            else if (val == "OFF")
            {
                result = LogLevel::OFF;
            }

            return result;
        }
    };
} // namespace lim_webserver
