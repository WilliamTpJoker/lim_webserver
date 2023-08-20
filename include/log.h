#ifndef _LOG_H_
#define _LOG_H_

#include "util.h"
#include "singleton.h"
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

#define LIM_DEFAULT_PATTERN "%d%T[%c][%p]%f:%l %r %t %F%T%m%n"

/**
 * @brief 使用流式方式将设定的日志级别的日志写入到logger
 */
#define LIM_LOG_LEVEL(logger, level) lim_webserver::LogEventWrap(lim_webserver::MakeShared<lim_webserver::LogEvent>(logger, __FILE__, __LINE__, 0, lim_webserver::GetThreadId(), lim_webserver::GetFiberId(), time(0), level)).getSS()

/**
 * @brief 使用流式方式将日志级别debug的日志写入到logger
 */
#define LIM_LOG_DEBUG(logger) LIM_LOG_LEVEL(logger, lim_webserver::LogLevel::DEBUG)

/**
 * @brief 使用流式方式将日志级别info的日志写入到logger
 */
#define LIM_LOG_INFO(logger) LIM_LOG_LEVEL(logger, lim_webserver::LogLevel::INFO)

/**
 * @brief 使用流式方式将日志级别warn的日志写入到logger
 */
#define LIM_LOG_WARN(logger) LIM_LOG_LEVEL(logger, lim_webserver::LogLevel::WARN)

/**
 * @brief 使用流式方式将日志级别error的日志写入到logger
 */
#define LIM_LOG_ERROR(logger) LIM_LOG_LEVEL(logger, lim_webserver::LogLevel::ERROR)

/**
 * @brief 使用流式方式将日志级别fatal的日志写入到logger
 */
#define LIM_LOG_FATAL(logger) LIM_LOG_LEVEL(logger, lim_webserver::LogLevel::FATAL)

#define LIM_LOG_ROOT() lim_webserver::LoggerMgr::GetInstance()->getRoot()

namespace lim_webserver
{
    class Logger;
    /**
     * @brief 日志级别
     */
    enum class LogLevel
    {
        UNKNOWN, // 未知级别
        DEBUG,   // DEBUG级别
        INFO,    // INFO级别
        WARN,    // WARN级别
        ERROR,   // ERROR级别
        FATAL    // FATAL级别
    };
    /**
     * @brief 日志级别处理器
     */
    class LogLevelHandler
    {
    public:
        static std::string ToString(LogLevel level); // 将日志级别转换成文本输出
    };

    /**
     * @brief 日志事件
     */
    class LogEvent
    {
    public:
        LogEvent(Shared_ptr<Logger> logger, const char *file, int32_t line, uint32_t elapse,
                 uint32_t thread_id, uint32_t fiber_id, uint64_t time, LogLevel level = LogLevel::DEBUG)
            : m_logger(logger), m_file(file), m_line(line), m_elapse(elapse), m_threadId(thread_id), m_fiberId(fiber_id), m_time(time), m_level(level) {}

        const char *getFile() const { return m_file; }
        int32_t getLine() const { return m_line; }
        uint32_t getElapse() const { return m_elapse; }
        uint32_t getThreadId() const { return m_threadId; }
        uint32_t getFiberId() const { return m_fiberId; }
        uint64_t getTime() const { return m_time; }
        LogLevel getLevel() const { return m_level; }
        std::string getContent() const { return m_ss.str(); }
        std::stringstream &getSS() { return m_ss; }

        Shared_ptr<Logger> getLogger() const { return m_logger; }

        void setLevel(LogLevel level) { m_level = level; }

    private:
        const char *m_file = nullptr; // 文件名
        LogLevel m_level;             // 级别
        int32_t m_line = 0;           // 行号
        uint32_t m_elapse = 0;        // 程序启动开始到现在的毫秒数
        uint32_t m_threadId = 0;      // 线程id
        uint32_t m_fiberId = 0;       // 协程id
        uint64_t m_time;              // 时间戳
        std::stringstream m_ss;       // 内容流
        Shared_ptr<Logger> m_logger;
    };
    /**
     * @brief 日志事件包装器
     */
    class LogEventWrap
    {
    public:
        LogEventWrap(Shared_ptr<LogEvent> e)
            : m_event(e)
        {
        }

