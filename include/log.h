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
#include "thread.h"

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
#define LIM_LOG_NAME(name) lim_webserver::LoggerMgr::GetInstance()->getLogger(name)

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
        FATAL,   // FATAL级别
        OFF      // 关闭级别
    };

    /**
     * @brief 日志级别处理器
     */
    class LogLevelHandler
    {
    public:
        // 将日志级别转换成文本输出
        static std::string ToString(LogLevel level);
        // 将文本转换成日志级别
        static LogLevel FromString(const std::string &val);
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

        // 获得文件路径
        const char *getFile() const { return m_file; }
        // 获得事件行号
        int32_t getLine() const { return m_line; }
        // 获得启动时常
        uint32_t getElapse() const { return m_elapse; }
        // 获得线程id
        uint32_t getThreadId() const { return m_threadId; }
        // 获得携程id
        uint32_t getFiberId() const { return m_fiberId; }
        // 获得当前时间
        uint64_t getTime() const { return m_time; }
        // 获得事件级别
        LogLevel getLevel() const { return m_level; }
        // 获得事件内容
        std::string getContent() const { return m_ss.str(); }
        // 获得事件内容流
        std::stringstream &getSS() { return m_ss; }
        // 获得事件对应的日志器
        Shared_ptr<Logger> getLogger() const { return m_logger; }

    private:
        const char *m_file = nullptr; // 文件名
        LogLevel m_level;             // 级别
        int32_t m_line = 0;           // 行号
        uint32_t m_elapse = 0;        // 程序启动开始到现在的毫秒数
        uint32_t m_threadId = 0;      // 线程id
        uint32_t m_fiberId = 0;       // 协程id
        uint64_t m_time;              // 时间戳
        std::stringstream m_ss;       // 内容流
        Shared_ptr<Logger> m_logger;  // 日志器
    };

    /**
     * @brief 日志事件包装器
     */
    class LogEventWrap
    {
    public:
        LogEventWrap(Shared_ptr<LogEvent> e)
            : m_event(e) {}
        // 析构时输出日志
        ~LogEventWrap();

        // 获得日志事件
        Shared_ptr<LogEvent> getEvent() const { return m_event; }

        // 获得日志内容流，以便左移操作补充内容
        std::stringstream &getSS() { return m_event->getSS(); }

    private:
        Shared_ptr<LogEvent> m_event; // 事件
    };

    /**
     * @brief 日志格式器
     */
    class LogFormatter
    {
    public:
        LogFormatter(const std::string &pattern);

        // 构造字符流
        std::string format(Shared_ptr<LogEvent> event);
        // 获得格式
        const std::string &getPattern() const { return m_pattern; }
        // 判断格式是否异常 true：异常
        bool isError() { return m_error; }

        // 格式器初始化
        void init();

    public:
        // 格式体虚父类
        class FormatItem
        {
        public:
            virtual ~FormatItem() {}

            // 构造字符流
            virtual void format(std::ostream &os, Shared_ptr<LogEvent> event) = 0;
        };

    private:
        std::string m_pattern;                       // 格式
        std::vector<Shared_ptr<FormatItem>> m_items; // 格式体容器
        bool m_error = false;                        // 异常标志符
    };

    /**
     * @brief 日志输出地
     */
    class LogAppender
    {
    public:
        using MutexType = Spinlock;
        virtual ~LogAppender(){};

        // 输出日志，必须重构
        virtual void log(LogLevel Level, Shared_ptr<LogEvent> event) = 0;
        // 输出为Yaml字符串格式，必须重构
        virtual std::string toYamlString() = 0;

        // 设置格式器
        void setFormatter(const std::string &pattern);
        void setFormatter(Shared_ptr<LogFormatter> formatter);
        // 获得格式器
        const Shared_ptr<LogFormatter> &getFormatter();

        // 设置输出地级别
        void setLevel(LogLevel level) { m_level = level; }
        // 获得日志器级别
        LogLevel getLevel() const { return m_level; }

        // 设置输出地所属的日志器
        void setLogger(Shared_ptr<Logger> logger);

    protected:
        LogLevel m_level;                     // 级别
        Shared_ptr<LogFormatter> m_formatter; // 格式器
        Shared_ptr<Logger> m_logger;          // 日志器
        MutexType m_mutex;                    // 锁
    };

    /**
     * @brief 日志器
     */
    class Logger : public std::enable_shared_from_this<Logger>
    {
    public:
        Logger(const std::string &name = "root");
        // 输出日志
        void log(LogLevel level, const Shared_ptr<LogEvent> &event);

        // 添加日志输出地
        void addAppender(Shared_ptr<LogAppender> appender);
        // 删除日志输出地
        void delAppender(Shared_ptr<LogAppender> appender);
        // 清空日志输出地
        void clearAppender();

        // 设置该日志的默认格式器(不会同步到已存在的日志输出地)
        void setFormatter(const std::string &pattern);
        void setFormatter(Shared_ptr<LogFormatter> formatter);

        // 获取该日志的默认格式器
        const Shared_ptr<LogFormatter> &getFormatter();
        // 获取日志的输出格式
        const std::string &getPattern();

        // 获取日志的默认级别
        LogLevel getLevel() const { return m_level; }
        // 设置日志的默认级别
        void setLevel(LogLevel val) { m_level = val; }
        // 获取日志的名称
        const std::string &getName() const { return m_name; }

        // 打印成Yaml格式字符串
        std::string toYamlString();

    private:
        std::string m_name;                             // 日志名称
        LogLevel m_level;                               // 日志级别
        std::list<Shared_ptr<LogAppender>> m_appenders; // Appender集合
        Shared_ptr<LogFormatter> m_formatter;           // 日志格式器
        RWMutex m_mutex;                                // 锁
    };
    /**
     * @brief 输出到控制台Appender
     */
    class StdoutLogAppender : public LogAppender
    {
    public:
        // 输出日志到控制台中
        void log(LogLevel Level, Shared_ptr<LogEvent> event) override;

        // 打印成Yaml格式字符串
        std::string toYamlString();

    private:
    };
    /**
     * @brief 输出到文件的Appender
     */
    class FileLogAppender : public LogAppender
    {
    public:
        FileLogAppender(const std::string &filename);

        // 输出日志到文件中
        void log(LogLevel Level, Shared_ptr<LogEvent> event) override;

        // 重新打开文件，打开成功返回true
        bool reopen();
        // 检测文件是否存在
        bool fileExist();

        // 打印成Yaml格式字符串
        std::string toYamlString();

    private:
        std::string m_filename;     // 文件名
        std::ofstream m_filestream; // 文件流
    };

    class LoggerManager
    {
    public:
        using MutexType = Spinlock;
        LoggerManager();
        
        // 传入日志器名称来获取日志器,如果不存在,返回全局日志器
        Shared_ptr<Logger> getLogger(const std::string &name);

        // 使用全局日志器
        Shared_ptr<Logger> getRoot() const { return m_root; }

        // 打印成Yaml格式字符串
        std::string toYamlString();

    private:
        MutexType m_mutex;                                                // 锁
        std::unordered_map<std::string, Shared_ptr<Logger>> m_logger_map; // 日志器容器
        Shared_ptr<Logger> m_root;                                        // 全局日志器
    };
    using LoggerMgr = Singleton<LoggerManager>;
}

#endif
