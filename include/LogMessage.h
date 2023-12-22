#pragma once

#include <memory>

#include "LogLevel.h"
#include "LogStream.h"

namespace lim_webserver
{
    class Logger;
    /**
     * @brief 日志消息类
     */
    class LogMessage
    {
    public:
        using ptr = std::shared_ptr<LogMessage>;
        /**
         * @brief 创建日志事件对象智能指针
         *
         * @param logger        日志器对象
         * @param file          源文件名
         * @param line          行号
         * @param time          时间戳
         * @param level         日志级别，默认为DEBUG级别
         */
        static ptr Create(std::shared_ptr<Logger> logger, const char *file, int32_t line, uint64_t time, LogLevel level,std::string name)
        {
            return std::make_shared<LogMessage>(logger, file, line, time, level, name);
        }

    public:
        /**
         * @brief 构造函数，用于创建日志事件对象
         *
         * @param logger        日志器对象
         * @param file          源文件名
         * @param line          行号
         * @param time          时间戳
         * @param level         日志级别，默认为DEBUG级别
         */
        LogMessage(std::shared_ptr<Logger> logger, const char *file, int32_t line, uint64_t time, LogLevel level, std::string name)
            : m_logger(logger), m_file(file), m_line(line), m_time(time), m_level(level), m_name(name) {}

        /**
         * @brief 获取文件路径。
         *
         * @return const char* 文件路径。
         */
        const char *getFile() const { return m_file; }
        /**
         * @brief 获取日志事件所在的行号。
         *
         * @return uint32_t 行号。
         */
        uint32_t getLine() const { return m_line; }
        /**
         * @brief 获取日志事件发生的时间戳。
         *
         * @return uint64_t 时间戳。
         */
        uint64_t getTime() const { return m_time; }
        /**
         * @brief 获取日志事件的级别。
         *
         * @return LogLevel 事件级别。
         */
        LogLevel getLevel() const { return m_level; }
        /**
         * @brief 获取日志事件的级别字符串。
         *
         * @return std::string 事件级别字符串。
         */
        std::string getLevelString() { return LogLevelHandler::ToString(m_level); }
        /**
         * @brief 获取用于向日志事件内容中追加文本的内容流。
         *
         * @return LogStream& 内容流的引用。
         */
        LogStream &getStream() { return m_logStream; }
        /**
         * @brief 获取与该日志事件关联的日志器。
         *
         * @return std::shared_ptr<Logger> 日志器的智能指针。
         */
        std::shared_ptr<Logger> getLogger() const { return m_logger; }

        const std::string &getName() const { return m_name; }

    private:
        const char *m_file = nullptr;     // 文件名
        LogLevel m_level;                 // 级别
        uint32_t m_line = 0;              // 行号
        uint64_t m_time;                  // 时间戳
        std::shared_ptr<Logger> m_logger; // 日志器
        std::string m_name;               // 日志名称
        LogStream m_logStream;            // 内容流
    };
} // namespace lim_webserver