        ~LogEventWrap();

        Shared_ptr<LogEvent> getEvent() const { return m_event; }

        std::stringstream &getSS() { return m_event->getSS(); }

    private:
        Shared_ptr<LogEvent> m_event;
    };
    /**
     * @brief 日志格式器
     */
    class LogFormatter
    {
    public:
        LogFormatter(const std::string &pattern);

        std::string format(Shared_ptr<LogEvent> event);
        const std::string &getPattern() const { return m_pattern; }

    public:
        class FormatItem
        {
        public:
            virtual ~FormatItem() {}
            virtual void format(std::ostream &os, Shared_ptr<LogEvent> event) = 0;
        };
        void init();

    private:
        std::string m_pattern;
        std::vector<Shared_ptr<FormatItem>> m_items;
        bool m_error = false;
    };
    /**
     * @brief 日志输出地
     */
    class LogAppender
    {
    public:
        virtual ~LogAppender(){};

        virtual void log(LogLevel Level, Shared_ptr<LogEvent> event) = 0;

        void setFormatter(Shared_ptr<LogFormatter> formatter) { m_formatter = std::move(formatter); }
        Shared_ptr<LogFormatter> getFormatter() const { return m_formatter; }

    protected:
        LogLevel m_Level;
        Shared_ptr<LogFormatter> m_formatter;
    };
    /**
     * @brief 日志器
     */
    class Logger
    {
    public:
        Logger();
        Logger(const std::string &name, LogLevel level, const std::string &pattern);
        // 输出日志
        void log(LogLevel level, const Shared_ptr<LogEvent> &event);

        void debug(Shared_ptr<LogEvent> event); // 写debug级别的日志
        void info(Shared_ptr<LogEvent> event);  // 写info级别的日志
        void warn(Shared_ptr<LogEvent> event);  // 写warn级别的日志
        void error(Shared_ptr<LogEvent> event); // 写error级别的日志
        void fatal(Shared_ptr<LogEvent> event); // 写fatal级别的日志

        // 添加日志输出地
        void addAppender(Shared_ptr<LogAppender> appender);
        // 删除日志输出地
        void delAppender(Shared_ptr<LogAppender> appender);

        // 设置该日志的默认格式器(不会同步到已存在的日志输出地)
        void setFormatter(const std::string &pattern) { m_formatter = std::make_shared<LogFormatter>(pattern); }
        // 获取该日志的默认格式器
        const Shared_ptr<LogFormatter> &getFormatter() const { return m_formatter; }

        // 获取日志的输出格式
        const std::string &getPattern() const { return m_formatter->getPattern(); }

        // 获取日志的默认级别
        LogLevel getLevel() const { return m_level; }
        // 设置日志的默认级别
        void setLevel(LogLevel val) { m_level = val; }

        // 获取日志的名称
        const std::string &getName() const { return m_name; }

    private:
        std::string m_name;                             // 日志名称
        LogLevel m_level;                               // 日志级别
        std::list<Shared_ptr<LogAppender>> m_appenders; // Appender集合
        Shared_ptr<LogFormatter> m_formatter;           // 日志格式器
    };
    /**
     * @brief 输出到控制台Appender
     */
    class StdoutLogAppender : public LogAppender
    {
    public:
        void log(LogLevel Level, Shared_ptr<LogEvent> event) override;

    private:
    };
    /**
     * @brief 输出到文件的Appender
     */
    class FileLogAppender : public LogAppender
    {
    public:
        FileLogAppender(const std::string &filename);
        void log(LogLevel Level, Shared_ptr<LogEvent> event) override;

        // 重新打开文件，打开成功返回true
        bool reopen();

    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };

    class LoggerManager
    {
    public:
        LoggerManager();
        // 传入日志器名称来获取日志器,如果不存在,返回全局日志器
        Shared_ptr<Logger> getLogger(const std::string &name);
        Shared_ptr<Logger> getRoot() const { return m_root; }

    private:
        std::unordered_map<std::string, Shared_ptr<Logger>> m_logger_map;
        Shared_ptr<Logger> m_root;
    };
    using LoggerMgr = lim_webserver::Singleton<LoggerManager>;
}

#endif