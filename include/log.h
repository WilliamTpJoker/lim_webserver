#ifndef __LIM_LOG_H__
#define __LIM_LOG_H__

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

#include "util.h"
#include "thread.h"
#include "singleton.h"

#define LIM_DEFAULT_PATTERN "%d%T%t %N%T%F%T[%c] [%p] %f:%l%T%m%n"

/**
 * @brief 使用流式方式将设定的日志级别的日志事件写入到logger
 *
 * @param logger 目标日志器
 * @param level  事件级别
 */
#define LIM_LOG_LEVEL(logger, level) lim_webserver::LogEventWrap(lim_webserver::LogEvent::create(logger, __FILE__, __LINE__, 0, lim_webserver::GetThreadId(), lim_webserver::GetFiberId(), time(0), level, lim_webserver::Thread::GetThisThreadName())).getSS()

/**
 * @brief 使用流式方式将日志级别debug的日志事件写入到logger
 *
 * @param logger 目标日志器
 */
#define LIM_LOG_DEBUG(logger) LIM_LOG_LEVEL(logger, lim_webserver::LogLevel::DEBUG)

/**
 * @brief 使用流式方式将日志级别info的日志事件写入到logger
 *
 * @param logger 目标日志器
 */
#define LIM_LOG_INFO(logger) LIM_LOG_LEVEL(logger, lim_webserver::LogLevel::INFO)

/**
 * @brief 使用流式方式将日志级别warn的日志事件写入到logger
 *
 * @param logger 目标日志器
 */
#define LIM_LOG_WARN(logger) LIM_LOG_LEVEL(logger, lim_webserver::LogLevel::WARN)

/**
 * @brief 使用流式方式将日志级别error的日志事件写入到logger
 *
 * @param logger 目标日志器
 */
#define LIM_LOG_ERROR(logger) LIM_LOG_LEVEL(logger, lim_webserver::LogLevel::ERROR)

/**
 * @brief 使用流式方式将日志级别fatal的日志事件写入到logger
 *
 * @param logger 目标日志器
 */
#define LIM_LOG_FATAL(logger) LIM_LOG_LEVEL(logger, lim_webserver::LogLevel::FATAL)

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

#define LogLevel_DEBUG lim_webserver::LogLevel::DEBUG
#define LogLevel_INFO lim_webserver::LogLevel::INFO
#define LogLevel_WARN lim_webserver::LogLevel::WARN
#define LogLevel_ERROR lim_webserver::LogLevel::ERROR
#define LogLevel_FATAL lim_webserver::LogLevel::FATAL

namespace lim_webserver
{
    class Logger;

