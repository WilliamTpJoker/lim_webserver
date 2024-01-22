#pragma once

#include <unordered_map>
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
        static std::string ToString(LogLevel level)
        {
            static const std::unordered_map<LogLevel, std::string> levelStrings = {
                {LogLevel::TRACE, "TRACE"},
                {LogLevel::DEBUG, "DEBUG"},
                {LogLevel::INFO, "INFO"},
                {LogLevel::WARN, "WARN"},
                {LogLevel::ERROR, "ERROR"},
                {LogLevel::FATAL, "FATAL"},
                {LogLevel::OFF, "OFF"},
            };
            auto it = levelStrings.find(level);
            if (it != levelStrings.end())
            {
                return it->second;
            }
            else
            {
                return std::string("UNKNOWN");
            }
        }
        /**
         * @brief 将文本表示的日志级别转换为枚举值
         */
        static LogLevel FromString(const std::string &val)
        {
            static const std::unordered_map<std::string, LogLevel> stringToLevel = {
                {"TRACE", LogLevel::TRACE},
                {"DEBUG", LogLevel::DEBUG},
                {"INFO", LogLevel::INFO},
                {"WARN", LogLevel::WARN},
                {"ERROR", LogLevel::ERROR},
                {"FATAL", LogLevel::FATAL},
                {"OFF", LogLevel::OFF}};

            auto it = stringToLevel.find(val);
            if (it != stringToLevel.end())
            {
                return it->second;
            }
            else
            {
                return LogLevel::UNKNOWN;
            }
        }
    };
} // namespace lim_webserver