    /**
     * UNKNOWN:  未知级别
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
        static std::string ToString(LogLevel level);
        /**
         * @brief 将文本表示的日志级别转换为枚举值
         */
        static LogLevel FromString(const std::string &val);
    };

    /**
     * @brief 日志事件类
     */
    class LogEvent
    {
    public:
        using ptr = std::shared_ptr<LogEvent>;
        /**
         * @brief 创建日志事件对象智能指针
         *
         * @param logger        日志器对象
         * @param file          源文件名
         * @param line          行号
         * @param elapse        程序启动开始到现在的毫秒数
         * @param thread_id     线程ID
         * @param fiber_id      协程ID
         * @param time          时间戳
         * @param level         日志级别，默认为DEBUG级别
         * @param thread_name   线程名
         */
        static ptr create(std::shared_ptr<Logger> logger, const char *file, int32_t line, uint32_t elapse, uint32_t thread_id,
                          uint32_t fiber_id, uint64_t time, LogLevel level, std::string thread_name)
        {
            return std::make_shared<LogEvent>(logger, file, line, elapse, thread_id, fiber_id, time, level, thread_name);
        }

    public:
        /**
         * @brief 构造函数，用于创建日志事件对象
         *
         * @param logger        日志器对象
         * @param file          源文件名
         * @param line          行号
         * @param elapse        程序启动开始到现在的毫秒数
         * @param thread_id     线程ID
         * @param fiber_id      协程ID
         * @param time          时间戳
         * @param level         日志级别，默认为DEBUG级别
         * @param thread_name   线程名
         */
        LogEvent(std::shared_ptr<Logger> logger, const char *file, int32_t line, uint32_t elapse, uint32_t thread_id,
                 uint32_t fiber_id, uint64_t time, LogLevel level, std::string thread_name)
            : m_logger(logger), m_file(file), m_line(line), m_elapse(elapse), m_threadId(thread_id),
              m_fiberId(fiber_id), m_time(time), m_level(level), m_threadName(thread_name) {}

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
         * @brief 获取程序从启动到当前时刻的时长，以毫秒为单位。
         *
         * @return uint32_t 时长，单位：毫秒。
         */
        uint32_t getElapse() const { return m_elapse; }
        /**
         * @brief 获取执行该日志事件的线程的ID。
         *
         * @return uint32_t 线程ID。
         */
        uint32_t getThreadId() const { return m_threadId; }
        /**
         * @brief 获取该日志事件所在的线程名。
         *
         * @return std::string 线程名。
         */
        const std::string &getThreadName() const { return m_threadName; }
        /**
         * @brief 获取执行该日志事件的协程的ID。
         *
         * @return uint32_t 协程ID。
         */
        uint32_t getFiberId() const { return m_fiberId; }
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
         * @brief 获取日志事件的内容。
         *
         * @return std::string 日志内容。
         */
        std::string getContent() const { return m_ss.str(); }
        /**
         * @brief 获取用于向日志事件内容中追加文本的内容流。
         *
         * @return std::stringstream& 内容流的引用。
         */
        std::stringstream &getSS() { return m_ss; }
        /**
         * @brief 获取与该日志事件关联的日志器。
         *
         * @return std::shared_ptr<Logger> 日志器的智能指针。
         */
        std::shared_ptr<Logger> getLogger() const { return m_logger; }

    private:
        const char *m_file = nullptr;     // 文件名
        LogLevel m_level;                 // 级别
        uint32_t m_line = 0;              // 行号
        uint32_t m_elapse = 0;            // 程序启动开始到现在的毫秒数
        uint32_t m_threadId = 0;          // 线程id
        uint32_t m_fiberId = 0;           // 协程id
        uint64_t m_time;                  // 时间戳
        std::stringstream m_ss;           // 内容流
        std::shared_ptr<Logger> m_logger; // 日志器
        std::string m_threadName;         // 线程名
    };

    /**
     * @brief 日志事件包装器
     */
    class LogEventWrap
    {
    public:
        LogEventWrap(LogEvent::ptr e)
            : m_event(e) {}
        /**
         * @brief 析构时输出日志
         */
        ~LogEventWrap();
        /**
         * @brief 获得日志事件
         */
        LogEvent::ptr getEvent() const { return m_event; }
        /**
         * @brief 获得日志内容流，以便左移操作补充内容
         */
        std::stringstream &getSS() { return m_event->getSS(); }

    private:
        LogEvent::ptr m_event; // 事件
    };

    /**
     * @brief 日志格式器
     */
    class LogFormatter
    {
    public:
        using ptr = std::shared_ptr<LogFormatter>;
        static ptr create(const std::string &pattern)
        {
            return std::make_shared<LogFormatter>(pattern);
        }

    public:
        LogFormatter(const std::string &pattern);

        /**
         * @brief 构造字符流
         */
        std::string format(LogEvent::ptr event);
        /**
         * @brief 获得格式
         */
        const std::string &getPattern() const { return m_pattern; }
        /**
         * @brief 判断格式是否异常
         * @return true：异常 false: 正常
         */
        bool isError() { return m_error; }

        /**
         * @brief 格式器初始化
         */
        void init();

    public:
        /**
         * @brief 格式体虚父类
         */
        class FormatItem
        {
        public:
            virtual ~FormatItem() {}

            /**
             * @brief 构造字符流
             */
            virtual void format(std::ostream &os, LogEvent::ptr event) = 0;
        };

    private:
        std::string m_pattern;                            // 格式
        std::vector<std::shared_ptr<FormatItem>> m_items; // 格式体容器
        bool m_error = false;                             // 异常标志符
    };

    /**
     * @brief 日志输出地
     */
    class LogAppender
    {
        friend class Logger;

    public:
        using ptr = std::shared_ptr<LogAppender>;
        using MutexType = Spinlock;

    public:
        virtual ~LogAppender(){};

        /**
         * @brief 输出日志，必须重构
         */
        virtual void log(LogLevel Level, LogEvent::ptr event) = 0;
        /**
         * @brief 输出为Yaml字符串格式，必须重构
         */
        virtual std::string toYamlString() = 0;

        /**
         * @brief 设置格式器
         */
        void setFormatter(const std::string &pattern);
        void setFormatter(LogFormatter::ptr formatter);
        /**
         * @brief 获得格式器
         */
        const LogFormatter::ptr &getFormatter();

        /**
         * @brief 设置输出地级别
         */
        void setLevel(LogLevel level) { m_level = level; }
        /**
         * @brief 获得输出地级别
         */
        LogLevel getLevel() const { return m_level; }

    protected:
        LogLevel m_level;              // 级别
        LogFormatter::ptr m_formatter; // 格式器
        bool m_custom_pattern = false; // 自定义格式
        MutexType m_mutex;
    };

    /**
     * @brief 日志器
     */
    class Logger : public std::enable_shared_from_this<Logger>
    {
    public:
        using ptr = std::shared_ptr<Logger>;
        static ptr create()
        {
            return std::make_shared<Logger>();
        }
        static ptr create(const std::string &name)
        {
            return std::make_shared<Logger>(name);
        }
        static ptr create(const std::string &name, LogLevel level, const std::string &pattern)
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
        void log(LogLevel level, const LogEvent::ptr &event);

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
        void setFormatter(const std::string &pattern);

        /**
         * @brief 获取日志的输出格式
         */
        const std::string &getFormatter();
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
        std::string m_name = "root";                 // 日志名称
        LogLevel m_level = LogLevel::DEBUG;          // 日志级别
        std::list<LogAppender::ptr> m_appenders;     // Appender集合
        std::string m_pattern = LIM_DEFAULT_PATTERN; // 日志格式
        MutexType m_mutex;
    };
    /**
     * @brief 输出到控制台Appender
     */
    class StdoutLogAppender : public LogAppender
    {
    public:
        using ptr = std::shared_ptr<StdoutLogAppender>;
        static ptr create()
        {
            return std::make_shared<StdoutLogAppender>();
        }

    public:
        /**
         * @brief 输出日志到控制台中
         */
        void log(LogLevel Level, LogEvent::ptr event) override;

        /**
         * @brief 打印成Yaml格式字符串
         */
        std::string toYamlString();
    };
    /**
     * @brief 输出到文件的Appender
     */
    class FileLogAppender : public LogAppender
    {
    public:
        using ptr = std::shared_ptr<FileLogAppender>;
        static ptr create(const std::string &filename)
        {
            return std::make_shared<FileLogAppender>(filename);
        }

    public:
        FileLogAppender(const std::string &filename);

        /**
         * @brief 输出日志到文件中
         */
        void log(LogLevel Level, LogEvent::ptr event) override;

        /**
         * @brief 重新打开文件，打开成功返回true
         */
        bool reopen();
        /**
         * @brief 检测文件是否存在
         */
        bool fileExist();

        /**
         * @brief 打印成Yaml格式字符串
         */
        std::string toYamlString();

    private:
        std::string m_filename;     // 文件名
        std::ofstream m_filestream; // 文件流
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

#endif
